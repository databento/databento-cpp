#include "databento/detail/http_stream_reader.hpp"

#include <cstddef>  // byte, size_t
#include <sstream>
#include <utility>  // move

#include "databento/exceptions.hpp"

using databento::detail::HttpStreamReader;

HttpStreamReader::HttpStreamReader(httplib::ClientImpl::StreamHandle handle)
    : handle_{std::move(handle)} {}

void HttpStreamReader::ReadExact(std::byte* buffer, std::size_t length) {
  std::size_t total_read = 0;
  while (total_read < length) {
    const auto n =
        handle_.read(reinterpret_cast<char*>(buffer) + total_read, length - total_read);
    if (n <= 0) {
      std::ostringstream ss;
      ss << "[HttpStreamReader::ReadExact] Expected " << length
         << " bytes but only read " << total_read;
      throw DbnResponseError{ss.str()};
    }
    total_read += static_cast<std::size_t>(n);
  }
}

std::size_t HttpStreamReader::ReadSome(std::byte* buffer, std::size_t max_length) {
  const auto n = handle_.read(reinterpret_cast<char*>(buffer), max_length);
  if (n < 0) {
    return 0;
  }
  return static_cast<std::size_t>(n);
}

databento::IReadable::Result HttpStreamReader::ReadSome(
    std::byte* buffer, std::size_t max_length, std::chrono::milliseconds /*timeout*/) {
  const auto read_size = ReadSome(buffer, max_length);
  if (read_size == 0) {
    return {0, Status::Closed};
  }
  return {read_size, Status::Ok};
}
