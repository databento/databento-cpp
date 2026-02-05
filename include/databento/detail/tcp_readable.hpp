#pragma once

#include <chrono>
#include <cstddef>

#include "databento/detail/tcp_client.hpp"
#include "databento/ireadable.hpp"

namespace databento::detail {
// Adapter wrapping TcpClient to implement IReadable interface and be passed
// as a non-owned pointer to ZstdDecodeStream.
class TcpReadable : public IReadable {
 public:
  explicit TcpReadable(TcpClient* client) : client_{client} {}

  void ReadExact(std::byte* buffer, std::size_t length) override;
  std::size_t ReadSome(std::byte* buffer, std::size_t max_length) override;
  IReadable::Result ReadSome(std::byte* buffer, std::size_t max_length,
                             std::chrono::milliseconds timeout) override;

 private:
  TcpClient* client_;
};
}  // namespace databento::detail
