#include <iostream>
#include <ranges>
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
  if (identifier.type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::IdentifierNode&) with not_type_id");
  }

  const SymbolTable* scope = identifier.table;

  while (scope != nullptr) {
    if (value_tables_.contains(scope) &&
        value_tables_.at(scope).contains(identifier.identifier->name)) {
      auto& value = value_tables_.at(scope).at(identifier.identifier->name);

      // bool is_corrent_type = std::visit(
      //     [&name = identifier.identifier->name](auto&& val, auto&& type) {
      //       using expected_type = std::decay_t<decltype(type)>;
      //       using val_type = std::decay_t<decltype(val)>;
      //
      //       if constexpr (requires {
      //                       typename expected_type::interpret_type;
      //                     }) {
      //         if constexpr (!std::is_same_v<
      //                           typename expected_type::interpret_type,
      //                           val_type>) {
      //           return false;
      //         }
      //         return true;
      //       }
      //
      //       return false;
      //     },
      //     value,
      //     type_store_.get_type(type_store_.resolve(identifier.type_id)).type);
      //
      // if (is_corrent_type) {
      return value;
      // } else {
      //   std::cerr << value.index() << std::endl;
      //   std::cerr << "incorrent type in variable "
      //             << identifier.identifier->name << " at position "
      //             << identifier << std::endl;
      //
      //   std::cerr << "Identifier type id: " << identifier.type_id <<
      //   std::endl; std::cerr << "Resolved type id: "
      //             << type_store_.resolve(identifier.type_id) << std::endl;
      //
      //   std::visit([](auto&& type) { std::cerr << type << std::endl; },
      //              type_store_.get_type(identifier.type_id).type);
      //
      //   std::visit(
      //       [](auto&& type) { std::cerr << type << std::endl; },
      //       type_store_.get_type(type_store_.resolve(identifier.type_id)).type);
      //
      //   throw std::runtime_error("incorrent type");
      // }
    } else {
      scope = scope->get_parent();
    }
  }

  throw std::runtime_error("variable " + identifier.identifier->name +
                           " is not defined");
}

calc_result_t InterpreterVisitor::operator()(const ast::LiteralNode& literal) {
  if (std::holds_alternative<tp::IntLiteral>(
          type_store_.get_type(type_store_.resolve(literal.type_id)).type) ||
      std::holds_alternative<tp::FloatLiteral>(
          type_store_.get_type(type_store_.resolve(literal.type_id)).type)) {
    std::cerr << "incorrent type in literal ";
    std::visit([](auto&& literal) { std::cerr << literal.value << ' '; },
               *literal.literal);
    std::cerr << "in position " << static_cast<tkn::Position>(literal)
              << std::endl;
  }

  // calc_result_t result = std::visit(
  //     [this, &node = literal](auto&& literal) -> calc_result_t {
  //       auto& expected_type =
  //           type_store_.get_type(type_store_.resolve(node.type_id)).type;
  //
  //       return std::visit(
  //           [&literal = literal.value](auto&& type) -> calc_result_t {
  //             using pure_type = std::decay_t<decltype(type)>;
  //             if constexpr (
  //                 requires { typename pure_type::interpret_type; } &&
  //                 requires {
  //                   static_cast<pure_type::interpret_type>(literal);
  //                 }) {
  //               return static_cast<pure_type::interpret_type>(literal);
  //             } else {
  //               std::cout << literal << std::endl;
  //               throw std::runtime_error("Literal type is not supported");
  //             }
  //           },
  //           expected_type);
  //     },
  //     *literal.literal);

  return std::visit(
      [&](auto&& in_literal, auto&& type) -> calc_result_t {
        using pure_type = std::decay_t<decltype(type)>;

        if constexpr (requires { typename pure_type::interpret_type; }) {
          using result_type = typename pure_type::interpret_type;

          if constexpr (requires {
                          result_type{
                              static_cast<result_type>(in_literal.value)};
                        }) {
            return result_type{static_cast<result_type>(in_literal.value)};
          } else {
            throw std::runtime_error("Incorrect literal at " +
                                     std::to_string(literal.start.line) + ":" +
                                     std::to_string(literal.start.offset));
          }
        } else {
          throw std::runtime_error("Incorrect type of literal");
        }
      },
      *literal.literal, type_store_.get_type(literal.type_id).type);
}

calc_result_t
InterpreterVisitor::operator()(const ast::FunctionCallNode& fn_call) {

  static const std::vector<std::string> print_names{
      "print_i8",   "print_i16",  "print_i32", "print_i64", "print_u8",
      "print_u16",  "print_u32",  "print_u64", "print_f32", "print_f64",
      "print_char", "print_bool", "print_void"};
  if (fn_call.name->type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::FunctionCallNode&) with not_type_id");
  }

  if (std::find(print_names.begin(), print_names.end(),
                fn_call.name->identifier->name) != print_names.end()) {
    for (std::size_t i = 0; i < fn_call.arguments.size(); ++i) {
      auto& argument = fn_call.arguments[i];
      auto value = operator()(argument);

      if (check_interrupt()) {
        return Dummy{};
      }

      std::visit(
          [&](auto&& value) {
            using value_type = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<value_type, bool>) {
              if (value) {
                std::cout << "true";
              } else {
                std::cout << "false";
              }
            } else if constexpr (std::is_same_v<value_type, std::int8_t> ||
                                 std::is_same_v<value_type, std::uint8_t>) {
              std::cout << static_cast<std::int16_t>(value);
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

  const Symbol* function_symbol =
      fn_call.table->get_function_symbol_in_position(
          fn_call.name->identifier->name, fn_call);

  const ast::FunctionDefinitionNode* definition = function_symbol->definition;

  if (definition == nullptr) {
    throw std::runtime_error("error in function symbol invariant");
  }

  if (fn_call.arguments.size() != definition->argument_list.size()) {
    throw std::runtime_error("wrong number of arguments");
  }

  if (!fn_call.arguments.empty()) {
    SymbolTable* argument_scope = definition->argument_list.front().first.table;

    auto argument_names = definition->argument_list |
                          std::views::transform([](const auto& argument) {
                            return argument.first.identifier->name;
                          });

    value_tables_.insert_or_assign(
        argument_scope, std::unordered_map<std::string, calc_result_t>{});

    for (auto&& [argument_name, argument_value] :
         std::views::zip(argument_names, fn_call.arguments)) {
      auto value = operator()(argument_value);

      if (check_interrupt()) {
        return Dummy{};
      }

      value_tables_.at(argument_scope).insert_or_assign(argument_name, value);
    }
  }

  auto result_value = operator()(*definition->body);

  if (check_interrupt()) {
    return Dummy{};
  }

  return result_value;
}

calc_result_t
InterpreterVisitor::operator()(const ast::ExpressionNode& exprsession) {
  if (exprsession.type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::ExpressionNode&) with not_type_id");
  }
  return std::visit(*this, *exprsession.node);
}

calc_result_t
InterpreterVisitor::operator()(const ast::BlockExpressionNode& expression) {
  if (expression.type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::BlockExpressionNode&) with not_type_id");
  }

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
  if (expression.type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::IfExpressionNode&) with not_type_id");
  }

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
  if (assignment.type_id == tp::no_type_id) {
    throw std::runtime_error("Call InterpreterVisitor::operator()(const "
                             "ast::AssignmentNode&) with not_type_id");
  }

  auto value = operator()(*assignment.right);
  if (check_interrupt()) {
    return Dummy{};
  }

  const SymbolTable* table = assignment.table;
  while (table != nullptr) {
    if (value_tables_.contains(table) &&
        value_tables_.at(table).contains(assignment.left->identifier->name)) {
      value_tables_.at(table).at(assignment.left->identifier->name) = value;
      break;
    }

    table = table->get_parent();
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
    if (!var_def.value.has_value()) {

      auto& type = type_store_.get_type(var_def.name->type_id);

      std::visit(
          [&](auto&& type) {
            using T = std::decay_t<decltype(type)>;

            if constexpr (!is_in_type_tuple_v<T, tp::StrangeTypeTuple> &&
                          !std::is_same_v<T, tp::NoType>) {
              using interpret_type = typename T::interpret_type;

              value_tables_.at(current_table)
                  .emplace(var_def.name->identifier->name, interpret_type{});
            } else {
              throw std::runtime_error(
                  "Only basic types can have value in runtime");
            }
          },
          type.type);

      return Dummy{};
    }
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

  std::visit(
      [&](auto&& type, auto&& real_value) {
        using real_type = std::decay_t<decltype(type)>;

        if constexpr (requires { static_cast<real_type>(real_value); }) {
          value_tables_.at(current_table)
              .emplace(var_def.name->identifier->name,
                       std::move(static_cast<std::decay_t<decltype(type)>>(
                           real_value)));
        } else {
          throw std::runtime_error("Incorrent types in variable definition");
        }
      },
      it->second, value);

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
