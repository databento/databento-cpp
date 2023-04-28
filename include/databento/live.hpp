#pragma once

#include <string>

#include "databento/live_blocking.hpp"
#include "databento/live_threaded.hpp"

namespace databento {
class ILogReceiver;

// A helper class for constructing a Live client, either an instance of
// LiveBlocking or LiveThreaded.
class LiveBuilder {
 public:
  LiveBuilder() = default;

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  LiveBuilder& SetKeyFromEnv();
  LiveBuilder& SetKey(std::string key);
  LiveBuilder& SetDataset(std::string dataset);
  // Whether to append the gateway send timestamp after each DBN message.
  LiveBuilder& SetSendTsOut(bool send_ts_out);
  // Sets the receiver of the logs to be used by the client.
  LiveBuilder& SetLogReceiver(ILogReceiver* log_receiver);
  // Attempts to construct an instance of a blocking live client or throws an
  // exception.
  LiveBlocking BuildBlocking();
  // Attempts to construct an instance of a threaded live client or throws an
  // exception.
  LiveThreaded BuildThreaded();

 private:
  void Validate();

  ILogReceiver* log_receiver_{};
  std::string key_;
  std::string dataset_;
  bool send_ts_out_{true};
};
}  // namespace databento
