#include <gtest/gtest.h>
#include <parser/parse.hpp>
#include <scanner/tokenize.hpp>
#include <utility/executor.hpp>
#include <visitors/interpreter_visitor.hpp>

TEST(ParserTest, Test1) {
  auto tokens = tokenize("var x : int;"
                         ""
                         "fn main() {"
                         "    x = 1 + 2 * 3;"
                         "    x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(execute_result), 7);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 7);
}

TEST(ParserTest, Test2) {
  auto tokens = tokenize("var a : int;"
                         "var x : int;"
                         ""
                         "fn main() {"
                         "    if (a < 0) {"
                         "        x = -1;"
                         "    } else if (a == 0) {"
                         "        x = 0;"
                         "    } else {"
                         "        x = 1;"
                         "    };"
                         "    x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  // std::unordered_map<std::string, calc_result_t> variables;
  // auto execute_result = execute_program(*result.value().first, variables);

  ASSERT_TRUE(result.has_value());

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  // ASSERT_EQ(std::get<std::int64_t>(execute_result), 0);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 0);
}

TEST(ParserTest, Test3) {
  auto tokens = tokenize("var p : int;"
                         "var q : int;"
                         ""
                         "fn main() {"
                         "    p = (1 & 3) | (4 ^ 2);"
                         "    q = p & (p | 7) ^ 5;"
                         "    q"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(execute_result), 2);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 2);
}

TEST(ParserTest, Test4) {
  auto tokens = tokenize("var a : int;"
                         "var b : int;"
                         ""
                         "fn main() {"
                         "    b = 2;"
                         "    a = (b = 5) + 3;"
                         "    a"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(execute_result), 8);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 8);
}

TEST(ParserTest, Test5) {
  auto tokens = tokenize("var s : str;"
                         ""
                         "fn main() {"
                         "    s = \"Hello,\" + \" world\";"
                         "    s"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::string>(execute_result), "Hello, world");
  ASSERT_EQ(std::get<std::string>(interpreter_result), "Hello, world");
}

TEST(ParserTest, Test6) {
  auto tokens = tokenize("var f : float;"
                         "var n : int;"
                         ""
                         "fn main() {"
                         "    f = 3.14 + (2.0 * 5.0);"
                         "    n = 10 % 3;"
                         "    f"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_DOUBLE_EQ(std::get<double>(execute_result), 13.14);
  ASSERT_DOUBLE_EQ(std::get<double>(interpreter_result), 13.14);
}

TEST(ParserTest, Test7) {
  auto tokens = tokenize("var x : int = 4;"
                         ""
                         "fn main() {"
                         "    if x > 0 {"
                         "        if x > 10 {"
                         "            x = x - 10;"
                         "        } else {"
                         "            x = x - 1;"
                         "        };"
                         "    } else {"
                         "        x = x + 1;"
                         "    };"
                         "    x * x"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 9);
}

TEST(ParserTest, Test8) {
  auto tokens = tokenize("var ok : bool;"
                         "var a : int;"
                         ""
                         "fn main() {"
                         "    ok = ! (a == 0);"
                         "    ok"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<bool>(execute_result), false);
  ASSERT_EQ(std::get<bool>(interpreter_result), false);
}

TEST(ParserTest, Test9) {
  auto tokens = tokenize("fn main() {"
                         "    var tmp : int;"
                         "    tmp = 1 + 2;"
                         "    tmp = tmp * 5;"
                         "    tmp"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(execute_result), 15);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 15);
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

  auto result = parse_program(tokens.begin(), tokens.end());

  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test11) {
  auto tokens =
      tokenize("var x : int;"
               "var y : int;"
               ""
               "fn inc() {"
               "    x = x + 1;"
               "    x"
               "}"
               ""
               "fn sum_squares() {"
               "    y = inc(42 * 54) * inc(aboba(-bebra), 5 & (3 -43));"
               "    y"
               "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test12) {
  auto tokens =
      tokenize("var x : int;"
               "var y : int;"
               ""
               "fn inc(a : int, b : char, c : float) {"
               "    x = x + 1;"
               "    x"
               "}"
               ""
               "fn sum_squares() {"
               "    y = inc(42 * 54) * inc(aboba(-bebra), 5 & (3 -43));"
               "    y"
               "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  ASSERT_TRUE(result.has_value());
}

TEST(ParserTest, Test13) {
  auto tokens = tokenize("var x : int = 4;"
                         ""
                         "fn main() {"
                         "    x = {"
                         "        var y : int = 42;"
                         "        y - x"
                         "    };"
                         "    x + 5"
                         "}");
  auto mr = std::pmr::monotonic_buffer_resource();

  auto result = parse_program(tokens.begin(), tokens.end());

  std::unordered_map<std::string, calc_result_t> variables;
  auto execute_result = execute_program(*result.value().first, variables);

  InterpreterVisitor visitor;
  auto interpreter_result = visitor(*result.value().first);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(std::get<std::int64_t>(execute_result), 43);
  ASSERT_EQ(std::get<std::int64_t>(interpreter_result), 43);
}
