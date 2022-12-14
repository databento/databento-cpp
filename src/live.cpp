#include "databento/live.hpp"

#include <utility>  // move

#include "databento/constants.hpp"      // kApiKeyLength
#include "databento/exceptions.hpp"     // InvalidArgumentError, LiveApiError
#include "databento/live_blocking.hpp"  // LiveBlocking
#include "databento/live_threaded.hpp"  // LiveThreaded

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

databento::LiveBlocking LiveBuilder::BuildBlocking() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  return databento::LiveBlocking{std::move(key_), gateway_, send_ts_out_};
}

databento::LiveThreaded LiveBuilder::BuildThreaded() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  return databento::LiveThreaded{std::move(key_), gateway_, send_ts_out_};
}
