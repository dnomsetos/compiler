#include <borrow_check/ir_structures.hpp>

namespace ir {

Terminator::Terminator(UnconditionalBranchInst&& inst)
    : terminator{std::move(inst)} {}
Terminator::Terminator(SwitchInst&& inst) : terminator{std::move(inst)} {}
Terminator::Terminator(ReturnInst&& inst) : terminator{std::move(inst)} {}

Instruction::Instruction(Alloca&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(BorrowShared&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(BorrowMut&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(Write&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(ReadImmutRef&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(ReadMutRef&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(WriteRef&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(FunctionCallInst&& inst) : inst{std::move(inst)} {}
Instruction::Instruction(Drop&& inst) : inst{std::move(inst)} {}

} // namespace ir
