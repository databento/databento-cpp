#include "databento/dbz_parser.hpp"

#include <zstd.h>

#include <algorithm>  // copy
#include <cstring>    // strncmp
#include <vector>

#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/record.hpp"

using databento::DbzParser;

namespace {
constexpr std::uint32_t kZstdMagicLowerBound = 0x184D2A50;
constexpr std::uint32_t kZstdMagicUpperBound = 0x184D2A5F;
constexpr std::size_t kFixedMetadataLen = 96;
constexpr std::uint8_t kSchemaVersion = 1;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kReservedLen = 39;
constexpr std::size_t kSymbolCstrLen = 22;

template <typename T>
T Consume(std::vector<std::uint8_t>::const_iterator& byte_it) {
  const auto res = *reinterpret_cast<const T*>(&*byte_it);
  byte_it += sizeof(T);
  return res;
}

template <>
std::uint8_t Consume(std::vector<std::uint8_t>::const_iterator& byte_it) {
  const auto res = *byte_it;
  byte_it += 1;
  return res;
}

const char* Consume(std::vector<std::uint8_t>::const_iterator& byte_it,
                    const std::ptrdiff_t num_bytes) {
  const auto* pos = &*byte_it;
  byte_it += num_bytes;
  return reinterpret_cast<const char*>(pos);
}
}  // namespace

template <typename Input>
DbzParser<Input>::~DbzParser() {
  // nullptr ok
  ZSTD_freeDStream(z_dstream_);
}

template <typename Input>
databento::Metadata DbzParser<Input>::ParseMetadata() {
  std::uint32_t magic{};
  input_.ReadExact(reinterpret_cast<std::uint8_t*>(&magic),
                   sizeof(std::uint32_t));
  if (magic < ::kZstdMagicLowerBound || magic > ::kZstdMagicUpperBound) {
    throw DbzResponseError{"Invalid metadata: no zstd magic number"};
  }
  std::uint32_t frame_size{};
  input_.ReadExact(reinterpret_cast<std::uint8_t*>(&frame_size),
                   sizeof(std::uint32_t));
  if (frame_size < ::kFixedMetadataLen) {
    throw DbzResponseError{
        "Frame length cannot be shorter than the fixed metadata size"};
  }
  std::vector<std::uint8_t> metadata_buffer(frame_size);
  Metadata res;
  input_.ReadExact(metadata_buffer.data(), frame_size);
  auto metadata_it = metadata_buffer.cbegin();

  if (std::strncmp(Consume(metadata_it, 3), "DBZ", 3) != 0) {
    throw DbzResponseError{"Invalid version string"};
  }
  res.version = Consume<std::uint8_t>(metadata_it);
  if (res.version > ::kSchemaVersion) {
    throw DbzResponseError{"Can't read newer version of DBZ"};
  }
  res.dataset = std::string{Consume(metadata_it, kDatasetCstrLen)};
  res.schema = static_cast<Schema>(Consume<std::uint16_t>(metadata_it));
  res.start =
      UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(metadata_it)}};
  res.end =
      UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(metadata_it)}};
  res.limit = Consume<std::uint64_t>(metadata_it);
  res.record_count = Consume<std::uint64_t>(metadata_it);
  res.compression =
      static_cast<Compression>(Consume<std::uint8_t>(metadata_it));
  res.stype_in = static_cast<SType>(Consume<std::uint8_t>(metadata_it));
  res.stype_out = static_cast<SType>(Consume<std::uint8_t>(metadata_it));
  // skip reserved
  metadata_it += ::kReservedLen;

  // Decompress variable-length metadata
  const auto compressed_size =
      static_cast<std::size_t>(metadata_buffer.cend() - metadata_it);
  const std::size_t buffer_size = compressed_size * 3;  //  3x is arbitrary
  std::vector<std::uint8_t> var_buffer(buffer_size);
  const std::size_t actual_size = ::ZSTD_decompress(
      var_buffer.data(), buffer_size, &*metadata_it, compressed_size);
  if (::ZSTD_isError(actual_size) == 1) {
    throw DbzResponseError{
        "Error decompressing zstd-compressed variable-length metadata"};
  }
  auto var_buffer_it = var_buffer.cbegin();
  const auto schema_definition_length = Consume<std::uint32_t>(var_buffer_it);
  if (schema_definition_length != 0) {
    throw DbzResponseError{
        "This version of dbz can't parse schema definitions"};
  }
  res.symbols =
      DbzParser::ParseRepeatedSymbol(var_buffer_it, var_buffer.cend());
  res.partial =
      DbzParser::ParseRepeatedSymbol(var_buffer_it, var_buffer.cend());
  res.not_found =
      DbzParser::ParseRepeatedSymbol(var_buffer_it, var_buffer.cend());
  res.mappings =
      DbzParser::ParseSymbolMappings(var_buffer_it, var_buffer.cend());

  // Change internal state based on metadata in preparation for parsing
  // records
  rtype_ = Record::TypeIdFromSchema(res.schema);
  z_dstream_ = ::ZSTD_createDStream();
  read_suggestion_ = ::ZSTD_initDStream(z_dstream_);
  in_buffer_ = std::vector<std::uint8_t>(read_suggestion_);
  // set pos = size so first ParseRecord reads from input_
  z_in_buffer_ = {in_buffer_.data(), in_buffer_.size(), in_buffer_.size()};
  out_buffer_ = std::vector<std::uint8_t>(Record::SizeOfType(rtype_));
  z_out_buffer_ = {out_buffer_.data(), out_buffer_.size(), 0};

  return res;
}

// assumes ParseMetadata has been called
template <typename Input>
databento::Record DbzParser<Input>::ParseRecord() {
  z_out_buffer_.pos = 0;
  do {
    // std::cout << "ReadSuggestion: " << read_suggestion_ << std::endl;
    // std::cout << "Out pos: " << z_out_buffer_.pos
    //           << " Size: " << z_out_buffer_.size << std::endl;
    // std::cout << "In pos: " << z_in_buffer_.pos
    //           << " Size: " << z_in_buffer_.size << std::endl;

    // shift values in vector. A no-op if z_in_buffer_.pos == z_in_buffer_.size
    const std::size_t unread_size = z_in_buffer_.size - z_in_buffer_.pos;
    if (unread_size > 0) {
      std::copy(
          in_buffer_.cbegin() + static_cast<std::ptrdiff_t>(z_in_buffer_.pos),
          in_buffer_.cend(), in_buffer_.begin());
    }
    const std::size_t new_size = unread_size + read_suggestion_;
    if (new_size != in_buffer_.size()) {
      in_buffer_.resize(new_size);
      z_in_buffer_.src = in_buffer_.data();
    }
    // z_in_buffer_.size <= in_buffer.size()
    z_in_buffer_.size = unread_size + input_.ReadSome(&in_buffer_[unread_size],
                                                      read_suggestion_);
    z_in_buffer_.pos = 0;

    read_suggestion_ =
        ::ZSTD_decompressStream(z_dstream_, &z_out_buffer_, &z_in_buffer_);
    if (::ZSTD_isError(read_suggestion_)) {
      throw DbzResponseError{std::string{"zstd error decompressing record: "} +
                             ZSTD_getErrorName(read_suggestion_)};
    }
    if (read_suggestion_ == 0) {
      break;
    }
  } while (z_out_buffer_.pos != z_out_buffer_.size);
  return Record{reinterpret_cast<RecordHeader*>(out_buffer_.data())};
}

template <typename Input>
std::string DbzParser<Input>::ParseSymbol(
    std::vector<std::uint8_t>::const_iterator& buffer_it) {
  return std::string{Consume(buffer_it, kSymbolCstrLen)};
}

template <typename Input>
std::vector<std::string> DbzParser<Input>::ParseRepeatedSymbol(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  if (buffer_it + sizeof(std::uint32_t) > buffer_end_it) {
    throw DbzResponseError{"Unexpected end of metadata buffer"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  if (buffer_it + static_cast<std::int64_t>(count * ::kSymbolCstrLen) >
      buffer_end_it) {
    throw DbzResponseError{"Unexpected end of metadata buffer"};
  }
  std::vector<std::string> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(ParseSymbol(buffer_it));
  }
  return res;
}

template <typename Input>
std::vector<databento::SymbolMapping> DbzParser<Input>::ParseSymbolMappings(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  if (buffer_it + sizeof(std::uint32_t) > buffer_end_it) {
    throw DbzResponseError{
        "Unexpected end of metadata buffer while parsing mappings"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  std::vector<SymbolMapping> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DbzParser::ParseSymbolMapping(buffer_it, buffer_end_it));
  }
  return res;
}

template <typename Input>
databento::SymbolMapping DbzParser<Input>::ParseSymbolMapping(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  constexpr std::size_t kMinSymbolMappingEncodedLen =
      kSymbolCstrLen + sizeof(std::uint32_t);
  constexpr std::size_t kMappingIntervalEncodedLen =
      sizeof(std::uint32_t) * 2 + kSymbolCstrLen;

  if (buffer_it + kMinSymbolMappingEncodedLen > buffer_end_it) {
    throw DbzResponseError{
        "Unexpected end of metadata buffer while parsing symbol mapping"};
  }
  SymbolMapping res;
  res.native = ParseSymbol(buffer_it);
  const auto interval_count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  const auto read_size =
      static_cast<std::ptrdiff_t>(interval_count * kMappingIntervalEncodedLen);
  if (buffer_it + read_size > buffer_end_it) {
    throw DbzResponseError{
        "Symbol mapping interval_count doesn't match size of buffer"};
  }
  res.intervals.reserve(interval_count);
  for (std::size_t i = 0; i < interval_count; ++i) {
    MappingInterval interval;
    interval.start_date = Consume<std::uint32_t>(buffer_it);
    interval.end_date = Consume<std::uint32_t>(buffer_it);
    interval.symbol = ParseSymbol(buffer_it);
    res.intervals.emplace_back(std::move(interval));
  }
  return res;
}

// explicit template instantiation
namespace databento {
template class DbzParser<detail::FileStream>;
template class DbzParser<detail::SharedChannel>;
}  // namespace databento
