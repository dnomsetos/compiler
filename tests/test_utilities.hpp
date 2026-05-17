#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>

#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

inline const std::string code_filename = "/code.txt";
inline const std::string ast_filename = "/ast.txt";
inline const std::string result_filename = "/result.txt";
inline const std::string ir_filename = "/ir.ll";

inline auto prepare_test_data(const std::string& dir_name)
    -> std::optional<ParseResult<ast::Program>> {
  std::ifstream code(TEST_DATA_DIR + dir_name + code_filename);
  if (!code.is_open()) {
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << code.rdbuf();

  auto tokens = tokenize(buffer.str());
  return parse_program(tokens.begin(), tokens.end());
}

template <typename T>
auto test_name_generator(
    const testing::TestParamInfo<typename T::ParamType>& info) -> std::string {
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

inline fs::path get_relative_path(const std::string& filename) {
  fs::path current_path = filename;
  return fs::relative(current_path, PROJECT_SOURCE_DIR);
}
