#include "databento/live_threaded.hpp"

#include <atomic>
#include <chrono>  // milliseconds
#include <condition_variable>
#include <exception>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>  // forward, move, swap

#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/live.hpp"                  // LiveBuilder
#include "databento/live_blocking.hpp"         // LiveBlocking
#include "databento/log.hpp"                   // ILogReceiver

using databento::LiveThreaded;

struct LiveThreaded::Impl {
  template <typename... A>
  explicit Impl(ILogReceiver* log_recv, A&&... args)
      : log_receiver{log_recv}, blocking{log_receiver, std::forward<A>(args)...} {}

  void NotifyOfStop() {
    const std::lock_guard<std::mutex> lock{last_cb_ret_mutex};
    last_cb_ret = KeepGoing::Stop;
    last_cb_ret_cv.notify_all();
  }

  ILogReceiver* log_receiver;
  std::atomic<std::thread::id> thread_id_{};
  // Set to false when destructor is called
  std::atomic<bool> keep_going{true};
  KeepGoing last_cb_ret{KeepGoing::Continue};
  std::mutex last_cb_ret_mutex;
  std::condition_variable last_cb_ret_cv;
  LiveBlocking blocking;
};

databento::LiveBuilder LiveThreaded::Builder() { return databento::LiveBuilder{}; }

LiveThreaded::LiveThreaded(LiveThreaded&& other) noexcept
    : impl_{std::move(other.impl_)}, thread_{std::move(other.thread_)} {}

LiveThreaded& LiveThreaded::operator=(LiveThreaded&& rhs) noexcept {
  if (impl_) {
    impl_->keep_going.store(false, std::memory_order_relaxed);
  }
  std::swap(impl_, rhs.impl_);
  std::swap(thread_, rhs.thread_);
  return *this;
}

LiveThreaded::~LiveThreaded() {
  if (impl_) {
    impl_->keep_going.store(false, std::memory_order_relaxed);
  }
}

LiveThreaded::LiveThreaded(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, bool send_ts_out,
                           VersionUpgradePolicy upgrade_policy,
                           std::optional<std::chrono::seconds> heartbeat_interval,
                           std::size_t buffer_size, std::string user_agent_ext)
    : impl_{std::make_unique<Impl>(log_receiver, std::move(key), std::move(dataset),
                                   send_ts_out, upgrade_policy, heartbeat_interval,
                                   buffer_size, std::move(user_agent_ext))} {}

LiveThreaded::LiveThreaded(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, std::string gateway, std::uint16_t port,
                           bool send_ts_out, VersionUpgradePolicy upgrade_policy,
                           std::optional<std::chrono::seconds> heartbeat_interval,
                           std::size_t buffer_size, std::string user_agent_ext)
    : impl_{std::make_unique<Impl>(log_receiver, std::move(key), std::move(dataset),
                                   std::move(gateway), port, send_ts_out,
                                   upgrade_policy, heartbeat_interval, buffer_size,
                                   std::move(user_agent_ext))} {}

const std::string& LiveThreaded::Key() const { return impl_->blocking.Key(); }

const std::string& LiveThreaded::Dataset() const { return impl_->blocking.Dataset(); }

const std::string& LiveThreaded::Gateway() const { return impl_->blocking.Gateway(); }

std::uint16_t LiveThreaded::Port() const { return impl_->blocking.Port(); }

bool LiveThreaded::SendTsOut() const { return impl_->blocking.SendTsOut(); }

databento::VersionUpgradePolicy LiveThreaded::UpgradePolicy() const {
  return impl_->blocking.UpgradePolicy();
}

std::optional<std::chrono::seconds> LiveThreaded::HeartbeatInterval() const {
  return impl_->blocking.HeartbeatInterval();
}

const std::vector<databento::LiveSubscription>& LiveThreaded::Subscriptions() const {
  return impl_->blocking.Subscriptions();
}

std::vector<databento::LiveSubscription>& LiveThreaded::Subscriptions() {
  return impl_->blocking.Subscriptions();
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in) {
  impl_->blocking.Subscribe(symbols, schema, stype_in);
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in, UnixNanos start) {
  impl_->blocking.Subscribe(symbols, schema, stype_in, start);
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                             SType stype_in, const std::string& start) {
  impl_->blocking.Subscribe(symbols, schema, stype_in, start);
}

void LiveThreaded::SubscribeWithSnapshot(const std::vector<std::string>& symbols,
                                         Schema schema, SType stype_in) {
  impl_->blocking.SubscribeWithSnapshot(symbols, schema, stype_in);
}

void LiveThreaded::Start(RecordCallback callback) {
  Start({}, std::move(callback), {});
}

void LiveThreaded::Start(databento::MetadataCallback metadata_callback,
                         RecordCallback record_callback) {
  Start(std::move(metadata_callback), std::move(record_callback), {});
}

void LiveThreaded::Start(MetadataCallback metadata_callback,
                         RecordCallback record_callback,
                         ExceptionCallback exception_callback) {
  // Deadlock check
  if (std::this_thread::get_id() == impl_->thread_id_) {
    std::ostringstream log_ss;
    log_ss << "[LiveThreaded::Start] Called Start from callback thread, which "
              "would cause a deadlock. Ignoring.";
    impl_->log_receiver->Receive(LogLevel::Warning, log_ss.str());
    return;
  }
  // Safe to pass raw pointer because `thread_` cannot outlive `impl_`
  thread_ = detail::ScopedThread{
      &LiveThreaded::ProcessingThread, impl_.get(), std::move(metadata_callback),
      std::move(record_callback), std::move(exception_callback)};
}

void LiveThreaded::Reconnect() { impl_->blocking.Reconnect(); }

void LiveThreaded::Resubscribe() { impl_->blocking.Resubscribe(); }

void LiveThreaded::BlockForStop() {
  std::unique_lock<std::mutex> lock{impl_->last_cb_ret_mutex};
  auto* impl = impl_.get();
  // wait for stop
  impl_->last_cb_ret_cv.wait(lock,
                             [impl] { return impl->last_cb_ret == KeepGoing::Stop; });
}

databento::KeepGoing LiveThreaded::BlockForStop(std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock{impl_->last_cb_ret_mutex};
  auto* impl = impl_.get();
  // wait for stop
  if (impl_->last_cb_ret_cv.wait_for(
          lock, timeout, [impl] { return impl->last_cb_ret == KeepGoing::Stop; })) {
    return KeepGoing::Stop;
  }
  return KeepGoing::Continue;
}

void LiveThreaded::ProcessingThread(Impl* impl, MetadataCallback&& metadata_callback,
                                    RecordCallback&& record_callback,
                                    ExceptionCallback&& exception_callback) {
  // Thread safety: non-const calls to `blocking` are only performed from this
  // thread

  static constexpr auto kMethodName = "LiveThreaded::ProcessingThread";
  constexpr std::chrono::milliseconds kTimeout{50};

  impl->thread_id_ = std::this_thread::get_id();
  const auto metadata_cb{std::move(metadata_callback)};
  const auto record_cb{std::move(record_callback)};
  const auto exception_cb{std::move(exception_callback)};
  // Start loop
  while (impl->keep_going.load(std::memory_order_relaxed)) {
    try {
      auto metadata = impl->blocking.Start();
      if (metadata_cb) {
        metadata_cb(std::move(metadata));
      }
    } catch (const std::exception& exc) {
      if (ExceptionHandler(impl, exception_cb, exc, kMethodName,
                           "Caught exception starting session: ") ==
          ExceptionAction::Restart) {
        continue;  // restart Start loop
      } else {
        return;
      }
    }
    // NextRecord loop
    while (impl->keep_going.load(std::memory_order_relaxed)) {
      try {
        const Record* rec = impl->blocking.NextRecord(kTimeout);
        if (rec) {
          if (record_cb(*rec) == KeepGoing::Stop) {
            impl->blocking.Stop();
            impl->NotifyOfStop();
            return;
          }
        }  // else timeout
      } catch (const std::exception& exc) {
        if (ExceptionHandler(impl, exception_cb, exc, kMethodName,
                             "Caught exception reading next record: ") ==
            ExceptionAction::Restart) {
          break;  // break out of NextRecord loop, to restart Start loop
        } else {
          impl->NotifyOfStop();
          return;
        }
      }
    }
  }
}

LiveThreaded::ExceptionAction LiveThreaded::ExceptionHandler(
    Impl* impl, const ExceptionCallback& exception_callback, const std::exception& exc,
    std::string_view pretty_function_name, std::string_view message) {
  if (exception_callback && exception_callback(exc) == ExceptionAction::Restart) {
    std::ostringstream log_ss;
    log_ss << pretty_function_name << ' ' << message << exc.what()
           << ". Attempting to restart session.";
    impl->log_receiver->Receive(LogLevel::Warning, log_ss.str());
    return ExceptionAction::Restart;
  }
  impl->blocking.Stop();
  std::ostringstream log_ss;
  log_ss << pretty_function_name << ' ' << message << exc.what()
         << ". Stopping thread.";
  impl->log_receiver->Receive(LogLevel::Error, log_ss.str());
  return ExceptionAction::Stop;
}
