#pragma once

#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>

class SemanticVisitor {
public:
  SemanticVisitor(TypeStore& type_store, GlobalSymbolTable& symbol_table);

  void visit(ast::Program& program);

  void visit(ast::VariableDefinitionNode& variable_definition);

  void visit(ast::FunctionDefinitionNode& function_definition);

  void visit(ast::StatementNode& statement);

  void visit(ast::ReturnStatementNode& return_stmt);

  void visit(ast::BreakStatementNode& break_stmt);

  void visit(ast::ContinueStatementNode& continue_stmt);

  void visit(ast::ExpressionNode& expression,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::BlockExpressionNode& expression,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::IfExpressionNode& expression,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::LoopExpressionNode& loop,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::AssignmentNode& assignment,
             tp::TypeId expected_type = tp::no_type_id);

  template <typename LogicalNode>
    requires is_in_type_tuple_v<LogicalNode, ast::LogicalBinaryNodeTuple>
  void visit(LogicalNode& logical_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::ComparisonNode& comparison_node,
             tp::TypeId expected_type = tp::no_type_id);

  template <typename BitwiseNode>
    requires is_in_type_tuple_v<BitwiseNode, ast::BitwiseBinaryNodeTuple>
  void visit(BitwiseNode& bitwise_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::ShiftNode& shift_node,
             tp::TypeId expected_type = tp::no_type_id);

  template <typename ArithmeticNode>
    requires is_in_type_tuple_v<ArithmeticNode, ast::ArithmeticBinaryNodeTuple>
  void visit(ArithmeticNode& arithmetic_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::CastNode& cast_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::UnaryNode& unary_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::PrimaryNode& primary_node,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::FunctionCallNode& function_call,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::IdentifierNode& identifier,
             tp::TypeId expected_type = tp::no_type_id);

  void visit(ast::LiteralNode& literal,
             tp::TypeId expected_type = tp::no_type_id);

private:
  template <typename BinaryNode, bool NeedUnify>
    requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
  void
  binary_node_helper(BinaryNode& binary_node,
                     std::function<bool(const tp::TypeVariant&)> type_check,
                     tp::TypeId expected_type = tp::no_type_id);

  TypeStore& type_store_;
  SymbolTable* symbol_table_;
};
