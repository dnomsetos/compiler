#include "parser/ast.hpp"
#include "utility/type_tuple.hpp"
#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <utility/token_utilities.hpp>
#include <variant>
#include <visitors/interpreter_visitor.hpp>

bool InterpreterVisitor::check_interrupt() const {
  return interrupt_index_ != without_interrupt;
}

calc_result_t
InterpreterVisitor::operator()(const ast::IdentifierNode& identifier) {
  if (auto it = variables_.find(identifier.identifier->name);
      it != variables_.end()) {
    return it->second;
  }

  throw std::runtime_error("variable " + identifier.identifier->name +
                           " is not defined");
}

calc_result_t InterpreterVisitor::operator()(const ast::LiteralNode& literal) {
  calc_result_t result = std::visit(
      [](auto&& literal) -> calc_result_t {
        if constexpr (std::is_same_v<std::decay_t<decltype(literal)>,
                                     tkn::IntLiteral>) {
          return static_cast<std::int64_t>(literal.value);
        } else {
          return literal.value;
        }
      },
      *literal.literal);

  return result;
}

calc_result_t
InterpreterVisitor::operator()(const ast::FunctionCallNode& fn_call) {
  if (fn_call.name->identifier->name == "println") {
    for (auto& argument : fn_call.arguments) {
      auto value = operator()(argument);
      if (check_interrupt()) {
        return Dummy{};
      }
      std::visit(
          [](auto&& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, bool>) {
              if (value) {
                std::cout << "true ";
              } else {
                std::cout << "false ";
              }
            } else {
              std::cout << value << ' ';
            }
          },

          value);
    }

    std::cout << std::endl;

    return Dummy{};
  }

  throw std::runtime_error("function calls are not supported");
}

calc_result_t
InterpreterVisitor::operator()(const ast::ExpressionNode& exprsession) {
  return std::visit(*this, *exprsession.node);
}

calc_result_t
InterpreterVisitor::operator()(const ast::BlockExpressionNode& expression) {
  for (auto& stmt : expression.statements) {
    operator()(stmt);
    if (check_interrupt()) {
      return Dummy{};
    }
  }

  if (!expression.value.has_value()) {
    return Dummy{};
  }

  auto result = operator()(*expression.value.value());
  if (check_interrupt()) {
    return Dummy{};
  }
  return result;
}

calc_result_t
InterpreterVisitor::operator()(const ast::IfExpressionNode& expression) {
  auto value = operator()(*expression.condition);
  if (check_interrupt()) {
    return Dummy{};
  }

  if (!std::holds_alternative<bool>(value)) {
    throw std::runtime_error("condition in if expression is not bool");
  }

  if (std::get<bool>(value)) {
    auto value = operator()(*expression.body);
    if (check_interrupt()) {
      return Dummy{};
    }

    return value;
  }

  for (auto& elif : expression.elif_bodies) {
    auto elif_condition = operator()(*elif.expr);
    if (check_interrupt()) {
      return Dummy{};
    }

    if (!std::holds_alternative<bool>(elif_condition)) {
      throw std::runtime_error("codition in else if is not bool");
    }

    if (std::get<bool>(elif_condition)) {
      auto value = operator()(*elif.block);
      if (check_interrupt()) {
        return Dummy{};
      }
      return value;
    }
  }

  if (!expression.else_body.has_value()) {
    return Dummy{};
  }

  auto result = operator()(*expression.else_body.value());
  if (check_interrupt()) {
    return Dummy{};
  }

  return result;
}

calc_result_t
InterpreterVisitor::operator()(const ast::AssignmentNode& assignment) {
  auto it = variables_.find(assignment.left->identifier->name);
  if (it == variables_.end()) {
    throw std::runtime_error("variable " + assignment.left->identifier->name +
                             " is not defined");
  }

  auto value = operator()(*assignment.right);
  if (check_interrupt()) {
    return Dummy{};
  }

  it->second = value;

  return value;
}

calc_result_t
InterpreterVisitor::operator()(const ast::LoopExpressionNode& loop) {
  for (;;) {
    for (auto& stmt : loop.body) {
      operator()(stmt);

      if (check_interrupt()) {
        if (interrupt_index_ == type_tuple_index_v<ast::ReturnStatementNode,
                                                   ast::InterruptNodeTuple>) {
          return Dummy{};
        } else {
          if (!desired_label_.has_value() || desired_label_ == loop.label) {
            std::size_t index = interrupt_index_;

            interrupt_index_ = without_interrupt;
            desired_label_.reset();

            if (index == type_tuple_index_v<ast::BreakStatementNode,
                                            ast::InterruptNodeTuple>) {
              if (desired_value_.has_value()) {
                auto result = std::move(desired_value_.value());
                desired_value_.reset();
                return result;
              } else {
                return Dummy{};
              }
            } else {
              break;
            }
          } else {
            return Dummy{};
          }
        }
      }
    }
  }

  return Dummy{};
}

template <typename InterruptNode>
  requires is_in_type_tuple_v<InterruptNode, ast::InterruptNodeTuple>
calc_result_t InterpreterVisitor::operator()(const InterruptNode& interrupt) {
  if constexpr (requires { interrupt.label; }) {
    if (interrupt.label.has_value()) {
      desired_label_.emplace(interrupt.label.value());
    }
  }

  if constexpr (requires { interrupt.value; }) {
    if (interrupt.value.has_value()) {
      auto value = operator()(*interrupt.value.value());

      if (check_interrupt()) {
        return Dummy{};
      }

      desired_value_.emplace(std::move(value));
    }
  }

  interrupt_index_ = type_tuple_index_v<InterruptNode, ast::InterruptNodeTuple>;
  return Dummy{};
}

template <typename BinaryNode>
  requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
calc_result_t InterpreterVisitor::operator()(const BinaryNode& node) {
  auto value = operator()(*node.left);
  if (check_interrupt()) {
    return Dummy{};
  }

  for (auto& [op, in_node] : node.right) {
    auto result = operator()(in_node);
    if (check_interrupt()) {
      return Dummy{};
    }

    if (value.index() != result.index()) {
      throw std::runtime_error("type mismatch in expression");
    }

    if constexpr (is_variant_v<std::decay_t<decltype(op)>>) {
      value = std::visit(
          [](auto&& op) -> std::function<calc_result_t(const calc_result_t&,
                                                       const calc_result_t&)> {
            return std::decay_t<decltype(op)>::binary_operation;
          },
          op)(value, result);
    } else {
      value = decltype(op)::binary_operation(value, result);
    }
  }

  return value;
}

calc_result_t InterpreterVisitor::operator()(const ast::UnaryNode& unary_node) {
  auto value = operator()(*unary_node.primary);
  if (check_interrupt()) {
    return Dummy{};
  }

  if (!unary_node.op.has_value()) {
    return value;
  }

  return std::visit(
      [](auto&& op) -> std::function<calc_result_t(const calc_result_t&)> {
        return std::decay_t<decltype(op)>::unary_operation;
      },
      *unary_node.op.value())(value);
}

calc_result_t InterpreterVisitor::operator()(const ast::PrimaryNode& primary) {
  return std::visit(*this, *primary.primary);
}

calc_result_t
InterpreterVisitor::operator()(const ast::StatementNode& statement) {
  std::visit(*this, *statement.node);
  return Dummy{};
}

calc_result_t
InterpreterVisitor::operator()(const ast::VariableDefinitionNode& var_def) {
  auto it = variables_.find(var_def.name->identifier->name);
  if (it != variables_.end()) {
    throw std::runtime_error("variable " + var_def.name->identifier->name +
                             " is already defined");
  }

  if (!var_def.type.has_value()) {
    if (!var_def.value.has_value()) {
      throw std::runtime_error("invalid variable definition");
    }

    auto value = operator()(*var_def.value.value());
    if (check_interrupt()) {
      return Dummy{};
    }

    variables_.emplace(var_def.name->identifier->name, std::move(value));

    return Dummy{};
  }

  auto type_it =
      std::find_if(std::begin(default_value_table),
                   std::end(default_value_table), [&var_def](const auto& p) {
                     return p.first == var_def.type.value()->identifier->name;
                   });

  if (type_it == std::end(default_value_table)) {
    throw std::runtime_error("unknown type");
  }

  if (!var_def.value.has_value()) {
    variables_.emplace(var_def.name->identifier->name, type_it->second);
    return Dummy{};
  }

  auto value = operator()(*var_def.value.value());
  if (check_interrupt()) {
    return Dummy{};
  }

  if (value.index() !=
      static_cast<std::size_t>(type_it - std::begin(default_value_table))) {
    throw std::runtime_error("type mismatch in variable definition");
  }

  variables_.emplace(var_def.name->identifier->name, std::move(value));

  return Dummy{};
}

calc_result_t
InterpreterVisitor::operator()(const ast::FunctionDefinitionNode&) {
  return Dummy{};
}

calc_result_t InterpreterVisitor::operator()(const ast::Program& program) {
  const ast::FunctionDefinitionNode* main = nullptr;

  for (auto& def : program.definitions) {
    if (std::holds_alternative<ast::FunctionDefinitionNode>(def) &&
        std::get<ast::FunctionDefinitionNode>(def).name->identifier->name ==
            "main") {
      main = &std::get<ast::FunctionDefinitionNode>(def);
    }

    std::visit(*this, def);
  }

  if (main == nullptr) {
    throw std::runtime_error("no main function");
  }

  if (!main->argument_list.empty()) {
    throw std::runtime_error("main function has arguments");
  }

  for (auto& stmt : main->body->statements) {
    operator()(stmt);
    if (check_interrupt()) {
      return Dummy{};
    }
  }

  if (!main->body->value.has_value()) {
    return Dummy{};
  }

  auto result = operator()(*main->body->value.value());
  if (check_interrupt()) {
    return Dummy{};
  }

  return result;
}
