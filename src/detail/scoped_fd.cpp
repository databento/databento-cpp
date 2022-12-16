#include "databento/detail/scoped_fd.hpp"

#include <unistd.h>  // close

#include <utility>  // swap

using databento::detail::ScopedFd;

ScopedFd::ScopedFd(ScopedFd&& other) noexcept : fd_{other.fd_} {
  other.fd_ = -1;
}

ScopedFd& ScopedFd::operator=(ScopedFd&& other) noexcept {
  std::swap(fd_, other.fd_);
  return *this;
}

ScopedFd::~ScopedFd() { Close(); }

void ScopedFd::Close() {
  if (fd_ != kUnset) {
    ::close(fd_);
    fd_ = kUnset;
  }
}
