#include "databento/file_stream.hpp"

#include <ios>  // ios, streamsize
#include <sstream>

#include "databento/exceptions.hpp"

using databento::InFileStream;

InFileStream::InFileStream(const std::filesystem::path& file_path)
    : stream_{file_path, std::ios::binary} {
  if (stream_.fail()) {
    throw InvalidArgumentError{"InFileStream", "file_path",
                               "Non-existent or invalid file at " + file_path.string()};
  }
}

void InFileStream::ReadExact(std::byte* buffer, std::size_t length) {
  const auto size = ReadSome(buffer, length);
  if (size != length) {
    std::ostringstream err_msg;
    err_msg << "Unexpected end of file, expected " << length << " bytes, got " << size;
    throw DbnResponseError{err_msg.str()};
  }
}

std::size_t InFileStream::ReadSome(std::byte* buffer, std::size_t max_length) {
  stream_.read(reinterpret_cast<char*>(buffer),
               static_cast<std::streamsize>(max_length));
  return static_cast<std::size_t>(stream_.gcount());
}

using databento::OutFileStream;

OutFileStream::OutFileStream(const std::filesystem::path& file_path)
    : stream_{file_path, std::ios::binary} {
  if (stream_.fail()) {
    throw InvalidArgumentError{"OutFileStream", "file_path",
                               "Non-existent or invalid file"};
  }
}

void OutFileStream::WriteAll(const std::byte* buffer, std::size_t length) {
  stream_.write(reinterpret_cast<const char*>(buffer),
                static_cast<std::streamsize>(length));
}
