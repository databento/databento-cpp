#pragma once

#include <fstream>
#include <string>

namespace databento {
namespace detail {
class FileStream {
 public:
  explicit FileStream(const std::string& file_path);

  // Read exactly `length` bytes into `buffer`.
  void ReadExact(std::uint8_t* buffer, std::size_t length);
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::uint8_t* buffer, std::size_t max_length);

 private:
  std::ifstream stream_;
};
}  // namespace detail
}  // namespace databento
