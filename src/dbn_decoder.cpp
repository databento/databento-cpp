#include "databento/dbn_decoder.hpp"

#include <date/date.h>

#include <algorithm>  // copy
#include <cstring>    // strncmp
#include <vector>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"
#include "dbn_constants.hpp"

using databento::DbnDecoder;

namespace {
template <typename T>
T Consume(const std::byte*& buf) {
  const auto res = *reinterpret_cast<const T*>(&*buf);
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
  const auto* pos = &*buf;
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
    : DbnDecoder(log_receiver,
                 std::make_unique<InFileStream>(std::move(file_stream))) {}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver,
                       std::unique_ptr<IReadable> input)
    : DbnDecoder(log_receiver, std::move(input),
                 VersionUpgradePolicy::UpgradeToV2) {}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver,
                       std::unique_ptr<IReadable> input,
                       VersionUpgradePolicy upgrade_policy)
    : log_receiver_{log_receiver},
      upgrade_policy_{upgrade_policy},
      input_{std::move(input)} {
  read_buffer_.reserve(kBufferCapacity);
  if (DetectCompression()) {
    input_ = std::make_unique<detail::ZstdDecodeStream>(
        std::move(input_), std::move(read_buffer_));
    // Reinitialize buffer and get it into the same state as uncompressed input
    read_buffer_ = std::vector<std::byte>();
    read_buffer_.reserve(kBufferCapacity);
    read_buffer_.resize(kMagicSize);
    input_->ReadExact(read_buffer_.data(), kMagicSize);
    const auto* buf_ptr = read_buffer_.data();
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

databento::Metadata DbnDecoder::DecodeMetadataFields(
    std::uint8_t version, const std::byte* buffer,
    const std::byte* buffer_end) {
  Metadata res;
  res.version = version;
  if (res.version > kDbnVersion) {
    throw DbnResponseError{
        "Can't decode newer version of DBN. Decoder version is " +
        std::to_string(kDbnVersion) + ", input version is " +
        std::to_string(res.version)};
  }
  res.dataset = Consume(buffer, kDatasetCstrLen, "dataset");
  const auto raw_schema = Consume<std::uint16_t>(buffer);
  if (raw_schema == kNullSchema) {
    res.has_mixed_schema = true;
    // must initialize
    res.schema = Schema::Mbo;
  } else {
    res.has_mixed_schema = false;
    res.schema = static_cast<Schema>(raw_schema);
  }
  res.start =
      UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer)}};
  res.end = UnixNanos{std::chrono::nanoseconds{Consume<std::uint64_t>(buffer)}};
  res.limit = Consume<std::uint64_t>(buffer);
  if (version == 1) {
    // skip deprecated record_count
    buffer += 8;
  }
  const auto raw_stype_in = Consume<std::uint8_t>(buffer);
  if (raw_stype_in == kNullSType) {
    res.has_mixed_stype_in = true;
    // must initialize
    res.stype_in = SType::InstrumentId;
  } else {
    res.has_mixed_stype_in = false;
    res.stype_in = static_cast<SType>(raw_stype_in);
  }
  res.stype_out = static_cast<SType>(Consume<std::uint8_t>(buffer));
  res.ts_out = static_cast<bool>(Consume<std::uint8_t>(buffer));
  if (version > 1) {
    res.symbol_cstr_len =
        static_cast<std::size_t>(Consume<std::uint16_t>(buffer));
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
    throw DbnResponseError{
        "This version of dbn can't parse schema definitions"};
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
  read_buffer_.resize(kMetadataPreludeSize);
  input_->ReadExact(&read_buffer_[4], 4);
  const auto [version, size] = DbnDecoder::DecodeMetadataVersionAndSize(
      read_buffer_.data(), kMetadataPreludeSize);
  version_ = version;
  read_buffer_.resize(size);
  input_->ReadExact(read_buffer_.data(), read_buffer_.size());
  buffer_idx_ = read_buffer_.size();
  auto metadata = DbnDecoder::DecodeMetadataFields(
      version_, read_buffer_.data(), read_buffer_.data() + read_buffer_.size());
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
    const databento::WithTsOut<U> v2{orig.rec.ToV2(), orig.ts_out};
    const auto v2_ptr = reinterpret_cast<const std::byte*>(&v2);
    std::copy(v2_ptr, v2_ptr + v2.rec.hd.Size(), compat_buffer->data());
  } else {
    const auto v2 = rec.Get<T>().ToV2();
    const auto v2_ptr = reinterpret_cast<const std::byte*>(&v2);
    std::copy(v2_ptr, v2_ptr + v2.hd.Size(), compat_buffer->data());
  }
  return databento::Record{
      reinterpret_cast<databento::RecordHeader*>(compat_buffer->data())};
}
}  // namespace

databento::Record DbnDecoder::DecodeRecordCompat(
    std::uint8_t version, VersionUpgradePolicy upgrade_policy, bool ts_out,
    std::array<std::byte, kMaxRecordLen>* compat_buffer, Record rec) {
  if (version == 1 && upgrade_policy == VersionUpgradePolicy::UpgradeToV2) {
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

// assumes DecodeMetadata has been called
const databento::Record* DbnDecoder::DecodeRecord() {
  // need some unread unread_bytes
  if (GetReadBufferSize() == 0) {
    if (FillBuffer() == 0) {
      return nullptr;
    }
  }
  // check length
  while (GetReadBufferSize() < BufferRecordHeader()->Size()) {
    if (FillBuffer() == 0) {
      if (GetReadBufferSize() > 0) {
        log_receiver_->Receive(
            LogLevel::Warning,
            "Unexpected partial record remaining in stream: " +
                std::to_string(GetReadBufferSize()) + " bytes");
      }
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

std::size_t DbnDecoder::GetReadBufferSize() const {
  return read_buffer_.size() - buffer_idx_;
}

databento::RecordHeader* DbnDecoder::BufferRecordHeader() {
  return reinterpret_cast<RecordHeader*>(&read_buffer_[buffer_idx_]);
}

bool DbnDecoder::DetectCompression() {
  read_buffer_.resize(kMagicSize);
  input_->ReadExact(read_buffer_.data(), kMagicSize);
  const auto* read_buffer_it = read_buffer_.data();
  if (std::strncmp(Consume(read_buffer_it, 3), kDbnPrefix, 3) == 0) {
    return false;
  }
  read_buffer_it = read_buffer_.data();
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

std::string DbnDecoder::DecodeSymbol(std::size_t symbol_cstr_len,
                                     std::byte const*& read_buffer_it) {
  return Consume(read_buffer_it, static_cast<std::ptrdiff_t>(symbol_cstr_len),
                 "symbol");
}

std::vector<std::string> DbnDecoder::DecodeRepeatedSymbol(
    std::size_t symbol_cstr_len, const std::byte*& read_buf,
    const std::byte* read_buf_end) {
  if (read_buf + sizeof(std::uint32_t) > read_buf_end) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buf)};
  if (read_buf + static_cast<std::ptrdiff_t>(count * symbol_cstr_len) >
      read_buf_end) {
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing symbol"};
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
    throw DbnResponseError{
        "Unexpected end of metadata buffer while parsing mappings"};
  }
  const auto count = std::size_t{Consume<std::uint32_t>(read_buf)};
  std::vector<SymbolMapping> res;
  res.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    res.emplace_back(DbnDecoder::DecodeSymbolMapping(symbol_cstr_len, read_buf,
                                                     read_buf_end));
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
