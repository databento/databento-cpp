#pragma once

#include <functional>  // function
#include <string>
#include <vector>

#include "databento/detail/scoped_fd.hpp"      // ScopedFd
#include "databento/detail/scoped_thread.hpp"  // ScopedThread
#include "databento/enums.hpp"                 // Schema, SType

namespace databento {
namespace test {
namespace mock {
class MockLsgServer {
 public:
  explicit MockLsgServer(std::function<void(MockLsgServer&)> serve_fn);

  std::uint16_t Port() const { return port_; }

  void Accept();
  std::string Receive();
  void Send(const std::string& msg);
  template <typename Rec>
  void SendRecord(Rec rec) {
    std::string rec_str{reinterpret_cast<const char*>(&rec), sizeof(rec)};
    Send(rec_str);
  }
  void Authenticate();
  void Subscribe(const std::string& dataset,
                 const std::vector<std::string>& symbols, Schema schema,
                 SType stype);
  void Start();

 private:
  int InitSocketAndSetPort();

  std::uint16_t port_{};
  detail::ScopedFd socket_{};
  detail::ScopedFd conn_fd_{};
  detail::ScopedThread thread_;
};
}  // namespace mock
}  // namespace test
}  // namespace databento
