#include <memory>
#include <memory_resource>

template <typename T> struct PmrDeleter {
  void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr != nullptr) {
      ptr->~T();
    }
  }
};

template <typename T, typename... Args>
auto make_unique_pmr(std::pmr::memory_resource* mr, Args&&... args)
    -> std::unique_ptr<T, PmrDeleter<T>> {
  void* mem = mr->allocate(sizeof(T), alignof(T));
  T* obj = new (mem) T(std::forward<Args>(args)...);
  return std::unique_ptr<T, PmrDeleter<T>>(obj);
}
