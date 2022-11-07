#pragma once

#include <httplib.h>

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

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
  void MockBadRequest(const std::string& path, const nlohmann::json& json);
  void MockGetJson(const std::string& path, const nlohmann::json& json);
  void MockGetJson(const std::string& path,
                   const std::map<std::string, std::string>& params,
                   const nlohmann::json& json);
  void MockPostJson(const std::string& path,
                    const std::map<std::string, std::string>& params,
                    const nlohmann::json& json);
  void MockStreamDbz(const std::string& path,
                     const std::map<std::string, std::string>& params,
                     const std::string& dbz_path);

 private:
  static void CheckParams(const std::map<std::string, std::string>& params,
                          const httplib::Request& req);

  httplib::Server server_{};
  const int port_{};
  std::thread listen_thread_;
  std::string api_key_;
};
}  // namespace mock
}  // namespace databento
