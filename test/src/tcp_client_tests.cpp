#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>

#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/tcp_client.hpp"
#include "databento/exceptions.hpp"
#include "mock/mock_tcp_server.hpp"

namespace databento {
namespace test {
class TcpClientTests : public testing::Test {
 protected:
  TcpClientTests()
      : testing::Test(),
        mock_server_{},
        target_{"127.0.0.1", mock_server_.Port()} {}

  mock::MockTcpServer mock_server_;
  detail::TcpClient target_;
};

TEST_F(TcpClientTests, TestWriteAllString) {
  const std::string msg = "testing 1, 2, 3";
  target_.WriteAll(msg);
  ASSERT_EQ(mock_server_.AwaitReceived(), msg);
}

TEST_F(TcpClientTests, TestWriteAllCStr) {
  const std::string msg = "testing 1, 2, 3";
  target_.WriteAll(msg.c_str(), msg.length());
  ASSERT_EQ(mock_server_.AwaitReceived(), msg);
}

TEST_F(TcpClientTests, TestFullRead) {
  const std::string kSendData = "Live data";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start");

  std::array<char, 10> buffer{0};
  // - 1 to leave NUL byte
  const auto res = target_.Read(buffer.data(), buffer.size() - 1);

  EXPECT_STREQ(buffer.data(), kSendData.c_str());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Ok);
  EXPECT_EQ(res.read_size, kSendData.length());
  EXPECT_EQ(res.read_size, buffer.size() - 1);
}

TEST_F(TcpClientTests, TestPartialRead) {
  const std::string kSendData = "Partial re";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start");

  std::array<char, 100> buffer{0};
  const auto res = target_.Read(buffer.data(), buffer.size());

  EXPECT_STREQ(buffer.data(), kSendData.c_str());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Ok);
  EXPECT_EQ(res.read_size, kSendData.length());
}

TEST_F(TcpClientTests, TestReadClose) {
  // server does one write than reads
  target_.WriteAll("start");

  std::array<char, 10> buffer{0};
  const auto res = target_.Read(buffer.data(), buffer.size());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Closed);
  EXPECT_EQ(res.read_size, 0);
}

TEST_F(TcpClientTests, TestReadTimeout) {
  std::mutex mutex;
  std::condition_variable cv;
  mock::MockTcpServer mock_server{[&mutex, &cv](mock::MockTcpServer& server) {
    // simulate slow or delayed send
    server.Accept();
    server.SetSend("hello");
    // wait for timeout
    {
      std::unique_lock<std::mutex> lock{mutex};
      cv.wait(lock);
    }
    // then send
    server.Send();
    server.Close();
  }};
  target_ = {"127.0.0.1", mock_server.Port()};

  std::array<char, 10> buffer{0};
  const auto res =
      target_.Read(buffer.data(), buffer.size(), std::chrono::milliseconds{5});
  {
    std::lock_guard<std::mutex> lock{mutex};
    cv.notify_one();
  }
  EXPECT_EQ(res.status, detail::TcpClient::Status::Timeout);
  EXPECT_EQ(res.read_size, 0);
}

TEST_F(TcpClientTests, TestReadCloseNoTimeout) {
  mock::MockTcpServer mock_server{[](mock::MockTcpServer& server) {
    server.Accept();
    server.Close();
  }};
  target_ = {"127.0.0.1", mock_server.Port()};

  constexpr std::chrono::milliseconds kTimeout{5};

  std::array<char, 10> buffer{0};
  const auto start = std::chrono::steady_clock::now();
  // server closing the connection should cause `Read` to return immediately,
  // not wait for the timeout
  const auto res = target_.Read(buffer.data(), buffer.size(), kTimeout);
  const auto end = std::chrono::steady_clock::now();
  EXPECT_EQ(res.status, detail::TcpClient::Status::Closed);
  EXPECT_EQ(res.read_size, 0);
  EXPECT_LT(end - start, kTimeout);
}
}  // namespace test
}  // namespace databento
