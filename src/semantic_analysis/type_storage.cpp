#include "semantic_analysis/types.hpp"
#include <iostream>
#include <stacktrace>

#include <semantic_analysis/type_storage.hpp>

// tp::TypeId TypeStore::new_var(tp::TypeId parent) {
//   tp::TypeId type_id = types_.size();
//
//   types_.emplace_back(tp::VariableType{.parent = parent});
//
//   return type_id;
// }

tp::TypeId TypeStore::get_function(tp::TypeId return_type,
                                   std::vector<tp::TypeId>&& args) {
  tp::FunctionType function_type(std::move(args), return_type);

  // std::cout << "function type args size: " << function_type.args.size()
  //           << std::endl;
  //
  // std::cout << std::stacktrace::current() << std::endl;
  //
  // std::cout << "hash: " << FunctionTypeHash{}(function_type) << std::endl;

  if (functions_types_.contains(function_type)) {
    return functions_types_.at(function_type);
  }

  tp::TypeId type_id = types_.size() + basic_types_.size();

  types_.emplace_back(function_type);

  functions_types_[function_type] = type_id;

  return type_id;
}

tp::TypeId TypeStore::get_reference_type(tp::TypeId base_type,
                                         bool is_mutable) {

  if (base_type == tp::no_type_id) {
    throw std::runtime_error(
        "Call TypeStore::get_reference_type with not_type_id");
  }

  tp::ReferenceType reference_type{.base_type = base_type,
                                   .is_mutable = is_mutable};

  if (reference_types_.contains(reference_type)) {
    return reference_types_.at(reference_type);
  }

  tp::TypeId type_id = types_.size() + basic_types_.size();

  types_.emplace_back(std::move(reference_type));

  reference_types_[reference_type] = type_id;

  return type_id;
}

tp::TypeId TypeStore::get_basic_type(tp::BasicTypeVariant type) {
  return type.index(); // invariant
}

tp::TypeId TypeStore::get_type_id_by_ast_type(ast::TypeNode& type) {
  auto optional_type = tp::get_type(type.type_name->name);

  if (!optional_type.has_value()) {
    throw std::runtime_error("Unknown type: " + type.type_name->name);
  }

  tp::TypeId base_type = get_basic_type(optional_type.value());

  if (!type.is_reference) {
    return base_type;
  }

  return get_reference_type(base_type, type.is_mutable);
}

tp::TypeId TypeStore::new_literal_type(tp::LiteralVariant&& literal) {
  tp::TypeId type_id = types_.size() + basic_types_.size();

  types_.emplace_back(std::move(literal));

  return type_id;
}

tp::TypeId TypeStore::new_undefined_type() {
  tp::TypeId type_id = types_.size() + basic_types_.size();

  types_.emplace_back(tp::UndefinedType{});

  return type_id;
}

tp::TypeId TypeStore::resolve(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    throw std::runtime_error("Call TypeStore::resolve with not_type_id");
  }

  if (type_id < basic_types_.size()) {
    return type_id;
  }

  const auto& raw = types_.at(type_id - basic_types_.size()).type;
  if (std::holds_alternative<tp::ReferenceType>(raw) ||
      std::holds_alternative<tp::FunctionType>(raw)) {
    return type_id;
  }

  return std::visit(
      [&](auto&& val) -> tp::TypeId {
        using val_type = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<val_type, tp::FunctionType>) {
          return type_id;
        } else {
          if (val.parent == tp::no_type_id) {
            return type_id;
          }

          tp::TypeId root = resolve(val.parent);

          val.parent = root;

          return root;
        }
      },
      tp::narrow_down(types_.at(type_id - basic_types_.size()).type));
}

bool TypeStore::unify(tp::TypeId type1, tp::TypeId type2) {
  if (type1 == tp::no_type_id || type2 == tp::no_type_id) {
    std::cerr << "Call TypeStore::unify with not_type_id: " << type1 << " "
              << type2 << std::endl;
    return false;
  }

  tp::TypeId root1 = resolve(type1);
  tp::TypeId root2 = resolve(type2);

  if (root1 == root2) {
    return true;
  }

  overloaded body{[&](tp::IntLiteral& literal) {
                    if (is_integer_type(root1)) {
                      literal.parent = root1;
                      return true;
                    }
                    return false;
                  },

                  [&](tp::FloatLiteral& literal) {
                    if (is_float_type(root1)) {
                      literal.parent = root1;
                      return true;
                    }
                    return false;
                  },

                  [&](tp::UndefinedType& undefined) {
                    undefined.parent = root1;
                    return true;
                  },

                  [&](auto&) { return root1 == root2; }};

  if (tp::is_basic_type(root1)) {
    if (tp::is_basic_type(root2)) {
      return root1 == root2;
    } else {
      return std::visit(body, get_mutable_type(root2).type);
    }
  } else {
    if (tp::is_basic_type(root2)) {
      std::swap(root1, root2);
      return std::visit(body, get_mutable_type(root2).type);
    } else {
      return std::visit(
          [&](auto&& type1, auto&& type2) -> bool {
            using T1 = std::decay_t<decltype(type1)>;
            using T2 = std::decay_t<decltype(type2)>;

            if constexpr (is_in_type_tuple_v<T1, tp::StrangeTypeTuple> &&
                          is_in_type_tuple_v<T2, tp::StrangeTypeTuple>) {
              if constexpr (std::is_same_v<T1, tp::UndefinedType>) {
                type1.parent = root2;
                return true;
              } else if constexpr (std::is_same_v<T2, tp::UndefinedType>) {
                type2.parent = root1;
                return true;
              } else if constexpr (std::is_same_v<T1, T2>) {
                if constexpr (std::is_same_v<T1, tp::FunctionType>) {
                  return root1 == root2;
                } else {
                  type1.parent = root2;
                  return true;
                }
              }
              return false;
            } else if constexpr (std::is_same_v<T1, tp::ReferenceType> &&
                                 std::is_same_v<T2, tp::ReferenceType>) {
              if (type1.is_mutable && !type2.is_mutable) {
                return false;
              }
              return unify(type1.base_type, type2.base_type);
            }

            // std::cerr << type1 << type2 << std::endl;
            // std::cerr << std::stacktrace::current() << std::endl;

            throw std::logic_error("Unreachable");
          },
          get_mutable_type(root1).type, get_mutable_type(root2).type);
    }
  }
}

const tp::Type& TypeStore::get_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    throw std::runtime_error("Call TypeStore::get_type with not_type_id");
  }

  tp::TypeId root = resolve(type_id);

  if (root < basic_types_.size()) {
    return basic_types_.at(root);
  }

  return types_.at(root - tp::basic_type_count);
}

void TypeStore::add_ast_type(ast::ASTTypeNode* ast_type) {
  ast_types_.push_back(ast_type);
}

void TypeStore::add_ast_var(ast::IdentifierNode* ast_var) {
  ast_vars_.push_back(ast_var);
}

void TypeStore::add_ast_deref(ast::LvalueDereferenceNode* deref) {
  ast_derefs_.push_back(deref);
}

void TypeStore::add_ast_lvalue_expr(ast::LvalueExpressionNode* lvalue_expr) {
  ast_lvalue_exprs_.push_back(lvalue_expr);
}

bool TypeStore::is_integer_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    return false;
  }

  return std::visit(
      [](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;

        return is_in_type_tuple_v<T, tp::IntegerTypeTuple> ||
               std::is_same_v<T, tp::IntLiteral>;
      },
      get_type(type_id).type);
}

bool TypeStore::is_signed_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    return false;
  }

  return std::visit(
      [](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;

        return is_in_type_tuple_v<T, tp::SignedIntegerTypeTuple>;
      },
      get_type(type_id).type);
}

bool TypeStore::is_float_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    return false;
  }

  return std::visit(
      [](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;

        return is_in_type_tuple_v<T, tp::FloatingPointTypeTuple> ||
               std::is_same_v<T, tp::FloatLiteral>;
      },
      get_type(type_id).type);
}

bool TypeStore::is_reference_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    return false;
  }

  return std::visit(
      [](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;

        return std::is_same_v<T, tp::ReferenceType>;
      },
      get_type(type_id).type);
}

void TypeStore::handle_ast_types() {
  for (auto* node : ast_vars_) {
    tp::TypeId root = resolve(node->type_id);

    // std::cout << "handle: " << *node << std::endl;

    SymbolTable* current_scope = node->table;

    if (root < basic_types_.size()) {
      node->type_id = root;

      current_scope->change_symbol_type(node->identifier->name, node->type_id);

      continue;
    }

    auto& type_variant = types_.at(root - basic_types_.size()).type;

    if (std::holds_alternative<tp::IntLiteral>(type_variant)) {
      node->type_id = get_basic_type(tp::I32{});
    } else if (std::holds_alternative<tp::FloatLiteral>(type_variant)) {
      node->type_id = get_basic_type(tp::F32{});
    } else if (std::holds_alternative<tp::ReferenceType>(type_variant) ||
               std::holds_alternative<tp::FunctionType>(type_variant)) {
      node->type_id = root;
    } else {
      throw std::runtime_error("Strange type");
    }

    current_scope->change_symbol_type(node->identifier->name, node->type_id);
  }

  for (auto* node : ast_types_) {
    tp::TypeId root = resolve(node->type_id);

    if (root < basic_types_.size()) {
      node->type_id = root;
      continue;
    }

    auto& type_variant = types_.at(root - basic_types_.size()).type;

    if (std::holds_alternative<tp::IntLiteral>(type_variant)) {
      node->type_id = get_basic_type(tp::I32{});
    } else if (std::holds_alternative<tp::FloatLiteral>(type_variant)) {
      node->type_id = get_basic_type(tp::F32{});
    } else if (std::holds_alternative<tp::UndefinedType>(type_variant)) {
      throw std::runtime_error("Strange type: unresolved undefined type");
    } else if (std::holds_alternative<tp::ReferenceType>(type_variant)) {
      // Reference types are already concrete; keep node->type_id as-is.
    } else if (std::holds_alternative<tp::FunctionType>(type_variant)) {
      // Function types are already concrete; keep node->type_id as-is.
    }
  }

  reference_types_.clear();
  for (auto& tp : types_) {
    if (!std::holds_alternative<tp::ReferenceType>(tp.type)) {
      continue;
    }
    auto& ref = std::get<tp::ReferenceType>(tp.type);
    tp::TypeId base = resolve(ref.base_type);
    if (base >= basic_types_.size()) {
      auto& base_variant = types_.at(base - basic_types_.size()).type;
      if (std::holds_alternative<tp::IntLiteral>(base_variant)) {
        base = get_basic_type(tp::I32{});
      } else if (std::holds_alternative<tp::FloatLiteral>(base_variant)) {
        base = get_basic_type(tp::F32{});
      }
    }
    ref.base_type = base;
  }
  for (std::size_t i = 0; i < types_.size(); ++i) {
    if (std::holds_alternative<tp::ReferenceType>(types_[i].type)) {
      reference_types_[std::get<tp::ReferenceType>(types_[i].type)] =
          basic_types_.size() + i;
    }
  }

  for (auto* deref : ast_derefs_) {
    tp::TypeId raw = deref->type_id;
    if (raw == tp::no_type_id) {
      continue;
    }
    if (raw < basic_types_.size()) {
      continue;
    }
    tp::TypeId root = resolve(raw);
    if (root < basic_types_.size()) {
      deref->type_id = root;
      continue;
    }
    auto& variant = types_.at(root - basic_types_.size()).type;
    if (std::holds_alternative<tp::IntLiteral>(variant)) {
      deref->type_id = get_basic_type(tp::I32{});
    } else if (std::holds_alternative<tp::FloatLiteral>(variant)) {
      deref->type_id = get_basic_type(tp::F32{});
    }
  }

  for (auto* lvalue_expr : ast_lvalue_exprs_) {
    tp::TypeId raw = lvalue_expr->type_id;
    if (raw == tp::no_type_id || raw < basic_types_.size()) {
      continue;
    }
    tp::TypeId root = resolve(raw);
    if (root < basic_types_.size()) {
      lvalue_expr->type_id = root;
      continue;
    }
    auto& variant = types_.at(root - basic_types_.size()).type;
    if (std::holds_alternative<tp::IntLiteral>(variant)) {
      lvalue_expr->type_id = get_basic_type(tp::I32{});
    } else if (std::holds_alternative<tp::FloatLiteral>(variant)) {
      lvalue_expr->type_id = get_basic_type(tp::F32{});
    }
  }
}

tp::Type& TypeStore::get_mutable_type(tp::TypeId type_id) {
  if (type_id < tp::basic_type_count) {
    throw std::runtime_error(
        "Call TypeStore::get_mutable_type with basic type");
  }
  return types_.at(type_id - tp::basic_type_count);
}
