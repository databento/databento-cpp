#pragma once

#ifdef _WIN32
#include <winsock2.h>  // SOCKET
#endif

namespace databento::detail {
#ifdef _WIN32
using Socket = SOCKET;
#else
using Socket = int;
#endif

// RAII wrapper that closes the file descriptor on destruction
class ScopedFd {
 public:
#ifdef _WIN32
  static constexpr Socket kUnset = INVALID_SOCKET;
#else
  static constexpr Socket kUnset = -1;
#endif

  ScopedFd() = default;
  explicit ScopedFd(Socket fd) : fd_{fd} {}
  ScopedFd(const ScopedFd&) = delete;
  ScopedFd& operator=(const ScopedFd&) = delete;
  ScopedFd(ScopedFd&& other) noexcept;
  ScopedFd& operator=(ScopedFd&& rhs) noexcept;
  ~ScopedFd();

  Socket Get() const { return fd_; }
  void Close();

 private:
  Socket fd_{kUnset};
};
}  // namespace databento::detail
