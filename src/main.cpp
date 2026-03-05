#include <iostream>

#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>

#include <cassert>

int main() {
  auto tokens = tokenize("var a : int;"
                         "var x : int;"
                         ""
                         "fn branch_test() {"
                         "    if (a < 0) {"
                         "        x = -1;"
                         "    } else if (a == 0) {"
                         "        x = 0;"
                         "    } else {"
                         "        x = 1;"
                         "    }"
                         "    x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  if (!result.has_value()) {
    std::cout << "error" << std::endl;
  } else {
    std::cout << "good" << std::endl;
  }
  return 0;
}
