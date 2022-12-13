#pragma once

#include <cstdint>
#include <functional>  // function
#include <mutex>
#include <string>

#include "databento/detail/scoped_thread.hpp"

namespace databento {
namespace test {
namespace mock {
class MockTcpServer {
 public:
  MockTcpServer();
  explicit MockTcpServer(std::function<void(MockTcpServer&)> serve_fn);
  MockTcpServer(const MockTcpServer&) = delete;
  MockTcpServer& operator=(const MockTcpServer&) = delete;
  MockTcpServer(MockTcpServer&&) = delete;
  MockTcpServer& operator=(MockTcpServer&&) = delete;
  ~MockTcpServer();

  std::uint16_t Port() const { return port_; }
  // Set the data the server will send to its client.
  void SetSend(std::string send);
  // Wait for the server to receive data. Returns the data the server receives.
  std::string AwaitReceived() const;

  // Lower-level methods
  void Accept();
  void Read();
  void Write();
  void Close();

 private:
  void Serve();
  int InitSocket();

  std::uint16_t port_{};
  int socket_{};
  int conn_fd_{};
  std::string received_;
  mutable std::mutex received_mutex_;
  std::string send_;
  mutable std::mutex send_mutex_;
  detail::ScopedThread thread_;
};
}  // namespace mock
}  // namespace test
}  // namespace databento
