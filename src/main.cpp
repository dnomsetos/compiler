#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <borrow_check/borrow_checker.hpp>
#include <borrow_check/control_flow_graph.hpp>
#include <ir_builder/builder.hpp>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <semantic_analysis/semantic_visitor.hpp>
#include <testing_utilities/print_visitor.hpp>
#include <testing_utilities/type_checker.hpp>
#include <utility/output.hpp>

const int number_of_required_args = 3;

const std::string help_mode = "--help";
const std::string print_ast_mode = "--print_ast";
const std::string semantic_mode = "--semantic";
const std::string ir_mode = "--build_ir";
const std::string custom_ir_mode = "--build_custom_ir";
const std::string borrow_checker = "--borrow_check";
const std::string emit_object_mode = "--emit_object";

const char* skip_empty_arg = "--skip_empty";
const char* out_file_arg = "--file";

void print_ast(const std::string& code, std::ostream& out, bool skip_empty) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Printing AST\n";
  }

  auto tokens = tokenize(code);

  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  PrintVisitor visitor(out, skip_empty);
  visitor(*ast.value().first);
}

void semantic(const std::string& code) {
  std::cout << "Semantic analysis\n";

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  TypeStore type_store;
  GlobalSymbolTable global_symbol_table{type_store};

  SemanticVisitor visitor(type_store, global_symbol_table);
  visitor.visit(*ast.value().first);

  TypeChecker type_checker(type_store);
  type_checker.visit(*ast.value().first);
}

void build_ir(const std::string& code, const std::string& module_name,
              std::ostream& out = std::cout) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Building IR\n";
  }

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  TypeStore type_store;
  GlobalSymbolTable global_symbol_table{type_store};

  SemanticVisitor semantic_visitor(type_store, global_symbol_table);
  semantic_visitor.visit(*ast.value().first);

  TypeChecker type_checker(type_store);
  type_checker.visit(*ast.value().first);

  BuildVisitor ir_visitor(module_name, type_store);
  ir_visitor.visit(*ast.value().first);

  ir_visitor.print_module(out);
}

void build_custom_ir(const std::string& code, const std::string& module_name,
                     std::ostream& out = std::cout) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Building custom IR\n";
  }

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  TypeStore type_store;
  GlobalSymbolTable global_symbol_table{type_store};

  SemanticVisitor semantic_visitor(type_store, global_symbol_table);
  semantic_visitor.visit(*ast.value().first);

  TypeChecker type_checker(type_store);
  type_checker.visit(*ast.value().first);

  ir::ControlFlowGraph ir_visitor(type_store);
  ir_visitor.visit(*ast.value().first);

  ir_visitor.print();
}

void run_borrow_checker(const std::string& code, const std::string& module_name,
                        std::ostream& out = std::cout) {
  if (out.rdbuf() == std::cout.rdbuf()) {
    out << "Building custom IR\n";
  }

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  TypeStore type_store;
  GlobalSymbolTable global_symbol_table{type_store};

  SemanticVisitor semantic_visitor(type_store, global_symbol_table);
  semantic_visitor.visit(*ast.value().first);

  TypeChecker type_checker(type_store);
  type_checker.visit(*ast.value().first);

  ir::ControlFlowGraph ir_visitor(type_store);
  ir_visitor.visit(*ast.value().first);

  ir_visitor.print();

  BorrowChecker borrow_checker_visitor;
  borrow_checker_visitor.check(ir_visitor.get_main_block(), {});
}

void emit_object(const std::string& code, const std::string& module_name,
                 const std::string& output_path) {
  std::cout << "Emitting object file\n";

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  TypeStore type_store;
  GlobalSymbolTable global_symbol_table{type_store};

  SemanticVisitor semantic_visitor(type_store, global_symbol_table);
  semantic_visitor.visit(*ast.value().first);

  TypeChecker type_checker(type_store);
  type_checker.visit(*ast.value().first);

  BuildVisitor ir_visitor(module_name, type_store);
  ir_visitor.visit(*ast.value().first);

  ir_visitor.emit_object(output_path);

  std::cout << "Object file written to: " << output_path << '\n';
}

std::string default_object_path(const std::string& input_file) {
  auto dot = input_file.rfind('.');
  if (dot != std::string::npos) {
    return input_file.substr(0, dot) + ".o";
  }
  return input_file + ".o";
}

void print_help() {
  std::cout << "Usage:\n";
  std::cout << "  compiler " << print_ast_mode << " <file> [" << out_file_arg
            << " <file>] [" << skip_empty_arg << " <true|false>]\n";
  std::cout << "  compiler " << semantic_mode << " <file>\n";
  std::cout << "  compiler " << ir_mode << " <file> [" << out_file_arg
            << " <outeput.ll>]\n";
  std::cout << "  compiler " << custom_ir_mode << std::endl;
  std::cout << "  compiler " << borrow_checker;
  std::cout << "  compiler " << emit_object_mode << " <file> [" << out_file_arg
            << " <output.o>]\n";
}

int main(int argc, char** argv) {
  if (argc < number_of_required_args) {
    print_help();
    return 1;
  }

  std::cout << std::fixed << std::setprecision(10);

  std::string mode = argv[1];
  std::string input_file = argv[2];
  std::string out_file;
  bool skip_empty = false;

  std::ifstream in(input_file);
  if (!in.is_open()) {
    std::cerr << "Cannot open input file\n";
    return 1;
  }

  std::string code((std::istreambuf_iterator<char>(in)),
                   (std::istreambuf_iterator<char>()));

  for (int i = number_of_required_args; i < argc; ++i) {
    if (std::strcmp(argv[i], out_file_arg) == 0 && i + 1 < argc) {
      out_file = argv[++i];
    } else if (std::strcmp(argv[i], skip_empty_arg) == 0) {
      skip_empty = true;
    }
  }

  if (mode == help_mode) {
    print_help();
  } else if (mode == print_ast_mode) {
    if (!out_file.empty()) {
      std::ofstream out(out_file);
      if (!out.is_open()) {
        std::cerr << "Cannot open output file\n";
        return 1;
      }
      print_ast(code, out, skip_empty);
    } else {
      print_ast(code, std::cout, skip_empty);
    }
  } else if (mode == semantic_mode) {
    semantic(code);
  } else if (mode == ir_mode) {
    if (!out_file.empty()) {
      std::ofstream out(out_file);

      if (!out.is_open()) {
        std::cerr << "Cannot open output file\n";
        return 1;
      }

      build_ir(code, input_file, out);
    } else {
      build_ir(code, input_file);
    }

  } else if (mode == emit_object_mode) {
    const std::string obj_path =
        out_file.empty() ? default_object_path(input_file) : out_file;
    emit_object(code, input_file, obj_path);
  } else if (mode == custom_ir_mode) {
    build_custom_ir(code, input_file);
  } else if (mode == borrow_checker) {
    run_borrow_checker(code, input_file);
  } else {
    std::cerr << "Unknown mode\n";
    return 1;
  }

  return 0;
}
