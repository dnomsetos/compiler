#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include <parser/ast.hpp>
#include <semantic_analysis/symbol_table.hpp>
#include <semantic_analysis/types.hpp>

struct FunctionTypeHash {
  std::size_t operator()(const tp::FunctionType& f) const {
    std::size_t seed = std::hash<std::size_t>{}(f.return_type);

    for (const auto& x : f.args) {
      seed ^= std::hash<std::size_t>{}(x) + 0x9e3779b97f4a7c15ULL +
              (seed << 6) + (seed >> 2);
    }

    return seed;
  }
};

class TypeStore {
public:
  TypeStore() = default;

  // tp::TypeId new_var(tp::TypeId parent = tp::no_type_id);

  tp::TypeId get_function(tp::TypeId return_type = tp::no_type_id,
                          std::vector<tp::TypeId>&& args = {});

  tp::TypeId new_literal_type(tp::LiteralVariant&&);

  tp::TypeId new_undefined_type();

  tp::TypeId get_basic_type(tp::BasicTypeVariant);

  tp::TypeId resolve(tp::TypeId type_id);

  bool unify(tp::TypeId type1, tp::TypeId type2);

  const tp::Type& get_type(tp::TypeId type_id);

  void add_ast_type(ast::ASTTypeNode* ast_type);

  void add_ast_var(ast::IdentifierNode* ast_var);

  bool is_integer_type(tp::TypeId type_id);

  bool is_float_type(tp::TypeId type_id);

  void handle_ast_types();

private:
  tp::Type& get_mutable_type(tp::TypeId type_id);

  // key is index in BasicTypeVariant
  using container_type_t = std::array<tp::Type, tp::basic_type_count>;

  inline static const container_type_t basic_types_ = [] {
    container_type_t result;
    std::ranges::transform(
        tp::basic_type_names, result.begin(),
        [](auto&& val) -> tp::Type { return tp::Type{val.second}; });
    return result;
  }();

  std::vector<tp::Type> types_;
  std::unordered_map<tp::FunctionType, tp::TypeId, FunctionTypeHash>
      functions_types_;
  std::vector<ast::ASTTypeNode*> ast_types_;
  std::vector<ast::IdentifierNode*> ast_vars_;
};
