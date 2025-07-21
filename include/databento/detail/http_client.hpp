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
class ILogReceiver;
namespace detail {
class HttpClient {
 public:
  HttpClient(ILogReceiver* log_receiver, const std::string& key,
             const std::string& gateway);
  HttpClient(ILogReceiver* log_receiver, const std::string& key,
             const std::string& gateway, std::uint16_t port);

  nlohmann::json GetJson(const std::string& path, const httplib::Params& params);
  nlohmann::json PostJson(const std::string& path, const httplib::Params& form_params);
  void GetRawStream(const std::string& path, const httplib::Params& params,
                    const httplib::ContentReceiver& callback);
  void PostRawStream(const std::string& path, const httplib::Params& form_params,
                     const httplib::ContentReceiver& callback);

 private:
  static bool IsErrorStatus(int status_code);
  static void CheckStatusAndStreamRes(const std::string& path, int status_code,
                                      std::string&& err_body,
                                      const httplib::Result& res);

  httplib::ResponseHandler MakeStreamResponseHandler(int& out_status);
  nlohmann::json CheckAndParseResponse(const std::string& path,
                                       httplib::Result&& res) const;
  void CheckWarnings(const httplib::Response& response) const;

  static const httplib::Headers kHeaders;

  ILogReceiver* log_receiver_;
  httplib::Client client_;
};
}  // namespace detail
}  // namespace databento
