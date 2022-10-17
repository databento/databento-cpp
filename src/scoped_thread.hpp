#pragma once

#include <thread>
#include <utility>  // forward

// An RAII thread that joins on destruction. For simplicity, this implementation
// is not default-constructible, so there's also no need to check if it's
// joinable in the destructor, but it's not movable.
class ScopedThread {
 public:
  template <typename F>
  explicit ScopedThread(F&& func) : thread_{std::forward<F>(func)} {}
  ScopedThread(ScopedThread&&) = delete;
  ScopedThread& operator=(ScopedThread&&) = delete;
  ScopedThread(const ScopedThread&) = delete;
  ScopedThread& operator=(const ScopedThread&) = delete;
  ~ScopedThread() { thread_.join(); }

 private:
  std::thread thread_;
};
