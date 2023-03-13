#include "databento/detail/file_stream.hpp"

#include <ios>  // streamsize

#include "databento/exceptions.hpp"

using databento::detail::FileStream;

FileStream::FileStream(const std::string& file_path) : stream_{file_path} {
  if (stream_.fail()) {
    throw InvalidArgumentError{"DbnFileStore", "file_path",
                               "Non-existent or invalid file"};
  }
}

void FileStream::ReadExact(std::uint8_t* buffer, std::size_t length) {
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(length));
}

std::size_t FileStream::ReadSome(std::uint8_t* buffer, std::size_t max_length) {
  return static_cast<std::size_t>(
      stream_.readsome(reinterpret_cast<char*>(buffer),
                       static_cast<std::streamsize>(max_length)));
}
