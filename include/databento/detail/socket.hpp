#pragma once

#include <cstdint>
#include <string>
namespace databento {
namespace detail {
// RAII wrapper around a socket.
class Socket {
 public:
  Socket(const std::string& addr, std::uint16_t port);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;
  ~Socket();

  int Get() const { return fd_; }

 private:
  int fd_;
};
}  // namespace detail
}  // namespace databento
