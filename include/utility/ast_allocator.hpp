#pragma once

#include <memory>
#include <memory_resource>

namespace ast::alloc {
inline std::pmr::monotonic_buffer_resource mr;

template <typename T> struct MonotonicBufferResourceDeleter {
  void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr != nullptr) {
      ptr->~T();
    }
  }
};

template <typename T, typename... Args>
auto make_unique_pmr(Args&&... args)
    -> std::unique_ptr<T, MonotonicBufferResourceDeleter<T>> {
  void* mem = mr.allocate(sizeof(T), alignof(T));
  T* obj = new (mem) T(std::forward<Args>(args)...);
  return std::unique_ptr<T, MonotonicBufferResourceDeleter<T>>(obj);
}

} // namespace ast::alloc
