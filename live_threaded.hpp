#pragma once

#include <chrono>
#include <cstdint>
#include <functional>  // function
#include <memory>      // unique_ptr
#include <optional>
#include <string>
#include <string_view>
#include <utility>  // pair
#include <vector>

#include "databento/datetime.hpp"              // UnixNanos
#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/enums.hpp"                 // Schema, SType
#include "databento/live_subscription.hpp"
#include "databento/timeseries.hpp"  // MetadataCallback, RecordCallback

namespace databento {
// Forward declaration
class ILogReceiver;
class LiveBuilder;

// A client for interfacing with Databento's real-time and intraday replay
// market data API. This client provides a threaded event-driven API for
// receiving the next record. Unlike Historical, each instance of LiveThreaded
// is associated with a particular dataset.
class LiveThreaded {
 public:
  enum class ExceptionAction : std::uint8_t {
    // Start a new session. Return this instead of calling `Start`, which would
    // cause a deadlock.
    Restart,
    // Close the connection and stop the callback thread.
    Stop,
  };
  using ExceptionCallback =
      std::function<ExceptionAction(const std::exception&)>;

  LiveThreaded(const LiveThreaded&) = delete;
  LiveThreaded& operator=(const LiveThreaded&) = delete;
  LiveThreaded(LiveThreaded&& other) noexcept;
  LiveThreaded& operator=(LiveThreaded&& rhs) noexcept;
  ~LiveThreaded();

  /*
   * Getters
   */

  const std::string& Key() const;
  const std::string& Dataset() const;
  const std::string& Gateway() const;
  std::uint16_t Port() const;
  bool SendTsOut() const;
  VersionUpgradePolicy UpgradePolicy() const;
  // The the first member of the pair will be true, when the heartbeat interval
  // was overridden.
  std::optional<std::chrono::seconds> HeartbeatInterval() const;
  const std::vector<LiveSubscription>& Subscriptions() const;
  std::vector<LiveSubscription>& Subscriptions();

  /*
   * Methods
   */

  // Add a new subscription. A single client instance supports multiple
  // subscriptions. Note there is no unsubscribe method. Subscriptions end
  // when the client disconnects when it's destroyed.
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in, UnixNanos start);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in, const std::string& start);
  void SubscribeWithSnapshot(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in);
  // Notifies the gateway to start sending messages for all subscriptions.
  // `metadata_callback` will be called exactly once, before any calls to
  // `record_callback`. `record_callback` will be called for records from all
  // subscriptions.
  //
  // This method should only be called once per instance.
  void Start(RecordCallback record_callback);
  void Start(MetadataCallback metadata_callback,
             RecordCallback record_callback);
  void Start(MetadataCallback metadata_callback, RecordCallback record_callback,
             ExceptionCallback exception_callback);
  // Closes the current connection, and attempts to reconnect to the gateway.
  void Reconnect();
  void Resubscribe();
  // Blocking wait with an optional timeout for the session to close when the
  // record_callback or the exception_callback return Stop.
  void BlockForStop();
  KeepGoing BlockForStop(std::chrono::milliseconds timeout);

 private:
  friend LiveBuilder;

  struct Impl;

  static void ProcessingThread(Impl* impl, MetadataCallback&& metadata_callback,
                               RecordCallback&& record_callback,
                               ExceptionCallback&& exception_callback);
  static ExceptionAction ExceptionHandler(
      Impl* impl, const ExceptionCallback& exception_callback,
      const std::exception& exc, std::string_view pretty_function_name,
      std::string_view message);

  LiveThreaded(ILogReceiver* log_receiver, std::string key, std::string dataset,
               bool send_ts_out, VersionUpgradePolicy upgrade_policy,
               std::optional<std::chrono::seconds> heartbeat_interval,
               std::size_t buffer_size);
  LiveThreaded(ILogReceiver* log_receiver, std::string key, std::string dataset,
               std::string gateway, std::uint16_t port, bool send_ts_out,
               VersionUpgradePolicy upgrade_policy,
               std::optional<std::chrono::seconds> heartbeat_interval,
               std::size_t buffer_size);

  // unique_ptr to be movable
  std::unique_ptr<Impl> impl_;
  detail::ScopedThread thread_;
};
}  // namespace databento
