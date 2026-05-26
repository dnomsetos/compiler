#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

#include <borrow_check/borrow_checker.hpp>
#include <borrow_check/control_flow_graph.hpp>
#include <ir_builder/builder.hpp>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <semantic_analysis/semantic_visitor.hpp>
#include <testing_utilities/print_visitor.hpp>
#include <testing_utilities/type_checker.hpp>
#include <utility/output.hpp>

namespace flags {

constexpr std::string_view help = "--help";
constexpr std::string_view print_ast = "--print_ast";
constexpr std::string_view semantic = "--semantic";
constexpr std::string_view build_ir = "--build_ir";
constexpr std::string_view build_custom = "--build_custom_ir";
constexpr std::string_view borrow_check = "--borrow_check";
constexpr std::string_view emit_object = "--emit_object";

constexpr std::string_view out_file = "--file";
constexpr std::string_view skip_empty = "--skip_empty";

} // namespace flags

std::string read_file(const std::string& path) {
  std::ifstream in(path);
  if (!in.is_open()) {
    throw std::runtime_error("Cannot open input file: " + path);
  }
  return {std::istreambuf_iterator<char>(in), {}};
}

std::string default_object_path(const std::string& input_file) {
  auto dot = input_file.rfind('.');
  return (dot != std::string::npos ? input_file.substr(0, dot) : input_file) +
         ".o";
}

std::optional<std::ofstream> open_output(const std::string& path) {
  if (path.empty())
    return std::nullopt;
  std::ofstream out(path);
  if (!out.is_open()) {
    throw std::runtime_error("Cannot open output file: " + path);
  }
  return out;
}

struct Frontend {
  std::deque<tkn::TokenInfo> tokens;
  alloc::pmr_unique_ptr<ast::Program> program;
  TypeStore type_store;
  GlobalSymbolTable symbol_table{type_store};

  explicit Frontend(const std::string& code) : symbol_table(type_store) {
    tokens = tokenize(code);

    auto result = parse_program(tokens.begin(), tokens.end());
    if (!result.has_value()) {
      throw std::runtime_error("Parse error");
    }
    program = std::move(result->first);
  }

  void run_semantic() {
    SemanticVisitor semantic(type_store, symbol_table);
    semantic.visit(*program);

    TypeChecker checker(type_store);
    checker.visit(*program);
  }
};

void stage_print_ast(const std::string& code, std::ostream& out,
                     bool skip_empty_nodes) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Printing AST\n";
  }

  Frontend fe(code);
  PrintVisitor visitor(out, skip_empty_nodes);
  visitor(*fe.program);
}

void stage_semantic(const std::string& code) {
  std::cout << "Semantic analysis\n";
  Frontend fe(code);
  fe.run_semantic();
}

void stage_build_ir(const std::string& code, const std::string& module_name,
                    std::ostream& out) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Building IR\n";
  }

  Frontend fe(code);
  fe.run_semantic();

  BuildVisitor ir(module_name, fe.type_store);
  ir.visit(*fe.program);
  ir.print_module(out);
}

void stage_build_custom_ir(const std::string& code) {
  std::cout << "Building custom IR\n";

  Frontend fe(code);
  fe.run_semantic();

  bc_ir::ControlFlowGraph cfg(fe.type_store);
  cfg.visit(*fe.program);
  cfg.print();
}

void stage_borrow_check(const std::string& code) {
  std::cout << "Running borrow checker\n";

  Frontend fe(code);
  fe.run_semantic();

  bc_ir::ControlFlowGraph cfg(fe.type_store);
  cfg.visit(*fe.program);
  cfg.print();

  BorrowChecker checker;
  for (const auto& func : cfg.get_functions()) {
    checker.check(func);
  }
}

void stage_emit_object(const std::string& code, const std::string& module_name,
                       const std::string& output_path) {
  std::cout << "Emitting object file\n";

  Frontend fe(code);
  fe.run_semantic();

  BuildVisitor ir(module_name, fe.type_store);
  ir.visit(*fe.program);
  ir.emit_object(output_path);

  std::cout << "Object file written to: " << output_path << '\n';
}

void print_help() {
  std::cout << "Usage:\n"
            << "  compiler " << flags::print_ast << " <file> ["
            << flags::out_file << " <out.txt>] [" << flags::skip_empty << "]\n"
            << "  compiler " << flags::semantic << " <file>\n"
            << "  compiler " << flags::build_ir << " <file> ["
            << flags::out_file << " <out.ll>]\n"
            << "  compiler " << flags::build_custom << " <file>\n"
            << "  compiler " << flags::borrow_check << " <file>\n"
            << "  compiler " << flags::emit_object << " <file> ["
            << flags::out_file << " <out.o>]\n"
            << "  compiler " << flags::help << "\n";
}

int main(int argc, char** argv) {
  if (argc < 3) {
    print_help();
    return 1;
  }

  std::cout << std::fixed << std::setprecision(10);

  const std::string_view mode = argv[1];
  const std::string input_file = argv[2];
  std::string out_file;
  bool skip_empty_nodes = false;

  for (int i = 3; i < argc; ++i) {
    if (argv[i] == flags::out_file && i + 1 < argc) {
      out_file = argv[++i];
    } else if (argv[i] == flags::skip_empty) {
      skip_empty_nodes = true;
    }
  }

  try {
    if (mode == flags::help) {
      print_help();
      return 0;
    }

    const std::string code = read_file(input_file);

    if (mode == flags::print_ast) {
      auto file_out = open_output(out_file);
      stage_print_ast(
          code, file_out ? static_cast<std::ostream&>(*file_out) : std::cout,
          skip_empty_nodes);

    } else if (mode == flags::semantic) {
      stage_semantic(code);

    } else if (mode == flags::build_ir) {
      auto file_out = open_output(out_file);
      stage_build_ir(code, input_file,
                     file_out ? static_cast<std::ostream&>(*file_out)
                              : std::cout);

    } else if (mode == flags::build_custom) {
      stage_build_custom_ir(code);

    } else if (mode == flags::borrow_check) {
      stage_borrow_check(code);

    } else if (mode == flags::emit_object) {
      const std::string obj_path =
          out_file.empty() ? default_object_path(input_file) : out_file;
      stage_emit_object(code, input_file, obj_path);

    } else {
      std::cerr << "Unknown mode: " << mode << "\n";
      std::cerr << "Run with " << flags::help << " for usage.\n";
      return 1;
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
