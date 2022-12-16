#pragma once

namespace databento {
namespace detail {
// RAII wrapper that closes the file descriptor on destruction
class ScopedFd {
 public:
  ScopedFd() = default;
  explicit ScopedFd(int fd) : fd_{fd} {}
  ScopedFd(const ScopedFd&) = delete;
  ScopedFd& operator=(const ScopedFd&) = delete;
  ScopedFd(ScopedFd&& other) noexcept;
  ScopedFd& operator=(ScopedFd&& rhs) noexcept;
  ~ScopedFd();

  int Get() const { return fd_; }
  void Close();

 private:
  static constexpr auto kUnset = -1;

  int fd_{kUnset};
};
}  // namespace detail
}  // namespace databento
