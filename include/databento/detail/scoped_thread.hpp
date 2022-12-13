#pragma once

#include <thread>
#include <utility>  // forward, move

namespace databento {
namespace detail {
// An RAII thread that joins if necessary on destruction.
class ScopedThread {
 public:
  ScopedThread() noexcept = default;
  template <typename F, typename... A>
  explicit ScopedThread(F&& func, A&&... args)
      : thread_{std::forward<F>(func), std::forward<A>(args)...} {}
  ScopedThread(ScopedThread&& other) noexcept
      : thread_{std::move(other.thread_)} {}
  ScopedThread& operator=(ScopedThread&& other) noexcept {
    if (thread_.joinable()) {
      thread_.join();
    }
    thread_ = std::move(other.thread_);
    return *this;
  }
  ScopedThread(const ScopedThread&) = delete;
  ScopedThread& operator=(const ScopedThread&) = delete;
  ~ScopedThread() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  std::thread thread_;
};
}  // namespace detail
}  // namespace databento
