#include <borrow_check/borrow_checker.hpp>

BorrowChecker::State::Applier::Applier(BorrowChecker::State& state)
    : state{state} {}

void BorrowChecker::State::Applier::operator()(
    const bc_ir::AccessKindVariant& inst) {

  std::visit(*this, inst);
}

void BorrowChecker::State::Applier::operator()(const bc_ir::Alloca& inst) {
  if (state.place_status.contains(inst.variable) &&
      state.place_status.at(inst.variable) != Status::Undefined) {
    throw std::runtime_error("Attempt to declare variable that is "
                             "already defined at " +
                             inst.position.to_string());
  }

  state.place_status.emplace(inst.variable, Status::Live);
}

void BorrowChecker::State::Applier::operator()(const bc_ir::BorrowMut& inst) {
  Status& resource_status = state.place_status.at(inst.resource);

  switch (resource_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to borrow undefined resource at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to borrow dead resource at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    throw std::runtime_error(
        "Attempt to borrow resource that is already shared borrowed at " +
        inst.position.to_string());

  case Status::MutBorrowed:
    throw std::runtime_error(
        "Attempt to borrow resource that is already mutably borrowed at " +
        inst.position.to_string());

  case Status::UnknownBorowed:
    throw std::runtime_error(
        "Attempt to borrow resource that is already unknown borrowed at " +
        inst.position.to_string());

  case Status::Live:
    resource_status = Status::MutBorrowed;
    state.mut_borrow[inst.resource] = &inst;
    state.shared_borrows[inst.resource].clear();
    break;
  }
}

void BorrowChecker::State::Applier::operator()(
    const bc_ir::BorrowShared& inst) {
  Status& resource_status = state.place_status.at(inst.resource);

  switch (resource_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to borrow undefined resource at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to borrow dead resource at " +
                             inst.position.to_string());

  case Status::MutBorrowed:
    throw std::runtime_error(
        "Attempt to borrow resource that is already mutably borrowed at " +
        inst.position.to_string());

  case Status::SharedBorrowed:
    state.shared_borrows[inst.resource].insert(&inst);
    break;

  case Status::UnknownBorowed:
  case Status::Live:
    resource_status = Status::SharedBorrowed;
    state.mut_borrow[inst.resource] = nullptr;
    state.shared_borrows[inst.resource].insert(&inst);
    break;
  }
}

void BorrowChecker::State::Applier::operator()(const bc_ir::Read& inst) {
  Status& variable_status = state.place_status.at(inst.target->variable);

  switch (variable_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to read undefined variable at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to read dead variable at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    // OK
    break;

  case Status::MutBorrowed:
    state.mut_borrow[inst.target->variable] = nullptr;
    variable_status = Status::Live;
    break;

  case Status::UnknownBorowed:
    state.mut_borrow[inst.target->variable] = nullptr;
    variable_status = Status::SharedBorrowed;
    break;

  case Status::Live:
    // OK
    break;
  }
}

void BorrowChecker::State::Applier::operator()(
    const bc_ir::ReadImmutRef& inst) {
  Status& variable_status = state.place_status.at(inst.target->resource);

  switch (variable_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to read undefined variable at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to read dead variable at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    if (!state.shared_borrows[inst.target->resource].contains(inst.target)) {
      throw std::runtime_error(
          "Attempt to read variable with use immutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }
    break;

  case Status::MutBorrowed:
    throw std::runtime_error(
        "Attempt to read variable with use immutable reference "
        "that is mutably borrowed at " +
        inst.position.to_string());

  case Status::UnknownBorowed:
    if (!state.shared_borrows[inst.target->resource].contains(inst.target)) {
      throw std::runtime_error(
          "Attempt to read variable with use immutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }

    variable_status = Status::SharedBorrowed;
    state.mut_borrow[inst.target->resource] = nullptr;
    break;

  case Status::Live:
    throw std::runtime_error(
        "Attempt to read variable that is not borrowed at " +
        inst.position.to_string());
  }
}

void BorrowChecker::State::Applier::operator()(const bc_ir::ReadMutRef& inst) {
  Status& variable_status = state.place_status.at(inst.target->resource);

  switch (variable_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to read undefined variable at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to read dead variable at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    throw std::runtime_error(
        "Attempt to read variable with use mutable reference, "
        "but this reference is shared borrowed at " +
        inst.position.to_string());

  case Status::MutBorrowed:
    if (state.mut_borrow[inst.target->resource] != inst.target) {
      throw std::runtime_error(
          "Attempt to read variable with use mutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }
    break;

  case Status::UnknownBorowed:
    if (state.mut_borrow[inst.target->resource] != inst.target) {
      throw std::runtime_error(
          "Attempt to read variable with use mutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }

    variable_status = Status::MutBorrowed;
    state.shared_borrows[inst.target->resource].clear();
    break;

  case Status::Live:
    throw std::runtime_error(
        "Attempt to read variable that is not borrowed at " +
        inst.position.to_string());
  }
}

void BorrowChecker::State::Applier::operator()(const bc_ir::Write& inst) {
  Status& variable_status = state.place_status.at(inst.target->variable);

  switch (variable_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to write to undefined variable at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to write to dead variable at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    state.shared_borrows[inst.target->variable].clear();
    variable_status = Status::Live;
    break;

  case Status::MutBorrowed:
    state.mut_borrow[inst.target->variable] = nullptr;
    variable_status = Status::Live;
    break;

  case Status::UnknownBorowed:
    state.shared_borrows[inst.target->variable].clear();
    state.mut_borrow[inst.target->variable] = nullptr;
    variable_status = Status::Live;
    break;

  case Status::Live:
    break;
  }
}

void BorrowChecker::State::Applier::operator()(const bc_ir::WriteRef& inst) {
  Status& variable_status = state.place_status.at(inst.target->resource);

  switch (variable_status) {
  case Status::Undefined:
    throw std::runtime_error("Attempt to write to undefined variable at " +
                             inst.position.to_string());

  case Status::Dead:
    throw std::runtime_error("Attempt to write to dead variable at " +
                             inst.position.to_string());

  case Status::SharedBorrowed:
    throw std::runtime_error(
        "Attempt to write to variable with use mutable reference, "
        "but this reference is shared borrowed at " +
        inst.position.to_string());

  case Status::MutBorrowed:
    if (state.mut_borrow[inst.target->resource] != inst.target) {
      throw std::runtime_error(
          "Attempt to write to variable with use mutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }
    break;

  case Status::UnknownBorowed:
    if (state.mut_borrow[inst.target->resource] != inst.target) {
      throw std::runtime_error(
          "Attempt to write to variable with use mutable reference, "
          "but this reference is dead at " +
          inst.position.to_string());
    }

    variable_status = Status::MutBorrowed;
    state.shared_borrows[inst.target->resource].clear();
    break;

  case Status::Live:
    throw std::runtime_error(
        "Attempt to write to variable that is not borrowed at " +
        inst.position.to_string());
  }
}

void BorrowChecker::State::Applier::operator()(const bc_ir::FunctionCallInst&) {
}

void BorrowChecker::State::Applier::operator()(const bc_ir::Drop& inst) {
  state.place_status[inst.target->variable] = Status::Dead;
  state.shared_borrows[inst.target->variable].clear();
  state.mut_borrow[inst.target->variable] = nullptr;
}
