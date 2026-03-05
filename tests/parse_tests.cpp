#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>

#include <gtest/gtest.h>

TEST(ParserTest, Test1) {
  auto tokens = tokenize("var x : int;"
                         ""
                         "fn main() {"
                         "    x = 1 + 2 * 3;"
                         "    x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test2) {
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
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test3) {
  auto tokens = tokenize("var p : int;"
                         "var q : int;"
                         ""
                         "fn bit_ops() {"
                         "    p = (1 & 3) | (4 ^ 2);"
                         "    q = p & (p | 7) ^ 5;"
                         "    q"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test4) {
  auto tokens = tokenize("var a : int;"
                         "var b : int;"
                         ""
                         "fn nested_assign() {"
                         "    b = 2;"
                         "    a = (b = 5) + 3;"
                         "    a"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test5) {
  auto tokens = tokenize("var s : string;"
                         ""
                         "fn greet() {"
                         "    s = \"Hello, \" + \" world\";"
                         "    s"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test6) {
  auto tokens = tokenize("var f : float;"
                         "var n : int;"
                         ""
                         "fn numeric_mix() {"
                         "    f = 3.14 + (2.0 * 5.0);"
                         "    n = 10 % 3;"
                         "    f"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test7) {
  auto tokens = tokenize("var x : int;"
                         ""
                         "fn compute() {"
                         "    if (x > 0) {"
                         "        if (x > 10) {"
                         "            x = x - 10;"
                         "        } else {"
                         "            x = x - 1;"
                         "        }"
                         "    } else {"
                         "        x = x + 1;"
                         "    }"
                         "    x * x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test8) {
  auto tokens = tokenize("var ok : int;"
                         "var a : int;"
                         ""
                         "fn logic_unary() {"
                         "    ok = ! (a == 0);"
                         "    ok"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test9) {
  auto tokens = tokenize("fn locals() {"
                         "    var tmp : int;"
                         "    tmp = 1 + 2;"
                         "    tmp = tmp * 5;"
                         "    tmp"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test10) {
  auto tokens = tokenize("var x : int;"
                         "var y : int;"
                         ""
                         "fn inc() {"
                         "    x = x + 1;"
                         "    x"
                         "}"
                         ""
                         "fn sum_squares() {"
                         "    y = inc() * inc();"
                         "    y"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();
  auto result = parse_program(tokens.begin(), tokens.end(), mr);
  ASSERT_TRUE(result.has_value());
}
