#include <semantic_analysis/types.hpp>

namespace tp {

std::ostream& operator<<(std::ostream& out, const IntLiteral&) {
  return out << "Integer literal";
}

std::ostream& operator<<(std::ostream& out, const FloatLiteral&) {
  return out << "Float literal";
}

// std::ostream& operator<<(std::ostream& out, const VariableType&) {
//   out << "VariableType" << std::endl;
//   return out;
// }

FunctionType::FunctionType(std::vector<TypeId> args, TypeId return_type)
    : args(std::move(args)), return_type(return_type) {}

std::ostream& operator<<(std::ostream& out, const FunctionType&) {
  out << "Function" << std::endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, const NoType&) {
  out << "NoType" << std::endl;
  return out;
}

Type::Type() : type{NoType{}} {}

Type::Type(BasicTypeVariant variant)
    : type{std::visit([](auto& val) -> TypeVariant { return val; }, variant)} {}

Type::Type(FunctionType&& type) : type{std::move(type)} {}

Type::Type(LiteralVariant&& type)
    : type{std::visit([](auto&& val) -> TypeVariant { return val; }, type)} {}

std::optional<BasicTypeVariant> get_type(const std::string& name) {
  auto it =
      std::find_if(std::begin(basic_type_names), std::end(basic_type_names),
                   [&name](auto&& val) { return val.first == name; });

  if (it != std::end(basic_type_names)) {
    return it->second;
  } else {
    return std::nullopt;
  }
}

std::ostream& operator<<(std::ostream& out, const Type& type) {
  std::visit([&out](auto&& type) { out << type << std::endl; }, type.type);

  return out;
}

} // namespace tp
