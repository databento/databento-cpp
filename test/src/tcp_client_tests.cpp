#include <gtest/gtest.h>
#include <netinet/in.h>  // IPPROTO_TCP, sockaddr_in, ntohs
#include <sys/socket.h>  // AF_INET, listen, socket
#include <unistd.h>      // close, socklen_t, write

#include <array>
#include <cerrno>
#include <cstdint>
#include <mutex>
#include <string>

#include "databento/detail/tcp_client.hpp"
#include "databento/exceptions.hpp"
#include "scoped_thread.hpp"

namespace databento {
namespace test {
class TcpClientTests : public testing::Test {
 protected:
  int InitSocket();

  TcpClientTests()
      : testing::Test(),
        socket_{InitSocket()},
        server_thread_{[this] { this->Serve(); }},
        target_{"127.0.0.1", port_} {}

  void TearDown() override {
    if (socket_ >= 0) {
      ::close(socket_);
    }
  }

  void Serve() {
    sockaddr_in addr{};
    auto addr_len = static_cast<socklen_t>(sizeof(addr));
    auto conn_fd =
        ::accept(socket_, reinterpret_cast<sockaddr*>(&addr), &addr_len);
    {
      const std::lock_guard<std::mutex> rec_guard{server_received_mutex_};
      server_received_.resize(1024);
      const auto read_size = ::read(conn_fd, &*server_received_.begin(), 1024);
      if (read_size <= 0) {
        throw TcpError{errno, "Server failed to read"};
      }
      server_received_.resize(static_cast<std::size_t>(read_size));
      const std::lock_guard<std::mutex> send_guard{server_send_mutex_};
      const auto write_size =
          ::write(conn_fd, server_send_.data(), server_send_.length());
      ASSERT_EQ(write_size, server_send_.length());
    }
    ::close(conn_fd);
  }

  std::uint16_t port_{};
  int socket_{};
  std::string server_received_;
  std::mutex server_received_mutex_;
  std::string server_send_;
  std::mutex server_send_mutex_;
  ScopedThread server_thread_;
  detail::TcpClient target_;
};

int TcpClientTests::InitSocket() {
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
  port_ = ntohs(addr_actual.sin_port);
  if (::listen(fd, 1) != 0) {
    throw TcpError{errno, "Error listening on port"};
  }
  return fd;
}

TEST_F(TcpClientTests, TestWriteAll) {
  const std::string msg = "testing 1, 2, 3";
  target_.WriteAll(msg.c_str(), msg.length());
  while (true) {
    const std::lock_guard<std::mutex> guard{server_received_mutex_};
    if (!server_received_.empty()) {
      ASSERT_EQ(server_received_, msg);
      break;
    }
  }
}

TEST_F(TcpClientTests, TestFullRead) {
  {
    const std::lock_guard<std::mutex> guard{server_send_mutex_};
    server_send_ = "Live data";
  }
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  // - 1 to leave NUL byte
  const auto read_size = target_.Read(buffer.data(), buffer.size() - 1);

  const std::lock_guard<std::mutex> guard{server_send_mutex_};
  EXPECT_STREQ(buffer.data(), server_send_.c_str());
  EXPECT_EQ(read_size, server_send_.length());
  EXPECT_EQ(read_size, buffer.size() - 1);
}

TEST_F(TcpClientTests, TestPartialRead) {
  {
    const std::lock_guard<std::mutex> guard{server_send_mutex_};
    server_send_ = "Partial re";
  }
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 100> buffer{0};
  const auto read_size = target_.Read(buffer.data(), buffer.size());

  const std::lock_guard<std::mutex> guard{server_send_mutex_};
  EXPECT_STREQ(buffer.data(), server_send_.c_str());
  EXPECT_EQ(read_size, server_send_.length());
}

TEST_F(TcpClientTests, TestEmptyRead) {
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  const auto read_size = target_.Read(buffer.data(), buffer.size());
  ASSERT_EQ(read_size, 0);
}

TEST_F(TcpClientTests, TestReadExactSuccess) {
  {
    const std::lock_guard<std::mutex> guard{server_send_mutex_};
    server_send_ = "Live data";
  }
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  // - 1 to leave NUL byte
  target_.ReadExact(buffer.data(), buffer.size() - 1);

  const std::lock_guard<std::mutex> guard{server_send_mutex_};
  EXPECT_STREQ(buffer.data(), server_send_.c_str());
}

TEST_F(TcpClientTests, TestReadExactFailure) {
  {
    const std::lock_guard<std::mutex> guard{server_send_mutex_};
    server_send_ = "Partial re";
  }
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 100> buffer{0};
  ASSERT_THROW(target_.ReadExact(buffer.data(), buffer.size()), TcpError);
}
}  // namespace test
}  // namespace databento
