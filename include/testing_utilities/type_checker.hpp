#pragma once

#include <parser/ast.hpp>
#include <semantic_analysis/types.hpp>

class TypeChecker {
public:
  void visit(const ast::IdentifierNode& identifier);

  void visit(const ast::LiteralNode& literal);

  void visit(const ast::FunctionCallNode& function_call);

  void visit(const ast::ExpressionNode& expression);

  void visit(const ast::BlockExpressionNode& expression);

  void visit(const ast::IfExpressionNode& expression);

  void visit(const ast::AssignmentNode& assignment);

  void visit(const ast::LoopExpressionNode& loop);

  void visit(const ast::BreakStatementNode& break_stmt);

  void visit(const ast::ContinueStatementNode& continue_stmt);

  void visit(const ast::ReturnStatementNode& return_stmt);

  template <typename BinaryNode>
    requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
  void visit(const BinaryNode& binary_node);

  void visit(const ast::CastNode& cast_node);

  void visit(const ast::UnaryNode& unary_node);

  void visit(const ast::PrimaryNode& primary_node);

  void visit(const ast::StatementNode& statement);

  void visit(const ast::VariableDefinitionNode& variable_definition);

  void visit(const ast::FunctionDefinitionNode& function_definition);

  void visit(const ast::Program& program);
};
