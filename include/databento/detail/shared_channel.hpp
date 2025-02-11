#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <memory>   // shared_ptr

#include "databento/ireadable.hpp"

namespace databento::detail {
// Copyable, thread-safe, unidirectional channel.
class SharedChannel : public IReadable {
 public:
  SharedChannel();

  // Write `data` of `length` bytes to the channel.
  void Write(const std::uint8_t* data, std::size_t length);
  // Signal the end of input.
  void Finish();
  // Read exactly `length` bytes.
  void ReadExact(std::uint8_t* buffer, std::size_t length) override;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::uint8_t* buffer, std::size_t length) override;

 private:
  class Channel;

  std::shared_ptr<Channel> channel_;
};
}  // namespace databento::detail
