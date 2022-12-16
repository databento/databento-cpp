#pragma once

#include <cstdint>
#include <string>

#include "databento/detail/scoped_fd.hpp"  // ScopedFd

namespace databento {
namespace detail {
class TcpClient {
 public:
  TcpClient(const std::string& gateway, std::uint16_t port);

  void WriteAll(const std::string& str);
  void WriteAll(const char* buffer, std::size_t size);
  // returns bytes read
  std::size_t Read(char* buffer, std::size_t max_size);
  void ReadExact(char* buffer, std::size_t size);

 private:
  static int InitSocket(const std::string& gateway, std::uint16_t port);

  ScopedFd socket_;
};
}  // namespace detail
}  // namespace databento
