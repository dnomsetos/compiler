#pragma once

#include <unordered_set>

#include <borrow_check/control_flow_graph.hpp>
#include <borrow_check/ir_structures.hpp>

class BorrowChecker {
private:
  enum class Status : std::uint8_t {
    Live,
    Dead,
    SharedBorrowed,
    MutBorrowed,
  };

  struct CheckerState {
    std::unordered_map<const Symbol*, Status> place_status;
    std::unordered_map<const Symbol*, std::unordered_set<ir::BorrowShared*>>
        shared_borrows;
    std::unordered_map<const Symbol*, ir::BorrowMut*> mut_borrow;

    bool operator==(const CheckerState& other) const = default;
  };

public:
  void check(ir::BasicBlock* block, CheckerState parent_state);

private:
  std::unordered_map<ir::BasicBlock*, CheckerState> states_;
};
