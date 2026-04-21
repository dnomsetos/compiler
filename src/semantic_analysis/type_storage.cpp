#include <iostream>

#include <semantic_analysis/type_storage.hpp>

tp::TypeId TypeStore::new_var(tp::TypeId parent) {
  tp::TypeId type_id = types_.size();

  types_.emplace_back(tp::VariableType{.parent = parent});

  return type_id;
}

tp::TypeId TypeStore::new_function(tp::TypeId return_type,
                                   std::vector<tp::TypeId>&& args) {
  tp::TypeId type_id = types_.size();

  types_.emplace_back(
      tp::Function{.args = std::move(args), .return_type = return_type});

  return type_id;
}

tp::TypeId TypeStore::new_literal_type(
    type_tuple_to_variant_t<tp::LiteralTypeTuple> type) {
  tp::TypeId type_id = types_.size();

  std::visit([this](auto&& val) { types_.emplace_back(std::move(val)); }, type);
  return type_id;
}

tp::TypeId TypeStore::new_basic_type(tp::BasicTypeVariant type) {
  auto it = basic_type_cache_.find(type.index());

  if (it != basic_type_cache_.end()) {
    return it->second;
  }

  tp::TypeId type_id = types_.size();

  std::visit([this](auto&& val) { types_.emplace_back(val); }, type);

  basic_type_cache_.emplace(type.index(), type_id);

  return type_id;
}

tp::TypeId TypeStore::resolve(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    throw std::runtime_error("Call TypeStore::resolve with not_type_id");
  }

  return std::visit(
      [&](auto&& val) -> tp::TypeId {
        using val_type = std::decay_t<decltype(val)>;

        if constexpr (!std::is_same_v<val_type, tp::VariableType> &&
                      !std::is_same_v<val_type, tp::IntLiteral> &&
                      !std::is_same_v<val_type, tp::FloatLiteral>) {
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
      types_.at(type_id).type);
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

  auto& type1_type = types_.at(root1);
  auto& type2_type = types_.at(root2);

  return std::visit(
      [&](auto&& type1, auto&& type2) -> bool {
        using T1 = std::decay_t<decltype(type1)>;
        using T2 = std::decay_t<decltype(type2)>;

        if constexpr (std::is_same_v<T1, tp::VariableType> ||
                      std::is_same_v<T1, tp::IntLiteral> ||
                      std::is_same_v<T1, tp::FloatLiteral>) {
          type1.parent = root2;
          return true;
        } else if constexpr (std::is_same_v<T2, tp::VariableType> ||
                             std::is_same_v<T2, tp::IntLiteral> ||
                             std::is_same_v<T2, tp::FloatLiteral>) {
          type2.parent = root1;
          return true;
        } else {
          return false;
        }
      },
      type1_type.type, type2_type.type);
}

tp::Type& TypeStore::get_type(tp::TypeId type_id) {
  tp::TypeId root = resolve(type_id);

  return types_.at(root);
}
