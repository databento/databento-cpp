#include "databento/detail/shared_channel.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <sstream>  // stringstream
#include <vector>

#include "databento/exceptions.hpp"  // DbzResponseError

namespace databento {
namespace detail {
class SharedChannel::Channel {
 public:
  Channel() = default;
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;
  Channel(Channel&&) = delete;
  Channel& operator=(Channel&&) = delete;
  ~Channel();

  void Write(const std::uint8_t* data, std::size_t length);
  void Finish();
  // Read exactly `length` bytes
  void ReadExact(std::uint8_t* buffer, std::size_t length);
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::uint8_t* buffer, std::size_t length);

 private:
  std::size_t Size();

  // protects all other data members of this class
  std::mutex mutex_;
  // could use eofbit, but seekg clears this
  bool is_finished_{false};
  std::condition_variable cv_;
  std::stringstream stream_;
};
}  // namespace detail
}  // namespace databento

using databento::detail::SharedChannel;

SharedChannel::SharedChannel() : channel_{std::make_shared<Channel>()} {}

void SharedChannel::Write(const std::uint8_t* data, std::size_t length) {
  channel_->Write(data, length);
}

void SharedChannel::Finish() { channel_->Finish(); }

void SharedChannel::ReadExact(std::uint8_t* buffer, std::size_t length) {
  channel_->ReadExact(buffer, length);
}

// Read at most `length` bytes. Returns the number of bytes read. Will only
// return 0 if the end of the stream is reached.
std::size_t SharedChannel::ReadSome(std::uint8_t* buffer,
                                    std::size_t max_length) {
  return channel_->ReadSome(buffer, max_length);
}

SharedChannel::Channel::~Channel() { Finish(); }

void SharedChannel::Channel::Write(const std::uint8_t* data,
                                   std::size_t length) {
  std::lock_guard<std::mutex> lock{mutex_};
  stream_.write(reinterpret_cast<const char*>(data),
                static_cast<std::streamsize>(length));
  cv_.notify_one();
}

void SharedChannel::Channel::Finish() {
  std::lock_guard<std::mutex> lock{mutex_};
  is_finished_ = true;
  cv_.notify_one();
}

void SharedChannel::Channel::ReadExact(std::uint8_t* buffer,
                                       std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  cv_.wait(lock, [this, length] { return Size() >= length || is_finished_; });
  if (Size() < length) {
    std::ostringstream err_msg;
    err_msg << "Reached end of the stream with only " << Size()
            << " bytes remaining";
    throw DbzResponseError{err_msg.str()};
  }
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(length));
}

std::size_t SharedChannel::Channel::ReadSome(std::uint8_t* buffer,
                                             std::size_t length) {
  std::unique_lock<std::mutex> lock{mutex_};
  cv_.wait(lock, [this] { return Size() > 0 || is_finished_; });
  if (Size() == 0) {
    return 0;
  }
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(std::min(Size(), length)));
  return static_cast<std::size_t>(stream_.gcount());
}

// lock_ must be held to call this method
std::size_t SharedChannel::Channel::Size() {
  const auto pos = stream_.tellg();
  stream_.seekg(0, std::ios::end);
  const auto remaining = stream_.tellg() - pos;
  // reset state
  stream_.seekg(pos, std::ios::beg);
  return static_cast<std::size_t>(remaining);
}
