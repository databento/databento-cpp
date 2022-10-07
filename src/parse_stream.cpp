#include "parse_stream.hpp"

#include <chrono>   // milliseconds
#include <sstream>  // ostringstream

#include "databento/exceptions.hpp"  // DbzResponseError

using databento::ParseStream;

namespace {
constexpr std::chrono::milliseconds kLockTimeout{500};
}

ParseStream::~ParseStream() { Finish(); }

void ParseStream::Write(const std::uint8_t* data, std::size_t length) {
  std::lock_guard<std::mutex> lock{mutex_};
  stream_.write(data, static_cast<std::streamsize>(length));
  cv_.notify_one();
}

void ParseStream::Finish() {
  is_finished_ = true;
  std::lock_guard<std::mutex> lock{mutex_};
  cv_.notify_one();
}

void ParseStream::ReadExact(std::uint8_t* buffer, std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  if (stream_size() < length &&
      !cv_.wait_for(lock, ::kLockTimeout, [this, length] {
        return stream_size() >= length || is_finished_;
      })) {
    throw DbzResponseError{
        std::string{
            "Timed out waiting for parser stream after 500ms. Stream size: "} +
        std::to_string(stream_size())};
  }
  if (stream_size() < length) {
    std::ostringstream err_msg;
    err_msg << "Reached end of the stream with only " << stream_size()
            << " bytes remaining";
    throw DbzResponseError{err_msg.str()};
  }
  stream_.read(buffer, static_cast<std::streamsize>(length));
}

std::size_t ParseStream::ReadSome(std::uint8_t* buffer, std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  if (stream_size() == 0 && !cv_.wait_for(lock, ::kLockTimeout, [this] {
        return stream_size() > 0 || is_finished_;
      })) {
    throw DbzResponseError{
        std::string{
            "Timed out waiting for parser stream after 500ms. Stream size: "} +
        std::to_string(stream_size())};
  }
  if (stream_size() == 0) {
    return 0;
  }
  stream_.read(buffer,
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
