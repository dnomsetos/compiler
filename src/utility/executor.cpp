#include "parser/ast.hpp"
#include "scanner/token.hpp"
#include "utility/type_tuple.hpp"
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <variant>

#include <utility/executor.hpp>

calc_result
execute_program(const ast::Program& program,
                std::unordered_map<std::string, calc_result>& variables) {
  const ast::FunctionDefinitionNode* main = nullptr;
  for (auto& def : program.definitions) {
    if (std::holds_alternative<ast::VariableDefinitionNode>(def)) {
      auto& var_def = std::get<ast::VariableDefinitionNode>(def);
      if (var_def.value.has_value()) {
        auto value = execute_expression(*var_def.value.value(), variables);
        if (var_def.type.has_value()) {
          auto& type = var_def.type.value()->identifier->name;
          if (type == "int") {
            if (!std::holds_alternative<std::int64_t>(value)) {
              throw std::runtime_error("invalid type");
            }
          } else if (type == "str") {
            if (!std::holds_alternative<std::string>(value)) {
              throw std::runtime_error("invalid type");
            }
          } else if (type == "bool") {
            if (!std::holds_alternative<bool>(value)) {
              throw std::runtime_error("invalid type");
            }
          } else if (type == "float") {
            if (!std::holds_alternative<double>(value)) {
              throw std::runtime_error("invalid type");
            }
          } else {
            throw std::runtime_error("unknown type");
          }
        }
        variables.emplace(var_def.name->identifier->name, value);
      } else if (var_def.type.has_value()) {
        auto type = var_def.type.value()->identifier->name;
        if (type == "int") {
          variables.emplace(var_def.name->identifier->name, int{0});
        } else if (type == "str") {
          variables.emplace(var_def.name->identifier->name, std::string{});
        } else if (type == "bool") {
          variables.emplace(var_def.name->identifier->name, bool{false});
        } else if (type == "float") {
          variables.emplace(var_def.name->identifier->name, double{0.0});
        } else {
          throw std::runtime_error("unknown type");
        }
      } else {
        throw std::runtime_error("invalid variable definition");
      }
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
  if (main->argument_lits.size() != 0) {
    throw std::runtime_error("main function has arguments");
  }
  for (auto& statement : main->body) {
    execute_statement(statement, variables);
  }
  return execute_expression(*main->return_value, variables);
}

void execute_statement(
    const ast::StatementNode& statement,
    std::unordered_map<std::string, calc_result>& variables) {
  if (std::holds_alternative<ast::ExpressionNode>(*statement.node)) {
    execute_expression(std::get<ast::ExpressionNode>(*statement.node),
                       variables);
  } else if (std::holds_alternative<ast::VariableDefinitionNode>(
                 *statement.node)) {
    auto& var_def = std::get<ast::VariableDefinitionNode>(*statement.node);
    if (var_def.value.has_value()) {
      auto value = execute_expression(*var_def.value.value(), variables);
      if (var_def.type.has_value()) {
        auto& type = var_def.type.value()->identifier->name;
        if (type == "int") {
          if (!std::holds_alternative<std::int64_t>(value)) {
            throw std::runtime_error("invalid type");
          }
        } else if (type == "str") {
          if (!std::holds_alternative<std::string>(value)) {
            throw std::runtime_error("invalid type");
          }
        } else if (type == "bool") {
          if (!std::holds_alternative<bool>(value)) {
            throw std::runtime_error("invalid type");
          }
        } else if (type == "float") {
          if (!std::holds_alternative<double>(value)) {
            throw std::runtime_error("invalid type");
          }
        } else {
          throw std::runtime_error("unknown type");
        }
      }
      variables.emplace(var_def.name->identifier->name, value);
    } else if (var_def.type.has_value()) {
      auto type = var_def.type.value()->identifier->name;
      if (type == "int") {
        variables.emplace(var_def.name->identifier->name, int{0});
      } else if (type == "str") {
        variables.emplace(var_def.name->identifier->name, std::string{});
      } else if (type == "bool") {
        variables.emplace(var_def.name->identifier->name, bool{false});
      } else if (type == "float") {
        variables.emplace(var_def.name->identifier->name, double{0.0});
      } else {
        throw std::runtime_error("unknown type");
      }
    } else {
      throw std::runtime_error("invalid variable definition");
    }
  } else if (std::holds_alternative<ast::IfStatementNode>(*statement.node)) {
    auto& if_stmt = std::get<ast::IfStatementNode>(*statement.node);
    auto value = execute_expression(*if_stmt.condition, variables);
    if (!std::holds_alternative<bool>(value)) {
      throw std::runtime_error(
          "type of condition in if statement must be bool");
    }
    if (std::get<bool>(value)) {
      for (auto& statement : if_stmt.body) {
        execute_statement(statement, variables);
      }
      return;
    }
    for (auto& ex_stmts : if_stmt.elif_bodies) {
      value = execute_expression(ex_stmts.expr, variables);
      if (!std::holds_alternative<bool>(value)) {
        throw std::runtime_error(
            "type of condition in elif statement must be bool");
      }
      if (std::get<bool>(value)) {
        for (auto& statement : ex_stmts.statements) {
          execute_statement(statement, variables);
        }
        return;
      }
    }
    for (auto& statement : if_stmt.else_body) {
      execute_statement(statement, variables);
    }
  }
}

calc_result
execute_expression(const ast::ExpressionNode& expression,
                   std::unordered_map<std::string, calc_result>& variables) {
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
  return true;
}

#define EXECUTE_BINARY_OP(name, prev_name, node, prev_node, oper)              \
  calc_result name(const node& expression,                                     \
                   std::unordered_map<std::string, calc_result>& variables) {  \
    auto left = prev_name(*expression.left, variables);                        \
    for (auto& [op, in_node] : expression.right) {                             \
      auto right = prev_name(in_node, variables);                              \
      if (left.index() != right.index()) {                                     \
        throw std::runtime_error("type mismatch in logical expression");       \
      }                                                                        \
      left = std::visit(                                                       \
          [](auto&& left, auto&& right) -> calc_result {                       \
            if constexpr (requires { left oper right; }) {                     \
              return left oper right;                                          \
            }                                                                  \
            throw std::runtime_error("type mismatch in logical expression");   \
          },                                                                   \
          left, right);                                                        \
    }                                                                          \
    return left;                                                               \
  }

EXECUTE_BINARY_OP(execute_or, execute_xor, ast::OrNode, ast::XorNode, |)
EXECUTE_BINARY_OP(execute_xor, execute_and, ast::XorNode, ast::AndNode, ^)
EXECUTE_BINARY_OP(execute_and, execute_equality, ast::AndNode,
                  ast::ComparisonNode, &)

calc_result
execute_equality(const ast::EqualityNode& expression,
                 std::unordered_map<std::string, calc_result>& variables) {
  auto left = execute_comparison(*expression.left, variables);
  for (auto& [op, in_node] : expression.right) {
    auto right = execute_comparison(in_node, variables);
    if (left.index() != right.index()) {
      throw std::runtime_error("type mismatch in equality expression");
    }
    left = std::visit(
        [op](auto&& l, auto&& r) -> calc_result {
          if constexpr (requires { l == r; }) {
            if (std::holds_alternative<tkn::Equal>(op)) {
              return l == r;
            }
          }
          if constexpr (requires { l != r; }) {
            if (std::holds_alternative<tkn::NotEqual>(op)) {
              return l != r;
            }
          }
          throw std::runtime_error("operator not supported");
        },
        left, right);
  }
  return left;
}

calc_result
execute_comparison(const ast::ComparisonNode& expression,
                   std::unordered_map<std::string, calc_result>& variables) {
  auto left = execute_addition(*expression.left, variables);
  for (auto& [op, in_node] : expression.right) {
    auto right = execute_addition(in_node, variables);
    if (left.index() != right.index()) {
      throw std::runtime_error("type mismatch in comparison expression");
    }
    left = std::visit(
        [op](auto&& l, auto&& r) -> calc_result {
          if constexpr (requires { l < r; }) {
            if (std::holds_alternative<tkn::Less>(op)) {
              return l < r;
            }
          }
          if constexpr (requires { l > r; }) {
            if (std::holds_alternative<tkn::Greater>(op)) {
              return l > r;
            }
          }
          if constexpr (requires { l <= r; }) {
            if (std::holds_alternative<tkn::LessEqual>(op)) {
              return l <= r;
            }
          }
          if constexpr (requires { l >= r; }) {
            if (std::holds_alternative<tkn::GreaterEqual>(op)) {
              return l >= r;
            }
          }
          throw std::runtime_error("operator not supported");
        },
        left, right);
  }
  return left;
}

calc_result
execute_addition(const ast::AdditionNode& expression,
                 std::unordered_map<std::string, calc_result>& variables) {
  auto left = execute_multiplication(*expression.left, variables);
  for (auto& [op, in_node] : expression.right) {
    auto right = execute_multiplication(in_node, variables);
    if (left.index() != right.index()) {
      throw std::runtime_error("type mismatch in addition expression");
    }
    left = std::visit(
        [op](auto&& l, auto&& r) -> calc_result {
          if constexpr (requires { l + r; }) {
            if (std::holds_alternative<tkn::Plus>(op)) {
              return l + r;
            }
          }
          if constexpr (requires { l - r; }) {
            if (std::holds_alternative<tkn::Minus>(op)) {
              return l - r;
            }
          }
          throw std::runtime_error("operator not supported");
        },
        left, right);
  }
  return left;
}

calc_result execute_multiplication(
    const ast::MultiplicationNode& expression,
    std::unordered_map<std::string, calc_result>& variables) {
  auto left = execute_unary(*expression.left, variables);
  for (auto& [op, in_node] : expression.right) {
    auto right = execute_unary(in_node, variables);
    if (left.index() != right.index()) {
      throw std::runtime_error("type mismatch in multiplication expression");
    }
    left = std::visit(
        [op](auto&& l, auto&& r) -> calc_result {
          if constexpr (requires { l * r; }) {
            if (std::holds_alternative<tkn::Multiply>(op)) {
              return l * r;
            }
          }
          if constexpr (requires { l / r; }) {
            if (std::holds_alternative<tkn::Divide>(op)) {
              return l / r;
            }
          }
          if constexpr (requires { l % r; }) {
            if (std::holds_alternative<tkn::Mod>(op)) {
              return l % r;
            }
          }
          throw std::runtime_error("operator not supported");
        },
        left, right);
  }
  return left;
}

calc_result
execute_unary(const ast::UnaryNode& expression,
              std::unordered_map<std::string, calc_result>& variables) {
  auto result = execute_primary(*expression.primary, variables);
  if (!expression.op.has_value()) {
    return result;
  }
  if (std::holds_alternative<tkn::Plus>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result {
          if constexpr (requires { +result; }) {
            return +result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }
  if (std::holds_alternative<tkn::Minus>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result {
          if constexpr (requires { -result; }) {
            return -result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }
  if (std::holds_alternative<tkn::Not>(*expression.op.value())) {
    return std::visit(
        [](auto&& result) -> calc_result {
          if constexpr (requires { !result; }) {
            return !result;
          }
          throw std::runtime_error("operator not supported");
        },
        result);
  }
  return result;
}

calc_result
execute_primary(const ast::PrimaryNode& expression,
                std::unordered_map<std::string, calc_result>& variables) {
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
    if (std::holds_alternative<tkn::IntLiteral>(literal)) {
      return static_cast<std::int64_t>(
          std::get<tkn::IntLiteral>(literal).value);
    }
    if (std::holds_alternative<tkn::BoolLiteral>(literal)) {
      return std::get<tkn::BoolLiteral>(literal).value;
    }
    if (std::holds_alternative<tkn::FloatLiteral>(literal)) {
      return std::get<tkn::FloatLiteral>(literal).value;
    }
    if (std::holds_alternative<tkn::StringLiteral>(literal)) {
      return std::get<tkn::StringLiteral>(literal).value;
    }
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
                std::cout << value;
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
