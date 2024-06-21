#include "databento/live_blocking.hpp"

#include <openssl/sha.h>  // SHA256, SHA256_DIGEST_LENGTH

#include <algorithm>  // copy
#include <cctype>     // tolower
#include <chrono>
#include <cstddef>  // ptrdiff_t
#include <cstdlib>
#include <ios>  //hex, setfill, setw
#include <sstream>

#include "databento/constants.hpp"  //  kApiKeyLength
#include "databento/dbn_decoder.hpp"
#include "databento/detail/tcp_client.hpp"
#include "databento/exceptions.hpp"  // LiveApiError
#include "databento/log.hpp"         // ILogReceiver
#include "databento/record.hpp"      // Record
#include "databento/symbology.hpp"   // JoinSymbolStrings
#include "databento/version.hpp"     // DATABENTO_VERSION

using databento::LiveBlocking;

namespace {
constexpr std::size_t kBucketIdLength = 5;
}  // namespace

LiveBlocking::LiveBlocking(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, bool send_ts_out,
                           VersionUpgradePolicy upgrade_policy,
                           std::chrono::seconds heartbeat_interval)

    : log_receiver_{log_receiver},
      key_{std::move(key)},
      dataset_{std::move(dataset)},
      gateway_{DetermineGateway()},
      port_{13000},
      send_ts_out_{send_ts_out},
      upgrade_policy_{upgrade_policy},
      heartbeat_interval_{heartbeat_interval},
      client_{gateway_, port_},
      session_id_{this->Authenticate()} {}

LiveBlocking::LiveBlocking(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, std::string gateway,
                           std::uint16_t port, bool send_ts_out,
                           VersionUpgradePolicy upgrade_policy,
                           std::chrono::seconds heartbeat_interval)
    : log_receiver_{log_receiver},
      key_{std::move(key)},
      dataset_{std::move(dataset)},
      gateway_{std::move(gateway)},
      port_{port},
      send_ts_out_{send_ts_out},
      upgrade_policy_{upgrade_policy},
      heartbeat_interval_{heartbeat_interval},
      client_{gateway_, port_},
      session_id_{this->Authenticate()} {}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in) {
  Subscribe(symbols, schema, stype_in, std::string{""});
}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in, UnixNanos start) {
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema) << "|stype_in=" << ToString(stype_in)
          << "|start=" << start.time_since_epoch().count();
  Subscribe(sub_msg.str(), symbols, false);
}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in,
                             const std::string& start) {
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema)
          << "|stype_in=" << ToString(stype_in);
  if (!start.empty()) {
    sub_msg << "|start=" << start;
  }
  Subscribe(sub_msg.str(), symbols, false);
}

void LiveBlocking::SubscribeWithSnapshot(
    const std::vector<std::string>& symbols, Schema schema, SType stype_in) {
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema)
          << "|stype_in=" << ToString(stype_in);

  Subscribe(sub_msg.str(), symbols, true);
}

void LiveBlocking::Subscribe(const std::string& sub_msg,
                             const std::vector<std::string>& symbols,
                             bool use_snapshot) {
  static constexpr auto kMethodName = "Live::Subscribe";
  constexpr std::ptrdiff_t kSymbolMaxChunkSize = 128;

  if (symbols.empty()) {
    throw InvalidArgumentError{kMethodName, "symbols",
                               "must contain at least one symbol"};
  }
  auto symbols_it = symbols.begin();
  while (symbols_it != symbols.end()) {
    const auto chunk_size =
        std::min(kSymbolMaxChunkSize, std::distance(symbols_it, symbols.end()));

    std::ostringstream chunked_sub_msg;
    chunked_sub_msg << sub_msg << "|symbols="
                    << JoinSymbolStrings(kMethodName, symbols_it,
                                         symbols_it + chunk_size)
                    << "|snapshot=" << use_snapshot << '\n';
    client_.WriteAll(chunked_sub_msg.str());

    symbols_it += chunk_size;
  }
}

databento::Metadata LiveBlocking::Start() {
  constexpr auto kMetadataPreludeSize = 8;
  client_.WriteAll("start_session\n");
  client_.ReadExact(read_buffer_.data(), kMetadataPreludeSize);
  const auto version_and_size = DbnDecoder::DecodeMetadataVersionAndSize(
      reinterpret_cast<std::uint8_t*>(read_buffer_.data()),
      kMetadataPreludeSize);
  std::vector<std::uint8_t> meta_buffer(version_and_size.second);
  client_.ReadExact(reinterpret_cast<char*>(meta_buffer.data()),
                    version_and_size.second);
  auto metadata =
      DbnDecoder::DecodeMetadataFields(version_and_size.first, meta_buffer);
  version_ = metadata.version;
  metadata.Upgrade(upgrade_policy_);
  return metadata;
}

const databento::Record& LiveBlocking::NextRecord() { return *NextRecord({}); }

const databento::Record* LiveBlocking::NextRecord(
    std::chrono::milliseconds timeout) {
  // need some unread_bytes
  const auto unread_bytes = buffer_size_ - buffer_idx_;
  if (unread_bytes == 0) {
    const auto read_res = FillBuffer(timeout);
    if (read_res.status == detail::TcpClient::Status::Timeout) {
      return nullptr;
    }
    if (read_res.status == detail::TcpClient::Status::Closed) {
      throw DbnResponseError{"Gateway closed the session"};
    }
  }
  // check length
  while (buffer_size_ - buffer_idx_ < BufferRecordHeader()->Size()) {
    const auto read_res = FillBuffer(timeout);
    if (read_res.status == detail::TcpClient::Status::Timeout) {
      return nullptr;
    }
    if (read_res.status == detail::TcpClient::Status::Closed) {
      throw DbnResponseError{"Gateway closed the session"};
    }
  }
  current_record_ = Record{BufferRecordHeader()};
  buffer_idx_ += current_record_.Size();
  current_record_ =
      DbnDecoder::DecodeRecordCompat(version_, upgrade_policy_, send_ts_out_,
                                     &compat_buffer_, current_record_);
  return &current_record_;
}

void LiveBlocking::Stop() { client_.Close(); }

void LiveBlocking::Reconnect() {
  client_ = detail::TcpClient{gateway_, port_};
  session_id_ = this->Authenticate();
}

std::string LiveBlocking::DecodeChallenge() {
  buffer_size_ =
      client_.ReadSome(read_buffer_.data(), read_buffer_.size()).read_size;
  if (buffer_size_ == 0) {
    throw LiveApiError{"Gateway closed socket during authentication"};
  }
  // first line is version
  std::string response{read_buffer_.data(), buffer_size_};
  {
    std::ostringstream log_ss;
    log_ss << "[LiveBlocking::DecodeChallenge] Challenge: " << response;
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  auto first_nl_pos = response.find('\n');
  if (first_nl_pos == std::string::npos) {
    throw LiveApiError::UnexpectedMsg("Received malformed initial message",
                                      response);
  }
  const auto find_start = first_nl_pos + 1;
  auto next_nl_pos = find_start == response.length()
                         ? std::string::npos
                         : response.find('\n', find_start);
  while (next_nl_pos == std::string::npos) {
    // read more
    buffer_size_ += client_
                        .ReadSome(&read_buffer_[buffer_size_],
                                  read_buffer_.size() - buffer_size_)
                        .read_size;
    if (buffer_size_ == 0) {
      throw LiveApiError{"Gateway closed socket during authentication"};
    }
    response = {read_buffer_.data(), buffer_size_};
    next_nl_pos = response.find('\n', find_start);
  }
  const auto challenge_line =
      response.substr(find_start, next_nl_pos - find_start);
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

std::string LiveBlocking::DetermineGateway() const {
  std::ostringstream gateway;
  for (const char c : dataset_) {
    gateway << (c == '.' ? '-' : static_cast<char>(std::tolower(c)));
  }
  gateway << ".lsg.databento.com";
  return gateway.str();
}

std::uint64_t LiveBlocking::Authenticate() {
  const std::string challenge_key = DecodeChallenge() + '|' + key_;

  const std::string auth = GenerateCramReply(challenge_key);
  const std::string req = EncodeAuthReq(auth);
  client_.WriteAll(req);
  const std::uint64_t session_id = DecodeAuthResp();

  std::ostringstream log_ss;
  log_ss << "[LiveBlocking::Authenticate] Successfully authenticated with "
            "session_id "
         << session_id;
  log_receiver_->Receive(LogLevel::Info, log_ss.str());

  return session_id;
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
  std::ostringstream req_stream;
  req_stream << "auth=" << auth << "|dataset=" << dataset_ << "|encoding=dbn|"
             << "ts_out=" << send_ts_out_ << "|client=C++ " DATABENTO_VERSION;
  if (heartbeat_interval_.count() > 0) {
    req_stream << "|heartbeat_interval_s=" << heartbeat_interval_.count();
  }
  req_stream << '\n';
  return req_stream.str();
}

std::uint64_t LiveBlocking::DecodeAuthResp() {
  // handle split packet read
  std::array<char, kMaxStrLen>::const_iterator nl_it;
  buffer_size_ = 0;
  do {
    buffer_idx_ = buffer_size_;
    const auto read_size = client_
                               .ReadSome(&read_buffer_[buffer_idx_],
                                         read_buffer_.size() - buffer_idx_)
                               .read_size;
    if (read_size == 0) {
      throw LiveApiError{
          "Unexpected end of message received from server after replying to "
          "CRAM"};
    }
    buffer_size_ += read_size;
    nl_it = std::find(read_buffer_.begin() + buffer_idx_,
                      read_buffer_.begin() + buffer_size_, '\n');
  } while (nl_it == read_buffer_.end());
  const std::string response{read_buffer_.cbegin(), nl_it};
  {
    std::ostringstream log_ss;
    log_ss << "[LiveBlocking::DecodeAuthResp] Authentication response: "
           << response;
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  // set in case Read call also read records. One beyond newline
  buffer_idx_ = response.length() + 1;

  std::size_t pos{};
  bool found_success{};
  bool is_error{};
  std::uint64_t session_id = 0;
  std::string err_details;
  while (true) {
    const size_t count = response.find('|', pos);
    if (count == response.length() - 1) {
      break;
    }
    // passing count = npos to substr will take remainder of string
    const std::string kv_pair = response.substr(pos, count - pos);
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
      err_details = kv_pair.substr(eq_pos + 1);
    } else if (key == "session_id") {
      session_id = std::stoull(kv_pair.substr(eq_pos + 1));
    }
    // no more keys to parse
    if (count == std::string::npos) {
      break;
    }
    pos = count + 1;
  }
  if (!found_success) {
    throw LiveApiError{
        "Did not receive success indicator from authentication attempt"};
  }
  if (is_error) {
    throw InvalidArgumentError{"LiveBlocking::LiveBlocking", "key",
                               "Failed to authenticate: " + err_details};
  }
  return session_id;
}

databento::detail::TcpClient::Result LiveBlocking::FillBuffer(
    std::chrono::milliseconds timeout) {
  // Shift data forward
  std::copy(read_buffer_.cbegin() + static_cast<std::ptrdiff_t>(buffer_idx_),
            read_buffer_.cend(), read_buffer_.begin());
  buffer_size_ -= buffer_idx_;
  buffer_idx_ = 0;
  const auto read_res = client_.ReadSome(
      &read_buffer_[buffer_size_], read_buffer_.size() - buffer_size_, timeout);
  buffer_size_ += read_res.read_size;
  return read_res;
}

databento::RecordHeader* LiveBlocking::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(&read_buffer_[buffer_idx_]);
}
