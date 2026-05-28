#pragma once

#include <unordered_set>

#include <borrow_check/control_flow_graph.hpp>
#include <borrow_check/ir_structures.hpp>
#include <borrow_check/places_state.hpp>

class BorrowChecker {
public:
  void check(const bc_ir::Function& function);

private:
  class State {
  public:
    State() = default;

    State(const State&) = default;

    State& operator=(const State& other) = default;

    void apply(const bc_ir::Instruction& inst);

    void meet(const State& other);

    void join(const State& other);

    void restore_invariants();

    bool operator==(const State& other) const = default;

  private:
    struct Applier {
      Applier(State& state);

      void operator()(const bc_ir::AccessKindVariant&);

      void operator()(const bc_ir::Alloca&);
      void operator()(const bc_ir::BorrowMut&);
      void operator()(const bc_ir::BorrowShared&);

      void operator()(const bc_ir::Read&);
      void operator()(const bc_ir::ReadImmutRef&);
      void operator()(const bc_ir::ReadMutRef&);

      void operator()(const bc_ir::WriteRef&);
      void operator()(const bc_ir::Write&);

      void operator()(const bc_ir::FunctionCallInst&);

      void operator()(const bc_ir::Drop&);

      State& state;
    };

    std::unordered_map<const Symbol*, Status> place_status;
    std::unordered_map<const Symbol*,
                       std::unordered_set<const bc_ir::BorrowShared*>>
        shared_borrows;
    std::unordered_map<const Symbol*, const bc_ir::BorrowMut*> mut_borrow;
  };

  std::unordered_map<const bc_ir::BasicBlock*, State> in_;
  std::unordered_map<const bc_ir::BasicBlock*, State> out_;
};
