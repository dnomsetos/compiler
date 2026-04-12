#include "parser/ast.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include <tables/symbol_table.hpp>
#include <visitors/interrupt_type_checker.hpp>

TypeChecker::TypeChecker(const SymbolTable* symbol_table)
    : context_{symbol_table} {}

std::uint8_t TypeChecker::operator()(const ast::ExpressionNode& expr) const {
  return std::visit(*this, *expr.node);
}

std::uint8_t
TypeChecker::operator()(const ast::BlockExpressionNode& expr) const {
  if (!expr.value.has_value()) {
    return static_cast<std::uint8_t>(
        type_tuple_index_v<tp::Void, tp::BasicTypeTypeTuple>);
  }
  return std::visit(*this, *expr.value.value()->node);
}

std::uint8_t TypeChecker::operator()(const ast::IfExpressionNode& expr) const {
  std::int8_t type = operator()(*expr.body);
  for (auto& elif : expr.elif_bodies) {
    if (operator()(*elif.block) != type) {
      throw std::runtime_error("Type mistmatch in if expression");
    }
  }
  if (expr.else_body.has_value()) {
    if (operator()(*expr.else_body.value()) != type) {
      throw std::runtime_error("Type mismatch in if expression");
    }
  }
  return type;
}

std::uint8_t
TypeChecker::operator()(const ast::LoopExpressionNode& expr) const {
  std::uint8_t type = tp::no_type;

  InterruptTypeChecker<ast::BreakStatementNode> interrupt_type_checker(
      *this, type);

  for (auto& stmt : expr.body) {
    operator()(stmt);
    if (interrupt_type_checker.is_failed()) {
      throw std::runtime_error("Type mismatch in loop expression");
    }
  }

  return interrupt_type_checker.get_type();
}

std::uint8_t TypeChecker::operator()(const ast::AssignmentNode& expr) const {
  return operator()(*expr.right);
}

template <typename T>
  requires is_in_type_tuple_v<T, ast::LogicalBinaryNodeTuple>
std::uint8_t TypeChecker::operator()(const T& expr) const {
  std::uint8_t type = operator()(*expr.left);

  for (auto& right : expr.right) {
    auto new_type = operator()(right.second);
    if (type != new_type) {
      throw std::runtime_error("Type mismatch in logical expression");
    }
  }
  auto int_it = std::find(
      std::begin(tp::IntegerIndexes), std::end(tp::IntegerIndexes), type);
  auto bool_it = std::find(
      std::begin(tp::BooleanIndexes), std::end(tp::BooleanIndexes), type);

  if (int_it == std::end(tp::IntegerIndexes) &&
      bool_it == std::end(tp::BooleanIndexes)) {
    throw std::runtime_error("Type mismatch in logical expression");
  }

  return type;
}

std::uint8_t TypeChecker::operator()(const ast::EqualityNode& expr) const {
  std::uint8_t type = operator()(*expr.left);

  for (auto& [op, right] : expr.right) {
    std::uint8_t right_type = operator()(right);

    if (type != right_type) {
      throw std::runtime_error("Type mismatch in equality expression");
    }
  }

  return type;
}

std::uint8_t TypeChecker::operator()(const ast::ComparisonNode& expr) const {
  std::uint8_t type = operator()(*expr.left);

  auto int_it = std::find(
      std::begin(tp::IntegerIndexes), std::end(tp::IntegerIndexes), type);
  auto float_it = std::find(std::begin(tp::FloatingPointIndexes),
                            std::end(tp::FloatingPointIndexes), type);
  auto char_it = std::find(
      std::begin(tp::CharacterIndexes), std::end(tp::CharacterIndexes), type);

  if (int_it == std::end(tp::IntegerIndexes) &&
      float_it == std::end(tp::FloatingPointIndexes) &&
      char_it == std::end(tp::CharacterIndexes)) {
    throw std::runtime_error("Type mismatch in comparison expression");
  }

  for (auto& [op, right] : expr.right) {
    std::uint8_t right_type = operator()(right);

    if (type != right_type) {
      throw std::runtime_error("Type mismatch in comparison expression");
    }
  }

  return type;
}

template <typename T>
  requires is_in_type_tuple_v<T, ast::ArithmeticBinaryNodeTuple>
std::uint8_t TypeChecker::operator()(const T& expr) const {
  std::uint8_t type = operator()(*expr.left);

  for (auto& right : expr.right) {
    auto new_type = operator()(right.second);
    if (type != new_type) {
      throw std::runtime_error("Type mismatch in arithmetic expression");
    }
  }

  auto int_it = std::find(
      std::begin(tp::IntegerIndexes), std::end(tp::IntegerIndexes), type);
  auto float_it = std::find(std::begin(tp::FloatingPointIndexes),
                            std::end(tp::FloatingPointIndexes), type);

  if (int_it == std::end(tp::IntegerIndexes) &&
      float_it == std::end(tp::FloatingPointIndexes)) {
    throw std::runtime_error("Type mismatch in arithmetic expression");
  }

  return type;
}

std::uint8_t TypeChecker::operator()(const ast::UnaryNode& expr) const {
  return operator()(*expr.primary);
}

std::uint8_t TypeChecker::operator()(const ast::FunctionCallNode& expr) const {
  auto* current_table = context_;
  while (current_table != nullptr) {
    try {
      auto& info = current_table->get_function(expr.name->identifier->name);
      return info.reutrn_type;
    } catch (const std::out_of_range&) {
      current_table = current_table->get_parent();
      continue;
    }
  }
  throw std::runtime_error("undefined function " + expr.name->identifier->name);
}

std::uint8_t TypeChecker::operator()(const ast::PrimaryNode& expr) const {
  return std::visit(*this, *expr.primary);
}

std::uint8_t TypeChecker::operator()(const ast::IdentifierNode& expr) const {
  auto* current_table = context_;
  while (current_table != nullptr) {
    try {
      auto& info = current_table->get_variable(expr.identifier->name);
      return info.basic_type;
    } catch (const std::out_of_range&) {
      current_table = current_table->get_parent();
      continue;
    }
  }
  throw std::runtime_error("undefined variable " + expr.identifier->name);
}

SymbolTable::SymbolTable(const ast::BlockExpressionNode& block,
                         const SymbolTable* parent)
    : parent_{parent} {
  TypeChecker checker(this);

  for (auto& x : block.statements) {
    if (std::holds_alternative<ast::VariableDefinitionNode>(*x.node)) {
      auto& var_def = std::get<ast::VariableDefinitionNode>(*x.node);

      auto optional_type = get_variable_type(checker, var_def);

      if (!optional_type.has_value()) {
        throw std::runtime_error("invalid type at position" +
                                 std::to_string(var_def.start.line) + ":" +
                                 std::to_string(var_def.start.offset));
      }

      variables_.emplace(var_def.name->identifier->name,
                         tp::VariableInfo{static_cast<tkn::Position>(var_def),
                                          optional_type.value()});
    }
  }
}

auto SymbolTable::get_variable(const std::string& name) const
    -> const tp::VariableInfo& {
  return variables_.at(name);
}

auto SymbolTable::get_function(const std::string& name) const
    -> const tp::FunctionInfo& {
  return functions_.at(name);
}

auto SymbolTable::get_variable_in_position(const std::string& name,
                                           const tkn::Position& position) const
    -> const tp::VariableInfo& {
  auto& result = get_variable(name);
  if (position.start.line > result.definition_position.start.line ||
      (position.start.line == result.definition_position.start.line &&
       position.start.offset > result.definition_position.start.offset)) {
    throw std::runtime_error(
        "variable " + name + " is not defined in line " +
        std::to_string(result.definition_position.start.line));
  }
  return result;
}

auto SymbolTable::get_function_in_position(const std::string& name,
                                           const tkn::Position& position) const
    -> const tp::FunctionInfo& {
  auto& result = get_function(name);
  if (position.start.line > result.definition_position.start.line ||
      (position.start.line == result.definition_position.start.line &&
       position.start.offset > result.definition_position.start.offset)) {
    throw std::runtime_error(
        "function" + name + " is not defined in line " +
        std::to_string(result.definition_position.start.line));
  }
  return result;
}

const SymbolTable* SymbolTable::get_parent() const { return parent_; }
#include <iostream>
GlobalSymbolTable::GlobalSymbolTable(const ast::Program& program) {
  TypeChecker checker(&root_);
  for (auto& def : program.definitions) {
    if (std::holds_alternative<ast::VariableDefinitionNode>(def)) {
      const auto& var_def = std::get<ast::VariableDefinitionNode>(def);

      auto type = get_variable_type(checker, var_def);

      // std::cout << "here" << type.value() << std::endl;

      if (!type.has_value()) {
        throw std::runtime_error("invalid type at position" +
                                 std::to_string(var_def.start.line) + ":" +
                                 std::to_string(var_def.start.offset) +
                                 "\nname: " + var_def.name->identifier->name);
      }

      // std::cout << "here2" << std::endl;

      root_.variables_.emplace(
          var_def.name->identifier->name,
          tp::VariableInfo{static_cast<tkn::Position>(var_def), type.value()});
    } else if (std::holds_alternative<ast::FunctionDefinitionNode>(def)) {
      const auto& func_def = std::get<ast::FunctionDefinitionNode>(def);

      auto optional_type = get_function_type(checker, func_def);

      if (!optional_type.has_value()) {
        throw std::runtime_error("invalid type at position" +
                                 std::to_string(func_def.start.line) + ":" +
                                 std::to_string(func_def.start.offset) +
                                 "\nname: " + func_def.name->identifier->name);
      }

      auto [return_type, arguments_types] = optional_type.value();

      InterruptTypeChecker<ast::ReturnStatementNode> return_type_checker(
          checker, return_type);

      return_type_checker(*func_def.body);

      root_.functions_.emplace(
          func_def.name->identifier->name,
          tp::FunctionInfo{static_cast<tkn::Position>(func_def), return_type,
                           std::move(arguments_types)});
    }
  }
}

const SymbolTable* GlobalSymbolTable::get_root() const { return &root_; }

#include <iostream>
inline auto get_type_by_name(const std::string& name)
    -> std::optional<std::uint8_t> {

  auto it = std::find_if(std::begin(tp::type_names), std::end(tp::type_names),
                         [&name](auto&& val) {
                           // std::cout << val.first << ' ' << name <<
                           // std::endl;
                           return val.first == name;
                         });

  if (it == std::end(tp::type_names)) {
    return std::nullopt;
  }

  // std::cout << "success" << std::endl;

  return it - std::begin(tp::type_names);
}

auto get_variable_type(TypeChecker& checker,
                       const ast::VariableDefinitionNode& var_def)
    -> std::optional<std::uint8_t> {

  // std::cout << var_def.type.value()->identifier->name << std::endl;

  if (var_def.type.has_value()) {
    auto optional_type_index =
        get_type_by_name(var_def.type.value()->identifier->name);

    if (!optional_type_index.has_value()) {
      return std::nullopt;
    }

    // std::cout << "success2" << std::endl;

    return optional_type_index.value();
  }

  if (!var_def.value.has_value()) {
    return std::nullopt;
  }

  std::uint8_t type = checker(*var_def.value.value());
  if (type >= sizeof(tp::type_names) / sizeof(tp::type_names[0])) {
    return std::nullopt;
  }

  return type;
}

auto get_function_type(TypeChecker& checker,
                       const ast::FunctionDefinitionNode& func_def)
    -> std::optional<std::pair<std::uint8_t, std::vector<std::uint8_t>>> {

  std::uint8_t return_type =
      type_tuple_index_v<tp::Void, tp::BasicTypeTypeTuple>;

  if (func_def.return_type.get() != nullptr) {
    auto optional_type_index =
        get_type_by_name(func_def.return_type->identifier->name);

    if (!optional_type_index.has_value()) {
      return std::nullopt;
    }

    return_type = optional_type_index.value();
  }

  if (auto& final_expr = func_def.body->value;
      final_expr.has_value() && checker(*final_expr.value()) != return_type) {
    return std::nullopt;
  }

  std::vector<std::uint8_t> argument_types;
  argument_types.reserve(func_def.argument_list.size());
  for (auto& [name, type] : func_def.argument_list) {
    auto optional_type_index = get_type_by_name(type.identifier->name);

    if (!optional_type_index.has_value()) {
      return std::nullopt;
    }

    argument_types.push_back(optional_type_index.value());
  }

  InterruptTypeChecker<ast::ReturnStatementNode> return_type_checker(
      checker, return_type);

  return_type_checker(*func_def.body);

  return std::make_pair(return_type, std::move(argument_types));
}
