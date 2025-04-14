#pragma once

#include <cstddef>  // byte, size_t

namespace databento {
// An abstract class for readable objects to allow for runtime polymorphism
// around DBN decoding.
class IReadable {
 public:
  virtual ~IReadable() = default;

  // Read exactly `length` bytes into `buffer`.
  virtual void ReadExact(std::byte* buffer, std::size_t length) = 0;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  virtual std::size_t ReadSome(std::byte* buffer, std::size_t max_length) = 0;
};
}  // namespace databento
