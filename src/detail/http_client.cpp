#include "databento/detail/http_client.hpp"

#include <chrono>   // seconds
#include <sstream>  // ostringstream

#include "databento/constants.hpp"  // kUserAgent
#include "databento/exceptions.hpp"  // HttpResponseError, HttpRequestError, JsonResponseError
#include "databento/log.hpp"         // ILogReceiver, LogLevel

using databento::detail::HttpClient;

constexpr std::chrono::seconds kTimeout{100};
const httplib::Headers HttpClient::kHeaders{
    {"accept", "application/json"},
    {"user-agent", kUserAgent},
};

HttpClient::HttpClient(databento::ILogReceiver* log_receiver, const std::string& key,
                       const std::string& gateway)
    : log_receiver_{log_receiver}, client_{gateway} {
  client_.set_default_headers(HttpClient::kHeaders);
  client_.set_basic_auth(key, "");
  client_.set_read_timeout(kTimeout);
  client_.set_write_timeout(kTimeout);
}

HttpClient::HttpClient(databento::ILogReceiver* log_receiver, const std::string& key,
                       const std::string& gateway, std::uint16_t port)
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
                                    const httplib::Params& form_params) {
  // params will be encoded as form data
  httplib::Result res = client_.Post(path, {}, form_params);
  return HttpClient::CheckAndParseResponse(path, std::move(res));
}

void HttpClient::GetRawStream(const std::string& path, const httplib::Params& params,
                              const httplib::ContentReceiver& callback) {
  const std::string full_path = httplib::append_query_params(path, params);
  std::string err_body{};
  int err_status{};
  const httplib::Result res = client_.Get(
      full_path, MakeStreamResponseHandler(err_status),
      [&callback, &err_body, &err_status](const char* data, std::size_t length) {
        // if an error response was received, read all content into
        // err_body
        if (err_status > 0) {
          err_body.append(data, length);
          return true;
        }
        return callback(data, length);
      });
  CheckStatusAndStreamRes(path, err_status, std::move(err_body), res);
}

void HttpClient::PostRawStream(const std::string& path,
                               const httplib::Params& form_params,
                               const httplib::ContentReceiver& callback) {
  std::string err_body{};
  int err_status{};
  httplib::Request req;
  req.method = "POST";
  req.set_header("Content-Type", "application/x-www-form-urlencoded");
  req.path = path;
  req.body = httplib::detail::params_to_query_str(form_params);
  req.response_handler = MakeStreamResponseHandler(err_status);
  req.content_receiver = [&callback, &err_body, &err_status](
                             const char* data, std::size_t length, std::uint64_t,
                             std::uint64_t) {
    // if an error response was received, read all content into
    // err_body
    if (err_status > 0) {
      err_body.append(data, length);
      return true;
    }
    return callback(data, length);
  };
  // NOLINTNEXTLINE(clang-analyzer-unix.BlockInCriticalSection): dependency code
  const httplib::Result res = client_.send(req);
  CheckStatusAndStreamRes(path, err_status, std::move(err_body), res);
}

httplib::ResponseHandler HttpClient::MakeStreamResponseHandler(int& out_status) {
  return [this, &out_status](const httplib::Response& resp) {
    if (HttpClient::IsErrorStatus(resp.status)) {
      out_status = resp.status;
    }
    CheckWarnings(resp);
    return true;
  };
}

void HttpClient::CheckStatusAndStreamRes(const std::string& path, int status_code,
                                         std::string&& err_body,
                                         const httplib::Result& res) {
  if (status_code > 0) {
    throw HttpResponseError{path, status_code, std::move(err_body)};
  }
  if (res.error() != httplib::Error::Success &&
      // canceled happens if `callback` returns false, which is based on the
      // user input, and therefore not exceptional
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
          msg << "[HttpClient::CheckWarnings] Server " << warning;
          log_receiver_->Receive(LogLevel::Warning, msg.str());
        }
        return;
      }
    } catch (const std::exception& exc) {
      std::ostringstream msg;
      msg << "[HttpClient::CheckWarnings] Failed to parse warnings from HTTP "
             "header: "
          << exc.what() << ". Raw contents: " << raw;
      log_receiver_->Receive(LogLevel::Warning, msg.str());
      return;
    }
    std::ostringstream msg;
    msg << "[HttpClient::CheckWarnings] Failed to parse warnings from HTTP "
           "header. Raw contents: "
        << raw;
    log_receiver_->Receive(LogLevel::Warning, msg.str());
  }
}

bool HttpClient::IsErrorStatus(int status_code) { return status_code >= 400; }
