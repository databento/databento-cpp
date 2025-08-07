#include "databento/live_blocking.hpp"

#include <openssl/sha.h>  // SHA256, SHA256_DIGEST_LENGTH

#include <algorithm>  // copy
#include <cctype>     // tolower
#include <chrono>
#include <cstddef>  // ptrdiff_t
#include <cstdlib>
#include <ios>  // hex, setfill, setw
#include <limits>
#include <sstream>
#include <variant>

#include "databento/constants.hpp"  //  kApiKeyLength
#include "databento/dbn_decoder.hpp"
#include "databento/detail/tcp_client.hpp"
#include "databento/exceptions.hpp"  // LiveApiError
#include "databento/live.hpp"        // LiveBuilder
#include "databento/log.hpp"         // ILogReceiver
#include "databento/record.hpp"      // Record
#include "databento/symbology.hpp"   // JoinSymbolStrings
#include "dbn_constants.hpp"         // kMetadataPreludeSize

using databento::LiveBlocking;

namespace {
constexpr std::size_t kBucketIdLength = 5;
}  // namespace

databento::LiveBuilder LiveBlocking::Builder() { return databento::LiveBuilder{}; }

LiveBlocking::LiveBlocking(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, bool send_ts_out,
                           VersionUpgradePolicy upgrade_policy,
                           std::optional<std::chrono::seconds> heartbeat_interval,
                           std::size_t buffer_size, std::string user_agent_ext)

    : log_receiver_{log_receiver},
      key_{std::move(key)},
      dataset_{std::move(dataset)},
      gateway_{DetermineGateway()},
      user_agent_ext_{std::move(user_agent_ext)},
      port_{13000},
      send_ts_out_{send_ts_out},
      upgrade_policy_{upgrade_policy},
      heartbeat_interval_{heartbeat_interval},
      client_{gateway_, port_},
      buffer_{buffer_size},
      session_id_{this->Authenticate()} {}

LiveBlocking::LiveBlocking(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, std::string gateway, std::uint16_t port,
                           bool send_ts_out, VersionUpgradePolicy upgrade_policy,
                           std::optional<std::chrono::seconds> heartbeat_interval,
                           std::size_t buffer_size, std::string user_agent_ext)
    : log_receiver_{log_receiver},
      key_{std::move(key)},
      dataset_{std::move(dataset)},
      gateway_{std::move(gateway)},
      user_agent_ext_{std::move(user_agent_ext)},
      port_{port},
      send_ts_out_{send_ts_out},
      upgrade_policy_{upgrade_policy},
      heartbeat_interval_{heartbeat_interval},
      client_{gateway_, port_},
      buffer_{buffer_size},
      session_id_{this->Authenticate()} {}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in) {
  Subscribe(symbols, schema, stype_in, std::string{""});
}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in, UnixNanos start) {
  IncrementSubCounter();
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema) << "|stype_in=" << ToString(stype_in)
          << "|start=" << start.time_since_epoch().count()
          << "|id=" << std::to_string(sub_counter_);
  Subscribe(sub_msg.str(), symbols, false);
  subscriptions_.emplace_back(
      LiveSubscription{symbols, schema, stype_in, start, sub_counter_});
}

void LiveBlocking::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in, const std::string& start) {
  IncrementSubCounter();
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema) << "|stype_in=" << ToString(stype_in)
          << "|id=" << std::to_string(sub_counter_);
  if (!start.empty()) {
    sub_msg << "|start=" << start;
  }
  Subscribe(sub_msg.str(), symbols, false);
  if (start.empty()) {
    subscriptions_.emplace_back(LiveSubscription{
        symbols, schema, stype_in, LiveSubscription::NoStart{}, sub_counter_});
  } else {
    subscriptions_.emplace_back(
        LiveSubscription{symbols, schema, stype_in, start, sub_counter_});
  }
}

void LiveBlocking::SubscribeWithSnapshot(const std::vector<std::string>& symbols,
                                         Schema schema, SType stype_in) {
  IncrementSubCounter();
  std::ostringstream sub_msg;
  sub_msg << "schema=" << ToString(schema) << "|stype_in=" << ToString(stype_in)
          << "|id=" << std::to_string(sub_counter_);

  Subscribe(sub_msg.str(), symbols, true);
  subscriptions_.emplace_back(LiveSubscription{
      symbols, schema, stype_in, LiveSubscription::Snapshot{}, sub_counter_});
}

void LiveBlocking::Subscribe(std::string_view sub_msg,
                             const std::vector<std::string>& symbols,
                             bool use_snapshot) {
  static constexpr auto kMethodName = "LiveBlocking::Subscribe";
  constexpr std::ptrdiff_t kSymbolMaxChunkSize = 500;

  if (symbols.empty()) {
    throw InvalidArgumentError{kMethodName, "symbols",
                               "must contain at least one symbol"};
  }
  auto symbols_it = symbols.begin();
  while (symbols_it != symbols.end()) {
    const auto distance_from_end = std::distance(symbols_it, symbols.end());
    const auto chunk_size = std::min(kSymbolMaxChunkSize, distance_from_end);

    std::ostringstream chunked_sub_msg;
    chunked_sub_msg << sub_msg << "|symbols="
                    << JoinSymbolStrings(kMethodName, symbols_it,
                                         symbols_it + chunk_size)
                    << "|snapshot=" << use_snapshot
                    << "|is_last=" << (distance_from_end <= kSymbolMaxChunkSize)
                    << '\n';
    if (log_receiver_->ShouldLog(LogLevel::Debug)) {
      std::ostringstream log_ss;
      log_ss << '[' << kMethodName
             << "] Sending subscription request: " << chunked_sub_msg.str();
      log_receiver_->Receive(LogLevel::Debug, log_ss.str());
    }
    client_.WriteAll(chunked_sub_msg.str());

    symbols_it += chunk_size;
  }
}

databento::Metadata LiveBlocking::Start() {
  log_receiver_->Receive(LogLevel::Info, "[LiveBlocking::Start] Starting session");

  client_.WriteAll("start_session\n");
  client_.ReadExact(buffer_.WriteBegin(), kMetadataPreludeSize);
  buffer_.Fill(kMetadataPreludeSize);
  const auto [version, size] = DbnDecoder::DecodeMetadataVersionAndSize(
      buffer_.ReadBegin(), kMetadataPreludeSize);
  buffer_.Consume(kMetadataPreludeSize);
  buffer_.Reserve(size);
  client_.ReadExact(buffer_.WriteBegin(), size);
  buffer_.Fill(size);
  auto metadata =
      DbnDecoder::DecodeMetadataFields(version, buffer_.ReadBegin(), buffer_.ReadEnd());
  buffer_.Consume(size);
  // Metadata may leave buffer misaligned. Shift records to ensure 8-byte
  // alignment
  buffer_.Shift();
  version_ = metadata.version;
  metadata.Upgrade(upgrade_policy_);
  return metadata;
}

const databento::Record& LiveBlocking::NextRecord() { return *NextRecord({}); }

const databento::Record* LiveBlocking::NextRecord(std::chrono::milliseconds timeout) {
  // need some unread_bytes
  const auto unread_bytes = buffer_.ReadCapacity();
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
  while (buffer_.ReadCapacity() < BufferRecordHeader()->Size()) {
    const auto read_res = FillBuffer(timeout);
    if (read_res.status == detail::TcpClient::Status::Timeout) {
      return nullptr;
    }
    if (read_res.status == detail::TcpClient::Status::Closed) {
      throw DbnResponseError{"Gateway closed the session"};
    }
  }
  current_record_ = Record{BufferRecordHeader()};
  const auto bytes_to_consume = current_record_.Size();
  buffer_.ConsumeNoShift(bytes_to_consume);
  current_record_ = DbnDecoder::DecodeRecordCompat(
      version_, upgrade_policy_, send_ts_out_, &compat_buffer_, current_record_);
  return &current_record_;
}

void LiveBlocking::Stop() { client_.Close(); }

void LiveBlocking::Reconnect() {
  if (log_receiver_->ShouldLog(LogLevel::Info)) {
    std::ostringstream log_msg;
    log_msg << "Reconnecting to " << gateway_ << ':' << port_;
    log_receiver_->Receive(LogLevel::Info, log_msg.str());
  }
  client_ = detail::TcpClient{gateway_, port_};
  buffer_.Clear();
  sub_counter_ = 0;
  session_id_ = this->Authenticate();
}

void LiveBlocking::Resubscribe() {
  for (auto& subscription : subscriptions_) {
    if (std::holds_alternative<UnixNanos>(subscription.start) ||
        std::holds_alternative<std::string>(subscription.start)) {
      subscription.start = LiveSubscription::NoStart{};
    }
    sub_counter_ = std::max(sub_counter_, subscription.id);
    std::ostringstream sub_msg;
    sub_msg << "schema=" << ToString(subscription.schema)
            << "|stype_in=" << ToString(subscription.stype_in)
            << "|id=" << std::to_string(sub_counter_);
    Subscribe(sub_msg.str(), subscription.symbols,
              std::holds_alternative<LiveSubscription::Snapshot>(subscription.start));
  }
}

std::string LiveBlocking::DecodeChallenge() {
  static constexpr auto kMethodName = "LiveBlocking::DecodeChallenge";
  const auto read_size =
      client_.ReadSome(buffer_.WriteBegin(), buffer_.WriteCapacity()).read_size;
  if (read_size == 0) {
    throw LiveApiError{"Gateway closed socket during authentication"};
  }
  buffer_.Fill(read_size);
  // first line is version
  std::string response{reinterpret_cast<const char*>(buffer_.ReadBegin()),
                       buffer_.ReadCapacity()};
  auto first_nl_pos = response.find('\n');
  if (first_nl_pos == std::string::npos) {
    throw LiveApiError::UnexpectedMsg("Received malformed initial message", response);
  }
  if (log_receiver_->ShouldLog(LogLevel::Debug)) {
    std::ostringstream log_ss;
    log_ss << '[' << kMethodName
           << "] Received greeting: " << response.substr(0, first_nl_pos);
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  const auto find_start = first_nl_pos + 1;
  auto next_nl_pos = find_start == response.length() ? std::string::npos
                                                     : response.find('\n', find_start);
  while (next_nl_pos == std::string::npos) {
    // read more
    buffer_.Fill(
        client_.ReadSome(buffer_.WriteBegin(), buffer_.WriteCapacity()).read_size);
    if (buffer_.ReadCapacity() == 0) {
      throw LiveApiError{"Gateway closed socket during authentication"};
    }
    response = {reinterpret_cast<const char*>(buffer_.ReadBegin()),
                buffer_.ReadCapacity()};
    next_nl_pos = response.find('\n', find_start);
  }
  const auto challenge_line = response.substr(find_start, next_nl_pos - find_start);
  if (log_receiver_->ShouldLog(LogLevel::Debug)) {
    std::ostringstream log_ss;
    log_ss << '[' << kMethodName << "] Received CRAM challenge: " << challenge_line;
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  if (challenge_line.compare(0, 4, "cram") != 0) {
    throw LiveApiError::UnexpectedMsg("Did not receive CRAM challenge when expected",
                                      challenge_line);
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
  static constexpr auto kMethodName = "LiveBlocking::Authenticate";
  const std::string challenge_key = DecodeChallenge() + '|' + key_;

  const std::string auth = GenerateCramReply(challenge_key);
  const std::string req = EncodeAuthReq(auth);
  if (log_receiver_->ShouldLog(LogLevel::Debug)) {
    std::ostringstream log_ss;
    log_ss << '[' << kMethodName << "] Sending CRAM reply: " << req;
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  client_.WriteAll(req);
  const std::uint64_t session_id = DecodeAuthResp();

  if (log_receiver_->ShouldLog(LogLevel::Info)) {
    std::ostringstream log_ss;
    log_ss << '[' << kMethodName << "] Successfully authenticated with session_id "
           << session_id;
    log_receiver_->Receive(LogLevel::Info, log_ss.str());
  }
  return session_id;
}

std::string LiveBlocking::GenerateCramReply(std::string_view challenge_key) {
  std::array<unsigned char, SHA256_DIGEST_LENGTH> sha{};
  const unsigned char* sha_res =
      ::SHA256(reinterpret_cast<const unsigned char*>(challenge_key.data()),
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

std::string LiveBlocking::EncodeAuthReq(std::string_view auth) {
  std::ostringstream req_stream;
  req_stream << "auth=" << auth << "|dataset=" << dataset_ << "|encoding=dbn|"
             << "ts_out=" << send_ts_out_ << "|client=" << kUserAgent;
  if (!user_agent_ext_.empty()) {
    req_stream << ' ' << user_agent_ext_;
  }
  if (heartbeat_interval_.has_value()) {
    req_stream << "|heartbeat_interval_s=" << heartbeat_interval_->count();
  }
  req_stream << '\n';
  return req_stream.str();
}

std::uint64_t LiveBlocking::DecodeAuthResp() {
  // handle split packet read
  const std::byte* newline_ptr;
  buffer_.Clear();
  do {
    const auto read_size =
        client_.ReadSome(buffer_.WriteBegin(), buffer_.WriteCapacity()).read_size;
    if (read_size == 0) {
      throw LiveApiError{
          "Unexpected end of message received from server after replying to "
          "CRAM"};
    }
    buffer_.Fill(read_size);
    newline_ptr =
        std::find(buffer_.ReadBegin(), buffer_.ReadEnd(), static_cast<std::byte>('\n'));
  } while (newline_ptr == buffer_.ReadEnd());
  const std::string response{
      reinterpret_cast<const char*>(buffer_.ReadBegin()),
      static_cast<std::size_t>(newline_ptr - buffer_.ReadBegin())};
  {
    std::ostringstream log_ss;
    log_ss << "[LiveBlocking::DecodeAuthResp] Authentication response: " << response;
    log_receiver_->Receive(LogLevel::Debug, log_ss.str());
  }
  // set in case Read call also read records. One beyond newline
  buffer_.Consume(response.length() + 1);

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
      throw LiveApiError::UnexpectedMsg("Malformed authentication response", response);
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
    throw LiveApiError{"Did not receive success indicator from authentication attempt"};
  }
  if (is_error) {
    throw InvalidArgumentError{"LiveBlocking::LiveBlocking", "key",
                               "Failed to authenticate: " + err_details};
  }
  return session_id;
}

void LiveBlocking::IncrementSubCounter() {
  if (sub_counter_ == std::numeric_limits<uint32_t>::max()) {
    log_receiver_->Receive(LogLevel::Warning,
                           "[LiveBlocking::Subscribe] Exhausted all subscription IDs");
  } else {
    ++sub_counter_;
  }
}

databento::detail::TcpClient::Result LiveBlocking::FillBuffer(
    std::chrono::milliseconds timeout) {
  buffer_.Shift();
  const auto read_res =
      client_.ReadSome(buffer_.WriteBegin(), buffer_.WriteCapacity(), timeout);
  buffer_.Fill(read_res.read_size);
  return read_res;
}

databento::RecordHeader* LiveBlocking::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(buffer_.ReadBegin());
}
