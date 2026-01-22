#include "databento/detail/live_connection.hpp"

#include <memory>

#include "databento/detail/tcp_readable.hpp"

using databento::detail::LiveConnection;

LiveConnection::LiveConnection(const std::string& gateway, std::uint16_t port)
    : client_{gateway, port} {}

void LiveConnection::WriteAll(std::string_view str) { client_.WriteAll(str); }

void LiveConnection::WriteAll(const std::byte* buffer, std::size_t size) {
  client_.WriteAll(buffer, size);
}

void LiveConnection::ReadExact(std::byte* buffer, std::size_t size) {
  if (zstd_stream_) {
    zstd_stream_->ReadExact(buffer, size);
  } else {
    client_.ReadExact(buffer, size);
  }
}

databento::IReadable::Result LiveConnection::ReadSome(std::byte* buffer,
                                                      std::size_t max_size) {
  return ReadSome(buffer, max_size, std::chrono::milliseconds{});
}

databento::IReadable::Result LiveConnection::ReadSome(
    std::byte* buffer, std::size_t max_size, std::chrono::milliseconds timeout) {
  if (zstd_stream_) {
    return zstd_stream_->ReadSome(buffer, max_size, timeout);
  }
  return client_.ReadSome(buffer, max_size, timeout);
}

void LiveConnection::Close() { client_.Close(); }

void LiveConnection::SetCompression(Compression compression) {
  if (compression == Compression::Zstd) {
    zstd_stream_.emplace(std::make_unique<TcpReadable>(&client_));
  }
}
