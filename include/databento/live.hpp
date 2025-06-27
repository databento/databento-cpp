#pragma once

#include <chrono>
#include <cstddef>
#include <string>

#include "databento/enums.hpp"  // VersionUpgradePolicy
#include "databento/live_blocking.hpp"
#include "databento/live_threaded.hpp"
#include "databento/publishers.hpp"

namespace databento {
// Forward declarations
class ILogReceiver;

// A helper class for constructing a Live client, either an instance of
// LiveBlocking or LiveThreaded.
class LiveBuilder {
 public:
  LiveBuilder();

  /*
   * Required setters
   */

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  LiveBuilder& SetKeyFromEnv();
  LiveBuilder& SetKey(std::string key);
  LiveBuilder& SetDataset(Dataset dataset);
  LiveBuilder& SetDataset(std::string dataset);

  /*
   * Optional setters
   */

  // Whether to append the gateway send timestamp after each DBN message.
  LiveBuilder& SetSendTsOut(bool send_ts_out);
  // Set the version upgrade policy for when receiving DBN data from a prior
  // version. Defaults to upgrading to DBNv3 (if not already).
  LiveBuilder& SetUpgradePolicy(VersionUpgradePolicy upgrade_policy);
  // Sets the receiver of the logs to be used by the client.
  LiveBuilder& SetLogReceiver(ILogReceiver* log_receiver);
  // Overrides the heartbeat interval.
  LiveBuilder& SetHeartbeatInterval(std::chrono::seconds heartbeat_interval);
  // Overrides the gateway and port. This is an advanced method.
  LiveBuilder& SetAddress(std::string gateway, std::uint16_t port);
  // Overrides the size of the buffer used for reading data from the TCP socket.
  LiveBuilder& SetBufferSize(std::size_t size);
  // Appends to the default user agent.
  LiveBuilder& ExtendUserAgent(std::string extension);

  /*
   * Build a live client instance
   */

  // Attempts to construct an instance of a blocking live client or throws
  // an exception.
  LiveBlocking BuildBlocking();
  // Attempts to construct an instance of a threaded live client or throws an
  // exception.
  LiveThreaded BuildThreaded();

 private:
  void Validate();

  ILogReceiver* log_receiver_{};
  std::string gateway_{};
  std::uint16_t port_{};
  std::string key_;
  std::string dataset_;

  bool send_ts_out_{false};
  VersionUpgradePolicy upgrade_policy_{VersionUpgradePolicy::UpgradeToV3};
  std::optional<std::chrono::seconds> heartbeat_interval_{};
  std::size_t buffer_size_;
  std::string user_agent_ext_;
};
}  // namespace databento
