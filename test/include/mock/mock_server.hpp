#pragma once

#include <string>
#include <thread>
// ignore warnings from httplib
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <httplib.h>
#pragma GCC diagnostic pop
#include <nlohmann/json.hpp>

namespace databento {
namespace mock {
class MockServer {
 public:
  explicit MockServer(std::string api_key)
      : port_{server_.bind_to_any_port("localhost")},
        api_key_{std::move(api_key)} {}
  MockServer(MockServer&&) = delete;
  MockServer& operator=(MockServer&&) = delete;
  MockServer(const MockServer&) = delete;
  MockServer& operator=(const MockServer&) = delete;
  ~MockServer() {
    server_.stop();
    if (listen_thread_.joinable()) {
      listen_thread_.join();
    }
  }

  int ListenOnThread();
  void MockGetJson(const std::string& path, const nlohmann::json& json);
  void MockGetJson(const std::string& path,
                   const std::map<std::string, std::string>& params,
                   const nlohmann::json& json);

 private:
  httplib::Server server_{};
  const int port_{};
  std::thread listen_thread_;
  std::string api_key_;
};
}  // namespace mock
}  // namespace databento
