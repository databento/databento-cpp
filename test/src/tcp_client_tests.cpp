#include <gtest/gtest.h>

#include <array>
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
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  // - 1 to leave NUL byte
  const auto read_size = target_.Read(buffer.data(), buffer.size() - 1);

  EXPECT_STREQ(buffer.data(), kSendData.c_str());
  EXPECT_EQ(read_size, kSendData.length());
  EXPECT_EQ(read_size, buffer.size() - 1);
}

TEST_F(TcpClientTests, TestPartialRead) {
  const std::string kSendData = "Partial re";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 100> buffer{0};
  const auto read_size = target_.Read(buffer.data(), buffer.size());

  EXPECT_STREQ(buffer.data(), kSendData.c_str());
  EXPECT_EQ(read_size, kSendData.length());
}

TEST_F(TcpClientTests, TestEmptyRead) {
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  const auto read_size = target_.Read(buffer.data(), buffer.size());
  ASSERT_EQ(read_size, 0);
}

TEST_F(TcpClientTests, TestReadExactSuccess) {
  const std::string kSendData = "Live data";
  mock_server_.SetSend(kSendData);
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 10> buffer{0};
  // - 1 to leave NUL byte
  target_.ReadExact(buffer.data(), buffer.size() - 1);

  EXPECT_STREQ(buffer.data(), kSendData.c_str());
}

TEST_F(TcpClientTests, TestReadExactFailure) {
  mock_server_.SetSend("Partial re");
  // server does one write than reads
  target_.WriteAll("start", 5);

  std::array<char, 100> buffer{0};
  ASSERT_THROW(target_.ReadExact(buffer.data(), buffer.size()), TcpError);
}
}  // namespace test
}  // namespace databento
