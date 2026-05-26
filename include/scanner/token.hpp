#pragma once

#include <cstdint>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <stdfloat>
#include <string>

#include <utility/type_tuple.hpp>

#if __STDCPP_FLOAT32_T__ != 1
namespace std {
using float32_t = _Float32;
}
#endif

#if __STDCPP_FLOAT64_T__ != 1
namespace std {
using float64_t = _Float64;
}
#endif

struct Dummy {
  friend bool operator==(const Dummy&, const Dummy&) = default;
};

using calc_result_t =
    std::variant<std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                 std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                 std::float32_t, std::float64_t, bool, char, Dummy>;

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
            if constexpr (std::is_same_v<std::decay_t<decltype(l)>,            \
                                         std::decay_t<decltype(r)>> &&         \
                          requires { l op r; }) {                              \
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
            if constexpr (std::is_same_v<std::decay_t<decltype(l)>,            \
                                         std::decay_t<decltype(r)>> &&         \
                          requires { l op r; }) {                              \
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
  };                                                                           \
  inline std::ostream& operator<<(std::ostream& os, const name&) {             \
    os << #op;                                                                 \
    return os;                                                                 \
  }

GENERATE_UNIVERSAL_OPERATION(Minus, -)
GENERATE_BINARY_OPERATION(LogicalAnd, &&)
GENERATE_BINARY_OPERATION(Plus, +)
GENERATE_BINARY_OPERATION(LogicalOr, ||)
GENERATE_BINARY_OPERATION(Asterisk, *)
GENERATE_BINARY_OPERATION(Divide, /)
GENERATE_BINARY_OPERATION(Mod, %)
GENERATE_BINARY_OPERATION(Equal, ==)
GENERATE_BINARY_OPERATION(NotEqual, !=)
GENERATE_BINARY_OPERATION(Less, <)
GENERATE_BINARY_OPERATION(Greater, >)
GENERATE_BINARY_OPERATION(LessEqual, <=)
GENERATE_BINARY_OPERATION(GreaterEqual, >=)
GENERATE_BINARY_OPERATION(Assignment, =)
GENERATE_BINARY_OPERATION(Ampersand, &)
GENERATE_BINARY_OPERATION(BitwiseOr, |)
GENERATE_BINARY_OPERATION(BitwiseXor, ^)
GENERATE_BINARY_OPERATION(LeftShift, <<)
GENERATE_BINARY_OPERATION(RightShift, >>)
GENERATE_UNARY_OPERATION(Not, !)
GENERATE_EMPTY_TOKEN(Fn)
GENERATE_EMPTY_TOKEN(Let)
GENERATE_EMPTY_TOKEN(Static)
GENERATE_EMPTY_TOKEN(Const)
GENERATE_EMPTY_TOKEN(Break)
GENERATE_EMPTY_TOKEN(Continue)
GENERATE_EMPTY_TOKEN(Return)
GENERATE_EMPTY_TOKEN(Arrow)
GENERATE_EMPTY_TOKEN(Semicolon)
GENERATE_EMPTY_TOKEN(Colon)
GENERATE_EMPTY_TOKEN(If)
GENERATE_EMPTY_TOKEN(Else)
GENERATE_EMPTY_TOKEN(Loop)
GENERATE_EMPTY_TOKEN(True)
GENERATE_EMPTY_TOKEN(False)
GENERATE_EMPTY_TOKEN(As)
GENERATE_EMPTY_TOKEN(LeftBrace)
GENERATE_EMPTY_TOKEN(RightBrace)
GENERATE_EMPTY_TOKEN(LeftParent)
GENERATE_EMPTY_TOKEN(RightParent)
GENERATE_EMPTY_TOKEN(Comma)
GENERATE_EMPTY_TOKEN(Mut)
GENERATE_EMPTY_TOKEN(EOFToken)

struct Label {
public:
  std::string name;

  friend bool operator==(const Label& left, const Label& right) = default;
};

struct Identifier {
public:
  std::string name;

  friend bool operator==(const Identifier& left,
                         const Identifier& right) = default;
};

struct CharLiteral {
public:
  char value;

  friend bool operator==(const CharLiteral& left,
                         const CharLiteral& right) = default;
};

struct IntLiteral {
public:
  std::int64_t value;

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

using LiteralTuple = TypeTuple<CharLiteral, IntLiteral, FloatLiteral,
                               BoolLiteral, StringLiteral>;

using UnaryOperatorTuple = TypeTuple<Ampersand, Asterisk, Minus, Not>;

using LogicalOperatorTuple = TypeTuple<LogicalAnd, LogicalOr, Not>;

using HighPriorityArithmeticOperatorTuple = TypeTuple<Asterisk, Divide, Mod>;

using LowPriorityArithmeticOperatorTuple = TypeTuple<Plus, Minus>;

using ComparisonOperatorTuple =
    TypeTuple<Less, Greater, LessEqual, GreaterEqual, Equal, NotEqual>;

using BitwiseOperatorTuple = TypeTuple<Ampersand, BitwiseOr, BitwiseXor>;

using ShiftOperatorTuple = TypeTuple<LeftShift, RightShift>;

using HelperTuple = TypeTuple<Arrow, Semicolon, Colon, Comma, LeftBrace,
                              RightBrace, LeftParent, RightParent, EOFToken>;

using VariableDefinitionTuple = TypeTuple<Let, Static, Const>;

using ControlFlowTuple = TypeTuple<If, Else, Loop>;

using KeywordsTuple =
    type_tuple_concat_t<VariableDefinitionTuple, ControlFlowTuple,
                        TypeTuple<Mut, Fn, True, False>>;

using InterruptTuple = TypeTuple<Break, Continue, Return>;

using TokenTuple = Concat<
    LiteralTuple, LogicalOperatorTuple, HighPriorityArithmeticOperatorTuple,
    LowPriorityArithmeticOperatorTuple, ComparisonOperatorTuple, HelperTuple,
    KeywordsTuple, InterruptTuple, BitwiseOperatorTuple, ShiftOperatorTuple,
    TypeTuple<Identifier, Assignment, Label, As>>::type;

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

  Position(Point start, std::size_t size) : start(start), size(size) {}

  friend bool operator==(const Position& left, const Position& right) = default;

  std::string to_string() const {
    return std::to_string(start.line) + ":" + std::to_string(start.offset);
  }
};

inline std::ostream& operator<<(std::ostream& os, const Position& position) {
  return os << position.start.line << ":" << position.start.offset << ":";
}

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
