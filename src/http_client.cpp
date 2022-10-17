#include "databento/http_client.hpp"

#include <chrono>  // seconds

#include "databento/exceptions.hpp"  // HttpResponseError, HttpRequestError, JsonResponseError
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
  httplib::Result res = client_.Get(path, params, httplib::Headers{});
  return HttpClient::CheckAndParseResponse(path, std::move(res));
}

nlohmann::json HttpClient::PostJson(const std::string& path,
                                    const httplib::Params& params) {
  // need to fully specify, otherwise sent as application/x-www-form-urlencoded
  const std::string full_path = httplib::append_query_params(path, params);
  httplib::Result res = client_.Post(full_path);
  return HttpClient::CheckAndParseResponse(path, std::move(res));
}

void HttpClient::GetRawStream(const std::string& path,
                              const httplib::Params& params,
                              httplib::ContentReceiver callback) {
  const std::string full_path = httplib::append_query_params(path, params);
  int err_status{};
  httplib::Result res = client_.Get(
      full_path,
      [&err_status](const httplib::Response& resp) {
        // only continue if good response status
        if (HttpClient::IsErrorStatus(resp.status)) {
          // instead of throwing here, store the HTTP status and return false to
          // have the client close the connection
          err_status = resp.status;
          return false;
        }
        return true;
      },
      std::move(callback));
  if (err_status > 0) {
    throw HttpResponseError{path, err_status, ""};
  }
  if (res.error() != httplib::Error::Success) {
    throw HttpRequestError{path, res.error()};
  }
}

nlohmann::json HttpClient::CheckAndParseResponse(const std::string& path,
                                                 httplib::Result&& res) {
  if (res.error() != httplib::Error::Success) {
    throw HttpRequestError{path, res.error()};
  }
  const auto status_code = res.value().status;
  if (HttpClient::IsErrorStatus(status_code)) {
    throw HttpResponseError{path, status_code, std::move(res.value().body)};
  }
  try {
    return nlohmann::json::parse(std::move(res.value().body));
  } catch (const nlohmann::json::parse_error& parse_err) {
    throw JsonResponseError::ParseError(path, parse_err);
  }
}

bool HttpClient::IsErrorStatus(int status_code) { return status_code >= 400; }
