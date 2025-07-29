#pragma once

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>          // Error
#include <nlohmann/json.hpp>  // json, parse_error

#include <cstdint>
#include <exception>
#include <string>
#include <string_view>
#include <utility>  // move

namespace databento {
// Base class for all databento client library exceptions.
class Exception : public std::exception {
 public:
  explicit Exception(std::string message) : message_{std::move(message)} {}

  const char* what() const noexcept override { return message_.c_str(); }
  const std::string& Message() const { return message_; }

 private:
  const std::string message_;
};

class HttpRequestError : public Exception {
 public:
  HttpRequestError(std::string request_path, httplib::Error error_code)
      : Exception{BuildMessage(request_path, error_code)},
        request_path_{std::move(request_path)},
        error_code_{error_code} {}

  const std::string& RequestPath() const { return request_path_; }
  httplib::Error ErrorCode() const { return error_code_; }

 private:
  static std::string BuildMessage(std::string_view request_path,
                                  httplib::Error error_code);

  const std::string request_path_;
  const httplib::Error error_code_;
};

// Exception indicating a 4XX or 5XX HTTP status code was received from the
// server.
class HttpResponseError : public Exception {
 public:
  HttpResponseError(std::string request_path, std::int32_t status_code,
                    std::string response_body)
      : Exception{BuildMessage(request_path, status_code, response_body)},
        request_path_{std::move(request_path)},
        status_code_{status_code},
        response_body_{std::move(response_body)} {}

  const std::string& RequestPath() const { return request_path_; }
  std::int32_t StatusCode() const { return status_code_; }
  const std::string& ResponseBody() const { return response_body_; }

 private:
  static std::string BuildMessage(std::string_view request_path,
                                  std::int32_t status_code,
                                  std::string_view response_body);

  const std::string request_path_;
  // int32 is the representation used by httplib
  const std::int32_t status_code_;
  const std::string response_body_;
};

// Exception indicating an issue with the TCP connection.
class TcpError : public Exception {
 public:
  explicit TcpError(int err_num, std::string message)
      : Exception{BuildMessage(err_num, std::move(message))}, err_num_{err_num} {};

  int ErrNum() const { return err_num_; }

 private:
  static std::string BuildMessage(int err_num, std::string message);

  int err_num_;
};

// Exception indicating an argument to a callable is invalid.
class InvalidArgumentError : public Exception {
 public:
  InvalidArgumentError(std::string method_name, std::string param_name,
                       std::string details)
      : Exception{BuildMessage(method_name, param_name, details)},
        method_name_{std::move(method_name)},
        param_name_{std::move(param_name)},
        details_{std::move(details)} {}

  const std::string& MethodName() const { return method_name_; }
  const std::string& ArgumentName() const { return param_name_; }
  const std::string& Details() const { return details_; }

 private:
  static std::string BuildMessage(std::string_view method_name,
                                  std::string_view param_name,
                                  std::string_view details);

  const std::string method_name_;
  const std::string param_name_;
  const std::string details_;
};

// Exception indicating an error parsing a JSON response from the Databento API.
class JsonResponseError : public Exception {
 public:
  static JsonResponseError ParseError(std::string_view path,
                                      const nlohmann::detail::parse_error& parse_error);
  static JsonResponseError MissingKey(std::string_view method_name,
                                      const nlohmann::json& key);
  static JsonResponseError TypeMismatch(std::string_view method_name,
                                        std::string_view expected_type_name,
                                        const nlohmann::json& json);
  static JsonResponseError TypeMismatch(std::string_view method_name,
                                        std::string_view expected_type_name,
                                        const nlohmann::json& key,
                                        const nlohmann::json& value);

 private:
  explicit JsonResponseError(std::string message) : Exception{std::move(message)} {}
};

// Exception indicating an error parsing a DBN response from the Databento API.
class DbnResponseError : public Exception {
 public:
  explicit DbnResponseError(std::string message) : Exception{std::move(message)} {}
};

// Exception indicating something internal to the live API, but unrelated to TCP
// went wrong.
class LiveApiError : public Exception {
 public:
  explicit LiveApiError(std::string message) : Exception{std::move(message)} {}

  static LiveApiError UnexpectedMsg(std::string_view message,
                                    std::string_view response);
};
}  // namespace databento
