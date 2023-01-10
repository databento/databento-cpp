#pragma once

// Ensure proper compilation when used outside of CMake, such
// as when installed at the system level
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

namespace databento {
namespace detail {
class HttpClient {
 public:
  HttpClient(const std::string& key, const std::string& gateway);
  HttpClient(const std::string& key, const std::string& gateway,
             std::uint16_t port);

  nlohmann::json GetJson(const std::string& path,
                         const httplib::Params& params);
  nlohmann::json PostJson(const std::string& path,
                          const httplib::Params& params);
  void GetRawStream(const std::string& path, const httplib::Params& params,
                    const httplib::ContentReceiver& callback);

 private:
  static nlohmann::json CheckAndParseResponse(const std::string& path,
                                              httplib::Result&& res);
  static bool IsErrorStatus(int status_code);

  static const httplib::Headers kHeaders;

  httplib::Client client_;
};
}  // namespace detail
}  // namespace databento
