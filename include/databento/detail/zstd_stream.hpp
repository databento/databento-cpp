#pragma once

#include <zstd.h>

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <memory>   // unique_ptr
#include <vector>

#include "databento/ireadable.hpp"

namespace databento {
namespace detail {
class ZstdStream : public IReadable {
 public:
  explicit ZstdStream(std::unique_ptr<IReadable> input);
  ZstdStream(std::unique_ptr<IReadable> input,
             std::vector<std::uint8_t>&& in_buffer);

  // Read exactly `length` bytes into `buffer`.
  void ReadExact(std::uint8_t* buffer, std::size_t length) override;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  size_t ReadSome(std::uint8_t* buffer, std::size_t max_length) override;

 private:
  std::unique_ptr<IReadable> input_;
  std::unique_ptr<ZSTD_DStream, std::size_t (*)(ZSTD_DStream*)> z_dstream_;
  std::size_t read_suggestion_;
  std::vector<std::uint8_t> in_buffer_;
  ZSTD_inBuffer z_in_buffer_;
};
}  // namespace detail
}  // namespace databento
