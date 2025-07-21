#include "mock/mock_lsg_server.hpp"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>  // sockaddr_in
#include <sys/socket.h>  // recv
#endif
#include <openssl/sha.h>  // SHA256_DIGEST_LENGTH

#include <chrono>
#include <cstddef>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/symbology.hpp"   // JoinSymbolStrings
#include "mock/mock_tcp_server.hpp"  // InitSocket

using databento::tests::mock::SocketStream;

void SocketStream::WriteAll(const std::byte* buffer, std::size_t length) {
// MSG_NOSIGNAL doesn't exist on Windows, but also isn't necessary
#ifdef _WIN32
  constexpr int MSG_NOSIGNAL = {};
#endif
  // Don't send a SIGPIPE if the connection is closed
  last_write_size_ =
      ::send(socket_, reinterpret_cast<const char*>(buffer), length, MSG_NOSIGNAL);
}

using databento::tests::mock::MockLsgServer;

MockLsgServer::MockLsgServer(std::string dataset, bool ts_out,
                             std::function<void(MockLsgServer&)> serve_fn)
    : MockLsgServer{std::move(dataset), ts_out, {}, std::move(serve_fn)} {}

MockLsgServer::MockLsgServer(std::string dataset, bool ts_out,
                             std::chrono::seconds heartbeat_interval,
                             std::function<void(MockLsgServer&)> serve_fn)
    : dataset_{std::move(dataset)},
      ts_out_{ts_out},
      heartbeat_interval_{heartbeat_interval},
      socket_{InitSocketAndSetPort()},
      thread_{std::move(serve_fn), std::ref(*this)} {}

void MockLsgServer::Accept() {
  sockaddr_in addr{};
  auto addr_len = static_cast<socklen_t>(sizeof(addr));
  conn_fd_ = detail::ScopedFd{
      ::accept(socket_.Get(), reinterpret_cast<sockaddr*>(&addr), &addr_len)};
}

std::string MockLsgServer::Receive() {
  std::string received(static_cast<std::size_t>(32 * 1024), 0);
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
  } while (c != '\n' && read_size < received.size());
  if (read_size == received.size()) {
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
  const auto auth = received.substr(auth_start, received.find('-') - auth_start);
  EXPECT_EQ(auth.length(), SHA256_DIGEST_LENGTH * 2);
  for (const char c : auth) {
    EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
        << "Expected hex character";
  }
  EXPECT_NE(received.find("dataset=" + dataset_), std::string::npos);
  EXPECT_NE(received.find("encoding=dbn"), std::string::npos);
  EXPECT_NE(received.find("ts_out=" + std::to_string(ts_out_)), std::string::npos);
  if (heartbeat_interval_.count() > 0) {
    EXPECT_NE(received.find("heartbeat_interval_s=" +
                            std::to_string(heartbeat_interval_.count())),
              std::string::npos);
  } else {
    EXPECT_EQ(received.find("heartbeat_interval_s="), std::string::npos);
  }
  Send("success=1|session_id=5|\n");
}

void MockLsgServer::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                              SType stype, bool is_last) {
  Subscribe(symbols, schema, stype, "", is_last);
}

void MockLsgServer::SubscribeWithSnapshot(const std::vector<std::string>& symbols,
                                          Schema schema, SType stype, bool is_last) {
  const auto received = Receive();
  EXPECT_NE(received.find("symbols=" +
                          JoinSymbolStrings("MockLsgServer::Subscribe", symbols)),
            std::string::npos);
  EXPECT_NE(received.find(std::string{"schema="} + ToString(schema)),
            std::string::npos);
  EXPECT_NE(received.find(std::string{"stype_in="} + ToString(stype)),
            std::string::npos);
  EXPECT_EQ(received.find("start="), std::string::npos);
  EXPECT_NE(received.find("id="), std::string::npos);
  EXPECT_NE(received.find("snapshot=1"), std::string::npos);
  EXPECT_NE(received.find(std::string{"is_last="} + std::to_string(is_last)),
            std::string::npos);
}

void MockLsgServer::Subscribe(const std::vector<std::string>& symbols, Schema schema,
                              SType stype, const std::string& start, bool is_last) {
  const auto received = Receive();
  EXPECT_NE(received.find("symbols=" +
                          JoinSymbolStrings("MockLsgServer::Subscribe", symbols)),
            std::string::npos);
  EXPECT_NE(received.find(std::string{"schema="} + ToString(schema)),
            std::string::npos);
  EXPECT_NE(received.find(std::string{"stype_in="} + ToString(stype)),
            std::string::npos);
  if (start.empty()) {
    EXPECT_EQ(received.find("start="), std::string::npos);
  } else {
    EXPECT_NE(received.find(std::string{"start="} + start), std::string::npos);
  }
  EXPECT_NE(received.find("id="), std::string::npos);
  EXPECT_NE(received.find("snapshot=0"), std::string::npos);
  EXPECT_NE(received.find(std::string{"is_last="} + std::to_string(is_last)),
            std::string::npos);
}

void MockLsgServer::Start() {
  const auto received = Receive();
  EXPECT_EQ(received, "start_session\n");

  SocketStream writable{conn_fd_.Get()};
  Metadata metadata{1,
                    dataset_,
                    {},
                    {},
                    UnixNanos{std::chrono::nanoseconds{kUndefTimestamp}},
                    0,
                    {},
                    SType::InstrumentId,
                    false,
                    kSymbolCstrLenV1,
                    {},
                    {},
                    {},
                    {}};
  DbnEncoder::EncodeMetadata(metadata, &writable);
}

void MockLsgServer::Close() { conn_fd_.Close(); }

databento::detail::Socket MockLsgServer::InitSocketAndSetPort() {
  const auto pair = MockTcpServer::InitSocket();
  port_ = pair.first;
  return pair.second;
}
