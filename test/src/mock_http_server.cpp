#include "mock/mock_http_server.hpp"

#include <fstream>    // ifstream
#include <ios>        // streamsize
#include <iostream>   // cerr
#include <sstream>    // ostringstream
#include <stdexcept>  // runtime_error
#include <vector>

using databento::mock::MockHttpServer;

int MockHttpServer::ListenOnThread() {
  listen_thread_ =
      detail::ScopedThread{[this] { this->server_.listen_after_bind(); }};
  return port_;
}

void MockHttpServer::MockBadRequest(const std::string& path,
                                    const nlohmann::json& json) {
  server_.Get(path, [json](const httplib::Request&, httplib::Response& resp) {
    resp.status = 400;
    resp.body = json.dump();
  });
}

void MockHttpServer::MockGetJson(const std::string& path,
                                 const nlohmann::json& json) {
  this->MockGetJson(path, {}, json);
}

void MockHttpServer::MockGetJson(
    const std::string& path, const std::map<std::string, std::string>& params,
    const nlohmann::json& json) {
  server_.Get(path, [json, params](const httplib::Request& req,
                                   httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    auto _auth = req.get_header_value("Authorization");
    CheckParams(params, req);
    resp.set_content(json.dump(), "application/json");
    resp.status = 200;
  });
}

void MockHttpServer::MockPostJson(
    const std::string& path, const std::map<std::string, std::string>& params,
    const nlohmann::json& json) {
  server_.Post(path, [json, params](const httplib::Request& req,
                                    httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    auto _auth = req.get_header_value("Authorization");
    CheckParams(params, req);
    resp.set_content(json.dump(), "application/json");
    resp.status = 200;
  });
}

void MockHttpServer::MockStreamDbz(
    const std::string& path, const std::map<std::string, std::string>& params,
    const std::string& dbz_path) {
  constexpr std::size_t kChunkSize = 32;

  // Read contents into buffer
  std::ifstream input_file{dbz_path, std::ios::binary | std::ios::ate};
  const auto size = static_cast<std::size_t>(input_file.tellg());
  input_file.seekg(0, std::ios::beg);
  std::vector<char> buffer(size);
  input_file.read(buffer.data(), static_cast<std::streamsize>(size));

  // Serve
  server_.Get(path, [buffer, kChunkSize, params](const httplib::Request& req,
                                                 httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    CheckParams(params, req);
    resp.status = 200;
    resp.set_header("Content-Disposition", "attachment; filename=test.dbz");
    resp.set_content_provider(
        "application/octet-stream",
        [buffer, kChunkSize](const std::size_t offset,
                             httplib::DataSink& sink) {
          if (offset < buffer.size()) {
            sink.write(&buffer[offset],
                       std::min(kChunkSize, buffer.size() - offset));
          } else {
            sink.done();
          }
          return true;
        });
  });
}

void MockHttpServer::CheckParams(
    const std::map<std::string, std::string>& params,
    const httplib::Request& req) {
  for (const auto& param : params) {
    if (!req.has_param(param.first)) {
      std::ostringstream err_msg;
      err_msg << "Missing query param " << param.first;
      std::cerr << err_msg.str() << '\n';
      throw std::runtime_error{err_msg.str()};
    }
    if (req.get_param_value(param.first) != param.second) {
      std::ostringstream err_msg;
      err_msg << "Incorrect query param value for " << param.first
              << ". Expected " << param.second << ", found "
              << req.get_param_value(param.first);
      std::cerr << err_msg.str() << '\n';
      throw std::runtime_error{err_msg.str()};
    }
  }
}
