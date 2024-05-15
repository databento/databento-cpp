#pragma once

#include <array>
#include <chrono>  // milliseconds
#include <cstdint>
#include <string>
#include <vector>

#include "databento/datetime.hpp"           // UnixNanos
#include "databento/dbn.hpp"                // Metadata
#include "databento/detail/tcp_client.hpp"  // TcpClient
#include "databento/enums.hpp"   // Schema, SType, VersionUpgradePolicy
#include "databento/record.hpp"  // Record, RecordHeader

namespace databento {
class ILogReceiver;

// A client for interfacing with Databento's real-time and intraday replay
// market data API. This client provides a blocking API for getting the next
// record. Unlike Historical, each instance of LiveBlocking is associated with a
// particular dataset.
class LiveBlocking {
 public:
  LiveBlocking(ILogReceiver* log_receiver, std::string key, std::string dataset,
               bool send_ts_out, VersionUpgradePolicy upgrade_policy);
  LiveBlocking(ILogReceiver* log_receiver, std::string key, std::string dataset,
               std::string gateway, std::uint16_t port, bool send_ts_out,
               VersionUpgradePolicy upgrade_policy);
  /*
   * Getters
   */

  const std::string& Key() const { return key_; }
  const std::string& Dataset() const { return dataset_; }
  const std::string& Gateway() const { return gateway_; }
  std::uint16_t Port() const { return port_; }
  bool SendTsOut() const { return send_ts_out_; }
  VersionUpgradePolicy UpgradePolicy() const { return upgrade_policy_; }

  /*
   * Methods
   */

  // Add a new subscription. A single client instance supports multiple
  // subscriptions. Note there is no unsubscribe method. Subscriptions end
  // when the client disconnects in its destructor.
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in, UnixNanos start);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in, const std::string& start);
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
  // Stops the session with the gateway. Once stopped, the session cannot be
  // restarted.
  void Stop();
  // Closes the current connection and attempts to reconnect to the gateway.
  void Reconnect();

 private:
  std::string DetermineGateway() const;
  std::uint64_t Authenticate();
  std::string DecodeChallenge();
  std::string GenerateCramReply(const std::string& challenge_key);
  std::string EncodeAuthReq(const std::string& auth);
  std::uint64_t DecodeAuthResp();
  void Subscribe(const std::string& sub_msg,
                 const std::vector<std::string>& symbols);
  detail::TcpClient::Result FillBuffer(std::chrono::milliseconds timeout);
  RecordHeader* BufferRecordHeader();

  static constexpr std::size_t kMaxStrLen = 24L * 1024;

  ILogReceiver* log_receiver_;
  std::string key_;
  std::string dataset_;
  std::string gateway_;
  std::uint16_t port_;
  bool send_ts_out_;
  std::uint8_t version_{};
  VersionUpgradePolicy upgrade_policy_;
  detail::TcpClient client_;
  // Must be 8-byte aligned for records
  alignas(RecordHeader) std::array<char, kMaxStrLen> read_buffer_{};
  std::size_t buffer_size_{};
  std::size_t buffer_idx_{};
  // Must be 8-byte aligned for records
  alignas(
      RecordHeader) std::array<std::uint8_t, kMaxRecordLen> compat_buffer_{};
  std::uint64_t session_id_;
  Record current_record_{nullptr};
};
}  // namespace databento
