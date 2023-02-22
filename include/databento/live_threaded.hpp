#pragma once

#include <functional>  // function
#include <memory>      // unique_ptr
#include <string>
#include <vector>

#include "databento/dbn.hpp"                   // Metadata
#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/enums.hpp"                 // Schema, SType
#include "databento/record.hpp"                // Record

namespace databento {
// A client for interfacing with Databento's live market data API. This client
// has a threaded event-driven API for receiving the next record. Unlike
// Historical, each instance of LiveThreaded is associated with a particular
// dataset.
class LiveThreaded {
 public:
  using Callback = std::function<void(const Record&)>;

  LiveThreaded(std::string key, std::string dataset, bool send_ts_out);
  LiveThreaded(std::string key, std::string dataset, std::string gateway,
               std::uint16_t port, bool send_ts_out);
  LiveThreaded(const LiveThreaded&) = delete;
  LiveThreaded& operator=(const LiveThreaded&) = delete;
  LiveThreaded(LiveThreaded&& other) noexcept;
  LiveThreaded& operator=(LiveThreaded&& rhs) noexcept;
  ~LiveThreaded();

  /*
   * Getters
   */

  const std::string& Key() const;
  const std::string& Gateway() const;

  /*
   * Methods
   */

  // Add a new subscription. A single client instance supports multiple
  // subscriptions. Note there is no unsubscribe method. Subscriptions end
  // when the client disconnects when it's destroyed.
  void Subscribe(const std::string& dataset,
                 const std::vector<std::string>& symbols, Schema schema,
                 SType stype_in);
  // Notifies the gateway to start sending messages for all subscriptions.
  // `callback` will be called for updates to all subscriptions.
  //
  // This method should only be called once per instance.
  Metadata Start(Callback callback);

 private:
  struct Impl;

  static void ProcessingThread(Impl* impl, Callback&& callback);

  // unique_ptr to be movable
  std::unique_ptr<Impl> impl_;
  detail::ScopedThread thread_;
};
}  // namespace databento
