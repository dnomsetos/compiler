#include <ranges>

#include <borrow_check/borrow_checker.hpp>
#include <borrow_check/places_state.hpp>

void BorrowChecker::State::apply(const bc_ir::Instruction& inst) {
  Applier applier{*this};

  std::visit(applier, inst.inst);
}

void BorrowChecker::State::meet(const BorrowChecker::State& other) {

  for (auto& key : other.place_status | std::views::keys) {
    if (!place_status.contains(key)) {
      place_status.emplace(key, Status::Undefined);
    }
  }

  for (auto& [key, value] : place_status) {
    if (!other.place_status.contains(key)) {
      value = ::meet(value, Status::Undefined);
      continue;
    }
  }

  restore_invariants();
}

void BorrowChecker::State::join(const BorrowChecker::State& other) {

  for (auto& key : other.place_status | std::views::keys) {
    if (!place_status.contains(key)) {
      place_status.emplace(key, Status::Undefined);
    }
  }

  for (auto& [key, value] : place_status) {
    if (!other.place_status.contains(key)) {
      value = ::join(value, Status::Undefined);
      continue;
    }
  }

  restore_invariants();
}

void BorrowChecker::State::restore_invariants() {

  for (auto& [key, value] : place_status) {
    if (value == Status::Dead || value == Status::Live ||
        value == Status::Undefined) {

      mut_borrow[key] = nullptr;
      shared_borrows[key].clear();

    } else if (value == Status::MutBorrowed) {
      shared_borrows[key].clear();

    } else if (value == Status::SharedBorrowed) {
      mut_borrow[key] = nullptr;
    }
  }
}

void BorrowChecker::check(const bc_ir::Function& function) {
  in_.clear();
  out_.clear();

  for (auto* block : function.blocks) {
    in_.try_emplace(block);
    out_.try_emplace(block);
  }

  std::unordered_set<bc_ir::BasicBlock*> worklist{function.entry.get()};

  while (!worklist.empty()) {
    bc_ir::BasicBlock* block = *worklist.begin();
    worklist.erase(block);

    State in_state{block->incoming_edges.empty()
                       ? State{}
                       : out_.at(block->incoming_edges.front().lock().get())};

    for (auto& pred : block->incoming_edges | std::views::drop(1)) {
      bc_ir::BasicBlock* pred_block = pred.lock().get();

      in_state.meet(out_.at(pred_block));
    }

    for (auto& inst : block->instructions) {
      in_state.apply(inst);
    }

    if (out_.at(block) != in_state) {
      out_.at(block) = in_state;

      std::visit(overloaded{[&](bc_ir::UnconditionalBranchInst& inst) {
                              worklist.emplace(inst.target.get());
                            },

                            [&](bc_ir::SwitchInst& inst) {
                              for (auto& case_block : inst.cases) {
                                worklist.emplace(case_block.get());
                              }
                            },

                            [&](bc_ir::ReturnInst&) {}},
                 block->terminator.get()->terminator);
    }
  }
}
