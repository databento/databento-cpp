#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <map>
#include <string>

#include "databento/detail/scoped_thread.hpp"

namespace databento {
namespace mock {
class MockHttpServer {
 public:
  explicit MockHttpServer(std::string api_key)
      : port_{server_.bind_to_any_port("localhost")},
        api_key_{std::move(api_key)} {}
  MockHttpServer(MockHttpServer&&) = delete;
  MockHttpServer& operator=(MockHttpServer&&) = delete;
  MockHttpServer(const MockHttpServer&) = delete;
  MockHttpServer& operator=(const MockHttpServer&) = delete;
  ~MockHttpServer() { server_.stop(); }

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
  detail::ScopedThread listen_thread_;
  std::string api_key_;
};
}  // namespace mock
}  // namespace databento
