#include "databento/http_client.hpp"

#include <chrono>  // seconds
#include <sstream>
#include <stdexcept>

#include "databento/version.hpp"  // DATABENTO_VERSION

using databento::HttpClient;

constexpr std::chrono::seconds kTimeout{100};
const httplib::Headers HttpClient::kHeaders{
    {"accept", "application/json"},
    {"user-agent", "Databento/" DATABENTO_VERSION " C++"},
};

HttpClient::HttpClient(const std::string& key, const std::string& gateway)
    : client_{gateway} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
  client_.set_read_timeout(kTimeout);
  client_.set_write_timeout(kTimeout);
}

HttpClient::HttpClient(const std::string& key, const std::string& gateway,
                       std::uint16_t port)
    : client_{gateway, port} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
  client_.set_read_timeout(kTimeout);
  client_.set_write_timeout(kTimeout);
}

nlohmann::json HttpClient::GetJson(const std::string& path,
                                   const httplib::Params& params) {
  const httplib::Result res = client_.Get(path, params, httplib::Headers{});
  return HttpClient::CheckAndParseResponse(path, res);
}

nlohmann::json HttpClient::PostJson(const std::string& path,
                                    const httplib::Params& params) {
  // need to fully specify, otherwise sent as application/x-www-form-urlencoded
  const std::string full_path = httplib::append_query_params(path, params);
  const httplib::Result res = client_.Post(full_path);
  return HttpClient::CheckAndParseResponse(path, res);
}

void HttpClient::GetRawStream(const std::string& path,
                              const httplib::Params& params,
                              httplib::ContentReceiver callback) {
  const std::string full_path = httplib::append_query_params(path, params);
  const httplib::Result res = client_.Get(
      full_path,
      [](const httplib::Response& resp) {
        // only continue if good response status
        return !HttpClient::IsErrorStatus(resp.status);
      },
      std::move(callback));
  client_.socket();
}

nlohmann::json HttpClient::CheckAndParseResponse(const std::string& path,
                                                 const httplib::Result& res) {
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

bool HttpClient::IsErrorStatus(int status_code) { return status_code >= 400; }
