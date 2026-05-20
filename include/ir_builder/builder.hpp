#pragma once

#include <string>

#include <parser/ast.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/type_storage.hpp>

#include <llvm-22/llvm/IR/BasicBlock.h>
#include <llvm-22/llvm/IR/Function.h>
#include <llvm-22/llvm/IR/IRBuilder.h>
#include <llvm-22/llvm/IR/Instructions.h>
#include <llvm-22/llvm/IR/LLVMContext.h>
#include <llvm-22/llvm/IR/Module.h>
#include <llvm-22/llvm/IR/Type.h>
#include <llvm-22/llvm/IR/Value.h>
#include <llvm-22/llvm/MC/TargetRegistry.h>
#include <llvm-22/llvm/Support/CodeGen.h>
#include <llvm-22/llvm/Support/FileSystem.h>
#include <llvm-22/llvm/Support/TargetSelect.h>
#include <llvm-22/llvm/Target/TargetMachine.h>
#include <llvm-22/llvm/Target/TargetOptions.h>
#include <llvm-22/llvm/TargetParser/Triple.h>

struct PairHash {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const {
    size_t h1 = std::hash<T1>{}(p.first);
    size_t h2 = std::hash<T2>{}(p.second);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

class BuildVisitor {
public:
  BuildVisitor(const std::string& module_name, TypeStore& type_store);

  void prepare();

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

  void visit(const ast::LogicalOrNode& node);
  void visit(const ast::LogicalAndNode& node);
  void visit(const ast::ComparisonNode& node);
  void visit(const ast::BitwiseOrNode& node);
  void visit(const ast::BitwiseXorNode& node);
  void visit(const ast::BitwiseAndNode& node);
  void visit(const ast::ShiftNode& node);
  void visit(const ast::AdditionNode& node);
  void visit(const ast::MultiplicationNode& node);

  void visit(const ast::CastNode& cast_node);
  void visit(const ast::UnaryNode& unary_node);
  void visit(const ast::PrimaryNode& primary_node);
  void visit(const ast::FunctionCallNode& function_call);
  void visit(const ast::IdentifierNode& identifier);
  void visit(const ast::LiteralNode& literal);

  void print_module(std::ostream& out);

  void emit_object(const std::string& output_path);

  void emit_executable(const std::string& output_path,
                       const std::vector<std::string>& link_objects);

private:
  template <typename T>
    requires is_in_type_tuple_v<T, ast::BinaryNodeTuple>
  void binary_node_helper(
      const T& binary_node,
      std::function<llvm::Value*(llvm::Value*, llvm::Value*)> accum_function);

  struct LoopContext {
    llvm::BasicBlock* body;
    llvm::BasicBlock* finish;
    llvm::AllocaInst* result;
    std::optional<std::string> label;
  };

  llvm::LLVMContext context_;
  llvm::IRBuilder<> builder_{context_};
  std::unique_ptr<llvm::Module> module_;
  TypeStore& type_store_;

  llvm::Value* current_value_ = nullptr;
  std::size_t tmp_counter_ = 0;

  std::vector<LoopContext> loop_stack_;

  std::unordered_map<std::pair<SymbolTable*, std::string>, llvm::Value*,
                     PairHash>
      variables_names_;

  std::unordered_map<std::string, llvm::Function*> functions_;
  llvm::Function* current_function_ = nullptr;
};
