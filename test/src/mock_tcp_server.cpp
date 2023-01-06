#include "mock/mock_tcp_server.hpp"

#include <gtest/gtest.h>  // ASSERT_EQ
#include <netinet/in.h>   // IPPROTO_TCP, ntohs, sockaddr_in, TCP_NODELAY
#include <sys/socket.h>   // AF_INET, listen, recv, setsocketopt, socket
#include <unistd.h>       // close, socklen_t, write

#include <thread>  // this_thread

#include "databento/exceptions.hpp"

using databento::test::mock::MockTcpServer;

MockTcpServer::MockTcpServer()
    : MockTcpServer([](MockTcpServer& self) { self.Serve(); }) {}

MockTcpServer::MockTcpServer(std::function<void(MockTcpServer&)> serve_fn)
    : socket_{InitSocketAndSetPort()},
      thread_{std::move(serve_fn), std::ref(*this)} {}

void MockTcpServer::SetSend(std::string send) {
  const std::lock_guard<std::mutex> lock{send_mutex_};
  send_ = std::move(send);
}

std::string MockTcpServer::AwaitReceived() const {
  while (true) {
    const std::lock_guard<std::mutex> lock{received_mutex_};
    if (!received_.empty()) {
      return received_;
    }
    std::this_thread::yield();
  }
}

void MockTcpServer::Serve() {
  Accept();
  Receive();
  Send();
  Close();
}

void MockTcpServer::Accept() {
  sockaddr_in addr{};
  auto addr_len = static_cast<socklen_t>(sizeof(addr));
  conn_fd_ = detail::ScopedFd{
      ::accept(socket_.Get(), reinterpret_cast<sockaddr*>(&addr), &addr_len)};
  // Disable Nagle's algorithm for finer control over when packets are sent
  // during testing
  const int flag = 1;
  const auto res =
      ::setsockopt(socket_.Get(), IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
  if (res < 0) {
    throw TcpError{errno, "Failed to disable Nagle's algorithm"};
  }
}

void MockTcpServer::Receive() {
  const std::lock_guard<std::mutex> rec_guard{received_mutex_};
  received_.resize(1024);
  const auto read_size = ::recv(conn_fd_.Get(), &*received_.begin(), 1024, {});
  if (read_size < 0) {
    throw TcpError{errno, "Server failed to read"};
  }
  received_.resize(static_cast<std::size_t>(read_size));
}

void MockTcpServer::Send() {
  const std::lock_guard<std::mutex> send_guard{send_mutex_};
  const auto write_size = ::write(conn_fd_.Get(), send_.data(), send_.length());
  ASSERT_EQ(write_size, send_.length());
}

void MockTcpServer::Close() { conn_fd_.Close(); }

std::pair<std::uint16_t, int> MockTcpServer::InitSocket() {
  const int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    throw TcpError{errno, "Invalid fd"};
  }
  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = 0;  // will be assigned
  addr_in.sin_addr.s_addr = INADDR_ANY;
  if (::bind(fd, reinterpret_cast<const sockaddr*>(&addr_in),
             sizeof(addr_in)) != 0) {
    throw TcpError{errno, "Failed to bind to port"};
  }
  sockaddr_in addr_actual{};
  auto addr_size = static_cast<socklen_t>(sizeof(addr_actual));
  if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr_actual),
                    &addr_size) == -1) {
    throw TcpError{errno, "Error fetching port"};
  }
  const auto port = ntohs(addr_actual.sin_port);
  if (::listen(fd, 1) != 0) {
    throw TcpError{errno, "Error listening on port"};
  }
  return {port, fd};
}

int MockTcpServer::InitSocketAndSetPort() {
  auto pair = InitSocket();
  port_ = pair.first;
  return pair.second;
}
