#include <cstring>
#include <fstream>
#include <iostream>

#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <utility/executor.hpp>
#include <visitors/interpreter_visitor.hpp>
#include <visitors/print_visitor.hpp>

const int number_of_required_args = 3;

const std::string help_mode = "--help";
const std::string print_ast_mode = "--print_ast";
const std::string interpret_mode = "--interpret";
const std::string execute_mode = "--execute";

const char* skip_empty_arg = "--skip_empty";
const char* out_file_arg = "--file";

void print_ast(const std::string& code, std::ostream& out, bool skip_empty) {
  out << "Printing AST\n";

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  PrintVisitor visitor(out, skip_empty);
  visitor(*ast.value().first);
}

void interpret(const std::string& code) {
  std::cout << "Interpreting\n";

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  InterpreterVisitor visitor;
  auto result = visitor(*ast.value().first);

  std::cout << "Returned from main: ";
  std::visit([](auto&& value) { std::cout << value << '\n'; }, result);
}

void execute(const std::string& code) {
  std::cout << "Executing\n";

  auto tokens = tokenize(code);
  auto ast = parse_program(tokens.begin(), tokens.end());

  if (!ast.has_value()) {
    std::cerr << "Failed to parse\n";
    return;
  }

  std::unordered_map<std::string, calc_result_t> variables;
  auto result = execute_program(*ast.value().first, variables);

  std::cout << "Returned from main: ";
  std::visit([](auto&& value) { std::cout << value << '\n'; }, result);
}

void print_help() {
  std::cout << "Usage:\n";
  std::cout << "  compiler " << print_ast_mode << " <file> [" << out_file_arg
            << " <file>] [" << skip_empty_arg << " <true|false>]\n";
  std::cout << "  compiler " << interpret_mode << " <file>\n";
  std::cout << "  compiler " << execute_mode << " <file>\n";
}

int main(int argc, char** argv) {
  if (argc < number_of_required_args) {
    print_help();
    return 1;
  }

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
      out_file = argv[i + 1];
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
  } else if (mode == interpret_mode) {
    interpret(code);
  } else if (mode == execute_mode) {
    execute(code);
  } else {
    std::cerr << "Unknown mode\n";
    return 1;
  }

  return 0;
}
