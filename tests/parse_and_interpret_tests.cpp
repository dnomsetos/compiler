#include <fstream>

#include "test_utilities.hpp"
#include <gtest/gtest.h>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <semantic_analysis/semantic_visitor.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>
#include <testing_utilities/print_visitor.hpp>
#include <testing_utilities/type_checker.hpp>

#define DIR_NAMES_LIST                                                         \
  "simple", "different_types", "if_expression", "if_elif_statement",           \
      "big_expression", "simple_cast", "simple_block_expression",              \
      "big_block_expression", "loop_expression", "loop_labels",                \
      "function_in_function", "mini_program", "strange_situation",             \
      "declarations_without_definitions"

class ParseTests : public testing::TestWithParam<std::string> {};

class InterpretTests : public testing::TestWithParam<std::string> {};

TEST_P(ParseTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  std::stringstream buffer;

  PrintVisitor visitor(buffer);
  visitor(*ast.value().first);

  std::ifstream result(TEST_DATA_DIR + dir_name + ast_filename,
                       std::ios::binary);
  ASSERT_TRUE(result.is_open());

  std::stringstream result_buffer;
  result_buffer << result.rdbuf();

  ASSERT_EQ(result_buffer.str(), buffer.str());
}

INSTANTIATE_TEST_SUITE_P(, ParseTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator<ParseTests>);
