#include "mock/mock_lsg_server.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // recv
#endif
#include <openssl/sha.h>  // SHA256_DIGEST_LENGTH

#include <cstdint>
#include <limits>
#include <sstream>

#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/symbology.hpp"   // JoinSymbolStrings
#include "mock/mock_tcp_server.hpp"  // InitSocket

using databento::test::mock::MockLsgServer;

MockLsgServer::MockLsgServer(std::string dataset, bool ts_out,
                             std::function<void(MockLsgServer&)> serve_fn)
    : dataset_{std::move(dataset)},
      ts_out_{ts_out},
      socket_{InitSocketAndSetPort()},
      thread_{std::move(serve_fn), std::ref(*this)} {}

void MockLsgServer::Accept() {
  sockaddr_in addr{};
  auto addr_len = static_cast<socklen_t>(sizeof(addr));
  conn_fd_ = detail::ScopedFd{
      ::accept(socket_.Get(), reinterpret_cast<sockaddr*>(&addr), &addr_len)};
}

std::string MockLsgServer::Receive() {
  std::string received(1024, 0);
  char c{};
  std::size_t read_size{};
  // Read char by char until newline
  do {
    const auto ret = ::recv(conn_fd_.Get(), &received[read_size], 1, {});
    if (ret == 0) {
      throw TcpError{{}, "Client closed socket"};
    }
    if (ret < 0) {
      throw TcpError{errno, "Server failed to read"};
    }
    c = received[read_size];
    ++read_size;
  } while (c != '\n' && read_size < 1024);
  if (read_size == 1024) {
    throw TcpError{{}, "Overran buffer in MockLsgServer"};
  }
  received.resize(read_size);
  return received;
}

std::size_t MockLsgServer::Send(const std::string& msg) {
  const auto write_size = UncheckedSend(msg);
  EXPECT_EQ(write_size, msg.length());
  return static_cast<std::size_t>(write_size);
}

::ssize_t MockLsgServer::UncheckedSend(const std::string& msg) {
// MSG_NOSIGNAL doesn't exist on Windows, but also isn't necessary
#ifdef _WIN32
  constexpr int MSG_NOSIGNAL = {};
#endif
  // Don't send a SIGPIPE if the connection is closed
  return ::send(conn_fd_.Get(), msg.data(), msg.length(), MSG_NOSIGNAL);
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
  EXPECT_NE(received.find("dataset=" + dataset_), std::string::npos);
  EXPECT_NE(received.find("encoding=dbn"), std::string::npos);
  EXPECT_NE(received.find("ts_out=" + std::to_string(ts_out_)),
            std::string::npos);
  Send("success=1|session_id=5|\n");
}

void MockLsgServer::Subscribe(const std::vector<std::string>& symbols,
                              Schema schema, SType stype) {
  const auto received = Receive();
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
  Send("DBN\1");
  // frame length: fixed size + length of schema definition, symbols, partial,
  // not_found, and mappings
  constexpr std::uint32_t kFrameLen = 100 + sizeof(std::uint32_t) * 5;
  SendBytes(kFrameLen);
  std::size_t bytes_written{};
  // dataset
  bytes_written += Send(dataset_);
  bytes_written += Send(std::string(16 - dataset_.length(), '\0'));
  // mixed schema
  bytes_written += SendBytes(std::numeric_limits<std::uint16_t>::max());
  // start
  bytes_written += SendBytes(std::uint64_t{0});
  // end
  bytes_written += SendBytes(std::numeric_limits<std::uint64_t>::max());
  // limit
  bytes_written += SendBytes(std::uint64_t{0});
  // record_count
  bytes_written += SendBytes(std::numeric_limits<std::uint64_t>::max());
  // stype_in
  bytes_written += SendBytes(SType::RawSymbol);
  // stype_out
  bytes_written += SendBytes(SType::InstrumentId);
  // padding
  bytes_written += Send(std::string(48 + sizeof(std::uint32_t) * 5, '\0'));

  ASSERT_EQ(bytes_written, kFrameLen);
}

void MockLsgServer::Close() { conn_fd_.Close(); }

databento::detail::Socket MockLsgServer::InitSocketAndSetPort() {
  const auto pair = MockTcpServer::InitSocket();
  port_ = pair.first;
  return pair.second;
}
