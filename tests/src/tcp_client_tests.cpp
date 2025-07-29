#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>

#include "databento/detail/tcp_client.hpp"
#include "databento/exceptions.hpp"
#include "mock/mock_tcp_server.hpp"

namespace databento::tests {
class TcpClientTests : public testing::Test {
 protected:
  TcpClientTests()
      : testing::Test(), mock_server_{}, target_{"127.0.0.1", mock_server_.Port()} {}

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
  target_.WriteAll(msg);
  ASSERT_EQ(mock_server_.AwaitReceived(), msg);
}

TEST_F(TcpClientTests, TestReadExact) {
  const std::string kSendData = "Read exactly";
  mock_server_.SetSend(kSendData);
  target_.WriteAll("start");

  std::array<std::byte, 13> buffer{};
  ASSERT_EQ(buffer.size() - 1, kSendData.size());

  target_.ReadExact(buffer.data(), buffer.size() - 1);
  ASSERT_STREQ(reinterpret_cast<const char*>(buffer.data()), kSendData.c_str());
}

TEST_F(TcpClientTests, TestFullReadSome) {
  const std::string kSendData = "Live data";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start");

  std::array<std::byte, 10> buffer{};
  // - 1 to leave NUL byte
  const auto res = target_.ReadSome(buffer.data(), buffer.size() - 1);

  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), kSendData.c_str());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Ok);
  EXPECT_EQ(res.read_size, kSendData.length());
  EXPECT_EQ(res.read_size, buffer.size() - 1);
}

TEST_F(TcpClientTests, TestPartialReadSome) {
  const std::string kSendData = "Partial re";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start");

  std::array<std::byte, 100> buffer{};
  const auto res = target_.ReadSome(buffer.data(), buffer.size());

  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), kSendData.c_str());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Ok);
  EXPECT_EQ(res.read_size, kSendData.length());
}

TEST_F(TcpClientTests, TestReadSomeClose) {
  // server does one write than reads
  target_.WriteAll("start");

  std::array<std::byte, 10> buffer{};
  const auto res = target_.ReadSome(buffer.data(), buffer.size());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Closed);
  EXPECT_EQ(res.read_size, 0);
}

TEST_F(TcpClientTests, TestReadSomeTimeout) {
  bool has_timed_out{};
  std::mutex has_timed_out_mutex;
  std::condition_variable has_timed_out_cv;
  const mock::MockTcpServer mock_server{
      [&has_timed_out, &has_timed_out_mutex,
       &has_timed_out_cv](mock::MockTcpServer& server) {
        // simulate slow or delayed send
        server.Accept();
        server.SetSend("hello");
        // wait for timeout
        {
          std::unique_lock<std::mutex> lock{has_timed_out_mutex};
          has_timed_out_cv.wait(lock, [&has_timed_out] { return has_timed_out; });
        }
        // then send
        server.Send();
        server.Close();
      }};
  target_ = {"127.0.0.1", mock_server.Port()};

  std::array<std::byte, 10> buffer{};
  const auto res =
      target_.ReadSome(buffer.data(), buffer.size(), std::chrono::milliseconds{5});
  {
    const std::lock_guard<std::mutex> lock{has_timed_out_mutex};
    has_timed_out = true;
    has_timed_out_cv.notify_one();
  }
  EXPECT_EQ(res.status, detail::TcpClient::Status::Timeout);
  EXPECT_EQ(res.read_size, 0);
}

TEST_F(TcpClientTests, TestReadCloseNoTimeout) {
  const mock::MockTcpServer mock_server{[](mock::MockTcpServer& server) {
    server.Accept();
    server.Close();
  }};
  target_ = {"127.0.0.1", mock_server.Port()};

  constexpr std::chrono::milliseconds kTimeout{5};

  std::array<std::byte, 10> buffer{};
  const auto start = std::chrono::steady_clock::now();
  // server closing the connection should cause `ReadSome` to return
  // immediately, not wait for the timeout
  const auto res = target_.ReadSome(buffer.data(), buffer.size(), kTimeout);
  const auto end = std::chrono::steady_clock::now();
  EXPECT_EQ(res.status, detail::TcpClient::Status::Closed);
  EXPECT_EQ(res.read_size, 0);
  EXPECT_LT(end - start, kTimeout);
}

TEST_F(TcpClientTests, ReadAfterClose) {
  const std::string kSendData = "Read after close";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start");

  std::array<std::byte, 10> buffer{};
  const auto res = target_.ReadSome(buffer.data(), buffer.size());
  EXPECT_EQ(res.status, detail::TcpClient::Status::Ok);
  EXPECT_GT(res.read_size, 0);
  target_.Close();
  ASSERT_THROW(target_.ReadSome(buffer.data(), buffer.size()), databento::TcpError);
}
}  // namespace databento::tests
