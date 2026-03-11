#include <unordered_map>

#include <parser/ast.hpp>
#include <utility/executor.hpp>

class InterpreterVisitor {
public:
  calc_result_t operator()(const ast::IdentifierNode& identifier);

  calc_result_t operator()(const ast::LiteralNode& literal);

  calc_result_t operator()(const ast::FunctionCallNode& function_call);

  calc_result_t operator()(const ast::ExpressionNode& expression);

  calc_result_t operator()(const ast::AssignmentNode& assignment);

  template <typename BinaryNode>
  calc_result_t operator()(const BinaryNode& binary_node);

  calc_result_t operator()(const ast::UnaryNode& unary_node);

  calc_result_t operator()(const ast::PrimaryNode& primary_node);

  calc_result_t operator()(const ast::StatementNode& statement);

  calc_result_t operator()(const ast::IfStatementNode& if_statement);

  calc_result_t operator()(const ast::VariableDefinitionNode& var_def);

  calc_result_t operator()(const ast::FunctionDefinitionNode& fn_def);

  calc_result_t operator()(const ast::Program& program);

private:
  std::unordered_map<std::string, calc_result_t> variables_;
};
