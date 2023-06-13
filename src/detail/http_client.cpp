#include "databento/detail/http_client.hpp"

#include <chrono>   // seconds
#include <sstream>  // ostringstream

#include "databento/exceptions.hpp"  // HttpResponseError, HttpRequestError, JsonResponseError
#include "databento/log.hpp"      // ILogReceiver, LogLevel
#include "databento/version.hpp"  // DATABENTO_VERSION

using databento::detail::HttpClient;

constexpr std::chrono::seconds kTimeout{100};
const httplib::Headers HttpClient::kHeaders{
    {"accept", "application/json"},
    {"user-agent", "Databento/" DATABENTO_VERSION " C++"},
};

HttpClient::HttpClient(databento::ILogReceiver* log_receiver,
                       const std::string& key, const std::string& gateway)
    : log_receiver_{log_receiver}, client_{gateway} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
  client_.set_read_timeout(kTimeout);
  client_.set_write_timeout(kTimeout);
}

HttpClient::HttpClient(databento::ILogReceiver* log_receiver,
                       const std::string& key, const std::string& gateway,
                       std::uint16_t port)
    : log_receiver_{log_receiver}, client_{gateway, port} {
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
                              const httplib::ContentReceiver& callback) {
  const std::string full_path = httplib::append_query_params(path, params);
  std::string err_body{};
  int err_status{};
  const httplib::Result res = client_.Get(
      full_path,
      [&err_status](const httplib::Response& resp) {
        if (HttpClient::IsErrorStatus(resp.status)) {
          err_status = resp.status;
        }
        return true;
      },
      [&callback, &err_body, &err_status](const char* data,
                                          std::size_t length) {
        // if an error response was received, read all content into err_status
        if (err_status > 0) {
          err_body.append(data, length);
          return true;
        }
        return callback(data, length);
      });
  if (err_status > 0) {
    throw HttpResponseError{path, err_status, std::move(err_body)};
  }
  if (res.error() != httplib::Error::Success &&
      // canceled happens if `callback` returns false, which is based on the
      // user input, and therefor not exceptional
      res.error() != httplib::Error::Canceled) {
    throw HttpRequestError{path, res.error()};
  }
}

nlohmann::json HttpClient::CheckAndParseResponse(const std::string& path,
                                                 httplib::Result&& res) const {
  if (res.error() != httplib::Error::Success) {
    throw HttpRequestError{path, res.error()};
  }
  auto& response = res.value();
  const auto status_code = response.status;
  if (HttpClient::IsErrorStatus(status_code)) {
    throw HttpResponseError{path, status_code, std::move(response.body)};
  }
  CheckWarnings(response);
  try {
    return nlohmann::json::parse(std::move(response.body));
  } catch (const nlohmann::json::parse_error& parse_err) {
    throw JsonResponseError::ParseError(path, parse_err);
  }
}

void HttpClient::CheckWarnings(const httplib::Response& response) const {
  // Returns empty string if not found. `get_header_value` is case insensitive
  const auto raw = response.get_header_value("X-Warning");
  if (!raw.empty()) {
    try {
      const auto json = nlohmann::json::parse(raw);
      if (json.is_array()) {
        for (const auto& warning_json : json.items()) {
          const std::string warning = warning_json.value();
          std::ostringstream msg;
          msg << __PRETTY_FUNCTION__ << " Server " << warning;
          log_receiver_->Receive(LogLevel::Warning, msg.str());
        }
        return;
      }
    } catch (const std::exception&) {
    }
    std::ostringstream msg;
    msg << __PRETTY_FUNCTION__
        << " Failed to parse warnings from HTTP header. Raw contents: " << raw;
    log_receiver_->Receive(LogLevel::Warning, msg.str());
  }
}

bool HttpClient::IsErrorStatus(int status_code) { return status_code >= 400; }
