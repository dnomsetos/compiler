#pragma once

#include "scanner/token.hpp"
#include "utility/type_tuple.hpp"
#include <deque>
#include <expected>
#include <memory_resource>

#include <parser/ast.hpp>
#include <stdexcept>

struct UnexpectedToken : std::runtime_error, tkn::Position {
  type_tuple_to_variant_t<tkn::TokenTuple> expected_token;
  type_tuple_to_variant_t<tkn::TokenTuple> real_token;

  template <typename T1, typename T2>
  UnexpectedToken(tkn::Position position, T1 expected_token, T2 real_token)
      : std::runtime_error(""), tkn::Position{position},
        expected_token{expected_token}, real_token{real_token} {}
};

namespace nterm {

#define GENERATE_NTERM(name)                                                   \
  struct name {};

GENERATE_NTERM(Definition)
GENERATE_NTERM(Statement)
GENERATE_NTERM(Expression)
GENERATE_NTERM(Assignment)
GENERATE_NTERM(Unary)
GENERATE_NTERM(Primary)
GENERATE_NTERM(Literal)

using NtermTuple = TypeTuple<Definition, Statement, Expression, Assignment,
                             Unary, Primary, Literal>;

using NtermVariant = type_tuple_to_variant_t<NtermTuple>;

} // namespace nterm

struct TryButCant : std::runtime_error, tkn::Position {
  nterm::NtermVariant nterm;

  template <typename T>
  TryButCant(tkn::Position position, T nterm)
      : std::runtime_error(""), tkn::Position{position}, nterm{nterm} {}
};

using ParseIter = std::deque<tkn::TokenInfo>::const_iterator;

template <typename T>
using ParseResult =
    std::expected<std::pair<std::unique_ptr<T, PmrDeleter<T>>, ParseIter>,
                  std::variant<UnexpectedToken, TryButCant>>;

auto parse_program(ParseIter begin, ParseIter end,
                   std::pmr::memory_resource& mr) -> ParseResult<ast::Program>;

auto parse_function_definition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::FunctionDefinitionNode>;

auto parse_variable_definition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::VariableDefinitionNode>;

auto parse_if_statement(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::IfStatementNode>;

auto parse_statement(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::StatementNode>;

auto parse_expression(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::ExpressionNode>;

auto parse_assignment(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AssignmentNode>;

inline auto parse_or(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::OrNode>;

inline auto parse_xor(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::XorNode>;

inline auto parse_and(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AndNode>;

inline auto parse_equality(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::EqualityNode>;

inline auto parse_comparison(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::ComparisonNode>;

inline auto parse_addition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AdditionNode>;

inline auto parse_multiplication(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::MultiplicationNode>;

auto parse_unary(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::UnaryNode>;

auto parse_primary(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::PrimaryNode>;

auto parse_literal(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::LiteralNode>;

auto parse_identifier(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::IdentifierNode>;
