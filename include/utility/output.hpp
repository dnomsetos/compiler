#pragma once

#include <scanner/token.hpp>

template <typename T> struct IsVariant : std::false_type {};

template <typename... Ts>
struct IsVariant<std::variant<Ts...>> : std::true_type {};

template <typename T> constexpr bool is_variant_v = IsVariant<T>::value;

std::ostream& operator<<(std::ostream& out, const Dummy&);
