#include "mock/mock_lsg_server.hpp"

#include <gtest/gtest.h>  // EXPECT_*
#include <netinet/in.h>   // sockaddr_in
#include <openssl/sha.h>  // SHA256_DIGEST_LENGTH
#include <sys/socket.h>   // recv
#include <unistd.h>       // close, socklen_t

#include "databento/exceptions.hpp"
#include "databento/symbology.hpp"   // JoinSymbolStrings
#include "mock/mock_tcp_server.hpp"  // InitSocket

using databento::test::mock::MockLsgServer;

MockLsgServer::MockLsgServer(std::function<void(MockLsgServer&)> serve_fn)
    : socket_{InitSocketAndSetPort()},
      thread_{std::move(serve_fn), std::ref(*this)} {}

void MockLsgServer::Accept() {
  sockaddr_in addr{};
  auto addr_len = static_cast<socklen_t>(sizeof(addr));
  conn_fd_ = detail::ScopedFd{
      ::accept(socket_.Get(), reinterpret_cast<sockaddr*>(&addr), &addr_len)};
}

std::string MockLsgServer::Receive() {
  std::string received(1024, 0);
  const auto read_size =
      ::recv(conn_fd_.Get(), &*received.begin(), received.size(), {});
  if (read_size == 0) {
    throw TcpError{{}, "Client closed socket"};
  } else if (read_size < 0) {
    throw TcpError{errno, "Server failed to read"};
  }
  received.resize(static_cast<std::size_t>(read_size));
  return received;
}

void MockLsgServer::Send(const std::string& msg) {
  const auto write_size = ::write(conn_fd_.Get(), msg.data(), msg.length());
  ASSERT_EQ(write_size, msg.length());
}

void MockLsgServer::Authenticate() {
  Send("lsg-test\n");
  // send challenge separate to test multiple reads to get CRAM challenge
  Send("cram=t7kNhwj4xqR0QYjzFKtBEG2ec2pXJ4FK\n");
  const auto received = Receive();
  const auto auth_start = received.find('=') + 1;
  const auto auth =
      received.substr(auth_start, received.find('-') - auth_start);
  EXPECT_EQ(auth.length(), SHA256_DIGEST_LENGTH * 2);
  for (const char c : auth) {
    EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
        << "Expected hex character";
  }
  EXPECT_NE(received.find("encoding=dbz"), std::string::npos);
  EXPECT_NE(received.find("ts_out=0"), std::string::npos);
  Send("success=1|session_id=5|\n");
}

void MockLsgServer::Subscribe(const std::string& dataset,
                              const std::vector<std::string>& symbols,
                              Schema schema, SType stype) {
  const auto received = Receive();
  EXPECT_NE(received.find("dataset=" + dataset), std::string::npos);
  EXPECT_NE(
      received.find("symbols=" +
                    JoinSymbolStrings("MockLsgServer::Subscribe", symbols)),
      std::string::npos);
  EXPECT_NE(
      received.find("symbols=" +
                    JoinSymbolStrings("MockLsgServer::Subscribe", symbols)),
      std::string::npos);
  EXPECT_NE(received.find(std::string{"schema="} + ToString(schema)),
            std::string::npos);
  EXPECT_NE(received.find(std::string{"stype_in="} + ToString(stype)),
            std::string::npos);
}

void MockLsgServer::Start() {
  const auto received = Receive();
  EXPECT_EQ(received, "start_session\n");
}

int MockLsgServer::InitSocketAndSetPort() {
  auto pair = MockTcpServer::InitSocket();
  port_ = pair.first;
  return pair.second;
}
