#include "databento/dbn_decoder.hpp"

#include <algorithm>  // copy
#include <cstddef>
#include <cstring>  // strncmp
#include <limits>
#include <vector>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"

using databento::DbnDecoder;

namespace {
constexpr std::size_t kMagicSize = 4;
constexpr std::uint32_t kZstdMagicNumber = 0xFD2FB528;
constexpr auto kDbnPrefix = "DBN";
constexpr std::size_t kFixedMetadataLen = 100;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kReservedLen = 53;
constexpr std::size_t kReservedLenV1 = 47;
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

std::string Consume(std::vector<std::uint8_t>::const_iterator& byte_it,
                    const std::ptrdiff_t num_bytes, const char* context) {
  const auto cstr = Consume(byte_it, num_bytes);
  // strnlen isn't portable
  const std::size_t str_len = std::find(cstr, cstr + num_bytes, '\0') - cstr;
  if (str_len == num_bytes) {
    throw databento::DbnResponseError{std::string{"Invalid "} + context +
                                      " missing null terminator"};
  }
  return std::string{cstr, str_len};
}
}  // namespace

DbnDecoder::DbnDecoder(detail::SharedChannel channel)
    : DbnDecoder(std::unique_ptr<IReadable>{
          new detail::SharedChannel{std::move(channel)}}) {}

DbnDecoder::DbnDecoder(detail::FileStream file_stream)
    : DbnDecoder(std::unique_ptr<IReadable>{
          new detail::FileStream{std::move(file_stream)}}) {}

DbnDecoder::DbnDecoder(std::unique_ptr<IReadable> input)
    : DbnDecoder(std::move(input), VersionUpgradePolicy::Upgrade) {}

DbnDecoder::DbnDecoder(std::unique_ptr<IReadable> input,
                       VersionUpgradePolicy upgrade_policy)
    : input_{std::move(input)}, upgrade_policy_{upgrade_policy} {
  read_buffer_.reserve(kBufferCapacity);
  if (DetectCompression()) {
    input_ = std::unique_ptr<detail::ZstdStream>(
        new detail::ZstdStream(std::move(input_), std::move(read_buffer_)));
    // Reinitialize buffer and get it into the same state as uncompressed input
    read_buffer_ = std::vector<std::uint8_t>();
    read_buffer_.reserve(kBufferCapacity);
    read_buffer_.resize(kMagicSize);
    input_->ReadExact(read_buffer_.data(), kMagicSize);
    auto read_buffer_it = read_buffer_.cbegin();
    if (std::strncmp(Consume(read_buffer_it, 3), kDbnPrefix, 3) != 0) {
      throw DbnResponseError{"Found Zstd input, but not DBN prefix"};
    }
  }
}

std::pair<std::uint8_t, std::size_t> DbnDecoder::DecodeMetadataVersionAndSize(
    const std::uint8_t* buffer, std::size_t size) {
  if (size < 8) {
    throw DbnResponseError{"Buffer too small to decode version and size"};
  }
  if (std::strncmp(reinterpret_cast<const char*>(buffer), kDbnPrefix, 3) != 0) {
    throw DbnResponseError{"Missing DBN prefix"};
  }
  const auto version = buffer[3];
  const auto frame_size = *reinterpret_cast<const std::uint32_t*>(&buffer[4]);
  if (frame_size < ::kFixedMetadataLen) {
    throw DbnResponseError{
        "Frame length cannot be shorter than the fixed metadata size"};
  }
  return {version, static_cast<std::size_t>(frame_size)};
}

databento::Metadata DbnDecoder::DecodeMetadataFields(
    std::uint8_t version, const std::vector<std::uint8_t>& buffer) {
  Metadata res;
  res.version = version;
  if (res.version > kDbnVersion) {
    throw DbnResponseError{
        "Can't decode newer version of DBN. Decoder version is " +
        std::to_string(kDbnVersion) + ", input version is " +
        std::to_string(res.version)};
  }
  auto read_buffer_it = buffer.cbegin();
  res.dataset = Consume(read_buffer_it, kDatasetCstrLen, "dataset");
  const auto raw_schema = Consume<std::uint16_t>(read_buffer_it);
  if (raw_schema == std::numeric_limits<std::uint16_t>::max()) {
    res.has_mixed_schema = true;
    // must initialize
    res.schema = Schema::Mbo;
  } else {
    res.has_mixed_schema = false;
    res.schema = static_cast<Schema>(raw_schema);
  }
  res.start = UnixNanos{
      std::chrono::nanoseconds{Consume<std::uint64_t>(read_buffer_it)}};
  res.end = UnixNanos{
      std::chrono::nanoseconds{Consume<std::uint64_t>(read_buffer_it)}};
  res.limit = Consume<std::uint64_t>(read_buffer_it);
  if (version == 1) {
    // skip deprecated record_count
    read_buffer_it += 8;
  }
  const auto raw_stype_in = Consume<std::uint8_t>(read_buffer_it);
  if (raw_stype_in == std::numeric_limits<std::uint8_t>::max()) {
    res.has_mixed_stype_in = true;
    // must initialize
    res.stype_in = SType::InstrumentId;
  } else {
    res.has_mixed_stype_in = false;
    res.stype_in = static_cast<SType>(raw_stype_in);
  }
  res.stype_out = static_cast<SType>(Consume<std::uint8_t>(read_buffer_it));
  res.ts_out = static_cast<bool>(Consume<std::uint8_t>(read_buffer_it));
  if (version > 1) {
    res.symbol_cstr_len =
        static_cast<std::size_t>(Consume<std::uint16_t>(read_buffer_it));
  } else {
    res.symbol_cstr_len = kSymbolCstrLenV1;
  }
  // skip reserved
  if (version == 1) {
    read_buffer_it += ::kReservedLenV1;
  } else {
    read_buffer_it += ::kReservedLen;
  }

  const auto schema_definition_length = Consume<std::uint32_t>(read_buffer_it);
  if (schema_definition_length != 0) {
    throw DbnResponseError{
        "This version of dbn can't parse schema definitions"};
  }
  res.symbols = DbnDecoder::DecodeRepeatedSymbol(res.symbol_cstr_len,
                                                 read_buffer_it, buffer.cend());
  res.partial = DbnDecoder::DecodeRepeatedSymbol(res.symbol_cstr_len,
                                                 read_buffer_it, buffer.cend());
  res.not_found = DbnDecoder::DecodeRepeatedSymbol(
      res.symbol_cstr_len, read_buffer_it, buffer.cend());
  res.mappings = DbnDecoder::DecodeSymbolMappings(
      res.symbol_cstr_len, read_buffer_it, buffer.cend());

  return res;
}

databento::Metadata DbnDecoder::DecodeMetadata() {
  // already read first 4 bytes detecting compression
  read_buffer_.resize(8);
  input_->ReadExact(&read_buffer_[4], 4);
  const auto version_and_size =
      DbnDecoder::DecodeMetadataVersionAndSize(read_buffer_.data(), 8);
  version_ = version_and_size.first;
  read_buffer_.resize(version_and_size.second);
  input_->ReadExact(read_buffer_.data(), read_buffer_.size());
  buffer_idx_ = read_buffer_.size();
  auto metadata = DbnDecoder::DecodeMetadataFields(version_, read_buffer_);
  ts_out_ = metadata.ts_out;
  return metadata;
}

namespace {
template <typename T, typename U>
databento::Record UpgradeRecord(
    bool ts_out,
    std::array<std::uint8_t, databento::kMaxRecordLen>* compat_buffer,
    databento::Record rec) {
  if (ts_out) {
    const auto orig = rec.Get<databento::WithTsOut<T>>();
    const databento::WithTsOut<U> v2{orig.rec.ToV2(), orig.ts_out};
    const auto v2_ptr = reinterpret_cast<const std::uint8_t*>(&v2);
    std::copy(v2_ptr, v2_ptr + v2.rec.hd.Size(), compat_buffer->data());
  } else {
    const auto v2 = rec.Get<T>().ToV2();
    const auto v2_ptr = reinterpret_cast<const std::uint8_t*>(&v2);
    std::copy(v2_ptr, v2_ptr + v2.hd.Size(), compat_buffer->data());
  }
  return databento::Record{
      reinterpret_cast<databento::RecordHeader*>(compat_buffer->data())};
}
}  // namespace

databento::Record DbnDecoder::DecodeRecordCompat(
    std::uint8_t version, VersionUpgradePolicy upgrade_policy, bool ts_out,
    std::array<std::uint8_t, kMaxRecordLen>* compat_buffer, Record rec) {
  if (version == 1 && upgrade_policy == VersionUpgradePolicy::Upgrade) {
    if (rec.RType() == RType::InstrumentDef) {
      return UpgradeRecord<InstrumentDefMsgV1, InstrumentDefMsgV2>(
          ts_out, compat_buffer, rec);
    } else if (rec.RType() == RType::SymbolMapping) {
      return UpgradeRecord<SymbolMappingMsgV1, SymbolMappingMsgV2>(
          ts_out, compat_buffer, rec);
    } else if (rec.RType() == RType::Error) {
      return UpgradeRecord<ErrorMsgV1, ErrorMsgV2>(ts_out, compat_buffer, rec);
    } else if (rec.RType() == RType::System) {
      return UpgradeRecord<SystemMsgV1, SystemMsgV2>(ts_out, compat_buffer,
                                                     rec);
    }
  }
  return rec;
}

// assumes ParseMetadata has been called
const databento::Record* DbnDecoder::DecodeRecord() {
  // need some unread unread_bytes
  const auto unread_bytes = read_buffer_.size() - buffer_idx_;
  if (unread_bytes == 0) {
    if (FillBuffer() == 0) {
      return nullptr;
    }
  }
  // check length
  while (read_buffer_.size() - buffer_idx_ < BufferRecordHeader()->Size()) {
    if (FillBuffer() == 0) {
      return nullptr;
    }
  }
  current_record_ = Record{BufferRecordHeader()};
  buffer_idx_ += current_record_.Size();
  current_record_ = DbnDecoder::DecodeRecordCompat(
      version_, upgrade_policy_, ts_out_, &compat_buffer_, current_record_);
  return &current_record_;
}

size_t DbnDecoder::FillBuffer() {
  // Shift data forward
  std::copy(read_buffer_.cbegin() + static_cast<std::ptrdiff_t>(buffer_idx_),
            read_buffer_.cend(), read_buffer_.begin());
  const auto unread_size = read_buffer_.size() - buffer_idx_;
  buffer_idx_ = 0;
  read_buffer_.resize(kBufferCapacity);
  const auto fill_size = input_->ReadSome(&read_buffer_[unread_size],
                                          kBufferCapacity - unread_size);
  read_buffer_.resize(unread_size + fill_size);
  return fill_size;
}

databento::RecordHeader* DbnDecoder::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(&read_buffer_[buffer_idx_]);
}

bool DbnDecoder::DetectCompression() {
  read_buffer_.resize(kMagicSize);
  input_->ReadExact(read_buffer_.data(), kMagicSize);
  auto read_buffer_it = read_buffer_.cbegin();
  if (std::strncmp(Consume(read_buffer_it, 3), kDbnPrefix, 3) == 0) {
    return false;
  }
  read_buffer_it = read_buffer_.cbegin();
  auto x = Consume<std::uint32_t>(read_buffer_it);
  if (x == kZstdMagicNumber) {
    return true;
  }
  // Zstandard skippable frames begin with 0x184D2A5? where the last 8 bits
  // can be set to any value
  constexpr auto kZstdSkippableFrame = 0x184D2A50;
  if ((x & kZstdSkippableFrame) == kZstdSkippableFrame) {
    throw DbnResponseError{
        "Legacy DBZ encoding is not supported. Please use the dbn CLI tool "
        "to "
        "convert it to DBN."};
  }
  throw DbnResponseError{
      "Couldn't detect input type. It doesn't appear to be Zstd or "
      "DBN."};
}

std::string DbnDecoder::DecodeSymbol(
    std::size_t symbol_cstr_len,
    std::vector<std::uint8_t>::const_iterator& read_buffer_it) {
  return Consume(read_buffer_it, static_cast<std::ptrdiff_t>(symbol_cstr_len),
                 "symbol");
}

std::vector<std::string> DbnDecoder::DecodeRepeatedSymbol(
    std::size_t symbol_cstr_len,
    std::vector<std::uint8_t>::const_iterator& read_buffer_it,
    std::vector<std::uint8_t>::const_iterator read_buffer_end_it) {
  if (read_buffer_it + sizeof(std::uint32_t) > read_buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buffer_it)};
  if (read_buffer_it + static_cast<std::ptrdiff_t>(count * symbol_cstr_len) >
      read_buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol"};
  }
  std::vector<std::string> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DecodeSymbol(symbol_cstr_len, read_buffer_it));
  }
  return res;
}

std::vector<databento::SymbolMapping> DbnDecoder::DecodeSymbolMappings(
    std::size_t symbol_cstr_len,
    std::vector<std::uint8_t>::const_iterator& read_buffer_it,
    std::vector<std::uint8_t>::const_iterator read_buffer_end_it) {
  if (read_buffer_it + sizeof(std::uint32_t) > read_buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing mappings"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buffer_it)};
  std::vector<SymbolMapping> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DbnDecoder::DecodeSymbolMapping(
        symbol_cstr_len, read_buffer_it, read_buffer_end_it));
  }
  return res;
}

databento::SymbolMapping DbnDecoder::DecodeSymbolMapping(
    std::size_t symbol_cstr_len,
    std::vector<std::uint8_t>::const_iterator& read_buffer_it,
    std::vector<std::uint8_t>::const_iterator read_buffer_end_it) {
  const auto min_symbol_mapping_encoded_len =
      static_cast<std::ptrdiff_t>(symbol_cstr_len + sizeof(std::uint32_t));
  const auto mapping_encoded_len = sizeof(std::uint32_t) * 2 + symbol_cstr_len;

  if (read_buffer_it + min_symbol_mapping_encoded_len > read_buffer_end_it) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol "
        "mapping"};
  }
  SymbolMapping res;
  res.raw_symbol = DecodeSymbol(symbol_cstr_len, read_buffer_it);
  const auto interval_count =
      std::size_t{Consume<std::uint32_t>(read_buffer_it)};
  const auto read_size =
      static_cast<std::ptrdiff_t>(interval_count * mapping_encoded_len);
  if (read_buffer_it + read_size > read_buffer_end_it) {
    throw DbnResponseError{
        "Symbol mapping interval_count doesn't match size of buffer"};
  }
  res.intervals.reserve(interval_count);
  for (std::size_t i = 0; i < interval_count; ++i) {
    MappingInterval interval;
    interval.start_date = Consume<std::uint32_t>(read_buffer_it);
    interval.end_date = Consume<std::uint32_t>(read_buffer_it);
    interval.symbol = DecodeSymbol(symbol_cstr_len, read_buffer_it);
    res.intervals.emplace_back(std::move(interval));
  }
  return res;
}
