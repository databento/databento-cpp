#pragma once

#include <condition_variable>
#include <functional>  // function
#include <mutex>
#include <string>
#include <vector>

#include "databento/detail/scoped_fd.hpp"      // ScopedFd
#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/enums.hpp"                 // Schema, SType
#include "databento/record.hpp"                // RecordHeader

namespace databento {
namespace test {
namespace mock {
class MockLsgServer {
 public:
  MockLsgServer(std::string dataset,
                std::function<void(MockLsgServer&)> serve_fn);

  std::uint16_t Port() const { return port_; }

  void Accept();
  void Authenticate();
  void Subscribe(const std::vector<std::string>& symbols, Schema schema,
                 SType stype);
  void Start();
  void Send(const std::string& msg);
  template <typename Rec>
  void SendRecord(Rec rec) {
    const std::string rec_str{reinterpret_cast<const char*>(&rec), sizeof(rec)};
    Send(rec_str);
  }
  // Send a record split across two packets. Waiting on a condition variable
  // between packets.
  template <typename Rec>
  void SplitSendRecord(Rec rec, std::mutex& mutex,
                       std::condition_variable& cv) {
    const std::string first_part{reinterpret_cast<const char*>(&rec),
                                 sizeof(RecordHeader)};
    Send(first_part);
    {
      std::unique_lock<std::mutex> lock{mutex};
      cv.wait(lock);
    }
    const std::string second_part{
        reinterpret_cast<const char*>(&rec) + sizeof(RecordHeader),
        sizeof(Rec) - sizeof(RecordHeader)};
    Send(second_part);
  }

 private:
  int InitSocketAndSetPort();
  std::string Receive();

  std::string dataset_;
  std::uint16_t port_{};
  detail::ScopedFd socket_{};
  detail::ScopedFd conn_fd_{};
  detail::ScopedThread thread_;
};
}  // namespace mock
}  // namespace test
}  // namespace databento
