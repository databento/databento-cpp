#include "databento/dbn_decoder.hpp"

#include <date/date.h>

#include <algorithm>  // copy
#include <cstring>    // strncmp
#include <optional>
#include <vector>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/record.hpp"
#include "databento/v3.hpp"
#include "databento/with_ts_out.hpp"
#include "dbn_constants.hpp"

using databento::DbnDecoder;

namespace {
template <typename T>
T Consume(const std::byte*& buf) {
  const auto res = *reinterpret_cast<const T*>(buf);
  buf += sizeof(T);
  return res;
}

template <>
std::uint8_t Consume(const std::byte*& buf) {
  const auto res = *buf;
  buf += 1;
  return static_cast<std::uint8_t>(res);
}

const char* Consume(const std::byte*& buf, const std::ptrdiff_t num_bytes) {
  const auto* pos = buf;
  buf += num_bytes;
  return reinterpret_cast<const char*>(pos);
}

std::string Consume(const std::byte*& buf, const std::ptrdiff_t num_bytes,
                    const char* context) {
  const auto cstr = Consume(buf, num_bytes);
  // strnlen isn't portable
  const auto str_len = std::find(cstr, cstr + num_bytes, '\0') - cstr;
  if (str_len == num_bytes) {
    throw databento::DbnResponseError{std::string{"Invalid "} + context +
                                      " missing null terminator"};
  }
  return std::string{cstr, static_cast<std::size_t>(str_len)};
}

date::year_month_day DecodeIso8601Date(std::uint32_t yyyymmdd_int) {
  const auto year = yyyymmdd_int / 10000;
  const auto remaining = yyyymmdd_int % 10000;
  const auto month = remaining / 100;
  const auto day = remaining % 100;
  return {date::year{static_cast<std::int32_t>(year)}, date::month{month},
          date::day{day}};
}
}  // namespace

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, InFileStream file_stream)
    : DbnDecoder(log_receiver, std::make_unique<InFileStream>(std::move(file_stream))) {
}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input)
    : DbnDecoder(log_receiver, std::move(input), VersionUpgradePolicy::UpgradeToV3) {}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
                       VersionUpgradePolicy upgrade_policy)
    : log_receiver_{log_receiver},
      upgrade_policy_{upgrade_policy},
      input_{std::move(input)} {
  if (DetectCompression()) {
    input_ = std::make_unique<detail::ZstdDecodeStream>(std::move(input_), buffer_);
    input_->ReadExact(buffer_.WriteBegin(), kMagicSize);
    buffer_.Fill(kMagicSize);
    const auto* buf_ptr = buffer_.ReadBegin();
    if (std::strncmp(Consume(buf_ptr, 3), kDbnPrefix, 3) != 0) {
      throw DbnResponseError{"Found Zstd input, but not DBN prefix"};
    }
  }
}

std::pair<std::uint8_t, std::size_t> DbnDecoder::DecodeMetadataVersionAndSize(
    const std::byte* buffer, std::size_t size) {
  if (size < 8) {
    throw DbnResponseError{"Buffer too small to decode version and size"};
  }
  if (std::strncmp(reinterpret_cast<const char*>(buffer), kDbnPrefix, 3) != 0) {
    throw DbnResponseError{"Missing DBN prefix"};
  }
  const auto version = static_cast<std::uint8_t>(buffer[3]);
  const auto frame_size = *reinterpret_cast<const std::uint32_t*>(&buffer[4]);
  if (frame_size < kFixedMetadataLen) {
    throw DbnResponseError{
        "Frame length cannot be shorter than the fixed metadata size"};
  }
  return {version, static_cast<std::size_t>(frame_size)};
}

databento::Metadata DbnDecoder::DecodeMetadataFields(std::uint8_t version,
                                                     const std::byte* buffer,
                                                     const std::byte* buffer_end) {
  Metadata res;
  res.version = version;
  if (res.version > kDbnVersion) {
    throw DbnResponseError{"Can't decode newer version of DBN. Decoder version is " +
                           std::to_string(kDbnVersion) + ", input version is " +
                           std::to_string(res.version)};
  }
  res.dataset = Consume(buffer, kDatasetCstrLen, "dataset");
  const auto raw_schema = Consume<std::uint16_t>(buffer);
  if (raw_schema == kNullSchema) {
    res.schema = std::nullopt;
  } else {
    res.schema = {static_cast<Schema>(raw_schema)};
  }
  res.start = UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer)}};
  res.end = UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer)}};
  res.limit = Consume<std::uint64_t>(buffer);
  if (version == 1) {
    // skip deprecated record_count
    buffer += 8;
  }
  const auto raw_stype_in = Consume<std::uint8_t>(buffer);
  if (raw_stype_in == kNullSType) {
    res.stype_in = std::nullopt;
  } else {
    res.stype_in = {static_cast<SType>(raw_stype_in)};
  }
  res.stype_out = static_cast<SType>(Consume<std::uint8_t>(buffer));
  res.ts_out = static_cast<bool>(Consume<std::uint8_t>(buffer));
  if (version > 1) {
    res.symbol_cstr_len = static_cast<std::size_t>(Consume<std::uint16_t>(buffer));
  } else {
    res.symbol_cstr_len = kSymbolCstrLenV1;
  }
  // skip reserved
  if (version == 1) {
    buffer += kMetadataReservedLenV1;
  } else {
    buffer += kMetadataReservedLen;
  }

  const auto schema_definition_length = Consume<std::uint32_t>(buffer);
  if (schema_definition_length != 0) {
    throw DbnResponseError{"This version of dbn can't parse schema definitions"};
  }
  res.symbols =
      DbnDecoder::DecodeRepeatedSymbol(res.symbol_cstr_len, buffer, buffer_end);
  res.partial =
      DbnDecoder::DecodeRepeatedSymbol(res.symbol_cstr_len, buffer, buffer_end);
  res.not_found =
      DbnDecoder::DecodeRepeatedSymbol(res.symbol_cstr_len, buffer, buffer_end);
  res.mappings =
      DbnDecoder::DecodeSymbolMappings(res.symbol_cstr_len, buffer, buffer_end);

  return res;
}

databento::Metadata DbnDecoder::DecodeMetadata() {
  // already read first 4 bytes detecting compression
  const auto read_size = kMetadataPreludeSize - kMagicSize;
  input_->ReadExact(buffer_.WriteBegin(), read_size);
  buffer_.Fill(read_size);
  const auto [version, size] = DbnDecoder::DecodeMetadataVersionAndSize(
      buffer_.ReadBegin(), kMetadataPreludeSize);
  buffer_.Consume(kMetadataPreludeSize);
  version_ = version;
  buffer_.Reserve(size);
  input_->ReadExact(buffer_.WriteBegin(), size);
  buffer_.Fill(size);
  auto metadata = DbnDecoder::DecodeMetadataFields(version_, buffer_.ReadBegin(),
                                                   buffer_.ReadEnd());
  buffer_.Consume(size);
  // Metadata may leave buffer misaligned. Shift records to ensure 8-byte
  // alignment
  buffer_.Shift();
  ts_out_ = metadata.ts_out;
  metadata.Upgrade(upgrade_policy_);
  return metadata;
}

namespace {
template <typename T, typename U>
databento::Record UpgradeRecord(
    bool ts_out, std::array<std::byte, databento::kMaxRecordLen>* compat_buffer,
    databento::Record rec) {
  if (ts_out) {
    const auto orig = rec.Get<databento::WithTsOut<T>>();
    const databento::WithTsOut<U> upgraded = {orig.rec.template Upgrade<U>(),
                                              orig.ts_out};
    const auto upgraded_ptr = reinterpret_cast<const std::byte*>(&upgraded);
    std::copy(upgraded_ptr, upgraded_ptr + upgraded.rec.hd.Size(),
              compat_buffer->data());
  } else {
    const auto& orig = rec.Get<T>();
    const U upgraded = orig.template Upgrade<U>();
    const auto upgraded_ptr = reinterpret_cast<const std::byte*>(&upgraded);
    std::copy(upgraded_ptr, upgraded_ptr + upgraded.hd.Size(), compat_buffer->data());
  }
  return databento::Record{
      reinterpret_cast<databento::RecordHeader*>(compat_buffer->data())};
}
}  // namespace

databento::Record DbnDecoder::DecodeRecordCompat(
    std::uint8_t version, VersionUpgradePolicy upgrade_policy, bool ts_out,
    std::array<std::byte, kMaxRecordLen>* compat_buffer, Record rec) {
  if (version == 1 && upgrade_policy == VersionUpgradePolicy::UpgradeToV2) {
    switch (rec.RType()) {
      case RType::InstrumentDef: {
        return UpgradeRecord<v1::InstrumentDefMsg, v2::InstrumentDefMsg>(
            ts_out, compat_buffer, rec);
      }
      case RType::SymbolMapping: {
        return UpgradeRecord<v1::SymbolMappingMsg, v2::SymbolMappingMsg>(
            ts_out, compat_buffer, rec);
      }
      case RType::Error: {
        return UpgradeRecord<v1::ErrorMsg, v2::ErrorMsg>(ts_out, compat_buffer, rec);
      }
      case RType::System: {
        return UpgradeRecord<v1::SystemMsg, v2::SystemMsg>(ts_out, compat_buffer, rec);
      }
      default: {
        break;
      }
    }
  } else if (version == 1 && upgrade_policy == VersionUpgradePolicy::UpgradeToV3) {
    switch (rec.RType()) {
      case RType::InstrumentDef: {
        return UpgradeRecord<v1::InstrumentDefMsg, v3::InstrumentDefMsg>(
            ts_out, compat_buffer, rec);
      }
      case RType::Statistics: {
        return UpgradeRecord<v1::StatMsg, v3::StatMsg>(ts_out, compat_buffer, rec);
      }
      case RType::SymbolMapping: {
        return UpgradeRecord<v1::SymbolMappingMsg, v3::SymbolMappingMsg>(
            ts_out, compat_buffer, rec);
      }
      case RType::Error: {
        return UpgradeRecord<v1::ErrorMsg, v3::ErrorMsg>(ts_out, compat_buffer, rec);
      }
      case RType::System: {
        return UpgradeRecord<v1::SystemMsg, v3::SystemMsg>(ts_out, compat_buffer, rec);
      }
      default: {
        break;
      }
    }
  } else if (version == 2 && upgrade_policy == VersionUpgradePolicy::UpgradeToV3) {
    switch (rec.RType()) {
      case RType::InstrumentDef: {
        return UpgradeRecord<v2::InstrumentDefMsg, v3::InstrumentDefMsg>(
            ts_out, compat_buffer, rec);
      }
      case RType::Statistics: {
        return UpgradeRecord<v2::StatMsg, v3::StatMsg>(ts_out, compat_buffer, rec);
      }
      default: {
        break;
      }
    }
  }
  return rec;
}

// assumes DecodeMetadata has been called
const databento::Record* DbnDecoder::DecodeRecord() {
  // need some unread unread_bytes
  if (buffer_.ReadCapacity() == 0) {
    if (FillBuffer() == 0) {
      return nullptr;
    }
  }
  // check length
  while (buffer_.ReadCapacity() < BufferRecordHeader()->Size()) {
    if (FillBuffer() == 0) {
      if (buffer_.ReadCapacity() > 0) {
        log_receiver_->Receive(LogLevel::Warning,
                               "Unexpected partial record remaining in stream: " +
                                   std::to_string(buffer_.ReadCapacity()) + " bytes");
      }
      return nullptr;
    }
  }
  current_record_ = Record{BufferRecordHeader()};
  buffer_.ConsumeNoShift(current_record_.Size());
  current_record_ = DbnDecoder::DecodeRecordCompat(version_, upgrade_policy_, ts_out_,
                                                   &compat_buffer_, current_record_);
  return &current_record_;
}

size_t DbnDecoder::FillBuffer() {
  if (buffer_.WriteCapacity() < kMaxRecordLen) {
    buffer_.Shift();
  }
  const auto fill_size =
      input_->ReadSome(buffer_.WriteBegin(), buffer_.WriteCapacity());
  buffer_.Fill(fill_size);
  return fill_size;
}

databento::RecordHeader* DbnDecoder::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(buffer_.ReadBegin());
}

bool DbnDecoder::DetectCompression() {
  input_->ReadExact(buffer_.WriteBegin(), kMagicSize);
  buffer_.Fill(kMagicSize);
  const auto* buffer_it = buffer_.ReadBegin();
  if (std::strncmp(Consume(buffer_it, 3), kDbnPrefix, 3) == 0) {
    return false;
  }
  buffer_it = buffer_.ReadBegin();
  auto x = Consume<std::uint32_t>(buffer_it);
  if (x == kZstdMagicNumber) {
    return true;
  }
  // Zstandard skippable frames begin with 0x184D2A5? where the last 8 bits
  // can be set to any value
  constexpr auto kZstdSkippableFrame = 0x184D2A50;
  if ((x & kZstdSkippableFrame) == kZstdSkippableFrame) {
    throw DbnResponseError{
        "Legacy DBZ encoding is not supported. Please use the dbn CLI tool "
        "to convert it to DBN."};
  }
  throw DbnResponseError{
      "Couldn't detect input type. It doesn't appear to be Zstd or DBN."};
}

std::string DbnDecoder::DecodeSymbol(std::size_t symbol_cstr_len,
                                     std::byte const*& read_buffer_it) {
  return Consume(read_buffer_it, static_cast<std::ptrdiff_t>(symbol_cstr_len),
                 "symbol");
}

std::vector<std::string> DbnDecoder::DecodeRepeatedSymbol(
    std::size_t symbol_cstr_len, const std::byte*& read_buf,
    const std::byte* read_buf_end) {
  if (read_buf + sizeof(std::uint32_t) > read_buf_end) {
    throw DbnResponseError{"Unexpected end of metadata buffer while parsing symbol"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buf)};
  if (read_buf + static_cast<std::ptrdiff_t>(count * symbol_cstr_len) > read_buf_end) {
    throw DbnResponseError{"Unexpected end of metadata buffer while parsing symbol"};
  }
  std::vector<std::string> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DecodeSymbol(symbol_cstr_len, read_buf));
  }
  return res;
}

std::vector<databento::SymbolMapping> DbnDecoder::DecodeSymbolMappings(
    std::size_t symbol_cstr_len, const std::byte*& read_buf,
    const std::byte* read_buf_end) {
  if (read_buf + sizeof(std::uint32_t) > read_buf_end) {
    throw DbnResponseError{"Unexpected end of metadata buffer while parsing mappings"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buf)};
  std::vector<SymbolMapping> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(
        DbnDecoder::DecodeSymbolMapping(symbol_cstr_len, read_buf, read_buf_end));
  }
  return res;
}

databento::SymbolMapping DbnDecoder::DecodeSymbolMapping(
    std::size_t symbol_cstr_len, const std::byte*& read_buf,
    const std::byte* read_buf_end) {
  const auto min_symbol_mapping_encoded_len =
      static_cast<std::ptrdiff_t>(symbol_cstr_len + sizeof(std::uint32_t));
  const auto mapping_encoded_len = sizeof(std::uint32_t) * 2 + symbol_cstr_len;

  if (read_buf + min_symbol_mapping_encoded_len > read_buf_end) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol "
        "mapping"};
  }
  SymbolMapping res;
  res.raw_symbol = DecodeSymbol(symbol_cstr_len, read_buf);
  const auto interval_count = std::size_t{Consume<std::uint32_t>(read_buf)};
  const auto read_size =
      static_cast<std::ptrdiff_t>(interval_count * mapping_encoded_len);
  if (read_buf + read_size > read_buf_end) {
    throw DbnResponseError{
        "Symbol mapping interval_count doesn't match size of buffer"};
  }
  res.intervals.reserve(interval_count);
  for (std::size_t i = 0; i < interval_count; ++i) {
    MappingInterval interval;
    auto raw_start_date = Consume<std::uint32_t>(read_buf);
    interval.start_date = DecodeIso8601Date(raw_start_date);
    auto raw_end_date = Consume<std::uint32_t>(read_buf);
    interval.end_date = DecodeIso8601Date(raw_end_date);
    interval.symbol = DecodeSymbol(symbol_cstr_len, read_buf);
    res.intervals.emplace_back(std::move(interval));
  }
  return res;
}
