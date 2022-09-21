#include "databento/historical.hpp"

#include <cstdlib>  // get_env
#include <stdexcept>
#include <utility>  // move

#include "databento/enums.hpp"

using databento::Historical;
using databento::HistoricalBuilder;

Historical::Historical(std::string key, HistoricalGateway gateway)
    : key_{std::move(key)}, gateway_{UrlFromGateway(gateway)} {}

HistoricalBuilder& HistoricalBuilder::keyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw std::runtime_error{
        "Expected environment variable DATABENTO_API_KEY to be set"};
  }
  return this->key(env_key);
}

HistoricalBuilder& HistoricalBuilder::key(std::string key) {
  key_ = std::move(key);
  return *this;
}

HistoricalBuilder& HistoricalBuilder::gateway(HistoricalGateway gateway) {
  gateway_ = gateway;
  return *this;
}

Historical HistoricalBuilder::Build() {
  if (key_.empty()) {
    throw std::logic_error{"key is unset"};
  }
  return Historical{std::move(key_), gateway_};
}
