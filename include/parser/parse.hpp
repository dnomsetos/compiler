#pragma once

#include <deque>
#include <expected>
#include <stdexcept>

#include <parser/ast.hpp>
#include <scanner/token.hpp>
#include <utility/allocator.hpp>
#include <utility/type_tuple.hpp>

struct UnexpectedToken : tkn::Position {
  type_tuple_to_variant_t<tkn::TokenTuple> expected_token;
  type_tuple_to_variant_t<tkn::TokenTuple> real_token;

  template <typename T1, typename T2>
  UnexpectedToken(tkn::Position position, T1 expected_token, T2 real_token)
      : tkn::Position{position}, expected_token{expected_token},
        real_token{real_token} {}
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
GENERATE_NTERM(Cast)
GENERATE_NTERM(Type)
GENERATE_NTERM(Dereference)
GENERATE_NTERM(LvalueExpression)

using NtermTuple =
    TypeTuple<Definition, Statement, Expression, Assignment, Unary, Primary,
              Literal, Cast, Type, Dereference, LvalueExpression>;

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
    std::expected<std::pair<alloc::pmr_unique_ptr<T>, ParseIter>,
                  std::variant<UnexpectedToken, TryButCant>>;

auto parse_program(ParseIter begin, ParseIter end) -> ParseResult<ast::Program>;

auto parse_function_definition(ParseIter begin)
    -> ParseResult<ast::FunctionDefinitionNode>;

auto parse_variable_definition(ParseIter begin)
    -> ParseResult<ast::VariableDefinitionNode>;

template <typename Node, typename Token>
auto parse_loop_interrupt_statment(ParseIter begin) -> ParseResult<Node>;

auto parse_return_statement(ParseIter begin)
    -> ParseResult<ast::ReturnStatementNode>;

auto parse_block_expression(ParseIter begin)
    -> ParseResult<ast::BlockExpressionNode>;

auto parse_loop(ParseIter begin) -> ParseResult<ast::LoopExpressionNode>;

auto parse_if_expression(ParseIter begin) -> ParseResult<ast::IfExpressionNode>;

auto parse_statement(ParseIter begin) -> ParseResult<ast::StatementNode>;

auto parse_expression(ParseIter begin) -> ParseResult<ast::ExpressionNode>;

auto parse_assignment(ParseIter begin) -> ParseResult<ast::AssignmentNode>;

auto parse_lvalue_expression(ParseIter begin)
    -> ParseResult<ast::LvalueExpressionNode>;

auto parse_logical_or(ParseIter begin) -> ParseResult<ast::LogicalOrNode>;

auto parse_logical_and(ParseIter begin) -> ParseResult<ast::LogicalAndNode>;

auto parse_comparison(ParseIter begin) -> ParseResult<ast::ComparisonNode>;

auto parse_bitwise_or(ParseIter begin) -> ParseResult<ast::BitwiseOrNode>;

auto parse_bitwise_xor(ParseIter begin) -> ParseResult<ast::BitwiseXorNode>;

auto parse_bitwise_and(ParseIter begin) -> ParseResult<ast::BitwiseAndNode>;

auto parse_shift(ParseIter begin) -> ParseResult<ast::ShiftNode>;

auto parse_addition(ParseIter begin) -> ParseResult<ast::AdditionNode>;

auto parse_multiplication(ParseIter begin)
    -> ParseResult<ast::MultiplicationNode>;

auto parse_cast(ParseIter begin) -> ParseResult<ast::CastNode>;

auto parse_unary(ParseIter begin) -> ParseResult<ast::UnaryNode>;

auto parse_lvalue_dereference(ParseIter begin)
    -> ParseResult<ast::LvalueDereferenceNode>;

auto parse_primary(ParseIter begin) -> ParseResult<ast::PrimaryNode>;

auto parse_literal(ParseIter begin) -> ParseResult<ast::LiteralNode>;

auto parse_type(ParseIter begin) -> ParseResult<ast::TypeNode>;

auto parse_identifier(ParseIter begin) -> ParseResult<ast::IdentifierNode>;
