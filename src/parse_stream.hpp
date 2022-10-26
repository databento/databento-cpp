#pragma once

#include <condition_variable>
#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <mutex>
#include <sstream>  // stringstream
#include <vector>

namespace databento {
class ParseStream {
  // protects all other data members of this class
  std::mutex mutex_;
  // could use eofbit, but seekg clears this
  bool is_finished_{false};
  std::condition_variable cv_;
  std::stringstream stream_;

 public:
  ParseStream() = default;
  ParseStream(const ParseStream&) = delete;
  ParseStream& operator=(const ParseStream&) = delete;
  ParseStream(ParseStream&&) = delete;
  ParseStream& operator=(ParseStream&&) = delete;
  ~ParseStream();

  void Write(const std::uint8_t* data, std::size_t length);
  void Finish();
  // Read exactly `length` bytes
  void ReadExact(std::uint8_t* buffer, std::size_t length);
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::uint8_t* buffer, std::size_t length);

 private:
  std::size_t stream_size();
};
}  // namespace databento
