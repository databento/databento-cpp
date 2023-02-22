#include "databento/detail/tcp_client.hpp"

#include <arpa/inet.h>   // inet_addr
#include <netinet/in.h>  // htons, IPPROTO_TCP
#include <sys/poll.h>    // pollfd, POLLHUP
#include <sys/socket.h>  // AF_INET, connect, sockaddr, sockaddr_in, socket, SOCK_STREAM
#include <unistd.h>  // close, read, ssize_t

#include <cerrno>  // errno

#include "databento/exceptions.hpp"  // TcpError

using databento::detail::TcpClient;

TcpClient::TcpClient(const std::string& gateway, std::uint16_t port)
    : socket_{InitSocket(gateway, port)} {}

void TcpClient::WriteAll(const std::string& str) {
  WriteAll(str.c_str(), str.length());
}

void TcpClient::WriteAll(const char* buffer, std::size_t size) {
  do {
    const ::ssize_t res = ::write(socket_.Get(), buffer, size);
    if (res < 0) {
      throw TcpError{errno, "Error writing to socket"};
    }
    size -= static_cast<std::size_t>(res);
    buffer += res;
  } while (size > 0);
}

void TcpClient::ReadExact(char* buffer, std::size_t size) {
  const ::ssize_t res = ::recv(socket_.Get(), buffer, size, MSG_WAITALL);
  if (res != static_cast<::ssize_t>(size)) {
    throw TcpError{errno, "Error reading from socket"};
  }
}

TcpClient::Result TcpClient::ReadSome(char* buffer, std::size_t max_size) {
  const ::ssize_t res = ::read(socket_.Get(), buffer, max_size);
  if (res < 0) {
    throw TcpError{errno, "Error reading from socket"};
  }
  return {static_cast<std::size_t>(res),
          res == 0 ? Status::Closed : Status::Ok};
}

TcpClient::Result TcpClient::ReadSome(char* buffer, std::size_t max_size,
                                      std::chrono::milliseconds timeout) {
  pollfd fds{socket_.Get(), POLLHUP | POLLIN, {}};
  // passing a timeout of -1 blocks indefinitely, which is the equivalent of
  // having no timeout
  const auto timeout_ms =
      timeout.count() ? static_cast<int>(timeout.count()) : -1;
  while (true) {
    const int poll_status = ::poll(&fds, 1, timeout_ms);
    if (poll_status > 0) {
      return ReadSome(buffer, max_size);
    }
    if (poll_status == 0) {
      return {0, Status::Timeout};
    }
    // Retry if EAGAIN or EINTR
    if (errno != EAGAIN && errno != EINTR) {
      throw TcpError{errno, "Incorrect poll"};
    }
  }
}

int TcpClient::InitSocket(const std::string& gateway, std::uint16_t port) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw TcpError{errno, "Failed to create socket"};
  }

  in_addr network_addr{};
  if (::inet_aton(gateway.c_str(), &network_addr) == 0) {
    throw InvalidArgumentError{"TcpClient::TcpClient", "addr",
                               "Unable to convert to a binary IPv4 address"};
  }
  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  addr_in.sin_addr = network_addr;
  if (::connect(fd, reinterpret_cast<const sockaddr*>(&addr_in),
                sizeof(sockaddr_in)) != 0) {
    ::close(fd);
    throw TcpError{errno, "Socket failed to connect"};
  }
  return fd;
}
