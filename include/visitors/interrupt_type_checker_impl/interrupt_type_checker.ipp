#if !defined(INTERRUPT_TYPE_CHECKER_GUARD) && !defined(_CLANGD)
#error "Include interrupt_type_checker.hpp"
#endif

#pragma once

#include <variant>

#include <visitors/interrupt_type_checker.hpp>

template <ast::InterruptNode TriggerNode>
InterruptTypeChecker<TriggerNode>::InterruptTypeChecker(
    const TypeChecker& type_checker, const std::uint8_t real_type)
    : type_checker_(type_checker), real_type_(real_type) {}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::check_type(const TriggerNode& trigger) {

  std::uint8_t type_index =
      type_tuple_index_v<tp::Void, tp::BasicTypeTypeTuple>;

  if (real_type_ == tp::no_type) {
    real_type_ = type_checker_(*trigger.value.value());
    return;
  }

  if (!trigger.value.has_value()) {
    if (real_type_ == type_index) {
      return;
    } else {
      is_failed_ = true;
      return;
    }
  }

  type_index = type_checker_(*trigger.value.value());

  if (type_index != real_type_) {
    is_failed_ = true;
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(const TriggerNode& trigger) {
  check_type(trigger);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::FunctionCallNode& function_call) {
  for (auto& arg : function_call.arguments) {
    operator()(arg);
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::ExpressionNode& expression) {
  std::visit(*this, *expression.node);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::BlockExpressionNode& block_expression) {
  for (auto& stmt : block_expression.statements) {
    operator()(stmt);

    if (is_failed_) {
      return;
    }

    if (std::holds_alternative<TriggerNode>(*stmt.node)) {
      check_type(std::get<TriggerNode>(*stmt.node));

      if (is_failed_) {
        return;
      }
    }
  }

  if (block_expression.value.has_value()) {
    operator()(*block_expression.value.value());
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::IfExpressionNode& if_expression) {
  operator()(*if_expression.condition);

  if (is_failed_) {
    return;
  }

  operator()(*if_expression.body);

  if (is_failed_) {
    return;
  }

  for (auto& block : if_expression.elif_bodies) {
    operator()(*block.expr);
    if (is_failed_) {
      return;
    }

    operator()(*block.block);
    if (is_failed_) {
      return;
    }
  }

  if (if_expression.else_body.has_value()) {
    operator()(*if_expression.else_body.value());
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::AssignmentNode& assignment) {
  operator()(*assignment.right);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::LoopExpressionNode& loop) {
  for (auto& stmt : loop.body) {
    operator()(stmt);

    if (is_failed_) {
      return;
    }

    if (std::holds_alternative<TriggerNode>(*stmt.node)) {
      check_type(std::get<TriggerNode>(*stmt.node));

      if (is_failed_) {
        return;
      }
    }
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::BreakStatementNode& break_stmt)
  requires(!std::is_same_v<TriggerNode, ast::BreakStatementNode>)
{

  if (break_stmt.value.has_value()) {
    operator()(*break_stmt.value.value());

    if (is_failed_) {
      return;
    }
  }

  check_type(break_stmt);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::ContinueStatementNode&) {}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::ReturnStatementNode& return_stmt)
  requires(!std::is_same_v<TriggerNode, ast::ReturnStatementNode>)
{

  if (return_stmt.value.has_value()) {
    operator()(*this, return_stmt.value.value());

    if (is_failed_) {
      return;
    }
  }

  check_type(return_stmt);
}

template <ast::InterruptNode TriggerNode>
template <typename BinaryNode>
  requires(is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>)
void InterruptTypeChecker<TriggerNode>::operator()(const BinaryNode& node) {

  operator()(*node.left);

  if (is_failed_) {
    return;
  }

  for (auto& [op, right] : node.right) {
    operator()(right);

    if (is_failed_) {
      return;
    }
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::UnaryNode& unary) {
  operator()(*unary.primary);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(const ast::LiteralNode&) {}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(const ast::IdentifierNode&) {
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::PrimaryNode& primary) {
  std::visit(*this, *primary.primary);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::StatementNode& statement) {
  std::visit(*this, *statement.node);
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::VariableDefinitionNode& variable_definition) {
  if (variable_definition.value.has_value()) {
    operator()(*variable_definition.value.value());
  }
}

template <ast::InterruptNode TriggerNode>
void InterruptTypeChecker<TriggerNode>::operator()(
    const ast::FunctionDefinitionNode&) {}

template <ast::InterruptNode TriggerNode>
bool InterruptTypeChecker<TriggerNode>::is_failed() const {
  return is_failed_;
}

template <ast::InterruptNode TriggerNode>
std::uint8_t InterruptTypeChecker<TriggerNode>::get_type() const {
  return real_type_;
}
