#include "databento/detail/socket.hpp"

#include <arpa/inet.h>   // inet_addr
#include <netinet/in.h>  // htons, IPPROTO_TCP
#include <sys/socket.h>  // AF_INET, connect, sockaddr, sockaddr_in, socket, SOCK_STREAM
#include <unistd.h>  // close

#include <cerrno>   // errno
#include <utility>  // swap

#include "databento/exceptions.hpp"  // InvalidArgumentError, TcpError

using databento::detail::Socket;

Socket::Socket(const std::string& addr, std::uint16_t port)
    : fd_{::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)} {
  if (fd_ == -1) {
    throw TcpError{errno, "Failed to create socket"};
  }

  in_addr network_addr{};
  if (::inet_aton(addr.c_str(), &network_addr) == 0) {
    throw InvalidArgumentError{"Socket::Socket", "addr",
                               "Unable to convert to a binary IPv4 address"};
  }
  const sockaddr_in addr_in{AF_INET, ::htons(port), network_addr, {}};
  if (::connect(fd_, reinterpret_cast<const sockaddr*>(&addr_in),
                sizeof(sockaddr_in)) != 0) {
    ::close(fd_);
    throw TcpError{errno, "Socket failed to connect"};
  }
}

Socket::Socket(Socket&& other) noexcept : fd_{other.fd_} { other.fd_ = -1; }

Socket& Socket::operator=(Socket&& other) noexcept {
  std::swap(fd_, other.fd_);
  return *this;
}

Socket::~Socket() {
  if (fd_ >= 0) {
    ::close(fd_);
  }
}
