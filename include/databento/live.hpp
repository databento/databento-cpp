#pragma once

#include <array>
#include <atomic>
#include <cstddef>     // size_t
#include <functional>  // function
#include <memory>
#include <string>
#include <vector>

#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/tcp_client.hpp"  // TcpClient
#include "databento/enums.hpp"              // LiveGateway, Schema
#include "databento/record.hpp"

namespace databento {
// A client for interfacing with Databento's live market data API.
class Live {
 public:
  using RecordCallback = std::function<void(const Record&)>;

  class AsyncToken {
   public:
    AsyncToken(Live* client, RecordCallback callback);

    // Wait for the next record(s). callback will be called at least once.
    void AwaitRecord();

   private:
    Live* client_;
    RecordCallback callback_;
  };

  class ThreadedToken {
   public:
    explicit ThreadedToken(AsyncToken&& async_token);

   private:
    void ProcessingThread(AsyncToken&& token);

    std::atomic<bool> keep_going_;
    detail::ScopedThread thread_;
  };

  Live(std::string key, LiveGateway gateway, bool send_ts_out);
  // Primarily for unit tests
  Live(std::string key, std::string gateway, std::uint16_t port,
       bool send_ts_out);

  /*
   * Getters
   */

  const std::string& Key() const { return key_; }
  const std::string& Gateway() const { return gateway_; }

  /*
   * Methods
   */
  void Subscribe(const std::string& dataset,
                 const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  // Notifies the gateway to start sending messages for all subscriptions.
  // `callback` will be called for updates to all subscriptions.
  std::unique_ptr<ThreadedToken> StartThreaded(const RecordCallback& callback);
  AsyncToken StartAsync(RecordCallback callback);

 private:
  std::string Authenticate();
  std::string DecodeChallenge();
  std::string GenerateCramReply(const std::string& challenge_key);
  std::string EncodeAuthReq(const std::string& auth);
  static void DecodeAuthResp(const std::string& response);
  void AwaitRecord(const RecordCallback& callback);

  static constexpr std::size_t kMaxStrLen = 24L * 1024;

  std::string key_;
  std::string gateway_;
  bool send_ts_out_;
  detail::TcpClient client_;
  std::array<char, kMaxStrLen> buffer_{};
  std::string session_id_;
  // std::atomic<bool> keep_going_{true};
  // detail::ScopedThread thread_;
};

// A helper class for constructing an instance of Live.
class LiveBuilder {
 public:
  LiveBuilder() = default;

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  LiveBuilder& SetKeyFromEnv();
  LiveBuilder& SetKey(std::string key);
  LiveBuilder& SetGateway(LiveGateway gateway);
  // Whether to prepend an 8-byte nanosecond timestamp as a header before each
  // DBZ message.
  LiveBuilder& SetSendTsOut(bool send_ts_out);
  // Attempts to construct an instance of Live or throws an exception if
  // no key has been set.
  Live Build();

 private:
  std::string key_;
  LiveGateway gateway_{LiveGateway::Origin};
  bool send_ts_out_{false};
};
}  // namespace databento
