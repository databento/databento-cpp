#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

// ignore warnings from httplib
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <httplib.h>
#pragma GCC diagnostic pop

namespace databento {
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
                    httplib::ContentReceiver callback);

 private:
  static nlohmann::json CheckAndParseResponse(const std::string& path,
                                              httplib::Result&& res);
  static bool IsErrorStatus(int status_code);

  static const httplib::Headers kHeaders;

  httplib::Client client_;
};
}  // namespace databento
