#pragma once

#include <httplib.h>

#include <chrono>   // milliseconds
#include <cstddef>  // byte, size_t

#include "databento/ireadable.hpp"

namespace databento::detail {
// Adapts an httplib StreamHandle to the IReadable interface, allowing
// HTTP response bodies to be read incrementally by DbnDecoder.
class HttpStreamReader : public IReadable {
 public:
  explicit HttpStreamReader(httplib::ClientImpl::StreamHandle handle);

  void ReadExact(std::byte* buffer, std::size_t length) override;
  std::size_t ReadSome(std::byte* buffer, std::size_t max_length) override;
  // timeout is ignored; historical data is always immediately available or EOF
  Result ReadSome(std::byte* buffer, std::size_t max_length,
                  std::chrono::milliseconds timeout) override;

 private:
  httplib::ClientImpl::StreamHandle handle_;
};
}  // namespace databento::detail
