#pragma once

#include <parser/ast.hpp>
#include <semantic_analysis/symbol_table.hpp>

template <ast::InterruptNode TriggerNode> class InterruptTypeChecker {
public:
  InterruptTypeChecker(const TypeChecker& type_checker,
                       const std::uint8_t real_type);

  void check_type(const TriggerNode& trigger);

  void operator()(const TriggerNode& trigger);

  void operator()(const ast::FunctionCallNode& function_call);

  void operator()(const ast::ExpressionNode& expression);

  void operator()(const ast::BlockExpressionNode& expression);

  void operator()(const ast::IfExpressionNode& expression);

  void operator()(const ast::AssignmentNode& assignment);

  void operator()(const ast::LoopExpressionNode& loop);

  void operator()(const ast::BreakStatementNode& break_stmt)
    requires(!std::is_same_v<TriggerNode, ast::BreakStatementNode>);

  void operator()(const ast::ContinueStatementNode& continue_stmt);

  void operator()(const ast::ReturnStatementNode& return_stmt)
    requires(!std::is_same_v<TriggerNode, ast::ReturnStatementNode>);

  template <typename BinaryNode>
    requires(is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>)
  void operator()(const BinaryNode& binary_node);

  void operator()(const ast::CastNode& cast_node);

  void operator()(const ast::UnaryNode& unary_node);

  void operator()(const ast::PrimaryNode& primary_node);

  void operator()(const ast::LiteralNode& literalNode);

  void operator()(const ast::IdentifierNode& identifier);

  void operator()(const ast::StatementNode& statement);

  void operator()(const ast::VariableDefinitionNode& variable_definition);

  void operator()(const ast::FunctionDefinitionNode& function_definition);

  bool is_failed() const;

  std::uint8_t get_type() const;

private:
  const TypeChecker& type_checker_;
  std::uint8_t real_type_;
  bool is_failed_ = false;
};

#define INTERRUPT_TYPE_CHECKER_GUARD
#include <semantic_analysis/interrupt_type_checker_impl/interrupt_type_checker.ipp>
#undef INTERRUPT_TYPE_CHECKER_GUARD
