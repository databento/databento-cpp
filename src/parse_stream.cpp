#include "parse_stream.hpp"

#include <sstream>  // ostringstream

#include "databento/exceptions.hpp"  // DbzResponseError

using databento::ParseStream;

ParseStream::~ParseStream() { Finish(); }

void ParseStream::Write(const std::uint8_t* data, std::size_t length) {
  std::lock_guard<std::mutex> lock{mutex_};
  stream_.write(reinterpret_cast<const char*>(data),
                static_cast<std::streamsize>(length));
  cv_.notify_one();
}

void ParseStream::Finish() {
  std::lock_guard<std::mutex> lock{mutex_};
  is_finished_ = true;
  cv_.notify_one();
}

void ParseStream::ReadExact(std::uint8_t* buffer, std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  cv_.wait(lock,
           [this, length] { return stream_size() >= length || is_finished_; });
  if (stream_size() < length) {
    std::ostringstream err_msg;
    err_msg << "Reached end of the stream with only " << stream_size()
            << " bytes remaining";
    throw DbzResponseError{err_msg.str()};
  }
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(length));
}

std::size_t ParseStream::ReadSome(std::uint8_t* buffer, std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  cv_.wait(lock, [this] { return stream_size() > 0 || is_finished_; });
  if (stream_size() == 0) {
    return 0;
  }
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(std::min(stream_size(), length)));
  return static_cast<std::size_t>(stream_.gcount());
}

// lock_ must be held to call this method
std::size_t ParseStream::stream_size() {
  const auto pos = stream_.tellg();
  stream_.seekg(0, std::ios::end);
  const auto remaining = stream_.tellg() - pos;
  // reset state
  stream_.seekg(pos, std::ios::beg);
  return static_cast<std::size_t>(remaining);
}
