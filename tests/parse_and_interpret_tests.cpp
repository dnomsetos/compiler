#include <fstream>
#include <ranges>

#include <gtest/gtest.h>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <semantic_analysis/semantic_visitor.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>
#include <testing_utilities/interpreter_visitor.hpp>
#include <testing_utilities/print_visitor.hpp>

#define DIR_NAMES_LIST                                                         \
  "simple", "different_types", "if_expression", "if_elif_statement",           \
      "big_expression", "simple_cast", "simple_block_expression",              \
      "big_block_expression", "loop_expression", "loop_labels",                \
      "function_in_function", "mini_program"

const std::string code_filename = "/code.txt";
const std::string ast_filename = "/ast.txt";
const std::string interpret_result_filename = "/interpret_result.txt";

class ParseTests : public testing::TestWithParam<std::string> {};

class InterpretTests : public testing::TestWithParam<std::string> {};

auto prepare_test_data(const std::string& dir_name)
    -> std::optional<ParseResult<ast::Program>> {
  std::ifstream code(PARSE_TEST_DATA_DIR + dir_name + code_filename);
  if (!code.is_open()) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << code.rdbuf();

  auto tokens = tokenize(buffer.str());
  return parse_program(tokens.begin(), tokens.end());
}

TEST_P(ParseTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  std::stringstream buffer;

  PrintVisitor visitor(buffer);
  visitor(*ast.value().first);

  std::ifstream result(
      PARSE_TEST_DATA_DIR + dir_name + ast_filename, std::ios::binary);
  ASSERT_TRUE(result.is_open());

  std::stringstream result_buffer;
  result_buffer << result.rdbuf();

  ASSERT_EQ(result_buffer.str(), buffer.str());
}

TEST_P(InterpretTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  testing::internal::CaptureStdout();

  TypeStore type_store;
  GlobalSymbolTable symbol_table{type_store};
  SemanticVisitor visitor(type_store, symbol_table);
  ASSERT_NO_THROW(visitor.visit(*ast.value().first));

  InterpreterVisitor interpreter(type_store);
  ASSERT_NO_THROW(interpreter(*ast.value().first, "main"));

  std::string output = testing::internal::GetCapturedStdout();

  std::ifstream result(PARSE_TEST_DATA_DIR + dir_name +
                       interpret_result_filename);
  ASSERT_TRUE(result.is_open());

  std::stringstream buffer;
  buffer << result.rdbuf();

  ASSERT_EQ(output, buffer.str());
}

auto test_name_generator(
    const testing::TestParamInfo<ParseTests::ParamType>& info) -> std::string {
  auto view = info.param | std::views::split('_');
  std::string result;
  for (auto&& part : view) {
    std::string str(part.begin(), part.end());
    if (!str.empty()) {
      str[0] = std::toupper(static_cast<unsigned char>(str[0]));
      result += str;
    }
  }
  return result;
}

INSTANTIATE_TEST_SUITE_P(, ParseTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator);

INSTANTIATE_TEST_SUITE_P(, InterpretTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator);
