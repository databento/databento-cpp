#pragma once

#include <array>
#include <chrono>  // milliseconds
#include <cstdint>
#include <string>
#include <vector>

#include "databento/detail/tcp_client.hpp"  // TcpClient
#include "databento/enums.hpp"              // LiveGateway, Schema, SType
#include "databento/record.hpp"             // Record

namespace databento {
// A client for interfacing with Databento's live market data API. This client
// has a blocking API for getting the next record.
class LiveBlocking {
 public:
  LiveBlocking(std::string key, LiveGateway gateway, bool send_ts_out);
  LiveBlocking(std::string key, std::string gateway, std::uint16_t port,
               bool send_ts_out);
  /*
   * Getters
   */

  const std::string& Key() const { return key_; }
  const std::string& Gateway() const { return gateway_; }

  /*
   * Methods
   */

  // Add a new subscription. A single client instance supports multiple
  // subscriptions. Note there is no unsubscribe method. Subscriptions end
  // when the client disconnects in its destructor.
  void Subscribe(const std::string& dataset,
                 const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  // Notifies the gateway to start sending messages for all subscriptions.
  //
  // This method should only be called once per instance.
  void Start();
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

 private:
  std::string Authenticate();
  std::string DecodeChallenge();
  std::string GenerateCramReply(const std::string& challenge_key);
  std::string EncodeAuthReq(const std::string& auth);
  void DecodeAuthResp();

  static constexpr std::size_t kMaxStrLen = 24L * 1024;

  std::string key_;
  std::string gateway_;
  bool send_ts_out_;
  detail::TcpClient client_;
  std::array<char, kMaxStrLen> buffer_{};
  std::size_t buffer_size_{};
  std::size_t buffer_idx_{};
  std::string session_id_;
  Record current_record_{nullptr};
};
}  // namespace databento
