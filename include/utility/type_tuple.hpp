#pragma once

#include <type_traits>
#include <variant>

template <typename... Types> struct TypeTuple {};

template <typename T> struct IsTypeTuple : std::false_type {};

template <typename... Ts>
struct IsTypeTuple<TypeTuple<Ts...>> : std::true_type {};

template <typename T>
concept TypeTupleLike = IsTypeTuple<T>::value;

template <typename T, TypeTupleLike Tuple> struct IsInTypeTuple;

template <typename T, typename Head, typename... Tail>
struct IsInTypeTuple<T, TypeTuple<Head, Tail...>>
    : std::conditional_t<std::is_same_v<T, Head>, std::true_type,
                         IsInTypeTuple<T, TypeTuple<Tail...>>> {};

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

template <typename T> struct TupleToVariant;

template <typename... Ts>
struct TupleToVariant<TypeTuple<Ts...>>
    : std::type_identity<std::variant<Ts...>> {};

template <TypeTupleLike Tuple> struct TupleSize;

template <typename... Ts>
struct TupleSize<TypeTuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T>
using type_tuple_to_variant_t = typename TupleToVariant<T>::type;
