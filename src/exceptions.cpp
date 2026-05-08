#include "databento/exceptions.hpp"

#ifdef _WIN32
#include <winbase.h>  // FormatMessage, FORMAT_MESSAGE_FROM_SYSTEM
#else
#include <cstring>  // strerror
#endif
#include <nlohmann/json.hpp>

#include <exception>  // exception
#include <optional>
#include <sstream>  // ostringstream
#include <utility>  // move

#include "databento/detail/json_helpers.hpp"

using databento::HttpRequestError;

std::string HttpRequestError::BuildMessage(std::string_view request_path,
                                           httplib::Error error_code) {
  std::ostringstream err_msg;
  err_msg << "Request to " << request_path << " failed with " << error_code;
  return err_msg.str();
}

using databento::TcpError;

std::string TcpError::BuildMessage(int err_num, std::string message) {
#ifdef _WIN32
  char errbuf[300];
  ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, {}, err_num, {}, errbuf, sizeof(errbuf),
                  {});
  return std::move(message) + ": " + errbuf;
#else
  return std::move(message) + ": " + std::strerror(err_num);
#endif
}

using databento::HttpResponseError;

HttpResponseError::HttpResponseError(const std::string& request_path,
                                     std::int32_t status_code,
                                     const std::string& response_body)
    : HttpResponseError{request_path, status_code, response_body,
                        ParseDetail(request_path, response_body)} {}

HttpResponseError::HttpResponseError(std::string request_path, std::int32_t status_code,
                                     const std::string& response_body,
                                     ParsedDetail parsed)
    : Exception{
          BuildMessage(request_path, status_code, response_body, parsed.case_str)},
      request_path_{std::move(request_path)},
      status_code_{status_code},
      case_{std::move(parsed.case_str)},
      detail_message_{std::move(parsed.detail_message)},
      docs_url_{std::move(parsed.docs_url)} {}

HttpResponseError::ParsedDetail HttpResponseError::ParseDetail(
    std::string_view request_path, const std::string& response_body) {
  // Best-effort parse of the historical API rich error format
  // {"detail": {"case": "...", "message": "...", "docs": "...", "payload": {...}}}
  // Falls back to {"detail": "..."} or leaves all fields empty for non-JSON bodies
  ParsedDetail out;
  try {
    const auto json = nlohmann::json::parse(response_body);
    const auto& detail = detail::CheckedAt(request_path, json, "detail");
    if (detail.is_string()) {
      out.detail_message = detail.get<std::string>();
    } else if (detail.is_object()) {
      out.case_str =
          detail::ParseAt<std::optional<std::string>>(request_path, detail, "case");
      out.detail_message =
          detail::ParseAt<std::optional<std::string>>(request_path, detail, "message");
      out.docs_url =
          detail::ParseAt<std::optional<std::string>>(request_path, detail, "docs");
    }
  } catch (const std::exception&) {  // NOLINT(bugprone-empty-catch)
    // Body wasn't JSON, didn't have a `detail` key, or had unexpected types; the
    // raw body is still embedded in `what()`, so leave the parsed fields empty.
  }
  return out;
}

std::string HttpResponseError::BuildMessage(
    std::string_view request_path, std::int32_t status_code,
    std::string_view response_body, const std::optional<std::string>& case_str) {
  std::ostringstream err_msg;
  err_msg << "Received an error response from request to " << request_path
          << " with status " << status_code << " and body '" << response_body << '\'';
  if (case_str) {
    err_msg << " (case: " << *case_str << ')';
  }
  return err_msg.str();
}

using databento::InvalidArgumentError;

std::string InvalidArgumentError::BuildMessage(std::string_view method_name,
                                               std::string_view param_name,
                                               std::string_view details) {
  std::ostringstream err_msg;
  err_msg << "Invalid argument '" << param_name << "' to " << method_name << ' '
          << details;
  return err_msg.str();
}

using databento::JsonResponseError;

JsonResponseError JsonResponseError::ParseError(
    std::string_view method_name, const nlohmann::json::parse_error& parse_error) {
  std::ostringstream err_msg;
  err_msg << "Error parsing JSON response to " << method_name << ' '
          << parse_error.what();
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::MissingKey(std::string_view path,
                                                const nlohmann::json& key) {
  std::ostringstream err_msg;
  err_msg << "Missing key '" << key << "' in response for " << path;
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::TypeMismatch(std::string_view method_name,
                                                  std::string_view expected_type_name,
                                                  const nlohmann::json& json) {
  std::ostringstream err_msg;
  err_msg << "Expected JSON " << expected_type_name << " response for " << method_name
          << ", got " << json.type_name();
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::TypeMismatch(std::string_view method_name,
                                                  std::string_view expected_type_name,
                                                  const nlohmann::json& key,
                                                  const nlohmann::json& value) {
  std::ostringstream err_msg;
  err_msg << "Expected " << expected_type_name << " values in JSON response for "
          << method_name << ", got " << value.type_name() << " " << value << " for key "
          << key;
  return JsonResponseError{err_msg.str()};
}

using databento::HeartbeatTimeoutError;

std::string HeartbeatTimeoutError::BuildMessage(std::chrono::seconds elapsed) {
  std::ostringstream err_msg;
  err_msg << "Heartbeat timeout: no data received for " << elapsed.count()
          << " seconds";
  return err_msg.str();
}

using databento::LiveApiError;

LiveApiError LiveApiError::UnexpectedMsg(std::string_view message,
                                         std::string_view response) {
  std::ostringstream err_msg;
  err_msg << message << " with response '" << response << '\'';
  return LiveApiError{err_msg.str()};
}
