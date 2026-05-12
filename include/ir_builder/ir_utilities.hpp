#pragma once

#include <semantic_analysis/type_storage.hpp>
#include <semantic_analysis/types.hpp>

#include <llvm-22/llvm/IR/IRBuilder.h>
#include <llvm-22/llvm/IR/Type.h>

llvm::Type* get_llvm_type(TypeStore& type_store, llvm::IRBuilder<>& builder,
                          llvm::LLVMContext& context, tp::TypeId type_id);
