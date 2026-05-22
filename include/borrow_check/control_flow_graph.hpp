#pragma once

#include <deque>

#include <borrow_check/ir_structures.hpp>
#include <parser/ast.hpp>
#include <semantic_analysis/type_storage.hpp>
#include <utility/allocator.hpp>
#include <utility/hash_utils.hpp>

namespace ir {

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

  BasicBlock* get_main_block() const { return main_block_; }

private:
  struct LoopContext {
    alloc::pmr_weak_ptr<BasicBlock> body;
    alloc::pmr_weak_ptr<BasicBlock> finish;
    std::optional<std::string> label;
    alloc::pmr_weak_ptr<BasicBlock> parent;
  };

  LoopContext& find_loop(const std::string& label = "");

  std::unordered_map<SymbolTable*, std::vector<Alloca*>> scopes_;
  TypeStore& type_store_;
  std::unordered_map<std::string, AccessKindVariant*> global_variables_;
  std::unordered_map<std::pair<std::string, SymbolTable*>, AccessKindVariant*,
                     PairHash>
      variables_;
  std::deque<Function> functions_;
  std::deque<LoopContext> loop_stack_;
  BasicBlock* current_block_;
  BasicBlock* main_block_;
};

} // namespace ir
