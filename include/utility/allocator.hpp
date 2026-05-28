#pragma once

#include <memory>
#include <memory_resource>

namespace alloc {

namespace {

std::pmr::monotonic_buffer_resource arena_resource;

};

inline std::pmr::memory_resource& mr = arena_resource;

template <typename T> struct MonotonicBufferResourceDeleter {
  void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr != nullptr) {
      ptr->~T();
    }
  }
};

template <typename T>
using pmr_unique_ptr =
    std::unique_ptr<T, alloc::MonotonicBufferResourceDeleter<T>>;

template <typename T> using pmr_shared_ptr = std::shared_ptr<T>;

template <typename T> using pmr_weak_ptr = std::weak_ptr<T>;

template <typename T, typename... Args>
auto make_unique_pmr(Args&&... args) -> pmr_unique_ptr<T> {
  void* mem = mr.allocate(sizeof(T), alignof(T));
  T* obj = new (mem) T(std::forward<Args>(args)...);
  return pmr_unique_ptr<T>(obj);
}

template <typename T, typename... Args>
auto make_shared_pmr(Args&&... args) -> std::shared_ptr<T> {
  return std::allocate_shared<T>(std::pmr::polymorphic_allocator<T>{&mr},
                                 std::forward<Args>(args)...);
}

} // namespace alloc
