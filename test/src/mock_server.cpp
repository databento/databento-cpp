#include "mock/mock_server.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

using databento::mock::MockServer;

int MockServer::ListenOnThread() {
  listen_thread_ = std::thread{[this] { this->server_.listen_after_bind(); }};
  return port_;
}

void MockServer::MockGetJson(const std::string& path,
                             const nlohmann::json& json) {
  this->MockGetJson(path, {}, json);
}

void MockServer::MockGetJson(const std::string& path,
                             const std::map<std::string, std::string>& params,
                             const nlohmann::json& json) {
  server_.Get(path, [json, params](const httplib::Request& req,
                                   httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    auto _auth = req.get_header_value("Authorization");
    for (const auto& param : params) {
      if (!req.has_param(param.first)) {
        std::ostringstream err_msg;
        err_msg << "Missing query param " << param.first;
        std::cerr << err_msg.str() << std::endl;
        throw std::runtime_error{err_msg.str()};
      }
      if (req.get_param_value(param.first) != param.second) {
        std::ostringstream err_msg;
        err_msg << "Incorrect query param value for " << param.first
                << ". Expected " << param.second << ", found "
                << req.get_param_value(param.first);
        std::cerr << err_msg.str() << std::endl;
        throw std::runtime_error{err_msg.str()};
      }
    }

    resp.set_content(json.dump(), "application/json");
    resp.status = 200;
  });
}
