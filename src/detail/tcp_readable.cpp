#include "databento/detail/tcp_readable.hpp"

using databento::detail::TcpReadable;

void TcpReadable::ReadExact(std::byte* buffer, std::size_t length) {
  client_->ReadExact(buffer, length);
}

std::size_t TcpReadable::ReadSome(std::byte* buffer, std::size_t max_length) {
  return client_->ReadSome(buffer, max_length).read_size;
}

databento::IReadable::Result TcpReadable::ReadSome(std::byte* buffer,
                                                   std::size_t max_length,
                                                   std::chrono::milliseconds timeout) {
  return client_->ReadSome(buffer, max_length, timeout);
}
