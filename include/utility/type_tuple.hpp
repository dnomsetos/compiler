#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename... Ts> struct TypeTuple {};

template <typename T> struct IsTypeTuple : std::false_type {};

template <typename... Ts>
struct IsTypeTuple<TypeTuple<Ts...>> : std::true_type {};

template <typename T>
concept TypeTupleLike = IsTypeTuple<T>::value;

template <TypeTupleLike T> struct TypeTupleSize;
template <typename... Ts>
struct TypeTupleSize<TypeTuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <TypeTupleLike Tuple>
inline constexpr std::size_t type_tuple_size_v = TypeTupleSize<Tuple>::value;

template <typename T, TypeTupleLike Tuple> struct IsInTypeTuple;

template <typename T, typename Head, typename... Tail>
struct IsInTypeTuple<T, TypeTuple<Head, Tail...>>
    : std::conditional_t<std::is_same_v<T, Head>, std::true_type,
                         IsInTypeTuple<T, TypeTuple<Tail...>>> {};

template <typename T, TypeTupleLike Tuple>
inline constexpr bool is_in_type_tuple_v = IsInTypeTuple<T, Tuple>::value;

template <typename T> struct IsInTypeTuple<T, TypeTuple<>> : std::false_type {};

template <TypeTupleLike... Types> struct Concat;

template <TypeTupleLike T> struct Concat<T> : std::type_identity<T> {};

template <typename... LeftTypes, typename... RightTypes>
struct Concat<TypeTuple<LeftTypes...>, TypeTuple<RightTypes...>>
    : std::type_identity<TypeTuple<LeftTypes..., RightTypes...>> {};

template <TypeTupleLike Head, TypeTupleLike... Tail>
struct Concat<Head, Tail...>
    : std::type_identity<
          typename Concat<Head, typename Concat<Tail...>::type>::type> {};

template <TypeTupleLike... Ts>
using type_tuple_concat_t = typename Concat<Ts...>::type;

template <typename T> struct TupleToVariant;

template <typename... Ts>
struct TupleToVariant<TypeTuple<Ts...>>
    : std::type_identity<std::variant<Ts...>> {};

template <typename T> struct VariantToTuple;

template <typename... Ts>
struct VariantToTuple<std::variant<Ts...>>
    : std::type_identity<TypeTuple<Ts...>> {};

template <TypeTupleLike Tuple> struct TupleSize;

template <typename... Ts>
struct TupleSize<TypeTuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T>
using type_tuple_to_variant_t = typename TupleToVariant<T>::type;

template <typename T>
using variant_to_type_tuple_t = typename VariantToTuple<T>::type;

template <typename T, TypeTupleLike Tuple, std::size_t I>
struct TypeTupleIndexHelper;

template <typename T, typename Head, typename... Tail, std::size_t I>
struct TypeTupleIndexHelper<T, TypeTuple<Head, Tail...>, I>
    : std::conditional_t<std::is_same_v<T, Head>,
                         std::integral_constant<std::size_t, I>,
                         TypeTupleIndexHelper<T, TypeTuple<Tail...>, I + 1>> {};

template <typename T, TypeTupleLike Tuple>
constexpr std::size_t type_tuple_index_v =
    TypeTupleIndexHelper<T, Tuple, 0>::value;

template <TypeTupleLike Tuple, std::uint8_t Index> struct TypeTupleAtHelper;

template <typename Head, typename... Tail>
struct TypeTupleAtHelper<TypeTuple<Head, Tail...>, 0>
    : std::type_identity<Head> {};

template <typename Head, typename... Tail, std::uint8_t Index>
struct TypeTupleAtHelper<TypeTuple<Head, Tail...>, Index>
    : TypeTupleAtHelper<TypeTuple<Tail...>, Index - 1> {};

template <TypeTupleLike Tuple, std::uint8_t Index>
using type_tuple_at_t = typename TypeTupleAtHelper<Tuple, Index>::type;

template <TypeTupleLike T> struct TypeTuplePopBack;

template <typename T>
struct TypeTuplePopBack<TypeTuple<T>> : std::type_identity<TypeTuple<>> {};

template <typename Head, typename... Tail>
struct TypeTuplePopBack<TypeTuple<Head, Tail...>>
    : std::type_identity<typename Concat<
          TypeTuple<Head>,
          typename TypeTuplePopBack<TypeTuple<Tail...>>::type>::type> {};

template <>
struct TypeTuplePopBack<TypeTuple<>> : std::type_identity<TypeTuple<>> {};

template <TypeTupleLike T>
using type_tuple_pop_back_t = typename TypeTuplePopBack<T>::type;

static_assert(
    std::is_same_v<TypeTuplePopBack<TypeTuple<int, bool, float>>::type,
                   TypeTuple<int, bool>>);

template <TypeTupleLike T> struct TypeTuplePopFront;

template <typename Head, typename... Tail>
struct TypeTuplePopFront<TypeTuple<Head, Tail...>>
    : std::type_identity<TypeTuple<Tail...>> {};

template <>
struct TypeTuplePopFront<TypeTuple<>> : std::type_identity<TypeTuple<>> {};

template <TypeTupleLike T>
using type_tuple_pop_front_t = typename TypeTuplePopFront<T>::type;

// get type tuple of type tuples
template <std::size_t Index, TypeTupleLike Tuple>
struct AlternatingConcatHelper;

template <typename T, typename U> struct DoubleTypeIdentity {
  using type1 = T;
  using type2 = U;
};

template <typename Head, typename... Tail>
  requires(TypeTupleLike<Head> && (TypeTupleLike<Tail> && ...))
struct AlternatingConcatHelper<0, TypeTuple<Head, Tail...>>
    : DoubleTypeIdentity<
          type_tuple_concat_t<TypeTuple<type_tuple_pop_front_t<Head>>,
                              TypeTuple<Tail...>>,
          type_tuple_at_t<Head, 0>> {};

template <typename Head, typename... Tail, std::size_t Index>
  requires(TypeTupleLike<Head> && (TypeTupleLike<Tail> && ...))
struct AlternatingConcatHelper<Index, TypeTuple<Head, Tail...>>
    : DoubleTypeIdentity<
          type_tuple_concat_t<TypeTuple<Head>,
                              typename AlternatingConcatHelper<
                                  Index - 1, TypeTuple<Tail...>>::type1>,
          typename AlternatingConcatHelper<Index - 1,
                                           TypeTuple<Tail...>>::type2> {};

namespace {
using R = AlternatingConcatHelper<
    2, TypeTuple<TypeTuple<int, int, char>, TypeTuple<float, double>,
                 TypeTuple<char, int*>>>;

static_assert(
    std::is_same_v<typename R::type1,
                   TypeTuple<TypeTuple<int, int, char>,
                             TypeTuple<float, double>, TypeTuple<int*>>>);

static_assert(std::is_same_v<typename R::type2, char>);
using R2 = AlternatingConcatHelper<
    1, TypeTuple<TypeTuple<int, int, char>, TypeTuple<float, double>,
                 TypeTuple<char, int*>>>;
} // namespace

// get type tuple of type tuples
template <std::size_t Index, TypeTupleLike Tuple>
struct AlternatingConcatHelper2;

template <TypeTupleLike... Tuples>
  requires((type_tuple_size_v<Tuples> == 0) && ...)
struct AlternatingConcatHelper2<0, TypeTuple<Tuples...>>
    : std::type_identity<TypeTuple<>> {};

template <std::size_t Index, TypeTupleLike... Tuples>
  requires([]<std::size_t... Is>(std::index_sequence<Is...>) -> bool {
    constexpr std::size_t first =
        type_tuple_size_v<type_tuple_at_t<TypeTuple<Tuples...>, 0>> +
        (Index > 0 ? 1 : 0);

    // clang-format off
  return (((type_tuple_size_v<type_tuple_at_t<TypeTuple<Tuples...>, Is>>) +
              (Is < Index ? 1 : 0) == first) && ...);
    // clang-format on
  }(std::make_index_sequence<sizeof...(Tuples)>{}))
struct AlternatingConcatHelper2<Index, TypeTuple<Tuples...>>
    : std::type_identity<type_tuple_concat_t<
          TypeTuple<typename AlternatingConcatHelper<
              Index, TypeTuple<Tuples...>>::type2>,
          typename AlternatingConcatHelper2<
              (Index + 1) % sizeof...(Tuples),
              typename AlternatingConcatHelper<
                  Index, TypeTuple<Tuples...>>::type1>::type>> {};

template <TypeTupleLike... Tuples>
using alternating_concat_t =
    typename AlternatingConcatHelper2<0, TypeTuple<Tuples...>>::type;

static_assert(std::is_same_v<
              TypeTuple<int, char, bool, float, double, long long, short, long,
                        long double>,
              alternating_concat_t<TypeTuple<int, float, short>,
                                   TypeTuple<char, double, long>,
                                   TypeTuple<bool, long long, long double>>>);

template <TypeTupleLike Subset, TypeTupleLike Set> struct IsSubsetOf;

template <typename Head, typename... Tail, TypeTupleLike Set>
struct IsSubsetOf<TypeTuple<Head, Tail...>, Set>
    : std::conditional_t<is_in_type_tuple_v<Head, Set>,
                         IsSubsetOf<TypeTuple<Tail...>, Set>, std::false_type> {
};

template <TypeTupleLike Set>
struct IsSubsetOf<TypeTuple<>, Set> : std::true_type {};

template <TypeTupleLike Subset, TypeTupleLike Set>
inline constexpr bool is_subset_of_v = IsSubsetOf<Subset, Set>::value;

static_assert(is_subset_of_v<TypeTuple<int, char, int>,
                             TypeTuple<double, char, int, float>>);
