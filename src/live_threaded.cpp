#include "databento/live_threaded.hpp"

#include <atomic>
#include <chrono>  // milliseconds
#include <exception>
#include <sstream>
#include <utility>  // forward, move, swap

#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/live_blocking.hpp"         // LiveBlocking
#include "databento/log.hpp"

using databento::LiveThreaded;

struct LiveThreaded::Impl {
  template <typename... A>
  explicit Impl(ILogReceiver* log_receiver, A&&... args)
      : log_receiver{log_receiver},
        blocking{log_receiver, std::forward<A>(args)...} {}

  ILogReceiver* log_receiver;
  std::atomic<bool> keep_going{true};
  LiveBlocking blocking;
};

LiveThreaded::LiveThreaded(LiveThreaded&& other) noexcept
    : impl_{std::move(other.impl_)}, thread_{std::move(other.thread_)} {}

LiveThreaded& LiveThreaded::operator=(LiveThreaded&& rhs) noexcept {
  if (impl_) {
    impl_->keep_going = false;
  }
  std::swap(impl_, rhs.impl_);
  std::swap(thread_, rhs.thread_);
  return *this;
}

LiveThreaded::~LiveThreaded() {
  if (impl_) {
    impl_->keep_going = false;
  }
}

LiveThreaded::LiveThreaded(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, bool send_ts_out)
    : impl_{new Impl{log_receiver, std::move(key), std::move(dataset),
                     send_ts_out}} {}

LiveThreaded::LiveThreaded(ILogReceiver* log_receiver, std::string key,
                           std::string dataset, std::string gateway,
                           std::uint16_t port, bool send_ts_out)
    : impl_{new Impl{log_receiver, std::move(key), std::move(dataset),
                     std::move(gateway), port, send_ts_out}} {}

const std::string& LiveThreaded::Key() const { return impl_->blocking.Key(); }

const std::string& LiveThreaded::Gateway() const {
  return impl_->blocking.Gateway();
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in) {
  impl_->blocking.Subscribe(symbols, schema, stype_in);
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in, UnixNanos start) {
  impl_->blocking.Subscribe(symbols, schema, stype_in, start);
}

void LiveThreaded::Subscribe(const std::vector<std::string>& symbols,
                             Schema schema, SType stype_in,
                             const std::string& start) {
  impl_->blocking.Subscribe(symbols, schema, stype_in, start);
}

void LiveThreaded::Start(databento::MetadataCallback metadata_callback,
                         RecordCallback record_callback) {
  // Safe to pass raw pointer because `thread_` cannot outlive `impl_`
  thread_ = detail::ScopedThread{&LiveThreaded::ProcessingThread, impl_.get(),
                                 std::move(metadata_callback),
                                 std::move(record_callback)};
}

void LiveThreaded::Start(RecordCallback callback) {
  // Safe to pass raw pointer because `thread_` cannot outlive `impl_`
  thread_ =
      detail::ScopedThread{&LiveThreaded::ProcessingThread, impl_.get(),
                           databento::MetadataCallback{}, std::move(callback)};
}

void LiveThreaded::ProcessingThread(Impl* impl,
                                    MetadataCallback&& metadata_callback,
                                    RecordCallback&& record_callback) {
  constexpr std::chrono::milliseconds kTimeout{50};

  try {
    {
      auto metadata = impl->blocking.Start();
      if (metadata_callback) {
        std::move(metadata_callback)(std::move(metadata));
      }
    }
    auto record_cb{std::move(record_callback)};
    // Thread safety: non-const calls to `blocking` are only performed from this
    // thread
    while (impl->keep_going.load()) {
      const Record* rec = impl->blocking.NextRecord(kTimeout);
      if (rec) {
        if (record_cb(*rec) == KeepGoing::Stop) {
          impl->blocking.Stop();
          return;
        }
      }  // else timeout
    }
  } catch (const std::exception& exc) {
    impl->blocking.Stop();
    std::ostringstream log_ss;
    log_ss << __PRETTY_FUNCTION__ << " Caught exception: " << exc.what()
           << ". Stopping thread.";
    impl->log_receiver->Receive(LogLevel::Error, log_ss.str());
  }
}
