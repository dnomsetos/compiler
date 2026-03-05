#include <expected>
#include <variant>

#include <parser/ast.hpp>
#include <parser/parse.hpp>
#include <scanner/token.hpp>
#include <utility/type_tuple.hpp>

template <typename Result, typename PartType, TypeTupleLike OperationTuple>
auto parse_left_associative(
    ParseIter begin, std::pmr::memory_resource& mr,
    ParseResult<PartType> (*parse_part)(ParseIter, std::pmr::memory_resource&))
    -> ParseResult<Result> {
  auto first_part = parse_part(begin, mr);
  if (!first_part.has_value()) {
    return std::unexpected(first_part.error());
  }
  auto result = make_unique_pmr<Result>(
      &mr, std::move(first_part.value().first), begin->position, &mr);
  begin = first_part.value().second;
  for (;;) {
    if (!tkn::is_in_type_tuple<OperationTuple>(*begin)) {
      break;
    }
    auto next_part = parse_part(begin + 1, mr);
    if (!next_part.has_value()) {
      break;
    }
    auto optional_variant =
        tkn::from_big_to_small<tkn::TokenTuple, OperationTuple>(
            begin->token_variant);
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

auto parse_program(ParseIter begin, ParseIter end,
                   std::pmr::memory_resource& mr) -> ParseResult<ast::Program> {
  auto program = make_unique_pmr<ast::Program>(&mr, &mr);
  for (;;) {
    auto prev_begin = begin;
    auto fn_def = parse_function_definition(begin, mr);
    if (fn_def.has_value()) {
      program->definitions.emplace_back(std::move(*fn_def.value().first));
      begin = fn_def.value().second;
    }
    auto var_def = parse_variable_definition(begin, mr);
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

auto parse_function_definition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::FunctionDefinitionNode> {
  if (!std::holds_alternative<tkn::Fn>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::Fn{}, begin->token_variant});
  }
  auto identifier = parse_identifier(begin + 1, mr);
  if (!identifier.has_value()) {
    return std::unexpected(identifier.error());
  }
  begin = identifier.value().second;
  if (!std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftParent{},
                                           begin->token_variant});
  }
  ++begin;
  auto result = make_unique_pmr<ast::FunctionDefinitionNode>(
      &mr, std::move(identifier.value().first), (begin - 2)->position, &mr);
  auto first_arg = parse_identifier(begin, mr);
  if (first_arg.has_value()) {
    begin = first_arg.value().second;
    if (!std::holds_alternative<tkn::Colon>(begin->token_variant)) {
      return std::unexpected(
          UnexpectedToken{begin->position, tkn::Colon{}, begin->token_variant});
    }
    ++begin;
    auto first_arg_type = parse_identifier(begin, mr);
    if (!first_arg_type.has_value()) {
      return std::unexpected(first_arg_type.error());
    }
    begin = first_arg_type.value().second;
    result->argument_lits.emplace_back(
        std::move(*first_arg.value().first),
        std::move(*first_arg_type.value().first));
    for (;;) {
      if (!std::holds_alternative<tkn::Comma>(begin->token_variant)) {
        break;
      }
      ++begin;
      auto arg = parse_identifier(begin, mr);
      if (!arg.has_value()) {
        return std::unexpected(arg.error());
      }
      begin = arg.value().second;
      if (!std::holds_alternative<tkn::Colon>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{begin->position, tkn::Colon{},
                                               begin->token_variant});
      }
      ++begin;
      auto arg_type = parse_identifier(begin, mr);
      if (!arg_type.has_value()) {
        return std::unexpected(arg_type.error());
      }
      begin = arg_type.value().second;
      result->argument_lits.emplace_back(std::move(*arg.value().first),
                                         std::move(*arg_type.value().first));
    }
  }
  if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightParent{},
                                           begin->token_variant});
  }
  ++begin;
  if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftBrace{},
                                           begin->token_variant});
  }
  ++begin;
  for (;;) {
    auto stmt = parse_statement(begin, mr);
    if (!stmt.has_value()) {
      break;
    }
    result->body.emplace_back(std::move(*stmt.value().first));
    begin = stmt.value().second;
  }
  auto return_expr = parse_expression(begin, mr);
  if (!return_expr.has_value()) {
    return std::unexpected(return_expr.error());
  }
  begin = return_expr.value().second;
  if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                           begin->token_variant});
  }
  result->return_value = std::move(return_expr.value().first);
  return std::make_pair(std::move(result), begin + 1);
}

auto parse_variable_definition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::VariableDefinitionNode> {
  if (!std::holds_alternative<tkn::Var>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::Var{}, begin->token_variant});
  }
  auto identifier = parse_identifier(begin + 1, mr);
  if (!identifier.has_value()) {
    return std::unexpected(identifier.error());
  }
  auto result = make_unique_pmr<ast::VariableDefinitionNode>(
      &mr, std::move(identifier.value().first), begin->position);
  begin = identifier.value().second;
  if (std::holds_alternative<tkn::Colon>(begin->token_variant)) {
    auto type = parse_identifier(begin + 1, mr);
    if (!type.has_value()) {
      return std::unexpected(type.error());
    }
    result->type = std::move(type.value().first);
    begin = type.value().second;
  }
  if (std::holds_alternative<tkn::Equal>(begin->token_variant)) {
    auto expr = parse_expression(begin + 1, mr);
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

auto parse_if_statement(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::IfStatementNode> {
  auto start_begin = begin;
  if (!std::holds_alternative<tkn::If>(begin->token_variant)) {
    return std::unexpected(
        UnexpectedToken{begin->position, tkn::If{}, begin->token_variant});
  }
  ++begin;
  if (!std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftParent{},
                                           begin->token_variant});
  }
  ++begin;
  auto cond_expr = parse_expression(begin, mr);
  if (!cond_expr.has_value()) {
    return std::unexpected(cond_expr.error());
  }
  begin = cond_expr.value().second;
  if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightParent{},
                                           begin->token_variant});
  }
  ++begin;
  if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::LeftBrace{},
                                           begin->token_variant});
  }
  ++begin;
  auto if_stmt = make_unique_pmr<ast::IfStatementNode>(
      &mr, std::move(cond_expr.value().first), start_begin->position, &mr);
  for (;;) {
    auto statement = parse_statement(begin, mr);
    if (!statement.has_value()) {
      break;
    }
    if_stmt->body.emplace_back(std::move(*statement.value().first));
    begin = statement.value().second;
  }
  if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
    return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                           begin->token_variant});
  }
  ++begin;
  if (std::holds_alternative<tkn::Else>(begin->token_variant)) {
    ++begin;
    while (std::holds_alternative<tkn::If>(begin->token_variant)) {
      ++begin;
      if (!std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::LeftParent{}, begin->token_variant});
      }
      ++begin;
      auto elif_cond_expr = parse_expression(begin, mr);
      if (!elif_cond_expr.has_value()) {
        return std::unexpected(elif_cond_expr.error());
      }
      begin = elif_cond_expr.value().second;
      if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::RightParent{}, begin->token_variant});
      }
      if_stmt->elif_bodies.emplace_back(
          std::move(*elif_cond_expr.value().first), &mr);
      ++begin;
      if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::LeftBrace{}, begin->token_variant});
      }
      ++begin;
      for (;;) {
        auto statement = parse_statement(begin, mr);
        if (!statement.has_value()) {
          break;
        }
        if_stmt->elif_bodies.back().statements.emplace_back(
            std::move(*statement.value().first));
        begin = statement.value().second;
      }
      if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::RightBrace{}, begin->token_variant});
      }
      ++begin;
      if (!std::holds_alternative<tkn::Else>(begin->token_variant)) {
        return std::make_pair(std::move(if_stmt), begin);
      }
      ++begin;
    }
    if (!std::holds_alternative<tkn::LeftBrace>(begin->token_variant)) {
      return std::unexpected(UnexpectedToken{begin->position, tkn::LeftBrace{},
                                             begin->token_variant});
    }
    ++begin;
    if_stmt->else_body.emplace();
    for (;;) {
      auto statement = parse_statement(begin, mr);
      if (!statement.has_value()) {
        break;
      }
      if_stmt->else_body.value().emplace_back(
          std::move(*statement.value().first));
      begin = statement.value().second;
    }
    if (!std::holds_alternative<tkn::RightBrace>(begin->token_variant)) {
      return std::unexpected(UnexpectedToken{begin->position, tkn::RightBrace{},
                                             begin->token_variant});
    }
    ++begin;
  }
  return std::make_pair(std::move(if_stmt), begin);
}

auto parse_statement(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::StatementNode> {
  auto var_def = parse_variable_definition(begin, mr);
  if (var_def.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::StatementNode>(
            &mr, std::move(*var_def.value().first), begin->position, &mr),
        var_def.value().second);
  }
  auto if_stmt = parse_if_statement(begin, mr);
  if (if_stmt.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::StatementNode>(
            &mr, std::move(*if_stmt.value().first), begin->position, &mr),
        if_stmt.value().second);
  }
  auto expr = parse_expression(begin, mr);
  if (expr.has_value()) {
    if (std::holds_alternative<tkn::Semicolon>(
            expr.value().second->token_variant)) {
      return std::make_pair(
          make_unique_pmr<ast::StatementNode>(
              &mr, std::move(*expr.value().first), begin->position, &mr),
          expr.value().second + 1);
    }
  }
  return std::unexpected(TryButCant{begin->position, nterm::Statement{}});
}

auto parse_expression(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::ExpressionNode> {
  auto assignment = parse_assignment(begin, mr);
  if (assignment.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::ExpressionNode>(
            &mr, std::move(*assignment.value().first), begin->position, &mr),
        assignment.value().second);
  }
  auto or_expr = parse_or(begin, mr);
  if (or_expr.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::ExpressionNode>(
            &mr, std::move(*or_expr.value().first), begin->position, &mr),
        or_expr.value().second);
  }
  return std::unexpected(TryButCant{begin->position, nterm::Expression{}});
}

auto parse_assignment(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AssignmentNode> {
  auto identifier = parse_identifier(begin, mr);
  if (identifier.has_value()) {
    ++begin;
    if (!std::holds_alternative<tkn::Assignment>(begin->token_variant)) {
      return std::unexpected(UnexpectedToken{begin->position, tkn::Assignment{},
                                             begin->token_variant});
    }
    ++begin;
    auto expr = parse_expression(begin, mr);
    if (expr.has_value()) {
      return std::make_pair(make_unique_pmr<ast::AssignmentNode>(
                                &mr, std::move(identifier.value().first),
                                std::move(expr.value().first), begin->position),
                            expr.value().second);
    }
  }
  return std::unexpected(TryButCant{begin->position, nterm::Assignment{}});
}

auto parse_or(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::OrNode> {
  return parse_left_associative<ast::OrNode, ast::XorNode, TypeTuple<tkn::Or>>(
      begin, mr, parse_xor);
}

auto parse_xor(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::XorNode> {
  return parse_left_associative<ast::XorNode, ast::AndNode,
                                TypeTuple<tkn::Xor>>(begin, mr, parse_and);
}

auto parse_and(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AndNode> {
  return parse_left_associative<ast::AndNode, ast::EqualityNode,
                                TypeTuple<tkn::And>>(begin, mr, parse_equality);
}

auto parse_equality(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::EqualityNode> {
  return parse_left_associative<ast::EqualityNode, ast::ComparisonNode,
                                tkn::EqualityOperatorTuple>(begin, mr,
                                                            parse_comparison);
}

auto parse_comparison(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::ComparisonNode> {
  return parse_left_associative<ast::ComparisonNode, ast::AdditionNode,
                                tkn::ComparisonOperatorTuple>(begin, mr,
                                                              parse_addition);
}

auto parse_addition(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::AdditionNode> {
  return parse_left_associative<ast::AdditionNode, ast::MultiplicationNode,
                                tkn::LowPriorityArithmeticOperatorTuple>(
      begin, mr, parse_multiplication);
}

auto parse_multiplication(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::MultiplicationNode> {
  return parse_left_associative<ast::MultiplicationNode, ast::UnaryNode,
                                tkn::HighPriorityArithmeticOperatorTuple>(
      begin, mr, parse_unary);
}

auto parse_unary(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::UnaryNode> {
  auto optional_variant =
      tkn::from_big_to_small<tkn::TokenTuple, tkn::UnaryOperatorTuple>(
          begin->token_variant);
  if (optional_variant.has_value()) {
    auto primary = parse_primary(begin + 1, mr);
    if (primary.has_value()) {
      auto unary = make_unique_pmr<ast::UnaryNode>(
          &mr, std::move(primary.value().first), begin->position);
      unary->op.emplace(
          make_unique_pmr<type_tuple_to_variant_t<tkn::UnaryOperatorTuple>>(
              &mr, std::move(optional_variant.value())));
      return std::make_pair(std::move(unary), primary.value().second);
    }
    return std::unexpected(TryButCant{begin->position, nterm::Unary{}});
  }
  auto primary = parse_primary(begin, mr);
  if (primary.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::UnaryNode>(&mr, std::move(primary.value().first),
                                        begin->position),
        primary.value().second);
  }
  return std::unexpected(TryButCant{begin->position, nterm::Unary{}});
}

auto parse_primary(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::PrimaryNode> {
  auto start_begin = begin;
  if (std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
    auto expr = parse_expression(begin + 1, mr);
    if (expr.has_value()) {
      auto current = expr.value().second;
      if (std::holds_alternative<tkn::RightParent>(current->token_variant)) {
        return std::make_pair(
            make_unique_pmr<ast::PrimaryNode>(
                &mr, std::move(*expr.value().first), begin->position, &mr),
            current + 1);
      }
    }
  }
  auto literal = parse_literal(begin, mr);
  if (literal.has_value()) {
    return std::make_pair(
        make_unique_pmr<ast::PrimaryNode>(
            &mr, std::move(*literal.value().first), begin->position, &mr),
        literal.value().second);
  }
  auto identifier = parse_identifier(begin, mr);
  if (identifier.has_value()) {
    begin = identifier.value().second;
    if (std::holds_alternative<tkn::LeftParent>(begin->token_variant)) {
      ++begin;
      auto result = make_unique_pmr<ast::FunctionCallNode>(
          &mr, std::move(identifier.value().first), (begin - 2)->position, &mr);
      auto first_expr = parse_expression(begin, mr);
      if (first_expr.has_value()) {
        begin = first_expr.value().second;
        for (;;) {
          if (!std::holds_alternative<tkn::Comma>(begin->token_variant)) {
            break;
          }
          ++begin;
          auto expr = parse_expression(begin, mr);
          if (!expr.has_value()) {
            return std::unexpected(expr.error());
          }
          result->arguments.emplace_back(std::move(*expr.value().first));
          begin = expr.value().second;
        }
      }
      if (!std::holds_alternative<tkn::RightParent>(begin->token_variant)) {
        return std::unexpected(UnexpectedToken{
            begin->position, tkn::RightParent{}, begin->token_variant});
      }
      return std::make_pair(
          make_unique_pmr<ast::PrimaryNode>(&mr, std::move(*result),
                                            start_begin->position, &mr),
          begin + 1);
    }
    return std::make_pair(make_unique_pmr<ast::PrimaryNode>(
                              &mr, std::move(*identifier.value().first),
                              start_begin->position, &mr),
                          begin);
  }
  return std::unexpected(TryButCant{start_begin->position, nterm::Primary{}});
}

auto parse_literal(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::LiteralNode> {
  if (tkn::is_in_type_tuple<tkn::LiteralTuple>(*begin)) {
    auto next = begin + 1;
    auto optional_variant =
        tkn::from_big_to_small<tkn::TokenTuple, tkn::LiteralTuple>(
            begin->token_variant);
    if (!optional_variant.has_value()) {
      return std::unexpected(TryButCant{begin->position, nterm::Literal{}});
    }
    return std::make_pair(
        make_unique_pmr<ast::LiteralNode>(
            &mr, std::move(optional_variant.value()), begin->position, &mr),
        next);
  }
  return std::unexpected(TryButCant{begin->position, nterm::Literal{}});
}

auto parse_identifier(ParseIter begin, std::pmr::memory_resource& mr)
    -> ParseResult<ast::IdentifierNode> {
  if (std::holds_alternative<tkn::Identifier>(begin->token_variant)) {
    auto next = begin + 1;
    return std::make_pair(make_unique_pmr<ast::IdentifierNode>(
                              &mr,
                              std::get<tkn::Identifier>(begin->token_variant),
                              begin->position, &mr),
                          next);
  }
  return std::unexpected(UnexpectedToken{begin->position, tkn::Identifier{},
                                         begin->token_variant});
}
