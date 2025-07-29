#include "databento/detail/zstd_stream.hpp"

#include <algorithm>
#include <sstream>
#include <utility>  // move

#include "databento/detail/buffer.hpp"
#include "databento/exceptions.hpp"
#include "databento/log.hpp"

using databento::detail::ZstdDecodeStream;

ZstdDecodeStream::ZstdDecodeStream(std::unique_ptr<IReadable> input)
    : input_{std::move(input)},
      z_dstream_{::ZSTD_createDStream(), ::ZSTD_freeDStream},
      read_suggestion_{::ZSTD_initDStream(z_dstream_.get())},
      in_buffer_{},
      z_in_buffer_{in_buffer_.data(), 0, 0} {}

ZstdDecodeStream::ZstdDecodeStream(std::unique_ptr<IReadable> input,
                                   detail::Buffer& in_buffer)
    : input_{std::move(input)},
      z_dstream_{::ZSTD_createDStream(), ::ZSTD_freeDStream},
      read_suggestion_{::ZSTD_initDStream(z_dstream_.get())},
      in_buffer_{in_buffer.ReadBegin(), in_buffer.ReadEnd()},
      z_in_buffer_{in_buffer_.data(), in_buffer_.size(), 0} {
  in_buffer.Consume(in_buffer.ReadCapacity());
}

void ZstdDecodeStream::ReadExact(std::byte* buffer, std::size_t length) {
  std::size_t size{};
  do {
    size += ReadSome(&buffer[size], length - size);
  } while (size < length && read_suggestion_ != 0);
  // check for end of stream without obtaining `length` bytes
  if (size < length) {
    std::ostringstream err_msg;
    err_msg << "Reached end of Zstd stream without " << length << " bytes, only "
            << size << " bytes available";
    throw DbnResponseError{err_msg.str()};
  }
}

std::size_t ZstdDecodeStream::ReadSome(std::byte* buffer, std::size_t max_length) {
  ZSTD_outBuffer z_out_buffer{buffer, max_length, 0};
  std::size_t read_size = 0;
  do {
    const auto unread_input = z_in_buffer_.size - z_in_buffer_.pos;
    if (unread_input > 0) {
      std::copy(in_buffer_.cbegin() + static_cast<std::ptrdiff_t>(z_in_buffer_.pos),
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

using databento::detail::ZstdCompressStream;

ZstdCompressStream::ZstdCompressStream(IWritable* output)
    : ZstdCompressStream{ILogReceiver::Default(), output} {}
ZstdCompressStream::ZstdCompressStream(ILogReceiver* log_receiver, IWritable* output)
    : log_receiver_{log_receiver},
      output_{output},
      z_cstream_{::ZSTD_createCStream(), ::ZSTD_freeCStream},
      in_buffer_{},
      z_in_buffer_{in_buffer_.data(), 0, 0},
      in_size_{::ZSTD_CStreamInSize()},
      out_buffer_(::ZSTD_CStreamOutSize()) {
  in_buffer_.reserve(in_size_);
  z_in_buffer_.src = in_buffer_.data();
  // enable checksums
  ::ZSTD_CCtx_setParameter(z_cstream_.get(), ZSTD_c_checksumFlag, 1);
}

ZstdCompressStream::~ZstdCompressStream() {
  ZSTD_outBuffer z_out_buffer{out_buffer_.data(), out_buffer_.size(), 0};
  while (true) {
    const std::size_t remaining = ::ZSTD_compressStream2(
        z_cstream_.get(), &z_out_buffer, &z_in_buffer_, ::ZSTD_e_end);
    if (remaining == 0) {
      break;
    }
    if (::ZSTD_isError(remaining) && log_receiver_) {
      log_receiver_->Receive(LogLevel::Error,
                             std::string{"Zstd error compressing end of stream: "} +
                                 ::ZSTD_getErrorName(remaining));
      break;
    }
  }
  assert(z_in_buffer_.pos == z_in_buffer_.size);
  // Forward compressed output
  if (z_out_buffer.pos > 0) {
    output_->WriteAll(out_buffer_.data(), z_out_buffer.pos);
  }
}

void ZstdCompressStream::WriteAll(const std::byte* buffer, std::size_t length) {
  in_buffer_.insert(in_buffer_.end(), buffer, buffer + length);
  z_in_buffer_ = {in_buffer_.data(), in_buffer_.size(), 0};
  // Wait for sufficient data before compressing
  if (in_buffer_.size() >= in_size_) {
    ZSTD_outBuffer z_out_buffer{out_buffer_.data(), out_buffer_.size(), 0};
    const std::size_t remaining = ::ZSTD_compressStream2(
        z_cstream_.get(), &z_out_buffer, &z_in_buffer_, ::ZSTD_e_continue);
    if (::ZSTD_isError(remaining)) {
      throw DbnResponseError{std::string{"Zstd error compressing: "} +
                             ::ZSTD_getErrorName(remaining)};
    }
    // Shift unread input to front
    const auto unread_input = z_in_buffer_.size - z_in_buffer_.pos;
    if (unread_input > 0) {
      std::copy(in_buffer_.cbegin() + static_cast<std::ptrdiff_t>(z_in_buffer_.pos),
                in_buffer_.cend(), in_buffer_.begin());
    }
    in_buffer_.resize(unread_input);
    if (z_out_buffer.pos > 0) {
      // Forward compressed output
      output_->WriteAll(out_buffer_.data(), z_out_buffer.pos);
    }
  }
}
