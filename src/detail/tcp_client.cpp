#include "databento/detail/tcp_client.hpp"

#include <sys/socket.h>  // MSG_WAITALL, recv
#include <unistd.h>      // read, ssize_t

#include <cerrno>  // errno
#include <sstream>

#include "databento/exceptions.hpp"  // TcpError

using databento::detail::TcpClient;

TcpClient::TcpClient(const std::string& gateway, std::uint16_t port)
    : socket_{gateway, port} {}

void TcpClient::WriteAll(const char* buffer, std::size_t size) {
  do {
    const ::ssize_t res = ::write(socket_.Get(), buffer, size);
    if (res < 0) {
      throw TcpError{errno, "Error writing to socket"};
    }
    size -= static_cast<std::size_t>(res);
    buffer += res;
  } while (size > 0);
}

std::size_t TcpClient::Read(char* buffer, std::size_t max_size) {
  const ::ssize_t res = ::read(socket_.Get(), buffer, max_size);
  if (res < 0) {
    throw TcpError{errno, "Error reading from socket"};
  }
  return static_cast<std::size_t>(res);
}

void TcpClient::ReadExact(char* buffer, std::size_t size) {
  // MSG_WAITALL to wait for size to be returned
  const auto res = ::recv(socket_.Get(), buffer, size, MSG_WAITALL);
  if (res < 0) {
    throw TcpError{errno, "Error reading from socket"};
  }
  // In certain circumstances recv can still return a partial read
  if (static_cast<std::size_t>(res) != size) {
    std::ostringstream err_msg;
    err_msg << "Insufficient data received; wanted " << size << " bytes, got "
            << res << " bytes";
    throw TcpError{{}, err_msg.str()};
    ;
  }
}
