#pragma once

#include <string>

#include "databento/live_blocking.hpp"
#include "databento/live_threaded.hpp"

namespace databento {
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
  // Whether to prepend an 8-byte nanosecond timestamp as a header before each
  // DBN message.
  LiveBuilder& SetSendTsOut(bool send_ts_out);
  // Attempts to construct an instance of a blocking live client or throws an
  // exception.
  LiveBlocking BuildBlocking();
  // Attempts to construct an instance of a threaded live client or throws an
  // exception.
  LiveThreaded BuildThreaded();

 private:
  void Validate() const;

  std::string key_;
  std::string dataset_;
  bool send_ts_out_{false};
};
}  // namespace databento
