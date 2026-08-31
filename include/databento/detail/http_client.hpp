#pragma once

// Ensure proper compilation when used outside of CMake, such
// as when installed at the system level
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <nlohmann/json.hpp>

#include <cstddef>  // size_t
#include <cstdint>
#include <functional>
#include <map>     // multimap
#include <memory>  // unique_ptr
#include <optional>
#include <string>

// Forward declare
namespace httplib {
class Client;
}  // namespace httplib

namespace databento {
// A callback for customizing the underlying httplib::Client.
using HttpClientCallback = std::function<void(httplib::Client&)>;

// Forward declare
class ILogReceiver;
class IReadable;
namespace detail {
// The underlying container of the httplib type alias for these changed in httplib
// 0.52.0
using HttpParams = std::multimap<std::string, std::string>;
using HttpHeaders = std::multimap<std::string, std::string>;
using HttpContentReceiver = std::function<bool(const char* data, std::size_t length)>;

class HttpClient {
 public:
  HttpClient(ILogReceiver* log_receiver, const std::string& key,
             const std::string& gateway, std::optional<HttpClientCallback> callback);
  HttpClient(ILogReceiver* log_receiver, const std::string& key,
             const std::string& gateway, std::uint16_t port,
             std::optional<HttpClientCallback> callback);
  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;
  HttpClient(HttpClient&&) noexcept;
  HttpClient& operator=(HttpClient&&) noexcept;
  ~HttpClient();

  nlohmann::json GetJson(const std::string& path, const HttpParams& params);
  nlohmann::json PostJson(const std::string& path, const HttpParams& form_params);
  void GetRawStream(const std::string& path, const HttpHeaders& headers,
                    const HttpContentReceiver& callback);
  void PostRawStream(const std::string& path, const HttpParams& form_params,
                     const HttpContentReceiver& callback);
  std::unique_ptr<IReadable> OpenPostStream(const std::string& path,
                                            const HttpParams& form_params);

 private:
  ILogReceiver* log_receiver_;
  // unique_ptr so this header only needs an incomplete httplib::Client
  std::unique_ptr<httplib::Client> client_;
};
}  // namespace detail
}  // namespace databento
