#include "databento/live_blocking.hpp"

#include <openssl/sha.h>  // SHA256, SHA256_DIGEST_LENGTH

#include <algorithm>  // copy
#include <ios>        //hex, setfill, setw

#include "databento/constants.hpp"   //  kApiKeyLength
#include "databento/exceptions.hpp"  // LiveApiError
#include "databento/record.hpp"      // Record
#include "databento/symbology.hpp"   // JoinSymbolStrings

using databento::LiveBlocking;

namespace {
constexpr std::size_t kBucketIdLength = 5;
}  // namespace

// TODO(cg): gateway resolution
LiveBlocking::LiveBlocking(std::string key, LiveGateway, bool send_ts_out)
    : LiveBlocking{std::move(key), "127.0.0.1", 8080, send_ts_out} {}

LiveBlocking::LiveBlocking(std::string key, std::string gateway,
                           std::uint16_t port, bool send_ts_out)
    : key_{std::move(key)},
      gateway_{std::move(gateway)},
      send_ts_out_{send_ts_out},
      client_{gateway_, port},
      session_id_{this->Authenticate()} {}

void LiveBlocking::Subscribe(const std::string& dataset,
                             const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in) {
  std::ostringstream sub_msg;
  sub_msg << "dataset=" << dataset << "|schema=" << ToString(schema)
          << "|stype_in=" << ToString(stype_in) << "|symbols="
          << JoinSymbolStrings("LiveBlocking::Subscribe", symbols);
  sub_msg << '\n';

  client_.WriteAll(sub_msg.str());
}

void LiveBlocking::Start() { client_.WriteAll("start_session\n"); }

const databento::Record& LiveBlocking::NextRecord() { return *NextRecord({}); }

const databento::Record* LiveBlocking::NextRecord(
    std::chrono::milliseconds timeout) {
  if (buffer_idx_ >= buffer_size_) {
    const auto read_res = client_.Read(buffer_.data(), buffer_.size(), timeout);
    if (read_res.status == detail::TcpClient::Status::Timeout) {
      return nullptr;
    }
    if (read_res.status == detail::TcpClient::Status::Closed) {
      throw LiveApiError{"Server closed the socket"};
    }
    buffer_size_ = read_res.read_size;
    buffer_idx_ = 0;
  }
  // assume buffer_idx_ is record-aligned, first byte is length of next record
  const auto raw_length = static_cast<std::uint8_t>(buffer_[buffer_idx_]);
  const auto length = static_cast<std::size_t>(raw_length) * 4;
  if (buffer_idx_ + length > buffer_size_) {
    // only a partial record remaining in buffer. Shift unread bytes to
    // beginning of buffer.
    std::copy(buffer_.cbegin() + buffer_idx_, buffer_.cend(), buffer_.begin());
    buffer_size_ -= buffer_idx_;
    buffer_idx_ = 0;
    const auto read_res = client_.Read(&buffer_[buffer_size_],
                                       buffer_.size() - buffer_size_, timeout);
    if (read_res.status == detail::TcpClient::Status::Timeout) {
      return nullptr;
    }
    if (read_res.status == detail::TcpClient::Status::Closed) {
      throw LiveApiError{"Server closed the socket"};
    }
    buffer_size_ += read_res.read_size;
    if (buffer_size_ < length) {
      return nullptr;
    }
  }
  current_record_ =
      Record{reinterpret_cast<RecordHeader*>(&buffer_[buffer_idx_])};
  buffer_idx_ += length;
  return &current_record_;
}

std::string LiveBlocking::DecodeChallenge() {
  buffer_size_ = client_.Read(buffer_.data(), buffer_.size()).read_size;
  if (buffer_size_ == 0) {
    throw LiveApiError{"Server closed socket during authentication"};
  }
  // first line is version
  std::string response{buffer_.data(), buffer_size_};
  auto first_nl_pos = response.find('\n');
  if (first_nl_pos == std::string::npos) {
    throw LiveApiError::UnexpectedMsg("Received malformed initial message",
                                      response);
  }
  const auto version_line = response.substr(0, first_nl_pos);
  std::size_t find_start{};
  if (first_nl_pos + 1 == response.length()) {
    // read more
    buffer_size_ = client_.Read(buffer_.data(), buffer_.size()).read_size;
    if (buffer_size_ == 0) {
      throw LiveApiError{"Server closed socket during authentication"};
    }
    response = {buffer_.data(), buffer_size_};
    find_start = 0;
  } else {
    find_start = first_nl_pos + 1;
  }
  const auto next_nl_pos = response.find('\n', find_start);
  if (next_nl_pos == std::string::npos) {
    throw LiveApiError::UnexpectedMsg("Received malformed initial message",
                                      response);
  }
  const auto challenge_line = response.substr(find_start, next_nl_pos);
  if (challenge_line.compare(0, 4, "cram") != 0) {
    throw LiveApiError::UnexpectedMsg(
        "Did not receive CRAM challenge when expected", challenge_line);
  }
  const auto equal_pos = challenge_line.find('=');
  if (equal_pos == std::string::npos || equal_pos + 1 > challenge_line.size()) {
    throw LiveApiError::UnexpectedMsg("Received malformed CRAM challenge",
                                      challenge_line);
  }
  return challenge_line.substr(equal_pos + 1);
}

std::string LiveBlocking::Authenticate() {
  const std::string challenge_key = DecodeChallenge() + '|' + key_;

  std::string auth = GenerateCramReply(challenge_key);
  const std::string req = EncodeAuthReq(auth);
  client_.WriteAll(req.c_str(), req.size());
  DecodeAuthResp();

  return auth;
}

std::string LiveBlocking::GenerateCramReply(const std::string& challenge_key) {
  std::array<unsigned char, SHA256_DIGEST_LENGTH> sha{};
  const unsigned char* sha_res =
      ::SHA256(reinterpret_cast<const unsigned char*>(challenge_key.c_str()),
               challenge_key.size(), sha.data());
  if (sha_res == nullptr) {
    throw LiveApiError{"Unable to generate SHA 256"};
  }

  std::ostringstream auth_stream;
  for (const unsigned char c : sha) {
    auth_stream << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<std::uint16_t>(c);
  }
  auth_stream << '-' << key_.substr(kApiKeyLength - kBucketIdLength);
  return auth_stream.str();
}

std::string LiveBlocking::EncodeAuthReq(const std::string& auth) {
  std::ostringstream reply_stream;
  reply_stream << "auth=" << auth << "|encoding=dbz|"
               << "ts_out=" << send_ts_out_ << '\n';
  return reply_stream.str();
}

void LiveBlocking::DecodeAuthResp() {
  // handle split packet read
  std::array<char, kMaxStrLen>::const_iterator nl_it;
  buffer_size_ = 0;
  do {
    buffer_idx_ = buffer_size_;
    const auto read_size =
        client_.Read(&buffer_[buffer_idx_], buffer_.size() - buffer_idx_)
            .read_size;
    if (read_size == 0) {
      throw LiveApiError{
          "Unexpected end of message received from server after replying to "
          "CRAM"};
    }
    buffer_size_ += read_size;
    nl_it = std::find(buffer_.begin() + buffer_idx_,
                      buffer_.begin() + buffer_size_, '\n');
  } while (nl_it == buffer_.end());
  // one beyond newline
  const std::string response{buffer_.cbegin(), nl_it + 1};
  // set in case Read call also read records
  buffer_idx_ = response.length();

  std::size_t pos{};
  std::size_t count{};
  bool found_success{};
  bool is_error{};
  std::string err_details;
  while ((count = response.find('|', pos)) != std::string::npos) {
    const std::string kv_pair = response.substr(pos, count);
    const std::size_t eq_pos = kv_pair.find('=');
    if (eq_pos == std::string::npos) {
      throw LiveApiError::UnexpectedMsg("Malformed authentication response",
                                        response);
    }
    const std::string key = kv_pair.substr(0, eq_pos);
    if (key == "success") {
      found_success = true;
      if (kv_pair.substr(eq_pos + 1) != "1") {
        is_error = true;
      }
    } else if (key == "error") {
      err_details = kv_pair.substr(eq_pos);
    }
    pos += count + 1;
  }
  if (!found_success) {
    throw LiveApiError{
        "Did not receive success indicator from authentication attempt"};
  }
  if (is_error) {
    throw InvalidArgumentError{"LiveBlocking::Live", "key",
                               "Failed to authenticate: " + err_details};
  }
}
