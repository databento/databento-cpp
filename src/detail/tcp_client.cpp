#include "databento/detail/tcp_client.hpp"

#ifdef _WIN32
#include <winsock2.h>  // closesocket, recv, send, socket
#else
#include <fcntl.h>       // fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <netdb.h>       // addrinfo, gai_strerror, getaddrinfo, freeaddrinfo
#include <netinet/in.h>  // htons, IPPROTO_TCP
#include <sys/poll.h>    // pollfd
#include <sys/socket.h>  // AF_INET, connect, recv, send, sockaddr, sockaddr_in, socket, SOCK_STREAM, getsockopt, SO_ERROR, SOL_SOCKET
#include <unistd.h>      // close, ssize_t

#include <cerrno>  // errno
#endif

#include <algorithm>  // max
#include <memory>     // unique_ptr
#include <sstream>
#include <thread>

#include "databento/exceptions.hpp"  // TcpError
#include "databento/log.hpp"         // ILogReceiver

using databento::detail::TcpClient;
using Status = databento::IReadable::Status;

namespace {
int GetErrNo() {
#ifdef _WIN32
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

int Poll(::pollfd* fds, std::uint32_t nfds, int timeout_ms) {
#ifdef _WIN32
  return ::WSAPoll(fds, nfds, timeout_ms);
#else
  return ::poll(fds, static_cast<::nfds_t>(nfds), timeout_ms);
#endif
}

int GetSockOpt(databento::detail::Socket fd, int level, int optname, int* optval) {
#ifdef _WIN32
  int len = sizeof(*optval);
  return ::getsockopt(fd, level, optname, reinterpret_cast<char*>(optval), &len);
#else
  socklen_t len = sizeof(*optval);
  return ::getsockopt(fd, level, optname, optval, &len);
#endif
}

#ifdef _WIN32
constexpr int kConnectInProgress = WSAEWOULDBLOCK;
#else
constexpr int kConnectInProgress = EINPROGRESS;
#endif

// Saves the current blocking state, sets non-blocking, and returns a RAII guard
// that restores the original state on destruction.
class BlockingGuard {
 public:
  explicit BlockingGuard(databento::detail::Socket socket) : _fd{socket} {
#ifdef _WIN32
    unsigned long mode = 1;
    ::ioctlsocket(fd, FIONBIO, &mode);
#else
    _original_flags = ::fcntl(_fd, F_GETFL, 0);
    ::fcntl(_fd, F_SETFL, _original_flags | O_NONBLOCK);
#endif
  }

  ~BlockingGuard() {
#ifdef _WIN32
    unsigned long mode = 0;
    ::ioctlsocket(fd, FIONBIO, &mode);
#else
    ::fcntl(_fd, F_SETFL, _original_flags);
#endif
  }

  BlockingGuard(const BlockingGuard&) = delete;
  BlockingGuard& operator=(const BlockingGuard&) = delete;
  BlockingGuard(BlockingGuard&&) = delete;
  BlockingGuard& operator=(BlockingGuard&&) = delete;

 private:
  databento::detail::Socket _fd;
#ifdef _WIN32
  // No state to save on Windows
#else
  int _original_flags;
#endif
};
}  // namespace

TcpClient::TcpClient(ILogReceiver* log_receiver, const std::string& gateway,
                     std::uint16_t port)
    : TcpClient{log_receiver, gateway, port, {}} {}

TcpClient::TcpClient(ILogReceiver* log_receiver, const std::string& gateway,
                     std::uint16_t port, RetryConf retry_conf)
    : socket_{InitSocket(log_receiver, gateway, port, retry_conf)} {}

void TcpClient::WriteAll(std::string_view str) {
  WriteAll(reinterpret_cast<const std::byte*>(str.data()), str.length());
}

void TcpClient::WriteAll(const std::byte* buffer, std::size_t size) {
  do {
    const ::ssize_t res =
        ::send(socket_.Get(), reinterpret_cast<const char*>(buffer), size, {});
    if (res < 0) {
      throw TcpError{::GetErrNo(), "Error writing to socket"};
    }
    size -= static_cast<std::size_t>(res);
    buffer += res;
  } while (size > 0);
}

void TcpClient::ReadExact(std::byte* buffer, std::size_t size) {
  const ::ssize_t res =
      ::recv(socket_.Get(), reinterpret_cast<char*>(buffer), size, MSG_WAITALL);
  if (res != static_cast<::ssize_t>(size)) {
    throw TcpError{::GetErrNo(), "Error reading from socket"};
  }
}

databento::IReadable::Result TcpClient::ReadSome(std::byte* buffer,
                                                 std::size_t max_size) {
  const ::ssize_t res =
      ::recv(socket_.Get(), reinterpret_cast<char*>(buffer), max_size, {});
  if (res < 0) {
    throw TcpError{::GetErrNo(), "Error reading from socket"};
  }
  return {static_cast<std::size_t>(res), res == 0 ? Status::Closed : Status::Ok};
}

databento::IReadable::Result TcpClient::ReadSome(std::byte* buffer,
                                                 std::size_t max_size,
                                                 std::chrono::milliseconds timeout) {
  pollfd fds{socket_.Get(), POLLIN, {}};
  // passing a timeout of -1 blocks indefinitely, which is the equivalent of
  // having no timeout
  const auto timeout_ms = timeout.count() ? static_cast<int>(timeout.count()) : -1;
  while (true) {
    const int poll_status = Poll(&fds, 1, timeout_ms);
    if (poll_status > 0) {
      return ReadSome(buffer, max_size);
    }
    if (poll_status == 0) {
      return {0, Status::Timeout};
    }
    // Retry if EAGAIN or EINTR
    const int err_num = ::GetErrNo();
    if (err_num != EAGAIN && err_num != EINTR) {
      throw TcpError{err_num, "Incorrect poll"};
    }
  }
}

void TcpClient::Close() { socket_.Close(); }

databento::detail::ScopedFd TcpClient::InitSocket(ILogReceiver* log_receiver,
                                                  const std::string& gateway,
                                                  std::uint16_t port,
                                                  RetryConf retry_conf) {
  static constexpr auto kMethod = "TcpClient::TcpClient";

  const detail::Socket fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw TcpError{::GetErrNo(), "Failed to create socket"};
  }
  ScopedFd scoped_fd{fd};

  addrinfo hints{};
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  ::addrinfo* out;
  const auto ret =
      ::getaddrinfo(gateway.c_str(), std::to_string(port).c_str(), &hints, &out);
  if (ret != 0) {
    throw InvalidArgumentError{kMethod, "addr", ::gai_strerror(ret)};
  }
  std::unique_ptr<addrinfo, decltype(&::freeaddrinfo)> res{out, &::freeaddrinfo};
  const auto max_attempts = std::max<std::uint32_t>(retry_conf.max_attempts, 1);
  const auto timeout_ms = static_cast<int>(
      std::chrono::duration_cast<std::chrono::milliseconds>(retry_conf.connect_timeout)
          .count());
  std::chrono::seconds backoff{1};
  for (std::uint32_t attempt = 0; attempt < max_attempts; ++attempt) {
    BlockingGuard guard{scoped_fd.Get()};

    const int connect_ret = ::connect(scoped_fd.Get(), res->ai_addr, res->ai_addrlen);
    bool connected = (connect_ret == 0);
    if (!connected && ::GetErrNo() == kConnectInProgress) {
      pollfd pfd{scoped_fd.Get(), POLLOUT, {}};
      const int poll_ret = Poll(&pfd, 1, timeout_ms);
      if (poll_ret > 0) {
        int so_error = 0;
        GetSockOpt(scoped_fd.Get(), SOL_SOCKET, SO_ERROR, &so_error);
        connected = (so_error == 0);
        if (!connected) {
          errno = so_error;
        }
      }
    }

    if (connected) {
      break;
    } else if (attempt + 1 == max_attempts) {
      std::ostringstream err_msg;
      err_msg << "Socket failed to connect after " << max_attempts << " attempt(s)";
      throw TcpError{::GetErrNo(), err_msg.str()};
    }
    std::ostringstream log_msg;
    log_msg << '[' << kMethod << "] Connection attempt " << (attempt + 1) << " to "
            << gateway << ':' << port << " failed, retrying in " << backoff.count()
            << " seconds";
    log_receiver->Receive(LogLevel::Warning, log_msg.str());

    std::this_thread::sleep_for(backoff);
    backoff = (std::min)(backoff * 2, retry_conf.max_wait);
  }
  return scoped_fd;
}
