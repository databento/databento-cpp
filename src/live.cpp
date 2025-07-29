#include "databento/live.hpp"

#include <chrono>
#include <utility>  // move

#include "databento/constants.hpp"      // kApiKeyLength
#include "databento/detail/buffer.hpp"  // kDefaultBufSize
#include "databento/exceptions.hpp"     // InvalidArgumentError, LiveApiError
#include "databento/live_blocking.hpp"  // LiveBlocking
#include "databento/live_threaded.hpp"  // LiveThreaded
#include "databento/log.hpp"

using databento::LiveBuilder;

LiveBuilder::LiveBuilder() : buffer_size_{detail::Buffer::kDefaultBufSize} {}

LiveBuilder& LiveBuilder::SetKeyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw Exception{"Expected environment variable DATABENTO_API_KEY to be set"};
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

LiveBuilder& LiveBuilder::SetDataset(std::string dataset) {
  dataset_ = std::move(dataset);
  return *this;
}

LiveBuilder& LiveBuilder::SetDataset(Dataset dataset) {
  dataset_ = ToString(dataset);
  return *this;
}

LiveBuilder& LiveBuilder::SetSendTsOut(bool send_ts_out) {
  send_ts_out_ = send_ts_out;
  return *this;
}

LiveBuilder& LiveBuilder::SetUpgradePolicy(VersionUpgradePolicy upgrade_policy) {
  upgrade_policy_ = upgrade_policy;
  return *this;
}

LiveBuilder& LiveBuilder::SetLogReceiver(databento::ILogReceiver* log_receiver) {
  log_receiver_ = log_receiver;
  return *this;
}

LiveBuilder& LiveBuilder::SetHeartbeatInterval(
    std::chrono::seconds heartbeat_interval) {
  heartbeat_interval_ = heartbeat_interval;
  return *this;
}

LiveBuilder& LiveBuilder::SetAddress(std::string gateway, std::uint16_t port) {
  gateway_ = std::move(gateway);
  port_ = port;
  return *this;
}

LiveBuilder& LiveBuilder::SetBufferSize(std::size_t size) {
  buffer_size_ = size;
  return *this;
}

LiveBuilder& LiveBuilder::ExtendUserAgent(std::string extension) {
  user_agent_ext_ = std::move(extension);
  return *this;
}

databento::LiveBlocking LiveBuilder::BuildBlocking() {
  Validate();
  if (gateway_.empty()) {
    return databento::LiveBlocking{log_receiver_,   key_,
                                   dataset_,        send_ts_out_,
                                   upgrade_policy_, heartbeat_interval_,
                                   buffer_size_,    user_agent_ext_};
  }
  return databento::LiveBlocking{
      log_receiver_, key_,           dataset_,        gateway_,
      port_,         send_ts_out_,   upgrade_policy_, heartbeat_interval_,
      buffer_size_,  user_agent_ext_};
}

databento::LiveThreaded LiveBuilder::BuildThreaded() {
  Validate();
  if (gateway_.empty()) {
    return databento::LiveThreaded{log_receiver_,   key_,
                                   dataset_,        send_ts_out_,
                                   upgrade_policy_, heartbeat_interval_,
                                   buffer_size_,    user_agent_ext_};
  }
  return databento::LiveThreaded{
      log_receiver_, key_,           dataset_,        gateway_,
      port_,         send_ts_out_,   upgrade_policy_, heartbeat_interval_,
      buffer_size_,  user_agent_ext_};
}

void LiveBuilder::Validate() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  if (dataset_.empty()) {
    throw Exception{"'dataset' is unset"};
  }
  if (log_receiver_ == nullptr) {
    log_receiver_ = databento::ILogReceiver::Default();
  }
}
