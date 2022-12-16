#pragma once

#include <cstdint>
#include <functional>  // function
#include <mutex>
#include <string>
#include <utility>  // move

#include "databento/detail/scoped_fd.hpp"      // ScopedFd
#include "databento/detail/scoped_thread.hpp"  // ScopedThread

namespace databento {
namespace test {
namespace mock {
class MockTcpServer {
 public:
  MockTcpServer();

  static std::pair<std::uint16_t, int> InitSocket();

  std::uint16_t Port() const { return port_; }
  // Set the data the server will send to its client.
  void SetSend(std::string send);
  // Wait for the server to receive data. Returns the data the server receives.
  std::string AwaitReceived() const;

 private:
  void Serve();
  int InitSocketAndSetPort();
  void Accept();
  void Receive();
  void Send();
  void Close();

  std::uint16_t port_{};
  detail::ScopedFd socket_{};
  detail::ScopedFd conn_fd_{-1};
  std::string received_;
  mutable std::mutex received_mutex_;
  std::string send_;
  mutable std::mutex send_mutex_;
  detail::ScopedThread thread_;
};
}  // namespace mock
}  // namespace test
}  // namespace databento
