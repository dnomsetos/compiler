#include <stdexcept>
#include <string>

#include <semantic_analysis/symbol_table.hpp>

SymbolTable::SymbolTable(std::size_t index, SymbolTable* parent,
                         ScopeVariant&& scope)
    : index_{index}, parent_{parent}, scope_{scope} {}

bool SymbolTable::insert_variable(const std::string& name, Symbol&& symbol) {
  return variable_symbols_.emplace(name, symbol).second;
}

bool SymbolTable::insert_function(const std::string& name, Symbol&& symbol) {
  return function_symbols_.emplace(name, symbol).second;
}

auto SymbolTable::get_variable_symbol(const std::string& name) const
    -> const Symbol* {
  if (variable_symbols_.contains(name)) {
    return &variable_symbols_.at(name);
  }

  if (parent_ == nullptr) {
    throw std::runtime_error("variable " + name + " is not defined");
  }

  return parent_->get_variable_symbol(name);
}

auto SymbolTable::get_function_symbol(const std::string& name) const
    -> const Symbol* {
  if (function_symbols_.contains(name)) {
    return &function_symbols_.at(name);
  }

  if (parent_ == nullptr) {
    throw std::runtime_error("function " + name + " is not defined");
  }

  return parent_->get_function_symbol(name);
}

auto SymbolTable::get_variable_symbol_in_position(
    const std::string& name, const tkn::Position& position) const
    -> const Symbol* {
  auto* result = get_variable_symbol(name);
  if (position.start.line < result->position.start.line ||
      (position.start.line == result->position.start.line &&
       position.start.offset < result->position.start.offset)) {
    throw std::runtime_error("variable " + name + " is not defined in line " +
                             std::to_string(position.start.line));
  }
  return result;
}

auto SymbolTable::get_function_symbol_in_position(
    const std::string& name, const tkn::Position& position) const
    -> const Symbol* {
  auto* result = get_function_symbol(name);
  if (position.start.line < result->position.start.line ||
      (position.start.line == result->position.start.line &&
       position.start.offset < result->position.start.offset)) {
    throw std::runtime_error("function " + name + " is not defined in line " +
                             std::to_string(position.start.line));
  }
  return result;
}

bool SymbolTable::check_variable_availability(const std::string& name) const {
  return variable_symbols_.contains(name) ||
         (parent_ == nullptr ? false
                             : parent_->check_variable_availability(name));
}

bool SymbolTable::check_function_availability(const std::string& name) const {
  return function_symbols_.contains(name) ||
         (parent_ == nullptr ? false
                             : parent_->check_function_availability(name));
}

const SymbolTable* SymbolTable::get_parent() const { return parent_; }

SymbolTable::ScopeVariant& SymbolTable::get_scope() { return scope_; }

SymbolTable* SymbolTable::create_simple_child() {
  children_.push_back(alloc::make_unique_pmr<SymbolTable>(
      children_.size(), this, SimpleScope{}));
  return &*children_.back();
}

SymbolTable* SymbolTable::create_loop_child(tp::TypeId expected_type) {
  children_.push_back(
      alloc::make_unique_pmr<SymbolTable>(children_.size(), this, LoopScope{}));

  std::get<LoopScope>(children_.back()->get_scope()).expected_type =
      expected_type;

  return &*children_.back();
}

SymbolTable* SymbolTable::create_loop_child(const std::string& label,
                                            tp::TypeId expected_type) {
  children_.push_back(
      alloc::make_unique_pmr<SymbolTable>(children_.size(), this, LoopScope{}));

  std::get<LoopScope>(children_.back()->get_scope()).expected_type =
      expected_type;
  std::get<LoopScope>(children_.back()->get_scope()).label_name = label;

  return &*children_.back();
}

SymbolTable* SymbolTable::create_function_child() {
  children_.push_back(alloc::make_unique_pmr<SymbolTable>(
      children_.size(), this, FunctionScope{}));

  return &*children_.back();
}

SymbolTable* SymbolTable::find_nearest_function() {
  if (std::holds_alternative<FunctionScope>(scope_)) {
    return this;
  }

  if (parent_ == nullptr) {
    return nullptr;
  }

  return parent_->find_nearest_function();
}

SymbolTable* SymbolTable::find_nearest_loop() {
  if (std::holds_alternative<LoopScope>(scope_)) {
    return this;
  }

  if (parent_ == nullptr) {
    return nullptr;
  }

  return parent_->find_nearest_loop();
}

SymbolTable* SymbolTable::find_loop_by_label(const std::string& label) {
  if (std::holds_alternative<LoopScope>(scope_) &&
      std::get<LoopScope>(scope_).label_name == label) {
    return this;
  }

  if (parent_ == nullptr) {
    return nullptr;
  }

  return parent_->find_loop_by_label(label);
}

GlobalSymbolTable::GlobalSymbolTable(TypeStore& type_store) : root_{} {
  auto add_symbol = [&]<typename T>(const std::string& name) {
    Symbol symbol{
        .position = tkn::Position{0, 0, 0},
        .type = type_store.new_function(type_store.new_basic_type(tp::Void{}),
                                        {type_store.new_basic_type(T{})}),

        .scope = &root_,
        .definition = nullptr};
    root_.insert_function(name, std::move(symbol));
  };
  add_symbol.template operator()<tp::I8>("print_i8");
  add_symbol.template operator()<tp::I16>("print_i16");
  add_symbol.template operator()<tp::I32>("print_i32");
  add_symbol.template operator()<tp::I64>("print_i64");
  add_symbol.template operator()<tp::U8>("print_u8");
  add_symbol.template operator()<tp::U16>("print_u16");
  add_symbol.template operator()<tp::U32>("print_u32");
  add_symbol.template operator()<tp::U64>("print_u64");
  add_symbol.template operator()<tp::F32>("print_f32");
  add_symbol.template operator()<tp::F64>("print_f64");
  add_symbol.template operator()<tp::Char>("print_char");
  add_symbol.template operator()<tp::Bool>("print_bool");
  add_symbol.template operator()<tp::Void>("print_void");
}

SymbolTable* GlobalSymbolTable::get_root() { return &root_; }
