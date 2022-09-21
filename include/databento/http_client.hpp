#pragma once

namespace databento {
class HttpClient {
 public:
  HttpClient();
  HttpClient(const HttpClient&) = delete;
  HttpClient operator=(const HttpClient&) = delete;
  HttpClient(HttpClient&& other) noexcept;
  HttpClient& operator=(HttpClient&& other) noexcept;
  ~HttpClient();
};
}