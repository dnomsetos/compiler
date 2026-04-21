#pragma once

#include <vector>

#include <parser/ast.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/types.hpp>

class TypeStore {
public:
  TypeStore() = default;

  tp::TypeId new_var(tp::TypeId parent = tp::no_type_id);

  tp::TypeId new_function(tp::TypeId return_type = tp::no_type_id,
                          std::vector<tp::TypeId>&& args = {});

  tp::TypeId new_literal_type(type_tuple_to_variant_t<tp::LiteralTypeTuple>);

  tp::TypeId new_basic_type(tp::BasicTypeVariant);

  tp::TypeId resolve(tp::TypeId type_id);

  bool unify(tp::TypeId type1, tp::TypeId type2);

  tp::Type& get_type(tp::TypeId type_id);

private:
  // key is index in BasicTypeVariant
  std::unordered_map<std::size_t, tp::TypeId> basic_type_cache_;
  std::vector<tp::Type> types_;
};
