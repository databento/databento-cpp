#pragma once

#include <array>
#include <chrono>  // milliseconds, steady_clock
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/dbn.hpp"       // Metadata
#include "databento/detail/buffer.hpp"
#include "databento/detail/live_connection.hpp"  // LiveConnection
#include "databento/enums.hpp"  // Schema, SType, VersionUpgradePolicy, Compression
#include "databento/live_subscription.hpp"
#include "databento/record.hpp"  // Record, RecordHeader

namespace databento {
// Forward declaration
class ILogReceiver;
class LiveBuilder;

// Timeouts for the Live client's connection and authentication phases.
struct TimeoutConf {
  std::chrono::seconds connect{10};
  std::chrono::seconds auth{30};
};

class LiveThreaded;

// A client for interfacing with Databento's real-time and intraday replay
// market data API. This client provides a blocking API for getting the next
// record. Unlike Historical, each instance of LiveBlocking is associated with a
// particular dataset.
class LiveBlocking {
 public:
  static LiveBuilder Builder();

  /*
   * Getters
   */

  const std::string& Key() const { return key_; }
  const std::string& Dataset() const { return dataset_; }
  const std::string& Gateway() const { return gateway_; }
  std::uint16_t Port() const { return port_; }
  bool SendTsOut() const { return send_ts_out_; }
  VersionUpgradePolicy UpgradePolicy() const { return upgrade_policy_; }
  std::optional<std::chrono::seconds> HeartbeatInterval() const {
    return heartbeat_interval_;
  }
  databento::Compression Compression() const { return compression_; }
  std::optional<databento::SlowReaderBehavior> SlowReaderBehavior() const {
    return slow_reader_behavior_;
  }
  const databento::TimeoutConf& TimeoutConf() const { return timeout_conf_; }
  std::uint64_t SessionId() const { return session_id_; }
  const std::vector<LiveSubscription>& Subscriptions() const { return subscriptions_; }
  std::vector<LiveSubscription>& Subscriptions() { return subscriptions_; }

  /*
   * Methods
   */

  // Add a new subscription. A single client instance supports multiple
  // subscriptions. Note there is no unsubscribe method. Subscriptions end
  // when the client disconnects in its destructor.
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema, SType stype_in,
                 UnixNanos start);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema, SType stype_in,
                 const std::string& start);
  void SubscribeWithSnapshot(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in);
  // Notifies the gateway to start sending messages for all subscriptions.
  //
  // This method should only be called once per instance.
  Metadata Start();
  // Block on getting the next record. The returned reference is valid until
  // this method is called again.
  //
  // This method should only be called after `Start`.
  const Record& NextRecord();
  // Block on getting the next record. The returned pointer is valid until
  // this method is called again. Will return `nullptr` if the `timeout` is
  // reached.
  //
  // This method should only be called after `Start`.
  const Record* NextRecord(std::chrono::milliseconds timeout);
  // Returns the next record from the internal buffer without performing any
  // I/O. Returns `nullptr` if no complete record is buffered. The returned
  // pointer is valid until the next call to `TryNextRecord`, `NextRecord`,
  // or `FillBuffer`.
  //
  // This method should only be called after `Start`.
  const Record* TryNextRecord();
  // Reads available data from the connection into the internal buffer using
  // the heartbeat timeout. Returns the number of bytes read and the status.
  // A `read_size` of 0 with `Status::Closed` indicates the connection was
  // closed by the gateway.
  //
  // This method should only be called after `Start`.
  IReadable::Result FillBuffer();
  // Reads available data from the connection into the internal buffer.
  // Returns the number of bytes read and the status. A `read_size` of 0 with
  // `Status::Closed` indicates the connection was closed by the gateway.
  //
  // This method should only be called after `Start`.
  IReadable::Result FillBuffer(std::chrono::milliseconds timeout);
  // Stops the session with the gateway. Once stopped, the session cannot be
  // restarted.
  void Stop();
  // Closes the current connection and attempts to reconnect to the gateway.
  void Reconnect();
  // Resubscribes to all subscriptions, removing the original `start` time, if
  // any. Usually performed after a `Reconnect()`.
  void Resubscribe();

 private:
  friend LiveBuilder;
  friend LiveThreaded;

  LiveBlocking(ILogReceiver* log_receiver, std::string key, std::string dataset,
               bool send_ts_out, VersionUpgradePolicy upgrade_policy,
               std::optional<std::chrono::seconds> heartbeat_interval,
               std::size_t buffer_size, std::string user_agent_ext,
               databento::Compression compression,
               std::optional<databento::SlowReaderBehavior> slow_reader_behavior,
               databento::TimeoutConf timeout_conf);
  LiveBlocking(ILogReceiver* log_receiver, std::string key, std::string dataset,
               std::string gateway, std::uint16_t port, bool send_ts_out,
               VersionUpgradePolicy upgrade_policy,
               std::optional<std::chrono::seconds> heartbeat_interval,
               std::size_t buffer_size, std::string user_agent_ext,
               databento::Compression compression,
               std::optional<databento::SlowReaderBehavior> slow_reader_behavior,
               databento::TimeoutConf timeout_conf);

  std::string DetermineGateway() const;
  std::uint64_t Authenticate();
  std::string DecodeChallenge(std::chrono::milliseconds timeout);
  std::string GenerateCramReply(std::string_view challenge_key);
  std::string EncodeAuthReq(std::string_view auth);
  std::uint64_t DecodeAuthResp(std::chrono::milliseconds timeout);
  void IncrementSubCounter();
  void Subscribe(std::string_view sub_msg, const std::vector<std::string>& symbols,
                 bool use_snapshot);
  const Record* ConsumeBufferedRecord();
  RecordHeader* BufferRecordHeader();
  std::chrono::milliseconds HeartbeatTimeout() const;
  void CheckHeartbeatTimeout() const;

  static constexpr std::size_t kMaxStrLen = 24L * 1024;

  ILogReceiver* log_receiver_;
  const std::string key_;
  const std::string dataset_;
  const std::string gateway_;
  const std::string user_agent_ext_;
  const std::uint16_t port_;
  const bool send_ts_out_;
  std::uint8_t version_{};
  const VersionUpgradePolicy upgrade_policy_;
  const std::optional<std::chrono::seconds> heartbeat_interval_;
  const databento::Compression compression_;
  const std::optional<databento::SlowReaderBehavior> slow_reader_behavior_;
  const databento::TimeoutConf timeout_conf_;
  detail::LiveConnection connection_;
  std::uint32_t sub_counter_{};
  std::vector<LiveSubscription> subscriptions_;
  detail::Buffer buffer_;
  // Must be 8-byte aligned for records
  alignas(RecordHeader) std::array<std::byte, kMaxRecordLen> compat_buffer_{};
  std::uint64_t session_id_;
  Record current_record_{nullptr};
  std::chrono::steady_clock::time_point last_read_time_{
      std::chrono::steady_clock::now()};
};
}  // namespace databento
