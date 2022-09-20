#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

// ignore warnings from httplib
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#include <httplib.h>
#pragma clang diagnostic pop

namespace databento {
class HttpClient {
  static const httplib::Headers kHeaders;

  httplib::Client client_;

 public:
  HttpClient(const std::string& key, const std::string& gateway);
  HttpClient(const std::string& key, const std::string& gateway,
             std::uint16_t port);

  nlohmann::json GetJson(const std::string& path,
                         const httplib::Params& params);
  void Post();

 private:
  static bool IsErrorStatus(int status_code);
};
}  // namespace databento