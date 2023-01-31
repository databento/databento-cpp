#include "databento/dbn_decoder.hpp"

#include <algorithm>  // copy
#include <cstring>    // strncmp
#include <vector>

#include "databento/datetime.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/record.hpp"

using databento::DbnDecoder;

namespace {
constexpr std::size_t kMagicSize = 4;
constexpr std::uint32_t kZstdMagicNumber = 0xFD2FB528;
constexpr auto kDbnPrefix = "DBN";
constexpr std::size_t kFixedMetadataLen = 100;
constexpr std::uint8_t kSchemaVersion = 1;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kReservedLen = 48;
constexpr std::size_t kSymbolCstrLen = 22;
constexpr std::size_t kBufferCapacity = 8UL * 1024;

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

DbnDecoder::DbnDecoder(detail::SharedChannel channel)
    : DbnDecoder(std::unique_ptr<IReadable>{
          new detail::SharedChannel{std::move(channel)}}) {}

DbnDecoder::DbnDecoder(detail::FileStream file_stream)
    : DbnDecoder(std::unique_ptr<IReadable>{
          new detail::FileStream{std::move(file_stream)}}) {}

DbnDecoder::DbnDecoder(std::unique_ptr<IReadable> input)
    : input_{std::move(input)} {
  buffer_.reserve(kBufferCapacity);
  if (DetectCompression()) {
    input_ = std::unique_ptr<detail::ZstdStream>(
        new detail::ZstdStream(std::move(input_), std::move(buffer_)));
    // Reinitialize buffer and get it into the same state as uncompressed input
    buffer_ = std::vector<std::uint8_t>(kMagicSize);
    input_->ReadExact(buffer_.data(), kMagicSize);
    auto buffer_it = buffer_.cbegin();
    if (std::strncmp(Consume(buffer_it, 3), kDbnPrefix, 3) != 0) {
      throw DbnResponseError{"Found Zstd input, but not DBN prefix"};
    }
    buffer_.resize(0);
  }
}

databento::Metadata DbnDecoder::ParseMetadata() {
  Metadata res;
  // already read first 4 bytes
  res.version = buffer_[3];
  if (res.version > kSchemaVersion) {
    throw DbnResponseError{
        "Can't decode newer version of DBN. Decoder version is " +
        std::to_string(kSchemaVersion) + ", input version is " +
        std::to_string(res.version)};
  }
  input_->ReadExact(buffer_.data(), 4);
  auto buffer_it = buffer_.cbegin();
  const auto length = Consume<std::uint32_t>(buffer_it);
  if (length < ::kFixedMetadataLen) {
    throw DbnResponseError{
        "Frame length cannot be shorter than the fixed metadata size"};
  }
  buffer_.resize(length);
  // iterator invalidated
  buffer_it = buffer_.cbegin();
  input_->ReadExact(buffer_.data(), length);
  res.dataset = std::string{Consume(buffer_it, kDatasetCstrLen)};
  res.schema = static_cast<Schema>(Consume<std::uint16_t>(buffer_it));
  res.start =
      UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer_it)}};
  res.end =
      UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer_it)}};
  res.limit = Consume<std::uint64_t>(buffer_it);
  res.record_count = Consume<std::uint64_t>(buffer_it);
  res.stype_in = static_cast<SType>(Consume<std::uint8_t>(buffer_it));
  res.stype_out = static_cast<SType>(Consume<std::uint8_t>(buffer_it));
  // skip reserved
  buffer_it += ::kReservedLen;

  const auto schema_definition_length = Consume<std::uint32_t>(buffer_it);
  if (schema_definition_length != 0) {
    throw DbnResponseError{
        "This version of dbn can't parse schema definitions"};
  }
  res.symbols = DbnDecoder::ParseRepeatedSymbol(buffer_it, buffer_.cend());
  res.partial = DbnDecoder::ParseRepeatedSymbol(buffer_it, buffer_.cend());
  res.not_found = DbnDecoder::ParseRepeatedSymbol(buffer_it, buffer_.cend());
  res.mappings = DbnDecoder::ParseSymbolMappings(buffer_it, buffer_.cend());
  buffer_idx_ = buffer_.size();

  return res;
}

// assumes ParseMetadata has been called
databento::Record DbnDecoder::ParseRecord() {
  // need some unread unread_bytes
  const auto unread_bytes = buffer_.size() - buffer_idx_;
  if (unread_bytes == 0) {
    if (FillBuffer() == 0) {
      throw DbnResponseError{"Reached end of DBN stream"};
    }
  }
  // check length
  while (buffer_.size() < BufferRecordHeader()->Size()) {
    if (FillBuffer() == 0) {
      throw DbnResponseError{"Reached end of DBN stream"};
    }
  }
  Record res{BufferRecordHeader()};
  buffer_idx_ += BufferRecordHeader()->Size();
  return res;
}

size_t DbnDecoder::FillBuffer() {
  // Shift data forward
  std::copy(buffer_.cbegin() + static_cast<std::ptrdiff_t>(buffer_idx_),
            buffer_.cend(), buffer_.begin());
  const auto unread_size = buffer_.size() - buffer_idx_;
  buffer_idx_ = 0;
  buffer_.resize(kBufferCapacity);
  const auto fill_size =
      input_->ReadSome(&buffer_[unread_size], kBufferCapacity - unread_size);
  buffer_.resize(unread_size + fill_size);
  return fill_size;
}

databento::RecordHeader* DbnDecoder::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(&buffer_[buffer_idx_]);
}

bool DbnDecoder::DetectCompression() {
  buffer_.resize(kMagicSize);
  input_->ReadExact(buffer_.data(), kMagicSize);
  auto buffer_it = buffer_.cbegin();
  if (std::strncmp(Consume(buffer_it, 3), kDbnPrefix, 3) == 0) {
    return false;
  }
  buffer_it = buffer_.cbegin();
  auto x = Consume<std::uint32_t>(buffer_it);
  if (x == kZstdMagicNumber) {
    return true;
  }
  throw DbnResponseError{
      "Couldn't detect input type. It doesn't appear to be Zstd or "
      "DBN."};
}

std::string DbnDecoder::ParseSymbol(
    std::vector<std::uint8_t>::const_iterator& buffer_it) {
  return std::string{Consume(buffer_it, kSymbolCstrLen)};
}

std::vector<std::string> DbnDecoder::ParseRepeatedSymbol(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  if (buffer_it + sizeof(std::uint32_t) > buffer_end_it) {
    throw DbnResponseError{"Unexpected end of metadata buffer"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  if (buffer_it + static_cast<std::int64_t>(count * ::kSymbolCstrLen) >
      buffer_end_it) {
    throw DbnResponseError{"Unexpected end of metadata buffer"};
  }
  std::vector<std::string> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(ParseSymbol(buffer_it));
  }
  return res;
}

std::vector<databento::SymbolMapping> DbnDecoder::ParseSymbolMappings(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  if (buffer_it + sizeof(std::uint32_t) > buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing mappings"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  std::vector<SymbolMapping> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DbnDecoder::ParseSymbolMapping(buffer_it, buffer_end_it));
  }
  return res;
}

databento::SymbolMapping DbnDecoder::ParseSymbolMapping(
    std::vector<std::uint8_t>::const_iterator& buffer_it,
    std::vector<std::uint8_t>::const_iterator buffer_end_it) {
  constexpr std::size_t kMinSymbolMappingEncodedLen =
      kSymbolCstrLen + sizeof(std::uint32_t);
  constexpr std::size_t kMappingIntervalEncodedLen =
      sizeof(std::uint32_t) * 2 + kSymbolCstrLen;

  if (buffer_it + kMinSymbolMappingEncodedLen > buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol "
        "mapping"};
  }
  SymbolMapping res;
  res.native_symbol = ParseSymbol(buffer_it);
  const auto interval_count = std::size_t{Consume<std::uint32_t>(buffer_it)};
  const auto read_size =
      static_cast<std::ptrdiff_t>(interval_count * kMappingIntervalEncodedLen);
  if (buffer_it + read_size > buffer_end_it) {
    throw DbnResponseError{
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