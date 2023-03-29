#include "databento/detail/file_stream.hpp"

#include <ios>  // streamsize
#include <sstream>

#include "databento/exceptions.hpp"

using databento::detail::FileStream;

FileStream::FileStream(const std::string& file_path) : stream_{file_path} {
  if (stream_.fail()) {
    throw InvalidArgumentError{"DbnFileStore", "file_path",
                               "Non-existent or invalid file"};
  }
}

void FileStream::ReadExact(std::uint8_t* buffer, std::size_t length) {
  const auto size = ReadSome(buffer, length);
  if (size != length) {
    std::ostringstream err_msg;
    err_msg << "Unexpected end of file, expected " << length << " bytes, got "
            << size;
    throw DbnResponseError{err_msg.str()};
  }
}

std::size_t FileStream::ReadSome(std::uint8_t* buffer, std::size_t max_length) {
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(max_length));
  return static_cast<std::size_t>(stream_.gcount());
}
