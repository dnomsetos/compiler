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
      "declarations_without_definitions", "simple_references",                 \
      "hard_references", "mutable_references"

class ParseTests : public testing::TestWithParam<std::string> {};

class SemanticTests : public testing::TestWithParam<std::string> {};

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

TEST_P(SemanticTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  std::stringstream buffer;

  TypeStore type_store;
  GlobalSymbolTable symbol_table{type_store};
  SemanticVisitor visitor(type_store, symbol_table);

  ASSERT_NO_THROW(visitor.visit(*ast.value().first));

  TypeChecker type_checker(type_store);
  ASSERT_NO_THROW(type_checker.visit(*ast.value().first));
}

INSTANTIATE_TEST_SUITE_P(, ParseTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator<ParseTests>);

INSTANTIATE_TEST_SUITE_P(, SemanticTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator<ParseTests>);
