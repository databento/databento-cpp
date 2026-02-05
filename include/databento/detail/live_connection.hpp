#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "databento/detail/tcp_client.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento::detail {
// Manages the TCP connection to the live gateway with optionally compressed reads for
// the DBN data.
class LiveConnection : IWritable {
 public:
  LiveConnection(const std::string& gateway, std::uint16_t port);

  void WriteAll(std::string_view str);
  void WriteAll(const std::byte* buffer, std::size_t size);
  void ReadExact(std::byte* buffer, std::size_t size);
  IReadable::Result ReadSome(std::byte* buffer, std::size_t max_size);
  IReadable::Result ReadSome(std::byte* buffer, std::size_t max_size,
                             std::chrono::milliseconds timeout);
  // Closes the socket.
  void Close();
  // Sets compression for subsequent reads.
  void SetCompression(Compression compression);

 private:
  TcpClient client_;
  std::optional<ZstdDecodeStream> zstd_stream_;
};
}  // namespace databento::detail
