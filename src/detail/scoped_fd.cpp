#include "databento/detail/scoped_fd.hpp"

#ifndef _WIN32
#include <unistd.h>  // close
#endif

#include <utility>  // swap

using databento::detail::ScopedFd;

ScopedFd::ScopedFd(ScopedFd&& other) noexcept : fd_{other.fd_} { other.fd_ = kUnset; }

ScopedFd& ScopedFd::operator=(ScopedFd&& other) noexcept {
  std::swap(fd_, other.fd_);
  return *this;
}

ScopedFd::~ScopedFd() { Close(); }

void ScopedFd::Close() {
  if (fd_ != kUnset) {
#ifdef _WIN32
    ::closesocket(fd_);
#else
    ::close(fd_);
#endif
    fd_ = kUnset;
  }
}
