#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <utility/type_tuple.hpp>

namespace tkn {

#define GENERATE_EMPTY_TOKEN(name)                                             \
  struct name {                                                                \
    friend bool operator==(const name& left, const name& right) = default;     \
  };

GENERATE_EMPTY_TOKEN(Plus)
GENERATE_EMPTY_TOKEN(Minus)
GENERATE_EMPTY_TOKEN(Multiply)
GENERATE_EMPTY_TOKEN(Divide)
GENERATE_EMPTY_TOKEN(Mod)
GENERATE_EMPTY_TOKEN(Equal)
GENERATE_EMPTY_TOKEN(NotEqual)
GENERATE_EMPTY_TOKEN(Less)
GENERATE_EMPTY_TOKEN(Greater)
GENERATE_EMPTY_TOKEN(LessEqual)
GENERATE_EMPTY_TOKEN(GreaterEqual)
GENERATE_EMPTY_TOKEN(Assignment)
GENERATE_EMPTY_TOKEN(And)
GENERATE_EMPTY_TOKEN(Or)
GENERATE_EMPTY_TOKEN(Xor)
GENERATE_EMPTY_TOKEN(Not)
GENERATE_EMPTY_TOKEN(Fn)
GENERATE_EMPTY_TOKEN(Var)
GENERATE_EMPTY_TOKEN(Arrow)
GENERATE_EMPTY_TOKEN(Semicolon)
GENERATE_EMPTY_TOKEN(Colon)
GENERATE_EMPTY_TOKEN(If)
GENERATE_EMPTY_TOKEN(Else)
GENERATE_EMPTY_TOKEN(LeftBrace)
GENERATE_EMPTY_TOKEN(RightBrace)
GENERATE_EMPTY_TOKEN(LeftParent)
GENERATE_EMPTY_TOKEN(RightParent)
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

using LiteralTuple = TypeTuple<IntLiteral, FloatLiteral, StringLiteral>;

using UnaryOperatorTuple = TypeTuple<Plus, Minus, Not>;

using HighPriorityArithmeticOperatorTuple = TypeTuple<Multiply, Divide, Mod>;

using LowPriorityArithmeticOperatorTuple = TypeTuple<Plus, Minus>;

using ComparisonOperatorTuple =
    TypeTuple<Less, Greater, LessEqual, GreaterEqual>;

using EqualityOperatorTuple = TypeTuple<Equal, NotEqual>;

using TokenTuple =
    TypeTuple<Plus, Minus, Multiply, Divide, Mod, Equal, NotEqual, Less,
              Greater, LessEqual, GreaterEqual, Assignment, And, Or, Xor, Not,
              Fn, Var, Arrow, Semicolon, Colon, If, Else, LeftBrace, RightBrace,
              LeftParent, RightParent, EOFToken, Identifier, IntLiteral,
              FloatLiteral, StringLiteral>;

struct Position {
  std::size_t line;
  std::size_t offset;
  std::size_t size;

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

template <TypeTupleLike Tuple> bool is_in_type_tuple(const TokenInfo& token) {
  return std::visit(
      [](auto&& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        return IsInTypeTuple<T, Tuple>::value;
      },
      token.token_variant);
}

template <TypeTupleLike BigTuple, TypeTupleLike SmallTuple>
auto from_big_to_small(const type_tuple_to_variant_t<BigTuple>& variant)
    -> std::optional<type_tuple_to_variant_t<SmallTuple>> {
  return std::visit(
      [](auto&& val) -> std::optional<type_tuple_to_variant_t<SmallTuple>> {
        using T = std::decay_t<decltype(val)>;

        if constexpr (IsInTypeTuple<T, SmallTuple>::value) {
          return val;
        }
        return std::nullopt;
      },
      variant);
}

} // namespace tkn
