#pragma once

#include <optional>
#include <vector>

#include <scanner/token.hpp>
#include <utility/allocator.hpp>
#include <utility/type_tuple.hpp>

namespace tp {

#define GENERATE_TYPE(name)                                                    \
  struct name {};

GENERATE_TYPE(I8)
GENERATE_TYPE(I16)
GENERATE_TYPE(I32)
GENERATE_TYPE(I64)
GENERATE_TYPE(U8)
GENERATE_TYPE(U16)
GENERATE_TYPE(U32)
GENERATE_TYPE(U64)
GENERATE_TYPE(F32)
GENERATE_TYPE(F64)
GENERATE_TYPE(Bool)
GENERATE_TYPE(Char)
GENERATE_TYPE(Void)

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
                        FloatingPointTypeTuple, VoidTypeTuple>;

using BasicTypeVariant = type_tuple_to_variant_t<BasicTypeTypeTuple>;

struct VariableInfo {
  tkn::Position definition_position;
  std::uint8_t basic_type;
};

struct FunctionInfo {
  tkn::Position definition_position;
  std::uint8_t reutrn_type;
  std::vector<std::uint8_t> arguments;
};

using Type = std::optional<BasicTypeVariant>;

// indexes of types in type_names match indexes from BasicTypeTuple
inline constexpr std::pair<const char*, BasicTypeVariant> type_names[] = {
    {"i8", I8{}},     {"i16", I16{}},   {"i32", I32{}}, {"i64", I64{}},
    {"u8", U8{}},     {"u16", U16{}},   {"u32", U32{}}, {"u64", U64{}},
    {"bool", Bool{}}, {"char", Char{}}, {"f32", F32{}}, {"f64", F64{}},
    {"void", Void{}},
};

inline constexpr std::uint8_t no_type =
    std::numeric_limits<std::uint8_t>::max();

inline constexpr std::uint8_t IntegerIndexes[] = {0, 1, 2, 3, 4, 5, 6, 7};

inline constexpr std::uint8_t BooleanIndexes[] = {8};

inline constexpr std::uint8_t CharacterIndexes[] = {9};

inline constexpr std::uint8_t FloatingPointIndexes[] = {10, 11};

} // namespace tp
