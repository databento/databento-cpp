#include "databento/exceptions.hpp"

#include <sstream>  // ostringstream

using databento::HttpRequestError;

std::string HttpRequestError::BuildMessage(const std::string& request_path,
                                           httplib::Error error_code) {
  std::ostringstream err_msg;
  err_msg << "Request to " << request_path << " failed with " << error_code;
  return err_msg.str();
}

using databento::HttpResponseError;

std::string HttpResponseError::BuildMessage(const std::string& request_path,
                                            std::int32_t status_code,
                                            const std::string& response_body) {
  std::ostringstream err_msg;
  err_msg << "Received an error response from request to " << request_path
          << " with status " << status_code << " and body '" << response_body
          << '\'';
  return err_msg.str();
}

using databento::InvalidArgumentError;

std::string InvalidArgumentError::BuildMessage(const std::string& method_name,
                                               const std::string& param_name,
                                               const std::string& details) {
  std::ostringstream err_msg;
  err_msg << "Invalid argument '" << param_name << "' to " << method_name << ' '
          << details;
  return err_msg.str();
}

using databento::JsonResponseError;

JsonResponseError JsonResponseError::ParseError(
    const std::string& method_name,
    const nlohmann::json::parse_error& parse_error) {
  std::ostringstream err_msg;
  err_msg << "Error parsing JSON response to " << method_name << ' '
          << parse_error.what();
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::MissingKey(const std::string& path,
                                                const nlohmann::json& key) {
  std::ostringstream err_msg;
  err_msg << "Missing key '" << key << "' in response for " << path;
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::TypeMismatch(
    const std::string& method_name, const std::string& expected_type_name,
    const nlohmann::json& json) {
  std::ostringstream err_msg;
  err_msg << "Expected JSON " << expected_type_name << " response for "
          << method_name << ", got " << json.type_name();
  return JsonResponseError{err_msg.str()};
}

JsonResponseError JsonResponseError::TypeMismatch(
    const std::string& method_name, const std::string& expected_type_name,
    const nlohmann::json& key, const nlohmann::json& value) {
  std::ostringstream err_msg;
  err_msg << "Expected " << expected_type_name
          << " values in JSON response for " << method_name << ", got "
          << value.type_name() << " " << value << " for key " << key;
  return JsonResponseError{err_msg.str()};
}
