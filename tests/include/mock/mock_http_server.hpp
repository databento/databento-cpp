#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <string>

#include "databento/detail/buffer.hpp"
#include "databento/detail/scoped_thread.hpp"
#include "databento/record.hpp"

namespace databento::tests::mock {
class MockHttpServer {
 public:
  explicit MockHttpServer(std::string api_key)
      : port_{server_.bind_to_any_port("localhost")}, api_key_{std::move(api_key)} {}
  MockHttpServer(MockHttpServer&&) = delete;
  MockHttpServer& operator=(MockHttpServer&&) = delete;
  MockHttpServer(const MockHttpServer&) = delete;
  MockHttpServer& operator=(const MockHttpServer&) = delete;
  ~MockHttpServer() { server_.stop(); }

  int ListenOnThread();
  void MockBadPostRequest(const std::string& path, const nlohmann::json& json);
  void MockGetJson(const std::string& path, const nlohmann::json& json);
  void MockGetJson(const std::string& path,
                   const std::map<std::string, std::string>& params,
                   const nlohmann::json& json);
  void MockGetJson(const std::string& path,
                   const std::map<std::string, std::string>& params,
                   const nlohmann::json& json, const nlohmann::json& warnings);
  void MockPostJson(const std::string& path,
                    const std::map<std::string, std::string>& params,
                    const nlohmann::json& json);
  void MockGetDbn(const std::string& path,
                  const std::map<std::string, std::string>& params,
                  const std::string& dbn_path);
  void MockPostDbn(const std::string& path,
                   const std::map<std::string, std::string>& params,
                   const std::string& dbn_path);
  void MockPostDbn(const std::string& path,
                   const std::map<std::string, std::string>& params, Record record,
                   std::size_t count, std::size_t chunk_size);
  void MockPostDbn(const std::string& path,
                   const std::map<std::string, std::string>& params, Record record,
                   std::size_t count, std::size_t extra_bytes, std::size_t chunk_size);

 private:
  using SharedConstBuffer = std::shared_ptr<const detail::Buffer>;

  static void CheckParams(const std::map<std::string, std::string>& params,
                          const httplib::Request& req);
  static void CheckFormParams(const std::map<std::string, std::string>& params,
                              const httplib::Request& req);
  static SharedConstBuffer EncodeToBuffer(const std::string& dbn_path);
  static httplib::Server::Handler MakeDbnStreamHandler(
      const std::map<std::string, std::string>& params, SharedConstBuffer&& buffer,
      std::size_t chunk_size);

  httplib::Server server_{};
  const int port_{};
  detail::ScopedThread listen_thread_;
  std::string api_key_;
};
}  // namespace databento::tests::mock
