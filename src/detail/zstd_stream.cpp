#include "databento/detail/zstd_stream.hpp"

#include <sstream>
#include <utility>  // move

#include "databento/exceptions.hpp"

using databento::detail::ZstdStream;

ZstdStream::ZstdStream(std::unique_ptr<IReadable> input)
    : ZstdStream{std::move(input), {}} {}

ZstdStream::ZstdStream(std::unique_ptr<IReadable> input,
                       std::vector<std::uint8_t>&& in_buffer)
    : input_{std::move(input)},
      z_dstream_{::ZSTD_createDStream(), ::ZSTD_freeDStream},
      read_suggestion_{::ZSTD_initDStream(z_dstream_.get())},
      in_buffer_{std::move(in_buffer)},
      z_in_buffer_{in_buffer_.data(), in_buffer_.size(), 0} {}

void ZstdStream::ReadExact(std::uint8_t* buffer, std::size_t length) {
  std::size_t size{};
  do {
    size += ReadSome(&buffer[size], length - size);
  } while (size < length && read_suggestion_ != 0);
  // check for end of stream without obtaining `length` bytes
  if (size < length) {
    std::ostringstream err_msg;
    err_msg << "Reached end of Zstd stream without " << length
            << " bytes, only " << size << " bytes available";
    throw DbnResponseError{err_msg.str()};
  }
}

size_t ZstdStream::ReadSome(std::uint8_t* buffer, std::size_t max_length) {
  ZSTD_outBuffer z_out_buffer{buffer, max_length, 0};
  std::size_t read_size = 0;
  do {
    const auto unread_input = z_in_buffer_.size - z_in_buffer_.pos;
    if (unread_input > 0) {
      std::copy(
          in_buffer_.cbegin() + static_cast<std::ptrdiff_t>(z_in_buffer_.pos),
          in_buffer_.cend(), in_buffer_.begin());
    }
    if (read_suggestion_ == 0) {
      // next frame
      read_suggestion_ = ::ZSTD_initDStream(z_dstream_.get());
    }
    const auto new_size = unread_input + read_suggestion_;
    if (new_size != in_buffer_.size()) {
      in_buffer_.resize(new_size);
      z_in_buffer_.src = in_buffer_.data();
    }
    read_size = input_->ReadSome(&in_buffer_[unread_input], read_suggestion_);
    z_in_buffer_.size = unread_input + read_size;
    z_in_buffer_.pos = 0;

    read_suggestion_ =
        ::ZSTD_decompressStream(z_dstream_.get(), &z_out_buffer, &z_in_buffer_);
    if (::ZSTD_isError(read_suggestion_)) {
      throw DbnResponseError{std::string{"Zstd error decompressing: "} +
                             ::ZSTD_getErrorName(read_suggestion_)};
    }
  } while (z_out_buffer.pos == 0 && read_size > 0);
  return z_out_buffer.pos;
}
