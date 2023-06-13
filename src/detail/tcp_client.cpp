#include "databento/detail/tcp_client.hpp"

#include <netdb.h>       // addrinfo, gai_strerror, getaddrinfo, freeaddrinfo
#include <netinet/in.h>  // htons, IPPROTO_TCP
#include <sys/poll.h>    // pollfd, POLLHUP
#include <sys/socket.h>  // AF_INET, connect, sockaddr, sockaddr_in, socket, SOCK_STREAM
#include <unistd.h>  // close, read, ssize_t

#include <algorithm>  // max
#include <cerrno>     // errno
#include <memory>     // unique_ptr
#include <sstream>
#include <thread>

#include "databento/exceptions.hpp"  // TcpError

using databento::detail::TcpClient;

TcpClient::TcpClient(const std::string& gateway, std::uint16_t port)
    : TcpClient{gateway, port, {}} {}

TcpClient::TcpClient(const std::string& gateway, std::uint16_t port,
                     RetryConf retry_conf)
    : socket_{InitSocket(gateway, port, retry_conf)} {}

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

void TcpClient::Close() { socket_.Close(); }

databento::detail::ScopedFd TcpClient::InitSocket(const std::string& gateway,
                                                  std::uint16_t port,
                                                  RetryConf retry_conf) {
  const int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw TcpError{errno, "Failed to create socket"};
  }
  ScopedFd scoped_fd{fd};

  addrinfo hints{};
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  ::addrinfo* out;
  const auto ret = ::getaddrinfo(gateway.c_str(), std::to_string(port).c_str(),
                                 &hints, &out);
  if (ret != 0) {
    throw InvalidArgumentError{"TcpClient::TcpClient", "addr",
                               ::gai_strerror(ret)};
  }
  std::unique_ptr<addrinfo, decltype(&::freeaddrinfo)> res{out,
                                                           &::freeaddrinfo};
  const auto max_attempts = std::max<std::uint32_t>(retry_conf.max_attempts, 1);
  std::chrono::seconds backoff{1};
  for (std::uint32_t attempt = 0; attempt < max_attempts; ++attempt) {
    if (::connect(scoped_fd.Get(), res->ai_addr, res->ai_addrlen) == 0) {
      break;
    } else if (attempt + 1 == max_attempts) {
      std::ostringstream err_msg;
      err_msg << "Socket failed to connect after " << max_attempts
              << " attempts";
      throw TcpError{errno, err_msg.str()};
    }
    // TODO(cg): Log
    std::this_thread::sleep_for(backoff);
    backoff = std::min(backoff * 2, retry_conf.max_wait);
  }
  return scoped_fd;
}
