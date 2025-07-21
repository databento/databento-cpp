#pragma once

#include <thread>
#include <utility>  // forward, move

namespace databento::detail {
// An RAII thread that joins if necessary on destruction, like std::jthread in
// C++20.
class ScopedThread {
 public:
  ScopedThread() noexcept = default;
  template <typename F, typename... A>
  explicit ScopedThread(F&& func, A&&... args)
      : thread_{std::forward<F>(func), std::forward<A>(args)...} {}
  ScopedThread(ScopedThread&& other) noexcept : thread_{std::move(other.thread_)} {}
  ScopedThread& operator=(ScopedThread&& other) noexcept {
    if (Joinable()) {
      Join();
    }
    thread_ = std::move(other.thread_);
    return *this;
  }
  ScopedThread(const ScopedThread&) = delete;
  ScopedThread& operator=(const ScopedThread&) = delete;
  ~ScopedThread() {
    if (Joinable()) {
      Join();
    }
  }

  std::thread::id Id() const { return thread_.get_id(); }
  bool Joinable() const { return thread_.joinable(); }
  void Join() { return thread_.join(); }

 private:
  std::thread thread_;
};
}  // namespace databento::detail
