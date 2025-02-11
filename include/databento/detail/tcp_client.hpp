#pragma once

#include <chrono>  // milliseconds
#include <cstdint>
#include <string>

#include "databento/detail/scoped_fd.hpp"  // ScopedFd

namespace databento::detail {
class TcpClient {
 public:
  enum class Status : std::uint8_t {
    Ok,
    Timeout,
    Closed,
  };

  struct Result {
    std::size_t read_size;
    Status status;
  };

  struct RetryConf {
    std::uint32_t max_attempts{1};
    std::chrono::seconds max_wait{std::chrono::minutes{1}};
  };

  TcpClient(const std::string& gateway, std::uint16_t port);
  TcpClient(const std::string& gateway, std::uint16_t port,
            RetryConf retry_conf);

  void WriteAll(const std::string& str);
  void WriteAll(const char* buffer, std::size_t size);
  void ReadExact(char* buffer, std::size_t size);
  Result ReadSome(char* buffer, std::size_t max_size);
  // Passing a timeout of 0 will block until data is available of the socket is
  // closed, the same behavior as the Read overload without a timeout.
  Result ReadSome(char* buffer, std::size_t max_size,
                  std::chrono::milliseconds timeout);
  // Closes the socket.
  void Close();

 private:
  static ScopedFd InitSocket(const std::string& gateway, std::uint16_t port,
                             RetryConf retry_conf);

  ScopedFd socket_;
};
}  // namespace databento::detail
