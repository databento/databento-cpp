#pragma once

#include <cstddef>
#include <memory>

#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento::detail {
class Buffer : public IReadable, public IWritable {
 public:
  Buffer() : Buffer(64 * std::size_t{1 << 10}) {}
  explicit Buffer(std::size_t init_capacity)
      : buf_{std::make_unique<std::byte[]>(init_capacity)},
        end_{buf_.get() + init_capacity},
        read_pos_{buf_.get()},
        write_pos_{buf_.get()} {}

  size_t Write(const char* data, std::size_t length);
  size_t Write(const std::byte* data, std::size_t length);
  void WriteAll(const char* data, std::size_t length);
  void WriteAll(const std::byte* data, std::size_t length) override;

  std::byte*& WriteBegin() { return write_pos_; }
  std::byte* WriteEnd() const { return end_; }
  std::size_t WriteCapacity() const {
    return static_cast<std::size_t>(end_ - write_pos_);
  }

  /// Will throw if `length > ReadCapacity()`.
  void ReadExact(std::byte* buffer, std::size_t length) override;
  std::size_t ReadSome(std::byte* buffer, std::size_t max_length) override;

  std::byte*& ReadBegin() { return read_pos_; }
  std::byte* ReadEnd() const { return write_pos_; }
  std::size_t ReadCapacity() const {
    return static_cast<std::size_t>(write_pos_ - read_pos_);
  }

  std::size_t Capacity() const {
    return static_cast<std::size_t>(end_ - buf_.get());
  }
  void Clear() {
    read_pos_ = buf_.get();
    write_pos_ = buf_.get();
  }
  void Reserve(std::size_t capacity);
  void Shift();

 private:
  std::unique_ptr<std::byte[]> buf_;
  std::byte* end_;
  std::byte* read_pos_{};
  std::byte* write_pos_{};
};
}  // namespace databento::detail
