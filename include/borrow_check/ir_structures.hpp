#pragma once

#include <deque>
#include <vector>

#include <semantic_analysis/symbol_table.hpp>
#include <utility/type_tuple.hpp>

#define ENABLE_DEBUG

namespace bc_ir {

#ifdef ENABLE_DEBUG
using debug_string = std::string;
#else
using debug_string = Dummy;
#endif

struct Dummy {
  template <typename T> consteval Dummy(T) {}
};

struct Alloca {
  tkn::Position position;
  const Symbol* variable;
  [[no_unique_address]] debug_string name;
};

struct BorrowShared {
  tkn::Position position;
  const Symbol* reference;
  const Symbol* resource;
  [[no_unique_address]] debug_string name;
  [[no_unique_address]] debug_string resource_name;
};

struct BorrowMut {
  tkn::Position position;
  const Symbol* reference;
  const Symbol* resource;
  [[no_unique_address]] debug_string name;
  [[no_unique_address]] debug_string resource_name;
};

using AccessKindTypeTuple = TypeTuple<Alloca, BorrowShared, BorrowMut>;
using AccessKindVariant = type_tuple_to_variant_t<AccessKindTypeTuple>;

struct Read {
  tkn::Position position;
  Alloca* target;
};

struct Write {
  tkn::Position position;
  Alloca* target;
};

struct ReadImmutRef {
  tkn::Position position;
  BorrowShared* target;
};

struct ReadMutRef {
  tkn::Position position;
  BorrowMut* target;
};

struct WriteRef {
  tkn::Position position;
  BorrowMut* target;
};

struct Drop {
  tkn::Position position;
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
  tkn::Position position;
  const Symbol* symbol;
  std::vector<ArgBinding> arg_bindings;
  [[no_unique_address]] debug_string name;
};

using StateChangeTypeTuple = TypeTuple<Read, Write, ReadImmutRef, ReadMutRef,
                                       WriteRef, Drop, FunctionCallInst>;

struct BasicBlock;

struct UnconditionalBranchInst {
  BasicBlock* target;
};

struct SwitchInst {
  std::pmr::vector<BasicBlock*> cases{&alloc::mr};
};

struct ReturnInst {};

struct DummyTerminator {};

using TerminatorTypeTuple =
    TypeTuple<UnconditionalBranchInst, SwitchInst, ReturnInst, DummyTerminator>;
using TerminatorVariant = type_tuple_to_variant_t<TerminatorTypeTuple>;

struct Terminator {
  Terminator(UnconditionalBranchInst&&);
  Terminator(SwitchInst&&);
  Terminator(ReturnInst&&);
  Terminator(DummyTerminator&&);

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
  std::vector<BasicBlock*> incoming_edges{};
  Terminator terminator{DummyTerminator{}};
  std::deque<Instruction> instructions{};
  [[no_unique_address]] debug_string name;
};

struct Function {
  const Symbol* symbol;
  std::deque<BasicBlock> blocks{};
  BasicBlock* entry{nullptr};
  std::deque<FunctionArgPlaceholder> arg_placeholders{};
  [[no_unique_address]] debug_string name;
};

}; // namespace bc_ir
