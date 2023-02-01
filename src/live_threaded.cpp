#include "databento/live_threaded.hpp"

#include <atomic>
#include <chrono>   // milliseconds
#include <utility>  // forward, move, swap

#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/live_blocking.hpp"         // LiveBlocking

using databento::LiveThreaded;

struct LiveThreaded::Impl {
  template <typename... A>
  explicit Impl(A&&... args) : blocking{std::forward<A>(args)...} {}

  std::atomic<bool> keep_going{true};
  LiveBlocking blocking;
};

LiveThreaded::LiveThreaded(LiveThreaded&& other) noexcept
    : impl_{std::move(other.impl_)}, thread_{std::move(other.thread_)} {}

LiveThreaded& LiveThreaded::operator=(LiveThreaded&& rhs) noexcept {
  impl_->keep_going = false;
  std::swap(impl_, rhs.impl_);
  std::swap(thread_, rhs.thread_);
  return *this;
}

LiveThreaded::~LiveThreaded() { impl_->keep_going = false; }

LiveThreaded::LiveThreaded(std::string key, std::string dataset,
                           bool send_ts_out)
    : impl_{new Impl{std::move(key), std::move(dataset), send_ts_out}} {}

LiveThreaded::LiveThreaded(std::string key, std::string dataset,
                           std::string gateway, std::uint16_t port,
                           bool send_ts_out)
    : impl_{new Impl{std::move(key), std::move(dataset), std::move(gateway),
                     port, send_ts_out}} {}

const std::string& LiveThreaded::Key() const { return impl_->blocking.Key(); }

const std::string& LiveThreaded::Gateway() const {
  return impl_->blocking.Gateway();
}

void LiveThreaded::Start(Callback callback) {
  // Safe to pass raw pointer because `thread_` cannot outlive `impl_`
  thread_ = detail::ScopedThread{&LiveThreaded::ProcessingThread, impl_.get(),
                                 std::move(callback)};
}

void LiveThreaded::ProcessingThread(Impl* impl, Callback&& callback) {
  constexpr std::chrono::milliseconds kTimeout{50};
  // Thread safety: non-const calls to `blocking` are only performed from this
  // thread
  impl->blocking.Start();
  while (impl->keep_going.load()) {
    const Record* rec = impl->blocking.NextRecord(kTimeout);
    if (rec) {
      callback(*rec);
    }
  }
}
