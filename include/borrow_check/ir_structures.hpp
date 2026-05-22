#pragma once

#include <deque>
#include <vector>

#include <semantic_analysis/symbol_table.hpp>
#include <type_traits>
#include <utility/allocator.hpp>
#include <utility/type_tuple.hpp>

#define ENABLE_DEBUG true

namespace ir {

struct Dummy {
  template <typename T> consteval Dummy(T) {}
};

using debug_string = std::conditional_t<ENABLE_DEBUG, std::string, Dummy>;

struct Alloca {
  const Symbol* variable;
  [[no_unique_address]] debug_string name;
};

struct BorrowShared {
  const Symbol* reference;
  const Symbol* resource;
  [[no_unique_address]] debug_string name;
  [[no_unique_address]] debug_string resource_name;
};

struct BorrowMut {
  const Symbol* reference;
  const Symbol* resource;
  [[no_unique_address]] debug_string name;
  [[no_unique_address]] debug_string resource_name;
};

using AccessKindTypeTuple = TypeTuple<Alloca, BorrowShared, BorrowMut>;
using AccessKindVariant = type_tuple_to_variant_t<AccessKindTypeTuple>;

struct Read {
  Alloca* target;
};

struct Write {
  Alloca* target;
};

struct ReadImmutRef {
  BorrowShared* target;
};

struct ReadMutRef {
  BorrowMut* target;
};

struct WriteRef {
  BorrowMut* target;
};

struct Drop {
  Alloca* target;
};

struct FunctionArgPlaceholder {
  std::size_t index;
  Symbol phantom_symbol;
  [[no_unique_address]] debug_string param_name;
};

struct ArgBinding {
  const Symbol* param;
  const Symbol* resource;
  bool is_mut;
  [[no_unique_address]] debug_string param_name;
  [[no_unique_address]] debug_string resource_name;
};

struct FunctionCallInst {
  const Symbol* symbol;
  std::vector<ArgBinding> arg_bindings;
  [[no_unique_address]] debug_string name;
};

using StateChangeTypeTuple = TypeTuple<Read, Write, ReadImmutRef, ReadMutRef,
                                       WriteRef, Drop, FunctionCallInst>;

struct BasicBlock;

struct UnconditionalBranchInst {
  alloc::pmr_shared_ptr<BasicBlock> target;
};

struct SwitchInst {
  std::deque<alloc::pmr_shared_ptr<BasicBlock>> cases;
};

struct ReturnInst {};

using TerminatorTypeTuple =
    TypeTuple<UnconditionalBranchInst, SwitchInst, ReturnInst>;
using TerminatorVariant = type_tuple_to_variant_t<TerminatorTypeTuple>;

struct Terminator {
  Terminator(UnconditionalBranchInst&&);
  Terminator(SwitchInst&&);
  Terminator(ReturnInst&&);

  TerminatorVariant terminator;
};

using InstructionTypeTuple =
    type_tuple_concat_t<TypeTuple<AccessKindVariant>, StateChangeTypeTuple>;
using InstructionVariant = type_tuple_to_variant_t<InstructionTypeTuple>;

struct Instruction {
  Instruction(Alloca&&);
  Instruction(BorrowShared&&);
  Instruction(BorrowMut&&);
  Instruction(Read&&);
  Instruction(Write&&);
  Instruction(ReadImmutRef&&);
  Instruction(ReadMutRef&&);
  Instruction(WriteRef&&);
  Instruction(FunctionCallInst&&);
  Instruction(Drop&&);

  InstructionVariant inst;
};

struct BasicBlock {
  alloc::pmr_unique_ptr<Terminator> terminator{nullptr};
  std::deque<Instruction> instructions;
  [[no_unique_address]] debug_string name;
};

struct Function {
  const Symbol* symbol;
  alloc::pmr_unique_ptr<BasicBlock> entry;
  std::deque<FunctionArgPlaceholder> arg_placeholders;
  [[no_unique_address]] debug_string name;
};

}; // namespace ir
