#include "databento/http_client.hpp"

#include <sstream>
#include <stdexcept>

#include "databento/version.hpp"

using databento::HttpClient;

const httplib::Headers HttpClient::kHeaders{
    {"accept", "application/json"},
    {"user-agent", "Databento/" DATABENTO_VERSION " C++"},
};

HttpClient::HttpClient(const std::string& key, const std::string& gateway)
    : client_{gateway} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
}

HttpClient::HttpClient(const std::string& key, const std::string& gateway,
                       std::uint16_t port)
    : client_{gateway, port} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
}

nlohmann::json HttpClient::GetJson(const std::string& path,
                                   const httplib::Params& params) {
  const httplib::Result res = client_.Get(path, params, httplib::Headers{});
  if (res.error() != httplib::Error::Success) {
    std::ostringstream err_msg;
    err_msg << "Request to " << path << " failed with " << res.error();
    throw std::runtime_error{err_msg.str()};
  }
  const auto status_code = res.value().status;
  if (HttpClient::IsErrorStatus(status_code)) {
    std::ostringstream err_msg;
    err_msg << "Received an error response from request to " << path
            << " with status " << status_code << " and body "
            << res.value().body;
    throw std::runtime_error{err_msg.str()};
  }
  return nlohmann::json::parse(res.value().body);
}

void HttpClient::Post() {}

bool HttpClient::IsErrorStatus(int status_code) { return status_code >= 400; }
