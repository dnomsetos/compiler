#include <iostream>
#include <stdexcept>
#include <type_traits>

#include <testing_utilities/interpreter_visitor.hpp>
#include <utility/output.hpp>
#include <utility/token_utilities.hpp>

InterpreterVisitor::InterpreterVisitor(TypeStore& type_store)
    : type_store_(type_store) {}

bool InterpreterVisitor::check_interrupt() const {
  return interrupt_index_ != without_interrupt;
}

calc_result_t
InterpreterVisitor::operator()(const ast::IdentifierNode& identifier) {
  const SymbolTable* scope = identifier.table;

  while (scope != nullptr) {
    try {
      std::cout << scope << std::endl;
      return value_tables_.at(scope).at(identifier.identifier->name);
      for (auto [key, value] : value_tables_.at(scope)) {
        std::cout << key << ": " << value.index() << std::endl;
      }
    } catch (const std::out_of_range&) {
      scope = scope->get_parent();
    }
  }

  throw std::runtime_error("variable " + identifier.identifier->name +
                           " is not defined");
}

calc_result_t InterpreterVisitor::operator()(const ast::LiteralNode& literal) {
  calc_result_t result =
      std::visit([](auto&& literal) -> calc_result_t { return literal.value; },
                 *literal.literal);

  return result;
}

calc_result_t
InterpreterVisitor::operator()(const ast::FunctionCallNode& fn_call) {

  if (fn_call.name->identifier->name == "println") {
    for (std::size_t i = 0; i < fn_call.arguments.size(); ++i) {
      auto& argument = fn_call.arguments[i];
      auto value = operator()(argument);

      if (check_interrupt()) {
        return Dummy{};
      }

      std::visit(
          [](auto&& value) {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, bool>) {
              if (value) {
                std::cout << "true";
              } else {
                std::cout << "false";
              }
            } else {
              std::cout << value;
            }
          },

          value);

      if (i != fn_call.arguments.size() - 1) {
        std::cout << ' ';
      }
    }

    std::cout << std::endl;

    return Dummy{};
  }

  const SymbolTable* table = fn_call.table;

  if (table == nullptr) {
    throw std::runtime_error("function " + fn_call.name->identifier->name +
                             " is not defined");
  }

  const Symbol* symbol = table->get_function_symbol_in_position(
      fn_call.name->identifier->name, static_cast<tkn::Position>(fn_call));

  ast::FunctionDefinitionNode* definition = symbol->definition;

  if (definition == nullptr) {
    throw std::runtime_error("function " + fn_call.name->identifier->name +
                             " is not defined");
  }

  return operator()(*definition->body);
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
  auto value = operator()(*assignment.right);
  if (check_interrupt()) {
    return Dummy{};
  }

  const SymbolTable* table = assignment.table;
  while (table != nullptr) {
    try {
      value_tables_.at(table).at(assignment.left->identifier->name) = value;
      break;
    } catch (const std::out_of_range&) {
      table = table->get_parent();
    }
  }

  if (table == nullptr) {
    throw std::runtime_error("variable " + assignment.left->identifier->name +
                             " is not defined");
  }

  return Dummy{};
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

    if constexpr (is_variant_v<std::decay_t<decltype(op)>>) {
      value = std::visit(
          [](auto&& op) -> std::function<calc_result_t(
                            const calc_result_t&, const calc_result_t&)> {
            return std::decay_t<decltype(op)>::binary_operation;
          },
          op)(value, result);
    } else {
      value = decltype(op)::binary_operation(value, result);
    }
  }

  return value;
}

calc_result_t InterpreterVisitor::operator()(const ast::CastNode& cast_node) {
  auto value = operator()(*cast_node.expression);

  if (check_interrupt()) {
    return Dummy{};
  }

  if (!cast_node.type.has_value()) {
    return value;
  }

  auto type_it = std::find_if(
      std::begin(tp::basic_type_names), std::end(tp::basic_type_names),
      [&name = cast_node.type.value()->identifier->name](auto&& val) {
        return val.first == name;
      });

  return std::visit(
      [](auto&& val, auto&& type) -> calc_result_t {
        if constexpr (requires {
                        static_cast<typename std::decay_t<
                            decltype(type)>::interpret_type>(val);
                      }) {
          return static_cast<
              typename std::decay_t<decltype(type)>::interpret_type>(val);
        } else {
          throw std::runtime_error("error cast");
        }
      },
      value, type_it->second);
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
  SymbolTable* current_table = var_def.table;

  if (value_tables_.find(current_table) == value_tables_.end()) {
    value_tables_.emplace(
        current_table, std::unordered_map<std::string, calc_result_t>{});
  }

  if (!var_def.type.has_value()) {
    auto value = operator()(*var_def.value.value());
    if (check_interrupt()) {
      return Dummy{};
    }

    value_tables_.at(current_table)
        .emplace(var_def.name->identifier->name, value);

    return Dummy{};
  }

  auto it = std::find_if(
      std::begin(tp::default_value_table), std::end(tp::default_value_table),
      [&var_def, this](auto&& val) {
        return type_store_.get_type(var_def.name->type_id).type.index() ==
               val.first.index();
      });

  if (!var_def.value.has_value()) {
    value_tables_.at(current_table)
        .emplace(var_def.name->identifier->name, it->second);
    return Dummy{};
  }

  auto value = operator()(*var_def.value.value());
  if (check_interrupt()) {
    return Dummy{};
  }

  value_tables_.at(current_table)
      .emplace(var_def.name->identifier->name, std::move(value));

  return Dummy{};
}

calc_result_t
InterpreterVisitor::operator()(const ast::FunctionDefinitionNode&) {
  return Dummy{};
}

calc_result_t InterpreterVisitor::operator()(const ast::Program& program,
                                             const std::string& function_name) {
  const ast::FunctionDefinitionNode* main = nullptr;

  for (auto& def : program.definitions) {
    if (std::holds_alternative<ast::FunctionDefinitionNode>(def) &&
        std::get<ast::FunctionDefinitionNode>(def).name->identifier->name ==
            function_name) {
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
