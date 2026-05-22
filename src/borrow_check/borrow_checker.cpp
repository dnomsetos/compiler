#include "borrow_check/ir_structures.hpp"
#include "utility/type_tuple.hpp"
#include <borrow_check/borrow_checker.hpp>
#include <stdexcept>

void BorrowChecker::check(ir::BasicBlock* block, CheckerState parent_state) {
  if (!states_.contains(block)) {
    states_.emplace(block, CheckerState{});
  }

  CheckerState& prev_state = states_.at(block);

  CheckerState& state = parent_state;

  for (auto& inst : block->instructions) {
    std::visit(
        overloaded{
            [&](ir::AccessKindVariant& inst) {
              std::visit(
                  overloaded{
                      [&](ir::BorrowShared& inst) {
                        Status& resource_status =
                            state.place_status.at(inst.resource);

                        if (resource_status == Status::Dead) {
                          throw std::runtime_error(
                              "Attempt to borrow dead resource at ");

                        } else if (resource_status == Status::MutBorrowed) {
                          throw std::runtime_error(
                              "Attempt to borrow resource that is "
                              "already mutably borrowed at ");
                        } else if (resource_status == Status::SharedBorrowed) {
                          state.shared_borrows[inst.resource].insert(&inst);
                        } else if (resource_status == Status::Live) {
                          resource_status = Status::SharedBorrowed;
                          state.mut_borrow[inst.resource] = nullptr;
                          state.shared_borrows[inst.resource].insert(&inst);
                        }
                      },

                      [&](ir::BorrowMut& inst) {
                        Status& resource_status =
                            state.place_status.at(inst.resource);

                        if (resource_status == Status::Dead) {
                          throw std::runtime_error(
                              "Attempt to borrow dead resource at ");

                        } else if (resource_status == Status::SharedBorrowed) {
                          throw std::runtime_error(
                              "Attempt to borrow resource that is "
                              "already shared borrowed at ");
                        } else if (resource_status == Status::MutBorrowed) {
                          throw std::runtime_error(
                              "Attempt to borrow resource that is "
                              "already mutably borrowed at ");
                        } else if (resource_status == Status::Live) {
                          resource_status = Status::MutBorrowed;
                          state.mut_borrow[inst.resource] = &inst;
                          state.shared_borrows[inst.resource].clear();
                        }
                      },

                      [&](ir::Alloca& inst) {
                        if (state.place_status.contains(inst.variable)) {
                          throw std::runtime_error(
                              "Attempt to declare variable that is "
                              "already defined at ");
                        }

                        state.place_status.emplace(inst.variable, Status::Live);
                      },

                      [&](ir::Read& inst) {
                        Status& variable_status =
                            state.place_status.at(inst.target->variable);

                        if (variable_status == Status::Dead) {
                          throw std::runtime_error(
                              "Attempt to read dead variable at ");

                        } else if (variable_status == Status::SharedBorrowed) {
                          state.shared_borrows[inst.target->variable].clear();
                        } else if (variable_status == Status::MutBorrowed) {
                          state.mut_borrow[inst.target->variable] = nullptr;
                          variable_status = Status::Live;
                        } else if (variable_status == Status::Live) {
                        }
                      },

                  },
                  inst);
            },

            [&](ir::Read& inst) {
              Status& variable_status =
                  state.place_status.at(inst.target->variable);

              if (variable_status == Status::Dead) {
                throw std::runtime_error("Attempt to read dead variable at ");
              } else if (variable_status == Status::SharedBorrowed) {
                variable_status = Status::Live;
              } else if (variable_status == Status::MutBorrowed) {
                state.mut_borrow[inst.target->variable] = nullptr;
                variable_status = Status::Live;
              } else if (variable_status == Status::Live) {
              }
            },

            [&](ir::Write& inst) {
              Status& variable_status =
                  state.place_status.at(inst.target->variable);

              if (variable_status == Status::Dead) {
                throw std::runtime_error(
                    "Attempt to write to dead variable at ");
              } else if (variable_status == Status::SharedBorrowed) {
                state.shared_borrows[inst.target->variable].clear();
                variable_status = Status::Live;
              } else if (variable_status == Status::MutBorrowed) {
                state.mut_borrow[inst.target->variable] = nullptr;
                variable_status = Status::Live;
              } else if (variable_status == Status::Live) {
              }
            },

            [&](ir::ReadImmutRef& inst) {
              Status& variable_status =
                  state.place_status.at(inst.target->resource);

              if (variable_status == Status::Dead) {
                throw std::runtime_error("Attempt to read dead variable at ");
              } else if (variable_status == Status::SharedBorrowed) {
                if (!state.shared_borrows[inst.target->resource].contains(
                        inst.target)) {
                  throw std::runtime_error(
                      "Attempt to read variable with use immutable reference, "
                      "but this reference is dead at ");
                }
              } else if (variable_status == Status::MutBorrowed) {
                throw std::runtime_error(
                    "Attempt to read variable with use immutable reference "
                    "that is mutably borrowed at ");
              } else if (variable_status == Status::Live) {
                throw std::runtime_error("Attempt to read variable "
                                         "that is not borrowed at ");
              }
            },

            [&](ir::ReadMutRef& inst) {
              Status& variable_status =
                  state.place_status.at(inst.target->resource);

              if (variable_status == Status::Dead) {
                throw std::runtime_error("Attempt to read dead variable at ");
              } else if (variable_status == Status::SharedBorrowed) {
                throw std::runtime_error(
                    "Attempt to read variable with use mutable reference, "
                    "but this reference is shared borrowed at ");
              } else if (variable_status == Status::MutBorrowed) {
                if (state.mut_borrow[inst.target->resource] != inst.target) {
                  throw std::runtime_error(
                      "Attempt to read variable with use mutable reference, "
                      "but this reference is dead at ");
                }
              } else if (variable_status == Status::Live) {
                throw std::runtime_error("Attempt to read variable "
                                         "that is not borrowed at ");
              }
            },

            [&](ir::WriteRef& inst) {
              Status& variable_status =
                  state.place_status.at(inst.target->resource);

              if (variable_status == Status::Dead) {
                throw std::runtime_error(
                    "Attempt to write to dead variable at ");
              } else if (variable_status == Status::SharedBorrowed) {
                throw std::runtime_error(
                    "Attempt to write to variable with use mutable reference, "
                    "but this reference is shared borrowed at ");
              } else if (variable_status == Status::MutBorrowed) {
                if (state.mut_borrow[inst.target->resource] != inst.target) {
                  throw std::runtime_error("Attempt to write to variable with "
                                           "use mutable reference, "
                                           "but this reference is dead at ");
                }
              } else if (variable_status == Status::Live) {
                throw std::runtime_error("Attempt to write to variable "
                                         "that is not borrowed at ");
              }
            },

            [&](ir::Drop& inst) {
              state.place_status[inst.target->variable] = Status::Dead;
              state.shared_borrows[inst.target->variable].clear();
              state.mut_borrow[inst.target->variable] = nullptr;
            },

            [&](ir::FunctionCallInst&) {}},
        inst.inst);
  }

  if (state != prev_state) {
    states_[block] = state;
    std::visit(overloaded{[&](ir::UnconditionalBranchInst& inst) {
                            check(inst.target.get(), state);
                          },
                          [&](ir::SwitchInst& inst) {
                            for (auto& target : inst.cases) {
                              check(target.get(), state);
                            }
                          },
                          [&](ir::ReturnInst&) {}},
               block->terminator.get()->terminator);
  }
}
