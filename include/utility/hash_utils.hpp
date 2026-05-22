#pragma once

#include <functional>

struct PairHash {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const {
    size_t h1 = std::hash<T1>{}(p.first);
    size_t h2 = std::hash<T2>{}(p.second);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

// for tp::FunctionType
struct FunctionTypeHash {
  template <typename T> std::size_t operator()(const T& f) const {
    std::size_t seed = std::hash<std::size_t>{}(f.return_type);

    for (const auto& x : f.args) {
      seed ^= std::hash<std::size_t>{}(x) + 0x9e3779b97f4a7c15ULL +
              (seed << 6) + (seed >> 2);
    }

    return seed;
  }
};

// for tp::ReferenceType
struct ReferenceTypeHash {
  template <typename T> std::size_t operator()(const T& f) const {
    std::size_t h1 = std::hash<std::size_t>{}(f.base_type);
    std::size_t h2 = std::hash<bool>{}(f.is_mutable);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};
