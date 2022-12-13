#include "databento/live.hpp"

#include <openssl/sha.h>  // SHA256

#include <cstddef>  // size_t
#include <cstdint>
#include <ios>       // hex
#include <iostream>  // cout
#include <sstream>   // ostringstream
#include <utility>   // move

#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError, LiveApiError
#include "databento/symbology.hpp"

using databento::Live;

namespace {
constexpr std::size_t kApiKeyLength = 32;
constexpr std::size_t kBucketIdLength = 5;
}  // namespace

// TODO(cg): gateway resolution
Live::Live(std::string key, LiveGateway, bool send_ts_out)
    : Live{std::move(key), "127.0.0.1", 8080, send_ts_out} {}

Live::Live(std::string key, std::string gateway, std::uint16_t port,
           bool send_ts_out)
    : key_{std::move(key)},
      gateway_{std::move(gateway)},
      send_ts_out_{send_ts_out},
      client_{gateway_, port},
      session_id_{this->Authenticate()} {}

// Live::~Live() { keep_going_ = false; }

std::string Live::Authenticate() {
  const std::string challenge_key = DecodeChallenge() + '|' + key_;
  std::cout << "Challenge key: " << challenge_key << '\n';

  std::string auth = GenerateCramReply(challenge_key);
  const std::string req = EncodeAuthReq(auth);
  std::cout << "Sending CRAM reply: " << req;
  client_.WriteAll(req.c_str(), req.size());

  const std::size_t read_size = client_.Read(buffer_.data(), buffer_.size());
  if (read_size == 0) {
    throw LiveApiError{"No data received from server when replying to CRAM"};
  }
  const std::string auth_response{buffer_.data(), read_size};
  DecodeAuthResp(auth_response);

  return auth;
}

void Live::Subscribe(const std::string& dataset,
                     const std::vector<std::string>& symbols, Schema schema,
                     SType stype_in) {
  std::ostringstream sub_msg;
  sub_msg << "dataset=" << dataset << "|schema=" << ToString(schema)
          << "|stype_in=" << ToString(stype_in)
          << "|symbols=" << JoinSymbolStrings("Live::Subscribe", symbols);
  sub_msg << '\n';

  std::cout << "Subscribing: " << sub_msg.str();
  client_.WriteAll(sub_msg.str());
}

std::unique_ptr<Live::ThreadedToken> Live::StartThreaded(
    const Live::RecordCallback& callback) {
  return std::unique_ptr<ThreadedToken>{
      new ThreadedToken{StartAsync(callback)}};
}

Live::AsyncToken Live::StartAsync(Live::RecordCallback callback) {
  client_.WriteAll("start_session");
  return AsyncToken{this, std::move(callback)};
}

std::string Live::DecodeChallenge() {
  std::cout << "Received from server:\n";
  while (true) {
    std::size_t read_size = client_.Read(buffer_.data(), buffer_.size());
    if (read_size == 0) {
      throw LiveApiError{"No data received from server when authenticating"};
    }
    const std::string challenge_response{buffer_.data(), read_size};
    std::size_t pos{};
    std::size_t count{};
    std::string challenge;
    while ((count = challenge_response.find('\n', pos)) != std::string::npos) {
      const std::string line = challenge_response.substr(pos, count);
      std::cout << line;
      if (line.compare(0, 4, "cram") == 0) {
        const auto equal_pos = line.find('=');
        if (equal_pos == std::string::npos || equal_pos + 1 > line.size()) {
          throw LiveApiError{"Malformed CRAM challenge"};
        }
        challenge = line.substr(equal_pos + 1);
      }
      pos += count;
    }
    if (!challenge.empty()) {
      return challenge;
    }
  }
}

std::string Live::GenerateCramReply(const std::string& challenge_key) {
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

std::string Live::EncodeAuthReq(const std::string& auth) {
  std::ostringstream reply_stream;
  reply_stream << "auth=" << auth << "|encoding=dbz|"
               << "ts_out=" << send_ts_out_ << '\n';
  return reply_stream.str();
}

void Live::DecodeAuthResp(const std::string& response) {
  std::size_t pos{};
  std::size_t count{};
  bool found_success{};
  bool is_error{};
  std::string err_details;
  while ((count = response.find('|', pos)) != std::string::npos) {
    const std::string kv_pair = response.substr(pos, count);
    const std::size_t eq_pos = kv_pair.find('=');
    if (eq_pos == std::string::npos) {
      throw LiveApiError{"Malformed authentication response: " + response};
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
    throw InvalidArgumentError{"Live::Live", "key",
                               "Failed to authenticate: " + err_details};
  }
}

void Live::AwaitRecord(const RecordCallback& callback) {
  const auto read_size = client_.Read(buffer_.data(), buffer_.size());
  auto buf_it = buffer_.begin();
  const auto buf_end = buffer_.begin() + read_size;
  while (buf_it != buf_end) {
    const auto length = static_cast<std::uint8_t>(*buf_it);
    const Record record{reinterpret_cast<RecordHeader*>(&*buf_it)};
    callback(record);
    buf_it += static_cast<std::ptrdiff_t>(length) * 4;
  }
}

Live::AsyncToken::AsyncToken(Live* client, RecordCallback callback)
    : client_{client}, callback_{std::move(callback)} {}

void Live::AsyncToken::AwaitRecord() { client_->AwaitRecord(callback_); }

Live::ThreadedToken::ThreadedToken(AsyncToken&& async_token)
    : keep_going_{true},
      thread_{&ThreadedToken::ProcessingThread, this, std::move(async_token)} {}

void Live::ThreadedToken::ProcessingThread(AsyncToken&& token) {
  while (keep_going_.load()) {
    token.AwaitRecord();
  }
}

using databento::LiveBuilder;

LiveBuilder& LiveBuilder::SetKeyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw Exception{
        "Expected environment variable DATABENTO_API_KEY to be set"};
  }
  return this->SetKey(env_key);
}

LiveBuilder& LiveBuilder::SetKey(std::string key) {
  if (key.length() != kApiKeyLength) {
    throw InvalidArgumentError{
        "LiveBuilder::SetKey", "key",
        "Must contain " + std::to_string(kApiKeyLength) + " characters"};
  }
  key_ = std::move(key);
  return *this;
}

LiveBuilder& LiveBuilder::SetGateway(LiveGateway gateway) {
  gateway_ = gateway;
  return *this;
}

LiveBuilder& LiveBuilder::SetSendTsOut(bool send_ts_out) {
  send_ts_out_ = send_ts_out;
  return *this;
}

Live LiveBuilder::Build() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  return Live{std::move(key_), gateway_, send_ts_out_};
}
