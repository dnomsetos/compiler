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

  if (!std::holds_alternative<tp::VariableType>(types_.at(type_id).type)) {
    return type_id;
  }

  auto& var_type = std::get<tp::VariableType>(types_.at(type_id).type);

  if (var_type.parent == tp::no_type_id) {
    return type_id;
  }

  tp::TypeId root = resolve(var_type.parent);

  var_type.parent = root;

  return root;
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

  if (std::holds_alternative<tp::VariableType>(type1_type.type)) {
    std::get<tp::VariableType>(type1_type.type).parent = root2;
    return true;
  }

  if (std::holds_alternative<tp::VariableType>(type2_type.type)) {
    std::get<tp::VariableType>(type2_type.type).parent = root1;
    return true;
  }

  return false;
}

tp::Type& TypeStore::get_type(tp::TypeId type_id) {
  if (type_id == tp::no_type_id) {
    throw std::runtime_error("Call TypeStore::get_type with not_type_id");
  }

  return types_.at(type_id);
}

tp::TypeId TypeStore::find_common_for_literal(tp::TypeId expected_type,
                                              tp::TypeId maybe_literal) {
  if (expected_type == tp::no_type_id) {
    return maybe_literal;
  }

  tp::TypeVariant& real_expected_type = get_type(expected_type).type;

  if (unify(maybe_literal, new_basic_type(tp::IntLiteral{}))) {
    if (std::visit(
            [](auto&& val) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(val)>,
                                        tp::IntegerTypeTuple>;
            },
            real_expected_type)) {
      return expected_type;
    } else {
      throw std::runtime_error("Type mismatch. Expected type "
                               "is not integer, but literal type is integer.");
    }
  } else if (unify(maybe_literal, new_basic_type(tp::FloatLiteral{}))) {
    if (std::visit(
            [](auto&& val) -> bool {
              return is_in_type_tuple_v<std::decay_t<decltype(val)>,
                                        tp::FloatingPointTypeTuple>;
            },
            real_expected_type)) {
      return expected_type;
    } else {
      throw std::runtime_error("Type mismatch in literal node. Expected type "
                               "is not float, but literal type is float.");
    }
  } else {
    return maybe_literal;
  }
}
