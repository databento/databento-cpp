#pragma once

#include <cstdint>
#include <functional>  // function
#include <mutex>
#include <string>
#include <utility>  // move

#include "databento/detail/scoped_fd.hpp"      // ScopedFd
#include "databento/detail/scoped_thread.hpp"  // ScopedThread

namespace databento::tests::mock {
class MockTcpServer {
 public:
  MockTcpServer();
  explicit MockTcpServer(std::function<void(MockTcpServer&)> serve_fn);

  static std::pair<std::uint16_t, detail::Socket> InitSocket();
  static std::pair<std::uint16_t, detail::Socket> InitSocket(std::uint16_t port);

  std::uint16_t Port() const { return port_; }
  // Set the data the server will send to its client.
  void SetSend(std::string send);
  // Wait for the server to receive data. Returns the data the server receives.
  std::string AwaitReceived() const;

  void Accept();
  void Receive();
  void Send();
  void Close();

 private:
  void Serve();
  detail::Socket InitSocketAndSetPort();

  std::uint16_t port_{};
  detail::ScopedFd socket_{};
  detail::ScopedFd conn_fd_{};
  std::string received_;
  mutable std::mutex received_mutex_;
  std::string send_;
  mutable std::mutex send_mutex_;
  detail::ScopedThread thread_;
};
}  // namespace databento::tests::mock
