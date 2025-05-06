#pragma once

#include <zstd.h>

#include <cstddef>  // size_t
#include <memory>   // unique_ptr
#include <vector>

#include "databento/detail/buffer.hpp"
#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"
#include "databento/log.hpp"

namespace databento::detail {
class ZstdDecodeStream : public IReadable {
 public:
  explicit ZstdDecodeStream(std::unique_ptr<IReadable> input);
  ZstdDecodeStream(std::unique_ptr<IReadable> input, detail::Buffer& in_buffer);

  // Read exactly `length` bytes into `buffer`.
  void ReadExact(std::byte* buffer, std::size_t length) override;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::byte* buffer, std::size_t max_length) override;

  IReadable* Input() const { return input_.get(); }

 private:
  std::unique_ptr<IReadable> input_;
  std::unique_ptr<ZSTD_DStream, std::size_t (*)(ZSTD_DStream*)> z_dstream_;
  std::size_t read_suggestion_;
  std::vector<std::byte> in_buffer_;
  ZSTD_inBuffer z_in_buffer_;
};

class ZstdCompressStream : public IWritable {
 public:
  explicit ZstdCompressStream(IWritable* output);
  ZstdCompressStream(ILogReceiver* log_receiver, IWritable* output);
  ZstdCompressStream(const ZstdCompressStream&) = delete;
  ZstdCompressStream& operator=(const ZstdCompressStream&) = delete;
  ZstdCompressStream(ZstdCompressStream&&) = delete;
  ZstdCompressStream& operator=(ZstdCompressStream&&) = delete;
  ~ZstdCompressStream() override;

  void WriteAll(const std::byte* buffer, std::size_t length) override;

 private:
  ILogReceiver* log_receiver_;
  IWritable* output_;
  std::unique_ptr<ZSTD_CStream, std::size_t (*)(ZSTD_CStream*)> z_cstream_;
  std::vector<std::byte> in_buffer_;
  ZSTD_inBuffer z_in_buffer_;
  std::size_t in_size_;
  std::vector<std::byte> out_buffer_;
};
}  // namespace databento::detail
