#include "test_utilities.hpp"
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <ir_builder/builder.hpp>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <semantic_analysis/semantic_visitor.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>

#define DIR_NAMES_LIST                                                         \
  "simple", "different_types", "if_expression", "if_elif_statement",           \
      "big_expression", "simple_cast", "simple_block_expression",              \
      "big_block_expression", "loop_expression", "loop_labels",                \
      "mini_program", "strange_situation", "declarations_without_definitions"

class IrTests : public testing::TestWithParam<std::string> {};
class ObjectiveTests : public testing::TestWithParam<std::string> {};

TEST_P(IrTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  std::stringstream buffer;

  TypeStore type_store;
  GlobalSymbolTable symbol_table(type_store);
  SemanticVisitor visitor(type_store, symbol_table);
  ASSERT_NO_THROW(visitor.visit(*ast.value().first));

  BuildVisitor visitor2(code_filename, type_store);
  ASSERT_NO_THROW(visitor2.visit(*ast.value().first));

  visitor2.print_module(buffer);

  std::ifstream result(PARSE_TEST_DATA_DIR + dir_name + ir_filename,
                       std::ios::binary);
  ASSERT_TRUE(result.is_open());

  std::stringstream result_buffer;
  result_buffer << result.rdbuf();

  ASSERT_EQ(result_buffer.str(), buffer.str());
}

TEST_P(ObjectiveTests, ) {
  std::string dir_name = GetParam();

  auto data = prepare_test_data(dir_name);
  ASSERT_TRUE(data.has_value()) << "error opening file code.txt";

  auto& ast = data.value();
  ASSERT_TRUE(ast.has_value());

  TypeStore type_store;
  GlobalSymbolTable symbol_table{type_store};
  SemanticVisitor visitor(type_store, symbol_table);
  ASSERT_NO_THROW(visitor.visit(*ast.value().first));

  BuildVisitor visitor2(code_filename, type_store);
  ASSERT_NO_THROW(visitor2.visit(*ast.value().first));

  visitor2.emit_object("out.o");

  std::system("gcc out.o " TEST_HELPERS_OBJ " -o out");
  std::system("./out > out_result.txt 2>&1");

  std::string output;

  std::ifstream out_file("out_result.txt");
  ASSERT_TRUE(out_file.is_open()) << "failed to run compiled binary";
  std::stringstream ss;
  ss << out_file.rdbuf();
  output = ss.str();

  std::ifstream result(PARSE_TEST_DATA_DIR + dir_name + result_filename);
  EXPECT_TRUE(result.is_open());
  std::stringstream buffer;
  buffer << result.rdbuf();

  EXPECT_EQ(output, buffer.str());

  std::remove("out.o");
  std::remove("out");
  std::remove("out_result.txt");
}

INSTANTIATE_TEST_SUITE_P(, IrTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator<IrTests>);
INSTANTIATE_TEST_SUITE_P(, ObjectiveTests, testing::Values(DIR_NAMES_LIST),
                         test_name_generator<ObjectiveTests>);
