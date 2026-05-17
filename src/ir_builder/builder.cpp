#include "scanner/token.hpp"
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <variant>

#include <ir_builder/builder.hpp>
#include <ir_builder/ir_utilities.hpp>
#include <parser/ast.hpp>

#include <llvm-22/llvm/IR/BasicBlock.h>
#include <llvm-22/llvm/IR/DerivedTypes.h>
#include <llvm-22/llvm/IR/Function.h>
#include <llvm-22/llvm/IR/GlobalVariable.h>
#include <llvm-22/llvm/IR/LegacyPassManager.h>
#include <llvm-22/llvm/IR/Verifier.h>
#include <llvm-22/llvm/Support/raw_os_ostream.h>
#include <llvm-22/llvm/TargetParser/Host.h>

inline llvm::AllocaInst* createAllocaAligned(llvm::IRBuilder<>& builder,
                                             llvm::Type* ty,
                                             const llvm::Twine& name = "") {
  const llvm::DataLayout& data_layout =
      builder.GetInsertBlock()->getParent()->getParent()->getDataLayout();
  llvm::AllocaInst* alloca = builder.CreateAlloca(ty, nullptr, name);
  alloca->setAlignment(data_layout.getABITypeAlign(ty));
  return alloca;
}

BuildVisitor::BuildVisitor(const std::string& module_name,
                           TypeStore& type_store)
    : module_(std::make_unique<llvm::Module>(module_name, context_)),
      type_store_(type_store) {
  prepare();
}

void BuildVisitor::prepare() {
#define DECL_FUNCTION(name, type)                                              \
  llvm::FunctionType* _##name =                                                \
      llvm::FunctionType::get(builder_.getVoidTy(), {builder_.type()}, false); \
  functions_.insert(                                                           \
      {#name, llvm::Function::Create(_##name, llvm::Function::ExternalLinkage, \
                                     #name, module_.get())});

  DECL_FUNCTION(print_i8, getInt8Ty);
  DECL_FUNCTION(print_i16, getInt16Ty);
  DECL_FUNCTION(print_i32, getInt32Ty);
  DECL_FUNCTION(print_i64, getInt64Ty);
  DECL_FUNCTION(print_f32, getFloatTy);
  DECL_FUNCTION(print_f64, getDoubleTy);
  DECL_FUNCTION(print_bool, getInt1Ty);
  DECL_FUNCTION(print_char, getInt8Ty);
  DECL_FUNCTION(print_u8, getInt8Ty);
  DECL_FUNCTION(print_u16, getInt16Ty);
  DECL_FUNCTION(print_u32, getInt32Ty);
  DECL_FUNCTION(print_u64, getInt64Ty);

#undef DECL_FUNCTION

  llvm::FunctionType* _print_void =
      llvm::FunctionType::get(builder_.getVoidTy(), {}, false);
  functions_.insert(
      {"print_void",
       llvm::Function::Create(_print_void, llvm::Function::ExternalLinkage,
                              "print_void", module_.get())});
}

void BuildVisitor::visit(const ast::Program& program) {
  for (auto& definition : program.definitions) {
    std::visit([&](auto&& def) { visit(def); }, definition);
  }

  if (llvm::verifyModule(*module_, &llvm::errs())) {
    llvm::errs() << "module is broken\n";
  }
}

void BuildVisitor::visit(
    const ast::FunctionDefinitionNode& function_definition) {

  llvm::Type* type = get_llvm_type(type_store_, builder_, context_,
                                   function_definition.name->type_id);

  llvm::FunctionType* function_type;

  if (llvm::isa<llvm::FunctionType>(type)) {
    function_type = llvm::cast<llvm::FunctionType>(type);
  } else {
    throw std::runtime_error("Type of function is not function type");
  }

  llvm::Function* function = llvm::Function::Create(
      function_type, llvm::Function::ExternalLinkage,
      function_definition.name->identifier->name, module_.get());

  functions_.insert({function_definition.name->identifier->name, function});

  current_function_ = function;

  llvm::BasicBlock* entry_block =
      llvm::BasicBlock::Create(context_, "entry", function);

  builder_.SetInsertPoint(entry_block);

  for (auto&& [arg_val, arg_name] :
       std::views::zip(function->args(), function_definition.argument_list)) {
    llvm::AllocaInst* slot = createAllocaAligned(
        builder_, arg_val.getType(), arg_name.first.identifier->name);
    builder_.CreateStore(&arg_val, slot);
    variables_names_.insert(
        {{arg_name.first.table, arg_name.first.identifier->name}, slot});
  }

  for (auto& statement : function_definition.body->statements) {
    if (builder_.GetInsertBlock()->getTerminator()) {
      break;
    }
    visit(statement);
  }

  if (function_definition.body->value.has_value()) {
    visit(*function_definition.body->value.value());
    if (!builder_.GetInsertBlock()->getTerminator()) {
      builder_.CreateRet(current_value_);
    }
  } else {
    if (!builder_.GetInsertBlock()->getTerminator()) {
      llvm::Type* ret_type = function->getReturnType();
      if (ret_type->isVoidTy()) {
        builder_.CreateRetVoid();
      } else {
        builder_.CreateRet(llvm::UndefValue::get(ret_type));
      }
    }
  }

  current_function_ = nullptr;
}

void BuildVisitor::visit(
    const ast::VariableDefinitionNode& variable_definition) {

  auto& real_var_name = variable_definition.name->identifier->name;

  auto key = std::make_pair(variable_definition.table, real_var_name);

  if (!variables_names_.contains(key)) {
    variables_names_.insert({key, nullptr});
  }

  if (!variable_definition.is_global) {
    llvm::AllocaInst* var;
    variables_names_.at(key) = var =
        createAllocaAligned(builder_,
                            get_llvm_type(type_store_, builder_, context_,
                                          variable_definition.name->type_id),
                            real_var_name);

    if (variable_definition.value.has_value()) {
      visit(*variable_definition.value.value());

      builder_.CreateStore(current_value_, var);
    }
  } else {
    llvm::Value* init_val = nullptr;

    if (variable_definition.value.has_value()) {
      visit(*variable_definition.value.value());
      init_val = current_value_;
    }

    llvm::GlobalVariable* var;

    if (init_val == nullptr) {
      llvm::Type* ty = get_llvm_type(type_store_, builder_, context_,
                                     variable_definition.name->type_id);
      llvm::Constant* zero = llvm::Constant::getNullValue(ty);

      var = new llvm::GlobalVariable(*module_, ty, false,
                                     llvm::GlobalValue::ExternalLinkage, zero,
                                     real_var_name);
    } else if (llvm::Constant* const_val =
                   llvm::dyn_cast<llvm::Constant>(init_val)) {
      var = new llvm::GlobalVariable(
          *module_,
          get_llvm_type(type_store_, builder_, context_,
                        variable_definition.name->type_id),
          false, llvm::GlobalValue::ExternalLinkage, const_val, real_var_name);
    } else {
      throw std::runtime_error("cannot init global variable");
    }

    variables_names_.at(key) = var;
  }
}

void BuildVisitor::visit(const ast::StatementNode& statement) {
  std::visit([&](auto&& stmt) { visit(stmt); }, *statement.node);
}

void BuildVisitor::visit(const ast::BreakStatementNode& break_stmt) {
  llvm::Value* result = nullptr;

  if (break_stmt.value.has_value()) {
    visit(*break_stmt.value.value());
    result = current_value_;
  } else {
    llvm::StructType* empty_tuple = llvm::StructType::get(context_, {});
    result = llvm::ConstantStruct::get(empty_tuple, {});
  }

  LoopContext* loop = &loop_stack_.back();

  if (break_stmt.label.has_value()) {
    bool found = false;

    for (auto& loop_context : std::views::reverse(loop_stack_)) {
      if (loop_context.label.has_value() &&
          loop_context.label.value() == break_stmt.label.value().name) {
        loop = &loop_context;
        found = true;
        break;
      }
    }

    if (!found) {
      throw std::runtime_error("Loop label not found");
    }
  }

  builder_.CreateStore(result, loop->result);
  builder_.CreateBr(loop->finish);
}

void BuildVisitor::visit(const ast::ContinueStatementNode& continue_stmt) {
  LoopContext* loop = &loop_stack_.back();

  if (continue_stmt.label.has_value()) {
    bool found = false;

    for (auto& loop_context : std::views::reverse(loop_stack_)) {
      if (loop_context.label.has_value() &&
          loop_context.label.value() == continue_stmt.label.value().name) {
        loop = &loop_context;
        found = true;
        break;
      }
    }

    if (!found) {
      throw std::runtime_error("Loop label not found");
    }
  }

  builder_.CreateBr(loop->body);
}

void BuildVisitor::visit(const ast::ReturnStatementNode& return_stmt) {
  if (return_stmt.value.has_value()) {
    visit(*return_stmt.value.value());
    builder_.CreateRet(current_value_);
  } else {
    builder_.CreateRetVoid();
  }
}

void BuildVisitor::visit(const ast::ExpressionNode& expression) {
  std::visit([&](auto&& val) { visit(val); }, *expression.node);
}

void BuildVisitor::visit(const ast::BlockExpressionNode& expression) {
  for (auto& statement : expression.statements) {
    if (builder_.GetInsertBlock()->getTerminator()) {
      break;
    }
    visit(statement);
  }

  if (expression.value.has_value()) {
    if (!builder_.GetInsertBlock()->getTerminator()) {
      visit(*expression.value.value());
    }
  } else {
    if (!builder_.GetInsertBlock()->getTerminator()) {
      llvm::StructType* empty_tuple = llvm::StructType::get(context_, {});
      current_value_ = llvm::ConstantStruct::get(empty_tuple, {});
    }
  }
}

void BuildVisitor::visit(const ast::IfExpressionNode& expression) {
  llvm::AllocaInst* result_alloc = createAllocaAligned(
      builder_,
      get_llvm_type(type_store_, builder_, context_, expression.type_id),
      "if.result");

  visit(*expression.condition);
  llvm::Value* condition = current_value_;

  llvm::BasicBlock* then_block =
      llvm::BasicBlock::Create(context_, "if.then", current_function_);

  std::vector<llvm::BasicBlock*> elif_cond_blocks;
  std::vector<llvm::BasicBlock*> elif_body_blocks;
  elif_cond_blocks.reserve(expression.elif_bodies.size());
  elif_body_blocks.reserve(expression.elif_bodies.size());

  for (std::size_t i = 0; i < expression.elif_bodies.size(); ++i) {
    elif_cond_blocks.push_back(
        llvm::BasicBlock::Create(context_, "elif.cond", current_function_));
    elif_body_blocks.push_back(
        llvm::BasicBlock::Create(context_, "elif.body", current_function_));
  }

  llvm::BasicBlock* else_block =
      llvm::BasicBlock::Create(context_, "if.else", current_function_);
  llvm::BasicBlock* merge_block =
      llvm::BasicBlock::Create(context_, "if.merge", current_function_);

  llvm::BasicBlock* first_false_block =
      elif_cond_blocks.empty() ? else_block : elif_cond_blocks[0];
  builder_.CreateCondBr(condition, then_block, first_false_block);

  builder_.SetInsertPoint(then_block);
  visit(*expression.body);
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateStore(current_value_, result_alloc);
    builder_.CreateBr(merge_block);
  }

  for (std::size_t i = 0; i < expression.elif_bodies.size(); ++i) {
    const auto& elif = expression.elif_bodies[i];

    builder_.SetInsertPoint(elif_cond_blocks[i]);
    visit(*elif.expr);
    llvm::Value* elif_cond = current_value_;

    llvm::BasicBlock* next_false = (i + 1 < elif_cond_blocks.size())
                                       ? elif_cond_blocks[i + 1]
                                       : else_block;
    builder_.CreateCondBr(elif_cond, elif_body_blocks[i], next_false);

    builder_.SetInsertPoint(elif_body_blocks[i]);
    visit(*elif.block);
    if (!builder_.GetInsertBlock()->getTerminator()) {
      builder_.CreateStore(current_value_, result_alloc);
      builder_.CreateBr(merge_block);
    }
  }

  builder_.SetInsertPoint(else_block);
  if (expression.else_body.has_value()) {
    visit(*expression.else_body.value());
    if (!builder_.GetInsertBlock()->getTerminator()) {
      builder_.CreateStore(current_value_, result_alloc);
      builder_.CreateBr(merge_block);
    }
  } else {
    llvm::StructType* empty_tuple = llvm::StructType::get(context_, {});
    builder_.CreateStore(llvm::ConstantStruct::get(empty_tuple, {}),
                         result_alloc);
    builder_.CreateBr(merge_block);
  }

  builder_.SetInsertPoint(merge_block);
  current_value_ = builder_.CreateLoad(
      get_llvm_type(type_store_, builder_, context_, expression.type_id),
      result_alloc);
}

void BuildVisitor::visit(const ast::LoopExpressionNode& loop) {
  llvm::AllocaInst* result = createAllocaAligned(
      builder_, get_llvm_type(type_store_, builder_, context_, loop.type_id),
      "result");

  llvm::BasicBlock* body =
      llvm::BasicBlock::Create(context_, "loop.body", current_function_);

  llvm::BasicBlock* finish =
      llvm::BasicBlock::Create(context_, "loop.finish", current_function_);

  LoopContext loop_context{
      .body = body, .finish = finish, .result = result, .label = std::nullopt};

  if (loop.label.has_value()) {
    loop_context.label = loop.label.value().name;
  }

  loop_stack_.push_back(loop_context);

  builder_.CreateBr(body);

  builder_.SetInsertPoint(body);

  for (auto& statement : loop.body) {
    if (builder_.GetInsertBlock()->getTerminator()) {
      break;
    }
    visit(statement);
  }

  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(body);
  }

  builder_.SetInsertPoint(finish);

  loop_stack_.pop_back();

  current_value_ = builder_.CreateLoad(
      get_llvm_type(type_store_, builder_, context_, loop.type_id), result);
}

void BuildVisitor::visit(const ast::AssignmentNode& assignment) {
  visit(*assignment.right);
  llvm::Value* rhs = current_value_;

  visit_lvalue_ptr(*assignment.left);
  llvm::Value* dest_ptr = current_value_;

  builder_.CreateStore(rhs, dest_ptr);

  llvm::StructType* empty_tuple = llvm::StructType::get(context_, {});
  current_value_ = llvm::ConstantStruct::get(empty_tuple, {});
}

void BuildVisitor::visit_lvalue_ptr(const ast::LvalueExpressionNode& lvalue) {
  std::visit([&](auto&& node) { visit_lvalue_ptr(node); }, *lvalue.lvalue);
}

void BuildVisitor::visit_lvalue_ptr(const ast::IdentifierNode& identifier) {
  const Symbol* symbol =
      identifier.table->get_variable_symbol(identifier.identifier->name);

  current_value_ =
      variables_names_.at({symbol->scope, identifier.identifier->name});
}

void BuildVisitor::visit_lvalue_ptr(
    const ast::LvalueDereferenceNode& dereference) {
  std::visit([&](auto&& ref_node) { visit(ref_node); }, *dereference.reference);
}

void BuildVisitor::visit(const ast::LogicalOrNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::AllocaInst* result_alloc =
      createAllocaAligned(builder_, builder_.getInt1Ty(), "or.result");

  llvm::BasicBlock* merge_block =
      llvm::BasicBlock::Create(context_, "or.merge", current_function_);

  llvm::BasicBlock* first_rhs =
      llvm::BasicBlock::Create(context_, "or.rhs", current_function_);

  builder_.CreateStore(builder_.getTrue(), result_alloc);
  builder_.CreateCondBr(current_value_, merge_block, first_rhs);

  llvm::BasicBlock* current_rhs = first_rhs;

  for (std::size_t i = 0; i < node.right.size(); ++i) {
    builder_.SetInsertPoint(current_rhs);
    visit(node.right[i].second);
    llvm::Value* rhs_val = current_value_;

    if (i + 1 < node.right.size()) {
      llvm::BasicBlock* next_rhs =
          llvm::BasicBlock::Create(context_, "or.rhs", current_function_);
      builder_.CreateStore(builder_.getTrue(), result_alloc);
      builder_.CreateCondBr(rhs_val, merge_block, next_rhs);
      current_rhs = next_rhs;
    } else {
      builder_.CreateStore(rhs_val, result_alloc);
      builder_.CreateBr(merge_block);
    }
  }

  builder_.SetInsertPoint(merge_block);
  current_value_ = builder_.CreateLoad(builder_.getInt1Ty(), result_alloc);
}

void BuildVisitor::visit(const ast::LogicalAndNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::AllocaInst* result_alloc =
      createAllocaAligned(builder_, builder_.getInt1Ty(), "and.result");

  llvm::BasicBlock* merge_block =
      llvm::BasicBlock::Create(context_, "and.merge", current_function_);

  llvm::BasicBlock* first_rhs =
      llvm::BasicBlock::Create(context_, "and.rhs", current_function_);

  builder_.CreateStore(builder_.getFalse(), result_alloc);
  builder_.CreateCondBr(current_value_, first_rhs, merge_block);

  llvm::BasicBlock* current_rhs = first_rhs;

  for (std::size_t i = 0; i < node.right.size(); ++i) {
    builder_.SetInsertPoint(current_rhs);
    visit(node.right[i].second);
    llvm::Value* rhs_val = current_value_;

    if (i + 1 < node.right.size()) {
      llvm::BasicBlock* next_rhs =
          llvm::BasicBlock::Create(context_, "and.rhs", current_function_);
      builder_.CreateStore(builder_.getFalse(), result_alloc);
      builder_.CreateCondBr(rhs_val, next_rhs, merge_block);
      current_rhs = next_rhs;
    } else {
      builder_.CreateStore(rhs_val, result_alloc);
      builder_.CreateBr(merge_block);
    }
  }

  builder_.SetInsertPoint(merge_block);
  current_value_ = builder_.CreateLoad(builder_.getInt1Ty(), result_alloc);
}

void BuildVisitor::visit(const ast::ComparisonNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::Value* accum = current_value_;

  for (auto& [op, expr] : node.right) {
    visit(expr);

    bool is_signed = type_store_.is_signed_type(expr.type_id);
    bool is_float = type_store_.is_float_type(expr.type_id);

    std::visit(
        [&](auto&& oper) {
          using T = std::decay_t<decltype(oper)>;

          if constexpr (std::is_same_v<T, tkn::Equal>) {
            accum = is_float ? builder_.CreateFCmpOEQ(accum, current_value_)
                             : builder_.CreateICmpEQ(accum, current_value_);
          }
          if constexpr (std::is_same_v<T, tkn::NotEqual>) {
            accum = is_float ? builder_.CreateFCmpONE(accum, current_value_)
                             : builder_.CreateICmpNE(accum, current_value_);
          }
          if constexpr (std::is_same_v<T, tkn::Less>) {
            if (is_float) {
              accum = builder_.CreateFCmpOLT(accum, current_value_);
            } else if (is_signed) {
              accum = builder_.CreateICmpSLT(accum, current_value_);
            } else {
              accum = builder_.CreateICmpULT(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::LessEqual>) {
            if (is_float) {
              accum = builder_.CreateFCmpOLE(accum, current_value_);
            } else if (is_signed) {
              accum = builder_.CreateICmpSLE(accum, current_value_);
            } else {
              accum = builder_.CreateICmpULE(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::Greater>) {
            if (is_float) {
              accum = builder_.CreateFCmpOGT(accum, current_value_);
            } else if (is_signed) {
              accum = builder_.CreateICmpSGT(accum, current_value_);
            } else {
              accum = builder_.CreateICmpUGT(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::GreaterEqual>) {
            if (is_float) {
              accum = builder_.CreateFCmpOGE(accum, current_value_);
            } else if (is_signed) {
              accum = builder_.CreateICmpSGE(accum, current_value_);
            } else {
              accum = builder_.CreateICmpUGE(accum, current_value_);
            }
          }
        },
        op);
  }

  current_value_ = accum;
}

void BuildVisitor::visit(const ast::BitwiseOrNode& node) {
  binary_node_helper(node,
                     [&](llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
                       return builder_.CreateOr(lhs, rhs);
                     });
}

void BuildVisitor::visit(const ast::BitwiseXorNode& node) {
  binary_node_helper(node,
                     [&](llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
                       return builder_.CreateXor(lhs, rhs);
                     });
}

void BuildVisitor::visit(const ast::BitwiseAndNode& node) {
  binary_node_helper(node,
                     [&](llvm::Value* lhs, llvm::Value* rhs) -> llvm::Value* {
                       return builder_.CreateAnd(lhs, rhs);
                     });
}

void BuildVisitor::visit(const ast::ShiftNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::Value* accum = current_value_;

  for (auto& [op, expr] : node.right) {
    visit(expr);

    std::visit(
        [&](auto&& oper) {
          using T = std::decay_t<decltype(oper)>;

          if constexpr (std::is_same_v<T, tkn::LeftShift>) {
            accum = builder_.CreateShl(accum, current_value_);
          }
          if constexpr (std::is_same_v<T, tkn::RightShift>) {
            if (type_store_.is_signed_type(expr.type_id)) {
              accum = builder_.CreateAShr(accum, current_value_);
            } else {
              accum = builder_.CreateLShr(accum, current_value_);
            }
          }
        },
        op);
  }

  current_value_ = accum;
}

void BuildVisitor::visit(const ast::AdditionNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::Value* accum = current_value_;

  for (auto& [op, expr] : node.right) {
    visit(expr);

    std::visit(
        [&](auto&& oper) {
          using T = std::decay_t<decltype(oper)>;

          if constexpr (std::is_same_v<T, tkn::Plus>) {
            if (type_store_.is_integer_type(expr.type_id)) {
              accum = builder_.CreateAdd(accum, current_value_);
            } else {
              accum = builder_.CreateFAdd(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::Minus>) {
            if (type_store_.is_integer_type(expr.type_id)) {
              accum = builder_.CreateSub(accum, current_value_);
            } else {
              accum = builder_.CreateFSub(accum, current_value_);
            }
          }
        },
        op);
  }

  current_value_ = accum;
}

void BuildVisitor::visit(const ast::MultiplicationNode& node) {
  visit(*node.left);

  if (node.right.empty()) {
    return;
  }

  llvm::Value* accum = current_value_;

  for (auto& [op, expr] : node.right) {
    visit(expr);

    std::visit(
        [&](auto&& oper) {
          using T = std::decay_t<decltype(oper)>;

          if constexpr (std::is_same_v<T, tkn::Asterisk>) {
            if (type_store_.is_integer_type(expr.type_id)) {
              accum = builder_.CreateMul(accum, current_value_);
            } else {
              accum = builder_.CreateFMul(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::Divide>) {
            if (type_store_.is_integer_type(expr.type_id)) {
              if (type_store_.is_signed_type(expr.type_id)) {
                accum = builder_.CreateSDiv(accum, current_value_);
              } else {
                accum = builder_.CreateUDiv(accum, current_value_);
              }
            } else {
              accum = builder_.CreateFDiv(accum, current_value_);
            }
          }
          if constexpr (std::is_same_v<T, tkn::Mod>) {
            if (type_store_.is_signed_type(expr.type_id)) {
              accum = builder_.CreateSRem(accum, current_value_);
            } else {
              accum = builder_.CreateURem(accum, current_value_);
            }
          }
        },
        op);
  }

  current_value_ = accum;
}

void BuildVisitor::visit(const ast::CastNode& cast_node) {
  visit(*cast_node.expression);

  if (!cast_node.type.has_value()) {
    return;
  }

  tp::TypeId src_id = cast_node.expression->type_id;
  tp::TypeId dst_id = cast_node.type_id;

  if (src_id == dst_id) {
    return;
  }

  llvm::Type* dst_ty = get_llvm_type(type_store_, builder_, context_, dst_id);
  llvm::Value* src = current_value_;

  bool src_int = type_store_.is_integer_type(src_id);
  bool dst_int = type_store_.is_integer_type(dst_id);
  bool src_float = type_store_.is_float_type(src_id);
  bool dst_float = type_store_.is_float_type(dst_id);
  bool src_signed = type_store_.is_signed_type(src_id);

  if (src_int && dst_int) {
    unsigned src_bits = src->getType()->getIntegerBitWidth();
    unsigned dst_bits = dst_ty->getIntegerBitWidth();

    if (dst_bits < src_bits) {
      current_value_ = builder_.CreateTrunc(src, dst_ty);
    } else if (dst_bits > src_bits) {
      if (src_signed) {
        current_value_ = builder_.CreateSExt(src, dst_ty);
      } else {
        current_value_ = builder_.CreateZExt(src, dst_ty);
      }
    }
  } else if (src_int && dst_float) {
    if (src_signed) {
      current_value_ = builder_.CreateSIToFP(src, dst_ty);
    } else {
      current_value_ = builder_.CreateUIToFP(src, dst_ty);
    }
  } else if (src_float && dst_int) {
    bool dst_signed = type_store_.is_signed_type(dst_id);
    if (dst_signed) {
      current_value_ = builder_.CreateFPToSI(src, dst_ty);
    } else {
      current_value_ = builder_.CreateFPToUI(src, dst_ty);
    }
  } else if (src_float && dst_float) {
    llvm::Type* src_ty = src->getType();
    if (src_ty->isFloatTy() && dst_ty->isDoubleTy()) {
      current_value_ = builder_.CreateFPExt(src, dst_ty);
    } else if (src_ty->isDoubleTy() && dst_ty->isFloatTy()) {
      current_value_ = builder_.CreateFPTrunc(src, dst_ty);
    }
  }
}

void BuildVisitor::visit(const ast::UnaryNode& unary_node) {
  if (!unary_node.op.has_value()) {
    visit(*unary_node.primary);
    return;
  }

  if (std::holds_alternative<tkn::Ampersand>(*unary_node.op.value())) {
    std::visit(
        [&](auto&& primary_val) {
          using T = std::decay_t<decltype(primary_val)>;
          if constexpr (std::is_same_v<T, ast::IdentifierNode>) {
            const Symbol* symbol = primary_val.table->get_variable_symbol(
                primary_val.identifier->name);
            current_value_ = variables_names_.at(
                {symbol->scope, primary_val.identifier->name});
          } else {
            visit(*unary_node.primary);
          }
        },
        *unary_node.primary->primary);
    return;
  }

  if (std::holds_alternative<tkn::Asterisk>(*unary_node.op.value())) {
    visit(*unary_node.primary);
    llvm::Value* ptr = current_value_;
    current_value_ = builder_.CreateLoad(
        get_llvm_type(type_store_, builder_, context_, unary_node.type_id),
        ptr);
    return;
  }

  visit(*unary_node.primary);

  std::visit(
      [&](auto&& val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, tkn::Minus>) {
          if (type_store_.is_integer_type(unary_node.primary->type_id)) {
            current_value_ = builder_.CreateNeg(current_value_);
          } else {
            current_value_ = builder_.CreateFNeg(current_value_);
          }
        }
        if constexpr (std::is_same_v<T, tkn::Not>) {
          current_value_ = builder_.CreateNot(current_value_);
        }
      },
      *unary_node.op.value());
}

void BuildVisitor::visit(const ast::PrimaryNode& primary_node) {
  std::visit([&](auto&& val) { visit(val); }, *primary_node.primary);
}

void BuildVisitor::visit(const ast::FunctionCallNode& function_call) {
  std::vector<llvm::Value*> args;
  args.reserve(function_call.arguments.size());

  for (auto& expr : function_call.arguments) {
    visit(expr);
    args.push_back(current_value_);
  }

  current_value_ = builder_.CreateCall(
      functions_.at(function_call.name->identifier->name), std::move(args));
}

void BuildVisitor::visit(const ast::IdentifierNode& identifier) {
  const Symbol* symbol =
      identifier.table->get_variable_symbol(identifier.identifier->name);

  auto key = std::make_pair(symbol->scope, identifier.identifier->name);

  llvm::Value* variable = variables_names_.at(key);

  current_value_ = builder_.CreateLoad(
      get_llvm_type(type_store_, builder_, context_, identifier.type_id),
      variable);
}

void BuildVisitor::visit(const ast::LiteralNode& literal) {
  std::visit(
      [&](auto&& val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, tkn::IntLiteral>) {
          current_value_ = llvm::ConstantInt::get(
              get_llvm_type(type_store_, builder_, context_, literal.type_id),
              val.value);
        }
        if constexpr (std::is_same_v<T, tkn::FloatLiteral>) {
          current_value_ = llvm::ConstantFP::get(
              get_llvm_type(type_store_, builder_, context_, literal.type_id),
              val.value);
        }
        if constexpr (std::is_same_v<T, tkn::BoolLiteral>) {
          current_value_ = llvm::ConstantInt::get(
              get_llvm_type(type_store_, builder_, context_, literal.type_id),
              val.value);
        }
        if constexpr (std::is_same_v<T, tkn::CharLiteral>) {
          current_value_ = llvm::ConstantInt::get(
              get_llvm_type(type_store_, builder_, context_, literal.type_id),
              val.value);
        }
      },
      *literal.literal);
}

void BuildVisitor::print_module(std::ostream& out) {
  llvm::raw_os_ostream output(out);
  module_->print(output, nullptr);
  output.flush();
}

void BuildVisitor::emit_object(const std::string& output_path) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
  module_->setTargetTriple(triple);

  std::string lookup_error;
  const llvm::Target* target =
      llvm::TargetRegistry::lookupTarget(triple, lookup_error);

  if (!target) {
    throw std::runtime_error("Cannot find LLVM target for triple '" +
                             triple.str() + "': " + lookup_error);
  }

  llvm::TargetOptions options;
  std::unique_ptr<llvm::TargetMachine> target_machine(
      target->createTargetMachine(triple, "generic", "", options,
                                  llvm::Reloc::PIC_, llvm::CodeModel::Small,
                                  llvm::CodeGenOptLevel::Default));

  if (!target_machine) {
    throw std::runtime_error("Failed to create TargetMachine for '" +
                             triple.str() + "'");
  }

  module_->setDataLayout(target_machine->createDataLayout());

  std::error_code file_error;
  llvm::raw_fd_ostream dest(output_path, file_error, llvm::sys::fs::OF_None);

  if (file_error) {
    throw std::runtime_error("Cannot open output file '" + output_path +
                             "': " + file_error.message());
  }

  llvm::legacy::PassManager pass_manager;
  const llvm::CodeGenFileType file_type = llvm::CodeGenFileType::ObjectFile;

  if (target_machine->addPassesToEmitFile(pass_manager, dest, nullptr,
                                          file_type)) {
    throw std::runtime_error("TargetMachine cannot emit an object file for '" +
                             triple.getTriple() + "'");
  }

  pass_manager.run(*module_);
  dest.flush();
}

void BuildVisitor::emit_executable(
    const std::string& output_path,
    const std::vector<std::string>& link_objects) {

  emit_object("out.o");

  std::string command = "gcc out.o ";
  for (auto& object : link_objects) {
    command += object + " ";
  }
  command += " -o " + output_path;

  std::system(command.c_str());

  std::remove("out.o");
}

template <typename T>
  requires is_in_type_tuple_v<T, ast::BinaryNodeTuple>
void BuildVisitor::binary_node_helper(
    const T& binary_node,
    std::function<llvm::Value*(llvm::Value*, llvm::Value*)> accum_function) {
  visit(*binary_node.left);

  if (binary_node.right.empty()) {
    return;
  }

  llvm::Value* accum = current_value_;

  for (auto& [op, expr] : binary_node.right) {
    visit(expr);

    accum = accum_function(accum, current_value_);
  }

  current_value_ = accum;
}
