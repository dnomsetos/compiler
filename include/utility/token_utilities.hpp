#include <scanner/token.hpp>

template <TypeTupleLike Tuple>
bool is_in_type_tuple(const tkn::TokenInfo& token) {
  return std::visit(
      [](auto&& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        return IsInTypeTuple<T, Tuple>::value;
      },
      token.token_variant);
}

template <TypeTupleLike BigTuple, TypeTupleLike SmallTuple>
auto variant_cast(const type_tuple_to_variant_t<BigTuple>& variant)
    -> std::optional<type_tuple_to_variant_t<SmallTuple>> {
  return std::visit(
      [](auto&& val) -> std::optional<type_tuple_to_variant_t<SmallTuple>> {
        using T = std::decay_t<decltype(val)>;

        if constexpr (IsInTypeTuple<T, SmallTuple>::value) {
          return val;
        }
        return std::nullopt;
      },
      variant);
}
