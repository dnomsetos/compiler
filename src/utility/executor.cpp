#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <variant>

#include <parser/ast.hpp>
#include <scanner/token.hpp>
#include <utility/executor.hpp>

std::ostream& operator<<(std::ostream& out, const Dummy&) {
  out << "void";
  return out;
}

void execute_variable_definition(
    const ast::VariableDefinitionNode& var_def,
    std::unordered_map<std::string, calc_result_t>& variables) {

  if (var_def.value.has_value()) {
    auto value = execute_expression(*var_def.value.value(), variables);
    if (var_def.type.has_value()) {
      auto& type = var_def.type.value()->identifier->name;

      auto it = std::find_if(
          std::begin(default_value_table), std::end(default_value_table),
          [&type](const auto& p) { return p.first == type; });

      if (it == std::end(default_value_table)) {
        throw std::runtime_error("unknown type");
      }

      if (static_cast<std::size_t>(it - std::begin(default_value_table)) !=
          value.index()) {
        throw std::runtime_error("invalid_type");
      }
    }
    variables.emplace(var_def.name->identifier->name, value);
  } else if (var_def.type.has_value()) {
    auto type = var_def.type.value()->identifier->name;

    auto it = std::find_if(std::begin(default_value_table),
                           std::end(default_value_table),
                           [&type](const auto& p) { return p.first == type; });

    if (it == std::end(default_value_table)) {
      throw std::runtime_error("unknown type");
    }

    variables.emplace(var_def.name->identifier->name, it->second);
  } else {
    throw std::runtime_error("invalid variable definition");
  }
}

calc_result_t
execute_program(const ast::Program& program,
                std::unordered_map<std::string, calc_result_t>& variables) {

  const ast::FunctionDefinitionNode* main = nullptr;

  for (auto& def : program.definitions) {
    if (std::holds_alternative<ast::VariableDefinitionNode>(def)) {
      auto& var_def = std::get<ast::VariableDefinitionNode>(def);

      execute_variable_definition(var_def, variables);

      continue;
    }

    if (std::get<ast::FunctionDefinitionNode>(def).name->identifier->name ==
        "main") {
      main = &std::get<ast::FunctionDefinitionNode>(def);
    }
  }

  if (main == nullptr) {
    throw std::runtime_error("no main function");
  }

  if (main->argument_list.size() != 0) {
    throw std::runtime_error("main function has arguments");
  }

  for (auto& statement : main->body->statements) {
    execute_statement(statement, variables);
  }

  if (!main->body->value.has_value()) {
    return Dummy{};
  }

  return execute_expression(*main->body->value.value(), variables);
}

void execute_statement(
    const ast::StatementNode& statement,
    std::unordered_map<std::string, calc_result_t>& variables) {

  if (std::holds_alternative<ast::ExpressionNode>(*statement.node)) {
    execute_expression(std::get<ast::ExpressionNode>(*statement.node),
                       variables);
  } else if (std::holds_alternative<ast::VariableDefinitionNode>(
                 *statement.node)) {
    auto& var_def = std::get<ast::VariableDefinitionNode>(*statement.node);

    execute_variable_definition(var_def, variables);
  }
}

calc_result_t execute_block_expression(
    const ast::BlockExpressionNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables) {

  for (auto& stmt : expression.statements) {
    execute_statement(stmt, variables);
  }

  if (!expression.value.has_value()) {
    return Dummy{};
  }

  return execute_expression(*expression.value.value(), variables);
}

calc_result_t execute_if_expression(
    const ast::IfExpressionNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables) {

  auto cond = execute_expression(*expression.condition, variables);

  if (!std::holds_alternative<bool>(cond)) {
    throw std::runtime_error("condition in if expression is not bool");
  }

  if (std::get<bool>(cond)) {
    return execute_block_expression(*expression.body, variables);
  }

  for (auto& elif : expression.elif_bodies) {
    auto elif_condition = execute_expression(*elif.expr, variables);

    if (!std::holds_alternative<bool>(elif_condition)) {
      throw std::runtime_error("codition in else if is not bool");
    }

    if (std::get<bool>(elif_condition)) {
      return execute_block_expression(*elif.block, variables);
    }
  }

  if (expression.else_body.has_value()) {
    return execute_block_expression(*expression.else_body.value(), variables);
  }

  return Dummy{};
}

calc_result_t
execute_expression(const ast::ExpressionNode& expression,
                   std::unordered_map<std::string, calc_result_t>& variables) {

  if (std::holds_alternative<ast::AssignmentNode>(*expression.node)) {
    auto& assignment = std::get<ast::AssignmentNode>(*expression.node);

    auto value = execute_expression(*assignment.right, variables);

    if (variables.find(assignment.left->identifier->name) == variables.end()) {
      throw std::runtime_error("variable " + assignment.left->identifier->name +
                               " is not defined");
    }

    if (variables[assignment.left->identifier->name].index() != value.index()) {
      std::cout << variables[assignment.left->identifier->name].index() << ' '
                << value.index() << std::endl;
      throw std::runtime_error("type mismatch in expression");
    }

    variables[assignment.left->identifier->name] = value;

    return value;
  }

  if (std::holds_alternative<ast::OrNode>(*expression.node)) {
    return execute_or(std::get<ast::OrNode>(*expression.node), variables);
  }

  if (std::holds_alternative<ast::BlockExpressionNode>(*expression.node)) {
    return execute_block_expression(
        std::get<ast::BlockExpressionNode>(*expression.node), variables);
  }

  if (std::holds_alternative<ast::IfExpressionNode>(*expression.node)) {
    return execute_if_expression(
        std::get<ast::IfExpressionNode>(*expression.node), variables);
  }

  throw std::runtime_error("unknown expression");
}

template <typename NodeT, typename PrevExecFn, typename OpHandler>
calc_result_t execute_binary_op_impl(
    const NodeT& expression,
    std::unordered_map<std::string, calc_result_t>& variables,
    PrevExecFn prev_exec, OpHandler op_handler,
    const char* type_name_for_error) {

  auto left = prev_exec(*expression.left, variables);

  for (auto& [op, in_node] : expression.right) {
    auto right = prev_exec(in_node, variables);

    if (left.index() != right.index()) {
      throw std::runtime_error(std::string("type mismatch in ") +
                               type_name_for_error);
    }

    left = std::visit(
        [&op_handler, &op]<typename T1, typename T2>(T1&& l,
                                                     T2&& r) -> calc_result_t {
          auto res = op_handler(op, std::forward<T1>(l), std::forward<T2>(r));

          if (res.has_value()) {
            return res.value();
          }
          throw std::runtime_error("operator not supported");
        },
        left, right);
  }

  return left;
}

calc_result_t
execute_or(const ast::OrNode& expression,
           std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_xor,
      [](const auto& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if (op == tkn::Or{}) {
          if constexpr (requires { l | r; }) {
            return l | r;
          }
        }
        return std::nullopt;
      },
      "logical expression");
}

calc_result_t
execute_xor(const ast::XorNode& expression,
            std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_and,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if (op == tkn::Xor{}) {
          if constexpr (requires { l ^ r; }) {
            return l ^ r;
          }
        }

        return std::nullopt;
      },
      "logical expression");
}

calc_result_t
execute_and(const ast::AndNode& expression,
            std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_equality,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if (op == tkn::And{}) {
          if constexpr (requires { l & r; }) {
            return l & r;
          }
        }
        return std::nullopt;
      },
      "logical expression");
}

calc_result_t
execute_equality(const ast::EqualityNode& expression,
                 std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_comparison,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if constexpr (requires { l == r; }) {
          if (std::holds_alternative<tkn::Equal>(op))
            return l == r;
        }

        if constexpr (requires { l != r; }) {
          if (std::holds_alternative<tkn::NotEqual>(op))
            return l != r;
        }
        return std::nullopt;
      },
      "equality expression");
}

calc_result_t
execute_comparison(const ast::ComparisonNode& expression,
                   std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_addition,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if constexpr (requires { l < r; }) {
          if (std::holds_alternative<tkn::Less>(op))
            return l < r;
        }

        if constexpr (requires { l > r; }) {
          if (std::holds_alternative<tkn::Greater>(op))
            return l > r;
        }

        if constexpr (requires { l <= r; }) {
          if (std::holds_alternative<tkn::LessEqual>(op))
            return l <= r;
        }

        if constexpr (requires { l >= r; }) {
          if (std::holds_alternative<tkn::GreaterEqual>(op))
            return l >= r;
        }
        return std::nullopt;
      },
      "comparison expression");
}

calc_result_t
execute_addition(const ast::AdditionNode& expression,
                 std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_multiplication,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if constexpr (requires { l + r; }) {
          if (std::holds_alternative<tkn::Plus>(op))
            return l + r;
        }

        if constexpr (requires { l - r; }) {
          if (std::holds_alternative<tkn::Minus>(op))
            return l - r;
        }
        return std::nullopt;
      },
      "addition expression");
}

calc_result_t execute_multiplication(
    const ast::MultiplicationNode& expression,
    std::unordered_map<std::string, calc_result_t>& variables) {

  return execute_binary_op_impl(
      expression, variables, execute_unary,
      [](auto const& op, auto&& l, auto&& r) -> std::optional<calc_result_t> {
        if constexpr (requires { l * r; }) {
          if (std::holds_alternative<tkn::Multiply>(op))
            return l * r;
        }

        if constexpr (requires { l / r; }) {
          if (std::holds_alternative<tkn::Divide>(op))
            return l / r;
        }

        if constexpr (requires { l % r; }) {
          if (std::holds_alternative<tkn::Mod>(op))
            return l % r;
        }
        return std::nullopt;
      },
      "multiplication expression");
}

calc_result_t
execute_unary(const ast::UnaryNode& expression,
              std::unordered_map<std::string, calc_result_t>& variables) {

  auto result = execute_primary(*expression.primary, variables);

  if (!expression.op.has_value()) {
    return result;
  }

  if (std::holds_alternative<tkn::Plus>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result_t {
          if constexpr (requires { +result; }) {
            return +result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }

  if (std::holds_alternative<tkn::Minus>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result_t {
          if constexpr (requires { -result; }) {
            return -result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }

  if (std::holds_alternative<tkn::Not>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result_t {
          if constexpr (requires { !result; }) {
            return !result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }

  return result;
}

calc_result_t
execute_primary(const ast::PrimaryNode& expression,
                std::unordered_map<std::string, calc_result_t>& variables) {

  if (std::holds_alternative<ast::IdentifierNode>(*expression.primary)) {
    auto& id =
        std::get<ast::IdentifierNode>(*expression.primary).identifier->name;

    if (variables.find(id) == variables.end()) {
      throw std::runtime_error("variable " + id + " is not defined");
    }

    return variables[id];
  }

  if (std::holds_alternative<ast::LiteralNode>(*expression.primary)) {
    auto& literal = *std::get<ast::LiteralNode>(*expression.primary).literal;

    return std::visit(
        [](auto&& literal) -> calc_result_t {
          if constexpr (std::is_same_v<std::decay_t<decltype(literal.value)>,
                                       std::uint64_t>) {
            return static_cast<std::int64_t>(literal.value);
          } else {
            return literal.value;
          }
        },
        literal);
  }

  if (std::holds_alternative<ast::ExpressionNode>(*expression.primary)) {
    return execute_expression(
        std::get<ast::ExpressionNode>(*expression.primary), variables);
  }

  if (std::holds_alternative<ast::FunctionCallNode>(*expression.primary)) {
    auto& name = std::get<ast::FunctionCallNode>(*expression.primary);

    if (name.name->identifier->name == "println") {
      for (auto& x : name.arguments) {
        auto result = execute_expression(x, variables);

        std::visit(
            [](auto&& value) {
              if constexpr (std::is_same_v<std::decay_t<decltype(value)>,
                                           bool>) {
                if (value) {
                  std::cout << "true ";
                } else {
                  std::cout << "false ";
                }
              } else if constexpr (requires { std::cout << value; }) {
                std::cout << value << ' ';
              }
            },
            result);
      }

      std::cout << std::endl;

      return Dummy{};
    } else {
      throw std::runtime_error(
          "calls to arbitrary functions are not supported");
    }
  }

  throw std::runtime_error("expression not supported");
}
