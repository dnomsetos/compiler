#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>

#include <utility/type_tuple.hpp>

struct Dummy {
  friend bool operator==(const Dummy&, const Dummy&) = default;
};

using calc_result_t =
    std::variant<std::int64_t, double, bool, std::string, Dummy>;

inline const std::pair<std::string, calc_result_t> default_value_table[] = {
    {"int", 0},
    {"float", 0.0},
    {"bool", false},
    {"str", ""},
};

namespace tkn {

#define GENERATE_EMPTY_TOKEN(name)                                             \
  struct name {                                                                \
    friend bool operator==(const name& left, const name& right) = default;     \
  };

#define GENERATE_BINARY_OPERATION(name, op)                                    \
  struct name {                                                                \
    friend bool operator==(const name& left, const name& right) = default;     \
    inline static std::function<calc_result_t(const calc_result_t&,            \
                                              const calc_result_t&)>           \
        binary_operation = [](auto&& l, auto&& r) -> calc_result_t {           \
      return std::visit(                                                       \
          [](auto&& l, auto&& r) -> calc_result_t {                            \
            if constexpr (requires { l op r; }) {                              \
              return l op r;                                                   \
            } else {                                                           \
              throw std::runtime_error("invalid arguments");                   \
            }                                                                  \
          },                                                                   \
          l, r);                                                               \
    };                                                                         \
  };                                                                           \
  inline std::ostream& operator<<(std::ostream& os, const name&) {             \
    os << #op;                                                                 \
    return os;                                                                 \
  }

#define GENERATE_UNIVERSAL_OPERATION(name, op)                                 \
  struct name {                                                                \
    friend bool operator==(const name& left, const name& right) = default;     \
    inline static std::function<calc_result_t(const calc_result_t&,            \
                                              const calc_result_t&)>           \
        binary_operation = [](const calc_result_t& l,                          \
                              const calc_result_t& r) -> calc_result_t {       \
      return std::visit(                                                       \
          [](auto&& l, auto&& r) -> calc_result_t {                            \
            if constexpr (requires { l op r; }) {                              \
              return l op r;                                                   \
            } else {                                                           \
              throw std::runtime_error("invalid arguments");                   \
            }                                                                  \
          },                                                                   \
          l, r);                                                               \
    };                                                                         \
    inline static std::function<calc_result_t(const calc_result_t&)>           \
        unary_operation = [](const calc_result_t& value) -> calc_result_t {    \
      return std::visit(                                                       \
          [](auto&& value) -> calc_result_t {                                  \
            if constexpr (requires { op value; }) {                            \
              return op value;                                                 \
            } else {                                                           \
              throw std::runtime_error("invalid arguments");                   \
            }                                                                  \
          },                                                                   \
          value);                                                              \
    };                                                                         \
  };                                                                           \
  inline std::ostream& operator<<(std::ostream& os, const name&) {             \
    os << #op;                                                                 \
    return os;                                                                 \
  }

#define GENERATE_UNARY_OPERATION(name, op)                                     \
  struct name {                                                                \
    friend bool operator==(const name& left, const name& right) = default;     \
    inline static std::function<calc_result_t(const calc_result_t&)>           \
        unary_operation = [](const calc_result_t& value) -> calc_result_t {    \
      return std::visit(                                                       \
          [](auto&& value) -> calc_result_t {                                  \
            if constexpr (requires { op value; }) {                            \
              return op value;                                                 \
            } else {                                                           \
              throw std::runtime_error("invalid arguments");                   \
            }                                                                  \
          },                                                                   \
          value);                                                              \
    };                                                                         \
  };

GENERATE_UNIVERSAL_OPERATION(Plus, +)
GENERATE_UNIVERSAL_OPERATION(Minus, -)
GENERATE_BINARY_OPERATION(Multiply, *)
GENERATE_BINARY_OPERATION(Divide, /)
GENERATE_BINARY_OPERATION(Mod, %)
GENERATE_BINARY_OPERATION(Equal, ==)
GENERATE_BINARY_OPERATION(NotEqual, !=)
GENERATE_BINARY_OPERATION(Less, <)
GENERATE_BINARY_OPERATION(Greater, >)
GENERATE_BINARY_OPERATION(LessEqual, <=)
GENERATE_BINARY_OPERATION(GreaterEqual, >=)
GENERATE_BINARY_OPERATION(Assignment, =)
GENERATE_BINARY_OPERATION(And, &)
GENERATE_BINARY_OPERATION(Or, |)
GENERATE_BINARY_OPERATION(Xor, ^)
GENERATE_UNARY_OPERATION(Not, !)
GENERATE_EMPTY_TOKEN(Fn)
GENERATE_EMPTY_TOKEN(Var)
GENERATE_EMPTY_TOKEN(Arrow)
GENERATE_EMPTY_TOKEN(Semicolon)
GENERATE_EMPTY_TOKEN(Colon)
GENERATE_EMPTY_TOKEN(If)
GENERATE_EMPTY_TOKEN(Else)
GENERATE_EMPTY_TOKEN(True)
GENERATE_EMPTY_TOKEN(False)
GENERATE_EMPTY_TOKEN(LeftBrace)
GENERATE_EMPTY_TOKEN(RightBrace)
GENERATE_EMPTY_TOKEN(LeftParent)
GENERATE_EMPTY_TOKEN(RightParent)
GENERATE_EMPTY_TOKEN(Comma)
GENERATE_EMPTY_TOKEN(EOFToken)

struct Identifier {
public:
  std::string name;

  friend bool operator==(const Identifier& left,
                         const Identifier& right) = default;
};

struct IntLiteral {
public:
  std::uint64_t value;

  friend bool operator==(const IntLiteral& left,
                         const IntLiteral& right) = default;
};

struct FloatLiteral {
public:
  double value;

  friend bool operator==(const FloatLiteral& left,
                         const FloatLiteral& right) = default;
};

struct StringLiteral {
  std::string value;

  friend bool operator==(const StringLiteral& left,
                         const StringLiteral& right) = default;
};

struct BoolLiteral {
  bool value;

  friend bool operator==(const BoolLiteral& left,
                         const BoolLiteral& right) = default;
};

using LiteralTuple =
    TypeTuple<IntLiteral, FloatLiteral, StringLiteral, BoolLiteral>;

using UnaryOperatorTuple = TypeTuple<Plus, Minus, Not>;

using LogicalOperatorTuple = TypeTuple<And, Or, Xor, Not>;

using HighPriorityArithmeticOperatorTuple = TypeTuple<Multiply, Divide, Mod>;

using LowPriorityArithmeticOperatorTuple = TypeTuple<Plus, Minus>;

using ComparisonOperatorTuple =
    TypeTuple<Less, Greater, LessEqual, GreaterEqual>;

using EqualityOperatorTuple = TypeTuple<Equal, NotEqual>;

using HelperTuple = TypeTuple<Arrow, Semicolon, Colon, Comma, LeftBrace,
                              RightBrace, LeftParent, RightParent, EOFToken>;

using KeywordsTuple = TypeTuple<Fn, Var, If, Else, True, False>;

using TokenTuple =
    Concat<LiteralTuple, LogicalOperatorTuple,
           HighPriorityArithmeticOperatorTuple,
           LowPriorityArithmeticOperatorTuple, EqualityOperatorTuple,
           ComparisonOperatorTuple, HelperTuple, KeywordsTuple,
           TypeTuple<Identifier, Assignment>>::type;

struct Point {
  std::size_t line;
  std::size_t offset;

  friend bool operator==(const Point& left, const Point& right) = default;
};

struct Position {
  Point start;
  std::size_t size;

  Position(std::size_t line, std::size_t offset, std::size_t size)
      : start{.line = line, .offset = offset}, size(size) {}

  Position(Point start, Point end)
      : start{start}, size(end.offset - start.offset) {}

  friend bool operator==(const Position& left, const Position& right) = default;
};

template <typename T>
concept Token = IsInTypeTuple<T, TokenTuple>::value;

using token_variant_t = type_tuple_to_variant_t<TokenTuple>;

struct TokenInfo {
  token_variant_t token_variant;
  Position position;

  friend bool operator==(const TokenInfo& left,
                         const TokenInfo& right) = default;
};

} // namespace tkn
