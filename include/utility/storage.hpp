#pragma once

#include <tuple>
#include <type_traits>

#include <scanner/token.hpp>
#include <utility/type_tuple.hpp>

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

template <TypeTupleLike T> struct TypeTupleToStorage;

template <typename... Ts> struct LastType;

template <typename T> struct LastType<T> : std::type_identity<T> {};

template <typename Head, typename... Tail>
struct LastType<Head, Tail...>
    : std::type_identity<typename LastType<Tail...>::type> {};

template <> struct LastType<> : std::type_identity<void> {};

template <typename... Ts> using last_type_t = typename LastType<Ts...>::type;

template <typename... Ts> struct Storage;

template <typename... Ts>
struct TypeTupleToStorage<TypeTuple<Ts...>>
    : std::type_identity<Storage<Ts...>> {};

template <typename T>
using type_tuple_to_storage_t = typename TypeTupleToStorage<T>::type;

static_assert(
    std::is_same_v<type_tuple_to_storage_t<
                       type_tuple_pop_back_t<TypeTuple<tkn::Position, int>>>,
                   Storage<tkn::Position>>);
static_assert(std::is_same_v<last_type_t<tkn::Position, int>, int>);

template <typename... Ts>
struct Storage
    : type_tuple_to_storage_t<type_tuple_pop_back_t<TypeTuple<Ts...>>>,
      last_type_t<Ts...> {
public:
  template <typename... Us,
            typename = std::enable_if_t<(sizeof...(Us) == sizeof...(Ts))>>
  Storage(Us&&... args)
      : Storage(std::make_tuple(std::forward<Us>(args)...),
                std::make_index_sequence<sizeof...(Us) - 1>{}) {}

protected:
  using StorageBase =
      type_tuple_to_storage_t<type_tuple_pop_back_t<TypeTuple<Ts...>>>;

  using LastType = last_type_t<Ts...>;

  template <typename Tuple, std::size_t... Is>
  Storage(Tuple&& tuple, std::index_sequence<Is...>)
      : StorageBase(std::get<Is>(std::forward<Tuple>(tuple))...),
        LastType(std::get<sizeof...(Is)>(std::forward<Tuple>(tuple))) {}
};

template <> struct Storage<> {};

static_assert(std::is_base_of_v<Storage<tkn::Position>,
                                Storage<tkn::Position, std::true_type>>);
