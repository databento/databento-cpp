#include "mock/mock_io.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

using databento::tests::mock::MockIo;

void MockIo::WriteAll(const std::byte* buffer, std::size_t length) {
  contents_.insert(contents_.end(), buffer, buffer + length);
}

void MockIo::ReadExact(std::byte* buffer, std::size_t length) {
  const auto remaining_bytes = contents_.size() - read_idx_;
  if (remaining_bytes < length) {
    throw std::runtime_error{"Not enough bytes remaining: expected " +
                             std::to_string(length) + " got " +
                             std::to_string(remaining_bytes)};
  }
  auto s_length = static_cast<std::ptrdiff_t>(length);
  std::copy(contents_.cbegin() + read_idx_,
            contents_.cbegin() + read_idx_ + s_length, buffer);
  read_idx_ += s_length;
}

std::size_t MockIo::ReadSome(std::byte* buffer, std::size_t max_length) {
  auto read_size = static_cast<std::ptrdiff_t>(
      std::min(max_length, contents_.size() - read_idx_));
  std::copy(contents_.cbegin() + read_idx_,
            contents_.cbegin() + read_idx_ + read_size, buffer);
  read_idx_ += read_size;
  return read_size;
}
