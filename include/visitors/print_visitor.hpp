#include <parser/ast.hpp>

class PrintVisitor {
public:
  explicit PrintVisitor(std::ostream& out, bool skip_empty = false);

  void operator()(const ast::IdentifierNode& identifier);

  void operator()(const ast::LiteralNode& literal);

  void operator()(const ast::FunctionCallNode& function_call);

  void operator()(const ast::ExpressionNode& expression);

  void operator()(const ast::BlockExpressionNode& expression);

  void operator()(const ast::IfExpressionNode& expression);

  void operator()(const ast::AssignmentNode& assignment);

  template <typename BinaryNode> void operator()(const BinaryNode& binary_node);

  void operator()(const ast::UnaryNode& unary_node);

  void operator()(const ast::PrimaryNode& primary_node);

  void operator()(const ast::StatementNode& statement);

  void operator()(const ast::VariableDefinitionNode& variable_definition);

  void operator()(const ast::FunctionDefinitionNode& function_definition);

  void operator()(const ast::Program& program);

private:
  std::string tabs_;
  std::ostream& out_;
  bool skip_empty_;
};
