#pragma once

#include <vector>

#include <parser/ast.hpp>
#include <scanner/token.hpp>
#include <utility/allocator.hpp>
#include <utility/type_tuple.hpp>

namespace ast {

struct FunctionDefinitionNode;

} // namespace ast

namespace tp {

using TypeId = std::size_t;

inline constexpr TypeId no_type_id = -1;

#define GENERATE_TYPE(name, type)                                              \
  struct name {                                                                \
    using interpret_type = type;                                               \
  };                                                                           \
  inline std::ostream& operator<<(std::ostream& out, const name&) {            \
    out << #name;                                                              \
    return out;                                                                \
  }

GENERATE_TYPE(I8, std::int8_t)
GENERATE_TYPE(I16, std::int16_t)
GENERATE_TYPE(I32, std::int32_t)
GENERATE_TYPE(I64, std::int64_t)
GENERATE_TYPE(U8, std::uint8_t)
GENERATE_TYPE(U16, std::uint16_t)
GENERATE_TYPE(U32, std::uint32_t)
GENERATE_TYPE(U64, std::uint64_t)
GENERATE_TYPE(F32, std::float32_t)
GENERATE_TYPE(F64, std::float64_t)
GENERATE_TYPE(Bool, bool)
GENERATE_TYPE(Char, char)
GENERATE_TYPE(Void, Dummy)
GENERATE_TYPE(IntLiteral, std::uint64_t);
GENERATE_TYPE(FloatLiteral, std::float64_t);

using LiteralTypeTuple = TypeTuple<IntLiteral, FloatLiteral>;

using SignedIntegerTypeTuple = TypeTuple<I8, I16, I32, I64>;

using UnsignedIntegerTypeTuple = TypeTuple<U8, U16, U32, U64>;

using IntegerTypeTuple =
    type_tuple_concat_t<SignedIntegerTypeTuple, UnsignedIntegerTypeTuple>;

using BooleanTypeTuple = TypeTuple<Bool>;

using CharTypeTuple = TypeTuple<Char>;

using FloatingPointTypeTuple = TypeTuple<F32, F64>;

using VoidTypeTuple = TypeTuple<Void>;

using BasicTypeTypeTuple =
    type_tuple_concat_t<IntegerTypeTuple, BooleanTypeTuple, CharTypeTuple,
                        FloatingPointTypeTuple, VoidTypeTuple,
                        LiteralTypeTuple>;

using BasicTypeVariant = type_tuple_to_variant_t<BasicTypeTypeTuple>;

struct VariableType {
  TypeId parent = no_type_id;
};

inline std::ostream& operator<<(std::ostream& out, const VariableType&) {
  out << "VariableType" << std::endl;
  return out;
}

struct Function {
  std::vector<TypeId> args;
  TypeId return_type;
};

inline std::ostream& operator<<(std::ostream& out, const Function&) {
  out << "Function" << std::endl;
  return out;
}

struct NoType {};

inline std::ostream& operator<<(std::ostream& out, const NoType&) {
  out << "NoType" << std::endl;
  return out;
}

using TypeTypeTuple =
    Concat<BasicTypeTypeTuple, TypeTuple<VariableType, Function, NoType>>::type;

using TypeVariant = type_tuple_to_variant_t<TypeTypeTuple>;

struct Type {
  TypeVariant type;
};

// indexes of types in type_names match indexes from BasicTypeTuple
inline constexpr std::pair<const char*, BasicTypeVariant> basic_type_names[] = {
    {"i8", I8{}},     {"i16", I16{}},   {"i32", I32{}}, {"i64", I64{}},
    {"u8", U8{}},     {"u16", U16{}},   {"u32", U32{}}, {"u64", U64{}},
    {"bool", Bool{}}, {"char", Char{}}, {"f32", F32{}}, {"f64", F64{}},
    {"void", Void{}},
};

inline std::optional<BasicTypeVariant> get_type(const std::string& name) {
  auto it =
      std::find_if(std::begin(basic_type_names), std::end(basic_type_names),
                   [&name](auto&& val) { return val.first == name; });

  if (it != std::end(basic_type_names)) {
    return it->second;
  } else {
    return std::nullopt;
  }
}

inline std::ostream& operator<<(std::ostream& out, const Type& type) {
  std::visit([&out](auto&& type) { out << type << std::endl; }, type.type);

  return out;
}

inline const std::pair<tp::TypeVariant, calc_result_t> default_value_table[] = {
    {I8{}, std::int8_t{0}},
    {I16{}, std::int16_t{0}},
    {I32{}, std::int32_t{0}},
    {I64{}, std::int64_t{0}},
    {U8{}, std::uint8_t{0}},
    {U16{}, std::uint16_t{0}},
    {U32{}, std::uint32_t{0}},
    {U64{}, std::uint64_t{0}},
    {F32{}, std::float32_t{0.0f}},
    {F64{}, std::float64_t{0.0}},
    {Bool{}, false},
    {Char{}, char(0)},
};

// inline constexpr std::uint8_t no_type =
//     std::numeric_limits<std::uint8_t>::max();
//
// inline constexpr std::uint8_t IntegerIndexes[] = {0, 1, 2, 3, 4, 5, 6, 7};
//
// inline constexpr std::uint8_t BooleanIndexes[] = {8};
//
// inline constexpr std::uint8_t CharacterIndexes[] = {9};
//
// inline constexpr std::uint8_t FloatingPointIndexes[] = {10, 11};

} // namespace tp
