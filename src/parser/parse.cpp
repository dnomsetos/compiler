#include <cassert>
#include <expected>
#include <iostream>
#include <variant>

#include <parser/ast.hpp>
#include <parser/parse.hpp>
#include <scanner/token.hpp>
#include <utility/token_utilities.hpp>
#include <utility/type_tuple.hpp>

template <typename Result, typename PartType, TypeTupleLike OperationTuple>
auto parse_left_associative(ParseIter begin,
                            ParseResult<PartType> (*parse_part)(ParseIter))
    -> ParseResult<Result> {

  auto first_part = parse_part(begin);
  if (!first_part.has_value()) {
    return std::unexpected(first_part.error());
  }

  auto result = ast::alloc::make_unique_pmr<Result>(
      std::move(first_part.value().first), begin->position);
  begin = first_part.value().second;

  for (;;) {
    if (!is_in_type_tuple<OperationTuple>(*begin)) {
      break;
    }

    auto next_part = parse_part(begin + 1);
    if (!next_part.has_value()) {
      break;
    }
    auto optional_variant =
        variant_cast<tkn::TokenTuple, OperationTuple>(begin->token_variant);

    if (!optional_variant.has_value()) {
      break;
    }

    if constexpr (TupleSize<OperationTuple>::value == 1) {
      result->right.emplace_back(
          std::move(std::get<0>(optional_variant.value())),
          std::move(*next_part.value().first));
    } else {
      result->right.emplace_back(std::move(optional_variant.value()),
                                 std::move(*next_part.value().first));
    }
    begin = next_part.value().second;
  }
  return std::make_pair(std::move(result), begin);
}

auto parse_program(ParseIter begin, ParseIter end)
    -> ParseResult<ast::Program> {

  auto program = ast::alloc::make_unique_pmr<ast::Program>();

  for (;;) {
    auto prev_begin = begin;

    auto fn_def = parse_function_definition(begin);
    if (fn_def.has_value()) {
      program->definitions.emplace_back(std::move(*fn_def.value().first));
      begin = fn_def.value().second;
    }

    auto var_def = parse_variable_definition(begin);
    if (var_def.has_value()) {
      program->definitions.emplace_back(std::move(*var_def.value().first));
      begin = var_def.value().second;
    }

    if (prev_begin == begin) {
      return std::unexpected(TryButCant{begin->position, nterm::Statement{}});
    }

    if (std::holds_alternative<tkn::EOFToken>(begin->token_variant)) {
      return std::make_pair(std::move(program), end);
    }
  }
}

auto parse_function_definition_argument_list(
    ParseIter begin,
    std::pmr::vector<std::pair<ast::IdentifierNode, ast::IdentifierNode>>&
        argument_list) -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
  auto first_arg = parse_identifier(begin);

  if (first_arg.has_value()) {
    begin = first_arg.value().second;
    if (!std::holds_alternative<tkn::Colon>(begin->token_variant)) {
      return UnexpectedToken{begin->position, tkn::Colon{},
                             begin->token_variant};
    }
    ++begin;

    auto first_arg_type = parse_identifier(begin);
    if (!first_arg_type.has_value()) {
      return std::visit(
          [](auto&& error)
              -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
            return error;
          },
          first_arg_type.error());
    }
    begin = first_arg_type.value().second;

    argument_list.emplace_back(std::move(*first_arg.value().first),
                               std::move(*first_arg_type.value().first));

    for (;;) {
      if (!std::holds_alternative<tkn::Comma>(begin->token_variant)) {
        break;
      }
      ++begin;

      auto arg = parse_identifier(begin);
      if (!arg.has_value()) {
        return std::visit(
            [](auto&& error)
                -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
              return error;
            },
            arg.error());
      }
      begin = arg.value().second;

      if (!std::holds_alternative<tkn::Colon>(begin->token_variant)) {
        return UnexpectedToken{begin->position, tkn::Colon{},
                               begin->token_variant};
      }
      ++begin;

      auto arg_type = parse_identifier(begin);
      if (!arg_type.has_value()) {
        return std::visit(
            [](auto&& error)
                -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
              return error;
            },
            arg_type.error());
      }
      begin = arg_type.value().second;

      argument_list.emplace_back(std::move(*arg.value().first),
                                 std::move(*arg_type.value().first));
    }
  }
  return begin;
}

auto parse_function_definition(ParseIter begin)
    -> ParseResult<ast::FunctionDefinitionNode> {

  if (!std::holds_alternative<tkn::Fn>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::Fn{}, begin->token_variant});
  }

  auto identifier = parse_identifier(begin + 1);
  if (!identifier.has_value()) {
    return std::unexpected(identifier.error());
  }
  begin = identifier.value().second;

  if (!std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftParent{},
                                           begin->token_variant});
  }
  ++begin;

  auto result = ast::alloc::make_unique_pmr<ast::FunctionDefinitionNode>(
      std::move(identifier.value().first), (begin - 2)->position);

  auto parse_list =
      parse_function_definition_argument_list(begin, result->argument_list);
  if (!std::holds_alternative<ParseIter>(parse_list)) {
    return std::visit(
        [](auto&& value) -> ParseResult<ast::FunctionDefinitionNode> {
          if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                       TryButCant> ||
                        std::is_same_v<std::decay_t<decltype(value)>,
                                       UnexpectedToken>) {
            return std::unexpected(value);
          } else {
            return std::unexpected(
                TryButCant{value->position, nterm::Definition{}});
          }
        },
        parse_list);
  }
  begin = std::get<ParseIter>(parse_list);

  if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightParent{},
                                           begin->token_variant});
  }
  ++begin;

  auto block = parse_block_expression(begin);
  if (!block.has_value()) {
    return std::unexpected(block.error());
  }
  begin = block.value().second;

  result->body = std::move(block.value().first);

  return std::make_pair(std::move(result), begin);
}

auto parse_variable_definition(ParseIter begin)
    -> ParseResult<ast::VariableDefinitionNode> {

  if (!std::holds_alternative<tkn::Var>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::Var{}, begin->token_variant});
  }

  auto identifier = parse_identifier(begin + 1);
  if (!identifier.has_value()) {
    return std::unexpected(identifier.error());
  }
  begin = identifier.value().second;

  auto result = ast::alloc::make_unique_pmr<ast::VariableDefinitionNode>(
      std::move(identifier.value().first), begin->position);

  if (std::holds_alternative<tkn::Colon>(begin->token_variant)) {
    auto type = parse_identifier(begin + 1);

    if (!type.has_value()) {
      return std::unexpected(type.error());
    }

    result->type = std::move(type.value().first);
    begin = type.value().second;
  }

  if (std::holds_alternative<tkn::Assignment>(begin->token_variant)) {
    auto expr = parse_expression(begin + 1);

    if (!expr.has_value()) {
      return std::unexpected(expr.error());
    }

    result->value = std::move(expr.value().first);
    begin = expr.value().second;
  }

  if (!std::holds_alternative<tkn::Semicolon>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::Semicolon{},
                                           begin->token_variant});
  }
  return std::make_pair(std::move(result), begin + 1);
}

template <typename Node, typename Token>
auto parse_loop_interrupt_statment(ParseIter begin) -> ParseResult<Node> {

  if (!std::holds_alternative<Token>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, Token{}, begin->token_variant});
  }

  auto result = ast::alloc::make_unique_pmr<Node>(begin->position);
  ++begin;

  if constexpr (requires { std::declval<Node>().label; }) {
    if (std::holds_alternative<tkn::Label>(begin->token_variant)) {
      result->label.emplace(std::get<tkn::Label>(begin->token_variant));
      ++begin;
    }
  }

  if constexpr (requires { std::declval<Node>().value; }) {
    auto expr = parse_expression(begin);
    if (expr.has_value()) {
      result->value = std::move(expr.value().first);
      begin = expr.value().second;
    }
  }

  if (!std::holds_alternative<tkn::Semicolon>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::Semicolon{},
                                           begin->token_variant});
  }
  ++begin;

  return std::make_pair(std::move(result), begin);
}

template auto
parse_loop_interrupt_statment<ast::BreakStatementNode, tkn::Break>(
    ParseIter begin) -> ParseResult<ast::BreakStatementNode>;

template auto
parse_loop_interrupt_statment<ast::ContinueStatementNode, tkn::Continue>(
    ParseIter begin) -> ParseResult<ast::ContinueStatementNode>;

template auto
parse_loop_interrupt_statment<ast::ReturnStatementNode, tkn::Return>(
    ParseIter begin) -> ParseResult<ast::ReturnStatementNode>;

auto parse_block_expression(ParseIter begin)
    -> ParseResult<ast::BlockExpressionNode> {

  if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftBrace{},
                                           begin->token_variant});
  }

  auto result = ast::BlockExpressionNode(begin->position);
  ++begin;

  for (;;) {
    auto stmt = parse_statement(begin);
    if (!stmt.has_value()) {
      break;
    }

    begin = stmt.value().second;
    result.statements.emplace_back(std::move(*stmt.value().first));
  }

  auto expr = parse_expression(begin);
  if (!expr.has_value()) {
    if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
      return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                             begin->token_variant});
    }

    return std::make_pair(ast::alloc::make_unique_pmr<ast::BlockExpressionNode>(
                              std::move(result)),
                          begin + 1);
  }

  result.value = std::move(expr.value().first);
  begin = expr.value().second;

  if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                           begin->token_variant});
  }

  return std::make_pair(
      ast::alloc::make_unique_pmr<ast::BlockExpressionNode>(std::move(result)),
      begin + 1);
}

auto parse_loop_expression(ParseIter begin)
    -> ParseResult<ast::LoopExpressionNode> {

  std::optional<tkn::Label> label;
  if (std::holds_alternative<tkn::Label>(begin->token_variant)) {
    label.emplace(std::get<tkn::Label>(begin->token_variant));
    ++begin;

    if (!std::holds_alternative<tkn::Colon>(begin->token_variant)) {
      return std::unexpected(
          UnexpectedToken{begin->position, tkn::Colon{}, begin->token_variant});
    }

    ++begin;
  }

  if (!std::holds_alternative<tkn::Loop>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::Loop{}, begin->token_variant});
  }
  ++begin;

  if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftParent{},
                                           begin->token_variant});
  }
  ++begin;

  auto result = ast::alloc::make_unique_pmr<ast::LoopExpressionNode>(
      std::move(label), begin->position);

  for (;;) {
    auto stmt = parse_statement(begin);
    if (!stmt.has_value()) {
      break;
    }

    result->body.push_back(std::move(*stmt.value().first));
    begin = stmt.value().second;
  }

  if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                           begin->token_variant});
  }

  return std::make_pair(std::move(result), begin + 1);
}

auto parse_if_expression(ParseIter begin)
    -> ParseResult<ast::IfExpressionNode> {

  if (!std::holds_alternative<tkn::If>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::If{}, begin->token_variant});
  }
  ++begin;

  auto condition = parse_expression(begin);
  if (!condition.has_value()) {
    return std::unexpected(condition.error());
  }
  begin = condition.value().second;

  auto block = parse_block_expression(begin);
  if (!block.has_value()) {
    return std::unexpected(block.error());
  }
  begin = block.value().second;

  auto if_expr = ast::alloc::make_unique_pmr<ast::IfExpressionNode>(
      std::move(condition.value().first), std::move(block.value().first),
      begin->position);

  if (!std::holds_alternative<tkn::Else>(begin->token_variant)) {
    return std::make_pair(std::move(if_expr), begin);
  }
  ++begin;

  while (std::holds_alternative<tkn::If>(begin->token_variant)) {
    ++begin;
    auto block_begin = begin;

    auto condition = parse_expression(begin);
    if (!condition.has_value()) {
      return std::unexpected(condition.error());
    }
    begin = condition.value().second;

    auto block = parse_block_expression(begin);
    if (!block.has_value()) {
      return std::unexpected(block.error());
    }
    begin = block.value().second;

    if_expr->elif_bodies.emplace_back(std::move(condition.value().first),
                                      std::move(block.value().first),
                                      block_begin->position);

    if (!std::holds_alternative<tkn::Else>(begin->token_variant)) {
      return std::make_pair(std::move(if_expr), begin);
    }
    ++begin;
  }

  auto else_block = parse_block_expression(begin);
  if (!else_block.has_value()) {
    return std::unexpected(else_block.error());
  }
  begin = else_block.value().second;

  if_expr->else_body = std::move(else_block.value().first);
  return std::make_pair(std::move(if_expr), begin);
}

auto parse_statement(ParseIter begin) -> ParseResult<ast::StatementNode> {

  auto block_expr = parse_block_expression(begin);
  if (block_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*block_expr.value().first), begin->position),
        block_expr.value().second);
  }

  auto var_def = parse_variable_definition(begin);
  if (var_def.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*var_def.value().first), begin->position),
        var_def.value().second);
  }

  auto if_expr = parse_if_expression(begin);
  if (if_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*if_expr.value().first), begin->position),
        if_expr.value().second);
  }

  auto loop_expr = parse_loop_expression(begin);
  if (loop_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*loop_expr.value().first), begin->position),
        loop_expr.value().second);
  }

  auto break_stmt =
      parse_loop_interrupt_statment<ast::BreakStatementNode, tkn::Break>(begin);
  if (break_stmt.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*break_stmt.value().first), begin->position),
        break_stmt.value().second);
  }

  auto continue_stmt =
      parse_loop_interrupt_statment<ast::ContinueStatementNode, tkn::Continue>(
          begin);
  if (continue_stmt.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::StatementNode>(
            std::move(*continue_stmt.value().first), begin->position),
        continue_stmt.value().second);
  }

  auto expr = parse_expression(begin);
  if (expr.has_value()) {
    if (!std::holds_alternative<tkn::Semicolon>(
            expr.value().second->token_variant)) {
      return std::unexpected(
          UnexpectedToken{expr.value().second->position, tkn::Semicolon{},
                          expr.value().second->token_variant});
    }
    return std::make_pair(ast::alloc::make_unique_pmr<ast::StatementNode>(
                              std::move(*expr.value().first), begin->position),
                          expr.value().second + 1);
  }

  return std::unexpected(TryButCant{begin->position, nterm::Statement{}});
}

auto parse_expression(ParseIter begin) -> ParseResult<ast::ExpressionNode> {

  auto block_expr = parse_block_expression(begin);
  if (block_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::ExpressionNode>(
            std::move(*block_expr.value().first), begin->position),
        block_expr.value().second);
  }

  auto if_expr = parse_if_expression(begin);
  if (if_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::ExpressionNode>(
            std::move(*if_expr.value().first), begin->position),
        if_expr.value().second);
  }

  auto loop_expr = parse_loop_expression(begin);
  if (loop_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::ExpressionNode>(
            std::move(*loop_expr.value().first), begin->position),
        loop_expr.value().second);
  }

  auto assignment = parse_assignment(begin);
  if (assignment.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::ExpressionNode>(
            std::move(*assignment.value().first), begin->position),
        assignment.value().second);
  }

  auto or_expr = parse_or(begin);
  if (or_expr.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::ExpressionNode>(
            std::move(*or_expr.value().first), begin->position),
        or_expr.value().second);
  }

  return std::unexpected(TryButCant{begin->position, nterm::Expression{}});
}

auto parse_assignment(ParseIter begin) -> ParseResult<ast::AssignmentNode> {

  auto identifier = parse_identifier(begin);
  if (identifier.has_value()) {
    ++begin;

    if (!std::holds_alternative<tkn::Assignment>(begin->token_variant)) {
      return std::unexpected(UnexpectedToken{begin->position, tkn::Assignment{},
                                             begin->token_variant});
    }
    ++begin;

    auto expr = parse_expression(begin);
    if (expr.has_value()) {
      return std::make_pair(ast::alloc::make_unique_pmr<ast::AssignmentNode>(
                                std::move(identifier.value().first),
                                std::move(expr.value().first), begin->position),
                            expr.value().second);
    }
  }
  return std::unexpected(TryButCant{begin->position, nterm::Assignment{}});
}

auto parse_or(ParseIter begin) -> ParseResult<ast::OrNode> {
  return parse_left_associative<ast::OrNode, ast::XorNode, TypeTuple<tkn::Or>>(
      begin, parse_xor);
}

auto parse_xor(ParseIter begin) -> ParseResult<ast::XorNode> {
  return parse_left_associative<ast::XorNode, ast::AndNode,
                                TypeTuple<tkn::Xor>>(begin, parse_and);
}

auto parse_and(ParseIter begin) -> ParseResult<ast::AndNode> {
  return parse_left_associative<ast::AndNode, ast::EqualityNode,
                                TypeTuple<tkn::And>>(begin, parse_equality);
}

auto parse_equality(ParseIter begin) -> ParseResult<ast::EqualityNode> {
  return parse_left_associative<ast::EqualityNode, ast::ComparisonNode,
                                tkn::EqualityOperatorTuple>(begin,
                                                            parse_comparison);
}

auto parse_comparison(ParseIter begin) -> ParseResult<ast::ComparisonNode> {
  return parse_left_associative<ast::ComparisonNode, ast::AdditionNode,
                                tkn::ComparisonOperatorTuple>(begin,
                                                              parse_addition);
}

auto parse_addition(ParseIter begin) -> ParseResult<ast::AdditionNode> {
  return parse_left_associative<ast::AdditionNode, ast::MultiplicationNode,
                                tkn::LowPriorityArithmeticOperatorTuple>(
      begin, parse_multiplication);
}

auto parse_multiplication(ParseIter begin)
    -> ParseResult<ast::MultiplicationNode> {
  return parse_left_associative<ast::MultiplicationNode, ast::UnaryNode,
                                tkn::HighPriorityArithmeticOperatorTuple>(
      begin, parse_unary);
}

auto parse_unary(ParseIter begin) -> ParseResult<ast::UnaryNode> {

  auto optional_variant =
      variant_cast<tkn::TokenTuple, tkn::UnaryOperatorTuple>(
          begin->token_variant);

  if (optional_variant.has_value()) {
    auto primary = parse_primary(begin + 1);

    if (primary.has_value()) {
      auto unary = ast::alloc::make_unique_pmr<ast::UnaryNode>(
          std::move(primary.value().first), begin->position);

      unary->op.emplace(ast::alloc::make_unique_pmr<
                        type_tuple_to_variant_t<tkn::UnaryOperatorTuple>>(
          std::move(optional_variant.value())));

      return std::make_pair(std::move(unary), primary.value().second);
    }
    return std::unexpected(TryButCant{begin->position, nterm::Unary{}});
  }

  auto primary = parse_primary(begin);
  if (primary.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::UnaryNode>(
            std::move(primary.value().first), begin->position),
        primary.value().second);
  }
  return std::unexpected(TryButCant{begin->position, nterm::Unary{}});
}

auto parse_function_call_argument_list(
    ParseIter begin, std::pmr::vector<ast::ExpressionNode>& arguments)
    -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
  auto first_expr = parse_expression(begin);

  if (first_expr.has_value()) {
    arguments.emplace_back(std::move(*first_expr.value().first));
    begin = first_expr.value().second;

    for (;;) {
      if (!std::holds_alternative<tkn::Comma>(begin->token_variant)) {
        break;
      }
      ++begin;

      auto expr = parse_expression(begin);
      if (!expr.has_value()) {
        return std::visit(
            [](auto&& error)
                -> std::variant<ParseIter, UnexpectedToken, TryButCant> {
              return error;
            },
            expr.error());
      }

      arguments.emplace_back(std::move(*expr.value().first));
      begin = expr.value().second;
    }
  }
  return begin;
}

auto parse_primary(ParseIter begin) -> ParseResult<ast::PrimaryNode> {

  auto start_begin = begin;

  if (std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
    auto expr = parse_expression(begin + 1);

    if (expr.has_value()) {
      auto current = expr.value().second;

      if (std::holds_alternative<tkn::RightParent>(current->token_variant)) {
        return std::make_pair(
            ast::alloc::make_unique_pmr<ast::PrimaryNode>(
                std::move(*expr.value().first), begin->position),
            current + 1);
      }
    }
  }

  auto literal = parse_literal(begin);
  if (literal.has_value()) {
    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::PrimaryNode>(
            std::move(*literal.value().first), begin->position),
        literal.value().second);
  }

  auto identifier = parse_identifier(begin);
  if (identifier.has_value()) {
    begin = identifier.value().second;

    if (std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
      ++begin;

      auto result = ast::alloc::make_unique_pmr<ast::FunctionCallNode>(
          std::move(identifier.value().first), (begin - 2)->position);

      auto parse_list =
          parse_function_call_argument_list(begin, result->arguments);

      if (!std::holds_alternative<ParseIter>(parse_list)) {
        return std::visit(
            [start_begin](auto&& value) -> ParseResult<ast::PrimaryNode> {
              if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                           TryButCant> ||
                            std::is_same_v<std::decay_t<decltype(value)>,
                                           UnexpectedToken>) {
                return std::unexpected(value);
              }
              return std::unexpected(
                  TryButCant{start_begin->position, nterm::Primary{}});
            },
            parse_list);
      }
      begin = std::get<ParseIter>(parse_list);

      if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::RightParent{}, begin->token_variant});
      }

      return std::make_pair(ast::alloc::make_unique_pmr<ast::PrimaryNode>(
                                std::move(*result), start_begin->position),
                            begin + 1);
    }

    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::PrimaryNode>(
            std::move(*identifier.value().first), start_begin->position),
        begin);
  }
  return std::unexpected(TryButCant{start_begin->position, nterm::Primary{}});
}

auto parse_literal(ParseIter begin) -> ParseResult<ast::LiteralNode> {

  if (is_in_type_tuple<tkn::LiteralTuple>(*begin)) {
    auto next = begin + 1;

    auto optional_variant =
        variant_cast<tkn::TokenTuple, tkn::LiteralTuple>(begin->token_variant);

    if (!optional_variant.has_value()) {
      return std::unexpected(TryButCant{begin->position, nterm::Literal{}});
    }

    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::LiteralNode>(
            std::move(optional_variant.value()), begin->position),
        next);
  }
  return std::unexpected(TryButCant{begin->position, nterm::Literal{}});
}

auto parse_identifier(ParseIter begin) -> ParseResult<ast::IdentifierNode> {

  if (std::holds_alternative<tkn::Identifier>(begin->token_variant)) {
    auto next = begin + 1;

    return std::make_pair(
        ast::alloc::make_unique_pmr<ast::IdentifierNode>(

            std::get<tkn::Identifier>(begin->token_variant), begin->position),
        next);
  }
  return std::unexpected(UnexpectedToken{begin->position, tkn::Identifier{},
                                         begin->token_variant});
}
