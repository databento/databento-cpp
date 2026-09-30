#include "databento/detail/dbn_fsm.hpp"

#include <date/date.h>
#include <dbn/dbn.h>

#include <algorithm>  // max
#include <chrono>
#include <cstring>  // memmove
#include <optional>
#include <string>
#include <utility>  // move

#include "databento/datetime.hpp"    // UnixNanos
#include "databento/exceptions.hpp"  // DbnResponseError, InvalidArgumentError

using databento::detail::DbnFsm;

namespace {
// The DBN library's decoder, which the header only knows as an opaque type
DbnDecoder* AsDecoder(databento::detail::CFfiDecoder* decoder) {
  return reinterpret_cast<DbnDecoder*>(decoder);
}

std::string ToStdString(DbnStrRef str) { return {str.data, str.len}; }

date::year_month_day DecodeIso8601Date(std::uint32_t yyyymmdd_int) {
  const auto year = yyyymmdd_int / 10000;
  const auto remaining = yyyymmdd_int % 10000;
  const auto month = remaining / 100;
  const auto day = remaining % 100;
  return {date::year{static_cast<std::int32_t>(year)}, date::month{month},
          date::day{day}};
}

template <typename CountFn, typename GetFn>
std::vector<std::string> DecodeSymbols(const DbnMetadata* ffi, CountFn count,
                                       GetFn get) {
  const auto symbol_count = count(ffi);
  std::vector<std::string> res;
  res.reserve(symbol_count);
  for (std::size_t i = 0; i < symbol_count; ++i) {
    res.emplace_back(ToStdString(get(ffi, i)));
  }
  return res;
}

std::optional<databento::Schema> DecodeSchema(const DbnMetadata* ffi) {
  std::uint16_t raw_schema{};
  if (DbnMetadata_schema(ffi, &raw_schema)) {
    return static_cast<databento::Schema>(raw_schema);
  }
  return std::nullopt;
}

std::optional<databento::SType> DecodeSTypeIn(const DbnMetadata* ffi) {
  std::uint8_t raw_stype_in{};
  if (DbnMetadata_stype_in(ffi, &raw_stype_in)) {
    return static_cast<databento::SType>(raw_stype_in);
  }
  return std::nullopt;
}

std::vector<databento::MappingInterval> DecodeMappingIntervals(const DbnMetadata* ffi,
                                                               std::size_t index) {
  const auto intervals_count = DbnMetadata_mapping_intervals_count(ffi, index);
  std::vector<databento::MappingInterval> res;
  res.reserve(intervals_count);
  for (std::size_t i = 0; i < intervals_count; ++i) {
    DbnMappingIntervalRef interval{};
    DbnMetadata_mapping_interval(ffi, index, i, &interval);
    res.emplace_back(databento::MappingInterval{DecodeIso8601Date(interval.start_date),
                                                DecodeIso8601Date(interval.end_date),
                                                ToStdString(interval.symbol)});
  }
  return res;
}

std::vector<databento::SymbolMapping> DecodeMappings(const DbnMetadata* ffi) {
  const auto mappings_count = DbnMetadata_mappings_count(ffi);
  std::vector<databento::SymbolMapping> res;
  res.reserve(mappings_count);
  for (std::size_t i = 0; i < mappings_count; ++i) {
    res.emplace_back(
        databento::SymbolMapping{ToStdString(DbnMetadata_mapping_raw_symbol(ffi, i)),
                                 DecodeMappingIntervals(ffi, i)});
  }
  return res;
}

databento::Metadata DecodeMetadata(const DbnMetadata* ffi) {
  return databento::Metadata{
      DbnMetadata_version(ffi),
      ToStdString(DbnMetadata_dataset(ffi)),
      DecodeSchema(ffi),
      databento::UnixNanos{std::chrono::nanoseconds{DbnMetadata_start(ffi)}},
      databento::UnixNanos{std::chrono::nanoseconds{DbnMetadata_end(ffi)}},
      DbnMetadata_limit(ffi),
      DecodeSTypeIn(ffi),
      static_cast<databento::SType>(DbnMetadata_stype_out(ffi)),
      DbnMetadata_ts_out(ffi),
      DbnMetadata_symbol_cstr_len(ffi),
      DecodeSymbols(ffi, DbnMetadata_symbols_count, DbnMetadata_symbols_get),
      DecodeSymbols(ffi, DbnMetadata_partial_count, DbnMetadata_partial_get),
      DecodeSymbols(ffi, DbnMetadata_not_found_count, DbnMetadata_not_found_get),
      DecodeMappings(ffi),
  };
}

const char* ErrorMessage(DbnDecoderError error) {
  switch (error) {
    case DbnDecoderError_InvalidUpgradePolicy:
      return "invalid version upgrade policy";
    case DbnDecoderError_InvalidInputVersion:
      return "invalid input DBN version";
    case DbnDecoderError_IncompatiblePolicyAndVersion:
      return "incompatible version upgrade policy and input DBN version";
    case DbnDecoderError_NullOptions:
    default:
      return "invalid options";
  }
}

databento::detail::CFfiDecoder* Create(databento::VersionUpgradePolicy upgrade_policy,
                                       std::size_t buffer_size) {
  DbnDecoderOptions options{};
  options.upgrade_policy = static_cast<std::uint8_t>(upgrade_policy);
  // `Space` can only return space for the largest record if the buffer is at
  // least that big
  options.buffer_size =
      buffer_size == 0 ? 0 : std::max(buffer_size, databento::kMaxRecordLen);
  DbnDecoderError error{};
  auto* decoder = DbnDecoder_create(&options, &error);
  if (decoder == nullptr) {
    throw databento::InvalidArgumentError{"DbnFsm::DbnFsm", "upgrade_policy",
                                          ErrorMessage(error)};
  }
  return reinterpret_cast<databento::detail::CFfiDecoder*>(decoder);
}
}  // namespace

void databento::detail::FreeCFfiDecoder(CFfiDecoder* decoder) {
  DbnDecoder_free(AsDecoder(decoder));
}

// A `buffer_size` of 0 leaves the buffer size to the DBN library
DbnFsm::DbnFsm(VersionUpgradePolicy upgrade_policy) : DbnFsm{upgrade_policy, 0} {}

DbnFsm::DbnFsm(VersionUpgradePolicy upgrade_policy, std::size_t buffer_size)
    : decoder_{Create(upgrade_policy, buffer_size), FreeCFfiDecoder} {}

std::byte* DbnFsm::Space(std::size_t* length) {
  if (batch_.HasUndrained()) {
    batch_.CopyUndrained();
  }
  return reinterpret_cast<std::byte*>(
      DbnDecoder_space(AsDecoder(decoder_.get()), length));
}

void DbnFsm::Fill(std::size_t length) {
  DbnDecoder_fill(AsDecoder(decoder_.get()), length);
}

void DbnFsm::WriteAll(const char* data, std::size_t length) {
  if (batch_.HasUndrained()) {
    batch_.CopyUndrained();
  }
  DbnDecoder_write_all(AsDecoder(decoder_.get()),
                       reinterpret_cast<const std::uint8_t*>(data), length);
}

void DbnFsm::WriteAll(const std::byte* data, std::size_t length) {
  if (batch_.HasUndrained()) {
    batch_.CopyUndrained();
  }
  DbnDecoder_write_all(AsDecoder(decoder_.get()),
                       reinterpret_cast<const std::uint8_t*>(data), length);
}

DbnFsm::Status DbnFsm::Process() {
  if (!batch_.HasUndrained()) {
    const auto outcome = DbnDecoder_process_many(
        AsDecoder(decoder_.get()),
        reinterpret_cast<const DbnRecordHeader**>(batch_.Data()), batch_.Capacity());
    switch (outcome.status) {
      case DbnProcessStatus_ReadMore: {
        return Status::ReadMore;
      }
      case DbnProcessStatus_Metadata: {
        const std::unique_ptr<DbnMetadata, void (*)(DbnMetadata*)> owned{
            DbnDecoder_take_metadata(AsDecoder(decoder_.get())), DbnMetadata_free};
        metadata_ = DecodeMetadata(owned.get());
        return Status::Metadata;
      }
      case DbnProcessStatus_Record: {
        batch_.Start(outcome.count);
        break;
      }
      case DbnProcessStatus_Error:
      default: {
        const char* error = DbnDecoder_last_error(AsDecoder(decoder_.get()));
        throw DbnResponseError{error == nullptr ? "Failed to decode DBN" : error};
      }
    }
  }
  // TODO(cg): change Record to wrap const RecordHeader
  // The decoder owns the record, which stays valid until the buffer is next
  // mutated. The C API returns it as const, but Record wraps a mutable header.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  last_record_ = Record{const_cast<RecordHeader*>(batch_.Next())};
  return Status::Record;
}

void DbnFsm::RecordBatch::CopyUndrained() {
  std::size_t size{};
  for (auto i = drained_; i < count_; ++i) {
    size += records_[i]->Size();
  }
  if (undrained_records_.size() < size) {
    undrained_records_.resize(size);
  }
  auto* dest = undrained_records_.data();
  for (auto i = drained_; i < count_; ++i) {
    const auto rec_size = records_[i]->Size();
    // Not `memcpy` because when this runs again before the batch is drained, the
    // records are already in `undrained_records_` and can overlap `dest`
    std::memmove(dest, records_[i], rec_size);
    records_[i] = reinterpret_cast<const RecordHeader*>(dest);
    dest += rec_size;
  }
}

databento::Metadata DbnFsm::TakeMetadata() {
  if (!metadata_.has_value()) {
    throw Exception{"No decoded metadata to take"};
  }
  auto metadata = std::move(*metadata_);
  metadata_.reset();
  return metadata;
}

std::size_t DbnFsm::UnreadBytes() const {
  std::size_t length{};
  DbnDecoder_data(AsDecoder(decoder_.get()), &length);
  return length;
}

void DbnFsm::Reset() {
  DbnDecoder_reset(AsDecoder(decoder_.get()));
  metadata_.reset();
  last_record_ = Record{nullptr};
  batch_ = RecordBatch{};
}
