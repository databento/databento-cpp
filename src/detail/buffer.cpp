#include "databento/detail/buffer.hpp"

#include <algorithm>
#include <sstream>

#include "databento/exceptions.hpp"
#include "stream_op_helper.hpp"

using databento::detail::Buffer;

size_t Buffer::Write(const char* data, std::size_t length) {
  return Write(reinterpret_cast<const std::byte*>(data), length);
}
size_t Buffer::Write(const std::byte* data, std::size_t length) {
  if (length > WriteCapacity()) {
    Shift();
  }
  const auto write_size = std::min(WriteCapacity(), length);
  std::copy(data, data + write_size, WriteBegin());
  Fill(write_size);
  return write_size;
}

void Buffer::WriteAll(const char* data, std::size_t length) {
  WriteAll(reinterpret_cast<const std::byte*>(data), length);
}
void Buffer::WriteAll(const std::byte* data, std::size_t length) {
  if (length > Capacity() - ReadCapacity()) {
    Reserve(ReadCapacity() + length);
  } else if (length >= WriteCapacity()) {
    Shift();
  }
  std::copy(data, data + length, WriteBegin());
  write_pos_ += length;
}

void Buffer::ReadExact(std::byte* buffer, std::size_t length) {
  if (length > ReadCapacity()) {
    std::ostringstream err_msg;
    err_msg << "Reached end of buffer without " << length << " bytes, only "
            << ReadCapacity() << " bytes available";
    throw databento::Exception{err_msg.str()};
  }
  ReadSome(buffer, length);
}

std::size_t Buffer::ReadSome(std::byte* buffer, std::size_t max_length) {
  const auto read_size = std::min(ReadCapacity(), max_length);
  std::copy(ReadBegin(), ReadBegin() + read_size, buffer);
  Consume(read_size);
  return read_size;
}

void Buffer::Reserve(std::size_t capacity) {
  if (capacity <= Capacity()) {
    return;
  }
  UniqueBufPtr new_buf{AlignedNew(capacity), AlignedDelete};
  const auto unread_bytes = ReadCapacity();
  std::copy(ReadBegin(), ReadEnd(), new_buf.get());
  buf_ = std::move(new_buf);
  end_ = buf_.get() + capacity;
  read_pos_ = buf_.get();
  write_pos_ = read_pos_ + unread_bytes;
}

void Buffer::Shift() {
  const auto unread_bytes = ReadCapacity();
  if (unread_bytes) {
    std::copy(ReadBegin(), ReadEnd(), buf_.get());
  }
  read_pos_ = buf_.get();
  write_pos_ = read_pos_ + unread_bytes;
}

namespace databento::detail {
std::ostream& operator<<(std::ostream& stream, const Buffer& buffer) {
  return StreamOpBuilder{stream}
      .SetTypeName("Buffer")
      .SetSpacer(" ")
      .Build()
      .AddField("buf_", buffer.buf_.get())
      .AddField("end_", buffer.end_)
      .AddField("read_pos", buffer.read_pos_)
      .AddField("write_pos_", buffer.write_pos_)
      .AddField("ReadCapacity", buffer.ReadCapacity())
      .AddField("WriteCapacity", buffer.WriteCapacity())
      .Finish();
}
}  // namespace databento::detail
