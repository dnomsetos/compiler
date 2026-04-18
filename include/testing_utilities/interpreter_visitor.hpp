#include <limits>
#include <unordered_map>

#include <parser/ast.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>

const std::size_t without_interrupt = std::numeric_limits<std::size_t>::max();

class InterpreterVisitor {
public:
  InterpreterVisitor(TypeStore& type_store);

  calc_result_t operator()(const ast::IdentifierNode& identifier);

  calc_result_t operator()(const ast::LiteralNode& literal);

  calc_result_t operator()(const ast::FunctionCallNode& function_call);

  calc_result_t operator()(const ast::ExpressionNode& expression);

  calc_result_t operator()(const ast::BlockExpressionNode& expression);

  calc_result_t operator()(const ast::IfExpressionNode& expression);

  calc_result_t operator()(const ast::AssignmentNode& assignment);

  calc_result_t operator()(const ast::LoopExpressionNode& loop);

  template <typename InterruptNode>
    requires is_in_type_tuple_v<InterruptNode, ast::InterruptNodeTuple>
  calc_result_t operator()(const InterruptNode& break_stmt);

  template <typename BinaryNode>
    requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
  calc_result_t operator()(const BinaryNode& binary_node);

  calc_result_t operator()(const ast::CastNode& cast_node);

  calc_result_t operator()(const ast::UnaryNode& unary_node);

  calc_result_t operator()(const ast::PrimaryNode& primary_node);

  calc_result_t operator()(const ast::StatementNode& statement);

  calc_result_t operator()(const ast::VariableDefinitionNode& var_def);

  calc_result_t operator()(const ast::FunctionDefinitionNode& fn_def);

  calc_result_t operator()(const ast::Program& program,
                           const std::string& name);

private:
  bool check_interrupt() const;

  TypeStore& type_store_;
  std::unordered_map<const SymbolTable*,
                     std::unordered_map<std::string, calc_result_t>>
      value_tables_;
  std::size_t interrupt_index_ = without_interrupt;
  std::optional<tkn::Label> desired_label_;
  std::optional<calc_result_t> desired_value_;
};
