#pragma once

#include <unordered_map>
#include <vector>

#include <parser/ast.hpp>
#include <semantic_analysis/type_storage.hpp>
#include <semantic_analysis/types.hpp>
#include <utility/allocator.hpp>

class SymbolTable;

class GlobalSymbolTable;

class TypeStore;

struct Symbol {
  tkn::Position position;
  tp::TypeId type;
  SymbolTable* scope;
  ast::FunctionDefinitionNode* definition;
  bool is_defined = false;
};

class SymbolTable {
public:
  struct FunctionScope {
    tp::TypeId return_type;
  };

  struct LoopScope {
    tp::TypeId result_type = tp::no_type_id;
    tp::TypeId expected_type;
    std::optional<std::string> label_name;
  };

  struct SimpleScope {};

  using ScopeTypeTuple = TypeTuple<FunctionScope, LoopScope, SimpleScope>;

  using ScopeVariant = type_tuple_to_variant_t<ScopeTypeTuple>;

  SymbolTable(std::size_t index = 0, SymbolTable* parent = nullptr,
              ScopeVariant&& scope = SimpleScope{});

  bool insert_variable(const std::string& name, Symbol&& symbol);

  bool insert_function(const std::string& name, Symbol&& symbol);

  friend GlobalSymbolTable;

  auto get_variable_symbol_maybe_undefined(const std::string& name) const
      -> const Symbol*;

  auto get_variable_symbol(const std::string& name) const -> const Symbol*;

  auto get_function_symbol(const std::string& name) const -> const Symbol*;

  auto get_variable_symbol_in_position_maybe_undefined(
      const std::string& name, const tkn::Position& position) const
      -> const Symbol*;

  auto get_variable_symbol_in_position(const std::string& name,
                                       const tkn::Position& position) const
      -> const Symbol*;

  auto get_function_symbol_in_position(const std::string& name,
                                       const tkn::Position& position) const
      -> const Symbol*;

  bool check_variable_availability(const std::string& name) const;

  bool check_function_availability(const std::string& name) const;

  const SymbolTable* get_parent() const;

  ScopeVariant& get_scope();

  SymbolTable* create_simple_child();

  SymbolTable* create_loop_child(tp::TypeId expected_type);

  SymbolTable* create_loop_child(const std::string& label,
                                 tp::TypeId expected_type);

  SymbolTable* create_function_child();

  SymbolTable* find_nearest_function();

  SymbolTable* find_nearest_loop();

  SymbolTable* find_loop_by_label(const std::string& label);

  void change_symbol_type(const std::string& name, tp::TypeId type);

  void define_symbol(const std::string& name);

private:
  std::size_t index_;
  SymbolTable* parent_ = nullptr;
  std::pmr::vector<alloc::pmr_unique_ptr<SymbolTable>> children_{&alloc::mr};
  std::unordered_map<std::string, Symbol> variable_symbols_;
  std::unordered_map<std::string, Symbol> function_symbols_;
  ScopeVariant scope_;
};

class GlobalSymbolTable {
public:
  explicit GlobalSymbolTable(TypeStore& type_store);

  SymbolTable* get_root();

private:
  SymbolTable root_;
};
