#pragma once

#include "scanner/token.hpp"
#include <unordered_map>
#include <vector>

#include <parser/ast.hpp>
#include <tables/types.hpp>
#include <utility/allocator.hpp>

class SymbolTable;

class GlobalSymbolTable;

class TypeChecker {
public:
  explicit TypeChecker(const SymbolTable* symbol_table);

  TypeChecker(const TypeChecker&) = delete;
  TypeChecker(TypeChecker&&) = delete;

  TypeChecker& operator=(const TypeChecker&) = delete;
  TypeChecker& operator=(TypeChecker&&) = delete;

  std::uint8_t operator()(const ast::ExpressionNode& expr) const;

  std::uint8_t operator()(const ast::BlockExpressionNode& expr) const;

  std::uint8_t operator()(const ast::IfExpressionNode& expr) const;

  std::uint8_t operator()(const ast::LoopExpressionNode& expr) const;

  std::uint8_t operator()(const ast::AssignmentNode& expr) const;

  template <typename T>
    requires is_in_type_tuple_v<T, ast::LogicalBinaryNodeTuple>
  std::uint8_t operator()(const T& expr) const;

  std::uint8_t operator()(const ast::EqualityNode& expr) const;

  std::uint8_t operator()(const ast::ComparisonNode& expr) const;

  template <typename T>
    requires is_in_type_tuple_v<T, ast::ArithmeticBinaryNodeTuple>
  std::uint8_t operator()(const T& expr) const;

  std::uint8_t operator()(const ast::UnaryNode& expr) const;

  std::uint8_t operator()(const ast::FunctionCallNode& expr) const;

  std::uint8_t operator()(const ast::PrimaryNode& expr) const;

  std::uint8_t operator()(const ast::IdentifierNode& expr) const;

private:
  const SymbolTable* context_;
};

class SymbolTable {
public:
  SymbolTable() = default;

  SymbolTable(const ast::BlockExpressionNode& block,
              const SymbolTable* parent = nullptr);

  friend GlobalSymbolTable;

  auto get_variable(const std::string& name) const -> const tp::VariableInfo&;

  auto get_function(const std::string& name) const -> const tp::FunctionInfo&;

  auto get_variable_in_position(const std::string& name,
                                const tkn::Position& position) const
      -> const tp::VariableInfo&;

  auto get_function_in_position(const std::string& name,
                                const tkn::Position& position) const
      -> const tp::FunctionInfo&;

  const SymbolTable* get_parent() const;

private:
  const SymbolTable* parent_ = nullptr;
  std::pmr::vector<alloc::pmr_unique_ptr<SymbolTable>> children_{&alloc::mr};
  std::unordered_map<std::string, tp::VariableInfo> variables_;
  std::unordered_map<std::string, tp::FunctionInfo> functions_;
};

class GlobalSymbolTable {
public:
  explicit GlobalSymbolTable(const ast::Program& program);

  const SymbolTable* get_root() const;

private:
  SymbolTable root_;
};

inline auto get_type_by_name(const std::string& name)
    -> std::optional<std::uint8_t>;

auto get_variable_type(TypeChecker& checker,
                       const ast::VariableDefinitionNode& var_def)
    -> std::optional<std::uint8_t>;

auto get_function_type(TypeChecker& checker,
                       const ast::FunctionDefinitionNode& func_def)
    -> std::optional<std::pair<std::uint8_t, std::vector<std::uint8_t>>>;
