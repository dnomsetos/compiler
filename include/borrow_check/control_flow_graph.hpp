#pragma once

#include <deque>

#include <borrow_check/ir_structures.hpp>
#include <parser/ast.hpp>
#include <semantic_analysis/type_storage.hpp>
#include <utility/allocator.hpp>
#include <utility/hash_utils.hpp>

namespace bc_ir {

class ControlFlowGraph {
public:
  ControlFlowGraph(TypeStore&);

  void visit(const ast::Program& program);

  void visit(const ast::FunctionDefinitionNode& function_definition);
  void visit(const ast::VariableDefinitionNode& variable_definition);

  void visit(const ast::StatementNode& statement);

  void visit(const ast::BreakStatementNode& break_stmt);
  void visit(const ast::ContinueStatementNode& continue_stmt);
  void visit(const ast::ReturnStatementNode& return_stmt);

  void visit(const ast::ExpressionNode& expression);

  void visit(const ast::BlockExpressionNode& expression);
  void visit(const ast::IfExpressionNode& expression);
  void visit(const ast::LoopExpressionNode& loop);
  void visit(const ast::AssignmentNode& assignment);

  template <typename BinaryNode>
    requires is_in_type_tuple_v<BinaryNode, ast::BinaryNodeTuple>
  void visit(const BinaryNode& binary_node);

  void visit(const ast::CastNode& cast_node);
  void visit(const ast::UnaryNode& unary_node);
  void visit(const ast::FunctionCallNode& function_call);

  void visit(const ast::LvalueDereferenceNode& lvalue);

  void print() const;

  auto get_functions() const -> const std::deque<Function>&;

private:
  struct LoopContext {
    BasicBlock* body;
    BasicBlock* finish;
    std::optional<std::string> label;
    BasicBlock* parent;
  };

  LoopContext& find_loop(const std::string& label = "");

  void emit_reference_arg(const ast::IdentifierNode& arg,
                          const std::string& arg_name, const Symbol* symbol,
                          std::size_t index, const std::string& fn_name);
  void emit_value_arg(const ast::IdentifierNode& arg,
                      const std::string& arg_name, const Symbol* symbol);

  void emit_borrow_inst(const ast::VariableDefinitionNode& variable_definition,
                        const std::string& name, const Symbol* symbol);
  void resolve_borrow_from_identifier(Instruction& borrow_inst,
                                      const ast::UnaryNode& right_unary_node);
  void resolve_borrow_from_ref(Instruction& borrow_inst,
                               const ast::UnaryNode& right_unary_node);

  void visit_value_assignment(const ast::AssignmentNode& assignment);
  void visit_ref_assignment(const ast::AssignmentNode& assignment);
  void emit_ref_rebind_from_ref(const ast::AssignmentNode& assignment,
                                AccessKindVariant& lvar,
                                const ast::UnaryNode& right_unary);
  void emit_ref_rebind_from_addr(const ast::AssignmentNode& assignment,
                                 const ast::IdentifierNode& lident,
                                 AccessKindVariant& lvar,
                                 const ast::UnaryNode& right_unary);

  AccessKindVariant& lookup_var(const ast::IdentifierNode& ident);

  std::unordered_map<SymbolTable*, std::vector<Alloca*>> scopes_;
  TypeStore& type_store_;
  std::unordered_map<std::string, AccessKindVariant*> global_variables_;
  std::unordered_map<std::pair<std::string, SymbolTable*>, AccessKindVariant*,
                     PairHash>
      variables_;
  std::deque<Function> functions_;
  std::deque<LoopContext> loop_stack_;
  BasicBlock* current_block_;
};

} // namespace bc_ir
