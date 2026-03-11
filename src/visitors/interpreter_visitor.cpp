#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <utility/token_utilities.hpp>
#include <visitors/interpreter_visitor.hpp>

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
          operator()(argument));
    }

    std::cout << std::endl;

    return Dummy{};
  }

  throw std::runtime_error("function calls are not supported");
}

calc_result_t
InterpreterVisitor::operator()(const ast::ExpressionNode& exprsession) {
  if (std::holds_alternative<ast::AssignmentNode>(*exprsession.node)) {
    return operator()(std::get<ast::AssignmentNode>(*exprsession.node));
  } else if (std::holds_alternative<ast::OrNode>(*exprsession.node)) {
    return operator()(std::get<ast::OrNode>(*exprsession.node));
  }

  throw std::runtime_error("unknown expression");
}

calc_result_t
InterpreterVisitor::operator()(const ast::AssignmentNode& assignment) {
  auto it = variables_.find(assignment.left->identifier->name);
  if (it == variables_.end()) {
    throw std::runtime_error("variable " + assignment.left->identifier->name +
                             " is not defined");
  }

  auto value = operator()(*assignment.right);
  it->second = value;

  return value;
}

template <typename BinaryNode>
calc_result_t InterpreterVisitor::operator()(const BinaryNode& node) {
  auto value = operator()(*node.left);

  for (auto& [op, in_node] : node.right) {
    auto result = operator()(in_node);

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
InterpreterVisitor::operator()(const ast::IfStatementNode& if_statement) {
  auto if_cond = operator()(*if_statement.condition);

  if (!std::holds_alternative<bool>(if_cond)) {
    throw std::runtime_error("expression in if condition must be bool");
  }

  if (std::get<bool>(if_cond)) {
    for (auto& stmt : if_statement.body) {
      operator()(stmt);
    }

    return Dummy{};
  }

  for (auto& elif : if_statement.elif_bodies) {
    auto elif_cond = operator()(elif.expr);

    if (!std::holds_alternative<bool>(elif_cond)) {
      throw std::runtime_error("expression in elif condition must be bool");
    }

    if (std::get<bool>(elif_cond)) {
      for (auto& stmt : elif.statements) {
        operator()(stmt);
      }

      return Dummy{};
    }
  }

  for (auto& stmt : if_statement.else_body) {
    operator()(stmt);
  }
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

    variables_.emplace(
        var_def.name->identifier->name, operator()(*var_def.value.value()));

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

  for (auto& stmt : main->body) {
    operator()(stmt);
  }
  return operator()(*main->return_value);
}
