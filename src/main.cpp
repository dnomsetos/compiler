#include <iostream>

#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <utility/executor.hpp>

int main() {
  auto tokens = tokenize("fn main() {"
                         "    var tmp : int = 42;"
                         "    println(tmp - 35);"
                         "    42 - tmp"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  if (!result.has_value()) {
    std::cout << "error" << std::endl;
  } else {
    std::unordered_map<std::string, calc_result> variables;
    auto execute_result = execute_program(*result.value().first, variables);
    if (std::holds_alternative<std::int64_t>(execute_result)) {
      std::cout << "returned from main: "
                << std::get<std::int64_t>(execute_result) << std::endl;
    }
  }
  return 0;
}
