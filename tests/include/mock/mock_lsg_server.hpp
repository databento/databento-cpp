#pragma once

#include <gtest/gtest.h>  // EXPECT_EQ

#ifdef _WIN32
#include <basetsd.h>   // SSIZE_T
#include <winsock2.h>  // send

using ssize_t = SSIZE_T;
#else
#include <sys/socket.h>  // send
#endif

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>  // function
#include <mutex>
#include <string>
#include <vector>

#include "databento/detail/scoped_fd.hpp"      // ScopedFd
#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/enums.hpp"                 // Schema, SType
#include "databento/iwritable.hpp"
#include "databento/record.hpp"  // RecordHeader

namespace databento::tests::mock {
class SocketStream : public databento::IWritable {
 public:
  explicit SocketStream(detail::Socket socket) : socket_{socket} {}

  void WriteAll(const std::byte* buffer, std::size_t length) override;
  ::ssize_t LastWriteSize() const { return last_write_size_; }

 private:
  detail::Socket socket_;
  ::ssize_t last_write_size_{};
};

class MockLsgServer {
 public:
  MockLsgServer(std::string dataset, bool ts_out,
                std::function<void(MockLsgServer&)> serve_fn);
  MockLsgServer(std::string dataset, bool ts_out,
                std::chrono::seconds heartbeat_interval,
                std::function<void(MockLsgServer&)> serve_fn);

  std::uint16_t Port() const { return port_; }

  void Accept();
  void Authenticate();
  void Subscribe(const std::vector<std::string>& symbols, Schema schema, SType stype,
                 bool is_last);
  void Subscribe(const std::vector<std::string>& symbols, Schema schema, SType stype,
                 const std::string& start, bool is_last);
  void SubscribeWithSnapshot(const std::vector<std::string>& symbols, Schema schema,
                             SType stype, bool is_last);
  void Start();
  std::size_t Send(const std::string& msg);
  ::ssize_t UncheckedSend(const std::string& msg);
  template <typename Rec>
  void SendRecord(const Rec& rec) {
    const std::string rec_str{reinterpret_cast<const char*>(&rec), sizeof(rec)};
    Send(rec_str);
  }
  // Send a record split across two packets. Waiting on a condition variable
  // between packets.
  template <typename Rec>
  void SplitSendRecord(Rec rec, const bool& send_remaining, std::mutex& mutex,
                       std::condition_variable& cv) {
    const std::string first_part{reinterpret_cast<const char*>(&rec),
                                 sizeof(RecordHeader)};
    Send(first_part);
    {
      std::unique_lock<std::mutex> lock{mutex};
      cv.wait(lock, [&send_remaining] { return send_remaining; });
    }
    const std::string second_part{
        reinterpret_cast<const char*>(&rec) + sizeof(RecordHeader),
        sizeof(Rec) - sizeof(RecordHeader)};
    Send(second_part);
  }

  void Close();

 private:
  detail::Socket InitSocketAndSetPort();
  detail::Socket InitSocketAndSetPort(int port);
  std::string Receive();

  template <typename T>
  std::size_t SendBytes(T bytes) {
    const auto write_size = ::send(
        conn_fd_.Get(), reinterpret_cast<const char*>(&bytes), sizeof(bytes), {});
    EXPECT_EQ(write_size, sizeof(bytes)) << ::strerror(errno);
    return static_cast<std::size_t>(write_size);
  }

  std::string dataset_;
  bool ts_out_;
  std::chrono::seconds heartbeat_interval_;
  std::uint16_t port_{};
  detail::ScopedFd socket_{};
  detail::ScopedFd conn_fd_{};
  detail::ScopedThread thread_;
};
}  // namespace databento::tests::mock
