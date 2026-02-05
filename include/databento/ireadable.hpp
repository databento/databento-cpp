#pragma once

#include <chrono>   // milliseconds
#include <cstddef>  // byte, size_t
#include <cstdint>  // uint8_t

namespace databento {
// An abstract class for readable objects to allow for runtime polymorphism
// around DBN decoding.
class IReadable {
 public:
  enum class Status : std::uint8_t {
    Ok,       // Data read successfully
    Timeout,  // Timeout reached before any data available
    Closed,   // Stream is closed/EOF
  };

  struct Result {
    std::size_t read_size;  // Number of bytes read
    Status status;          // Status of the read operation
  };

  virtual ~IReadable() = default;

  // Read exactly `length` bytes into `buffer`.
  virtual void ReadExact(std::byte* buffer, std::size_t length) = 0;
  // Read at most `length` bytes. Returns the number of bytes read. Will only return 0
  // if the end of the stream is reached.
  virtual std::size_t ReadSome(std::byte* buffer, std::size_t max_length) = 0;
  // Read at most `max_length` bytes with timeout support. Returns Result with bytes
  // read and status. Status will be Timeout if no data available within timeout period,
  // Closed if stream is closed, or Ok if data was read. A timeout of 0 means wait
  // indefinitely (same as the no-timeout overload).
  virtual Result ReadSome(std::byte* buffer, std::size_t max_length,
                          std::chrono::milliseconds timeout) = 0;
};
}  // namespace databento
