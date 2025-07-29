#include "databento/exceptions.hpp"

#ifdef _WIN32
#include <winbase.h>  // FormatMessage, FORMAT_MESSAGE_FROM_SYSTEM
#else
#include <cstring>  // strerror
#endif
#include <sstream>  // ostringstream
#include <utility>  // move

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

std::string HttpResponseError::BuildMessage(std::string_view request_path,
                                            std::int32_t status_code,
                                            std::string_view response_body) {
  std::ostringstream err_msg;
  err_msg << "Received an error response from request to " << request_path
          << " with status " << status_code << " and body '" << response_body << '\'';
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

using databento::LiveApiError;

LiveApiError LiveApiError::UnexpectedMsg(std::string_view message,
                                         std::string_view response) {
  std::ostringstream err_msg;
  err_msg << message << " with response '" << response << '\'';
  return LiveApiError{err_msg.str()};
}
