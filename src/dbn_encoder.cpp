#include "databento/dbn_encoder.hpp"

#include <date/date.h>

#include <cstddef>
#include <cstdint>
#include <numeric>  // accumulate
#include <string>
#include <vector>

#include "databento/constants.hpp"
#include "databento/dbn.hpp"
#include "databento/exceptions.hpp"
#include "databento/iwritable.hpp"
#include "dbn_constants.hpp"

using databento::DbnEncoder;

namespace {
void EncodeChars(const char* bytes, std::size_t length,
                 databento::IWritable* output) {
  output->WriteAll(reinterpret_cast<const std::byte*>(bytes), length);
}

void EncodeFixedLenCStr(std::size_t cstr_len, const std::string& str,
                        databento::IWritable* output) {
  // >= to ensure space for null padding
  if (str.size() >= cstr_len) {
    throw databento::InvalidArgumentError{
        "EncodeFixedLenCStr", "str",
        std::string{"String is too long to encode, maximum length of "} +
            std::to_string(cstr_len - 1)};
  }
  output->WriteAll(reinterpret_cast<const std::byte*>(str.data()),
                   str.length());
  // Null padding
  std::vector<std::byte> filler(cstr_len - str.length());
  output->WriteAll(filler.data(), filler.size());
}

template <typename T>
void EncodeAsBytes(T bytes, databento::IWritable* output) {
  output->WriteAll(reinterpret_cast<const std::byte*>(&bytes), sizeof(bytes));
}

void EncodeDate(date::year_month_day date, databento::IWritable* output) {
  auto date_int = static_cast<std::uint32_t>(std::int32_t{date.year()}) * 10000;
  date_int += std::uint32_t{date.month()} * 100;
  date_int += std::uint32_t{date.day()};
  EncodeAsBytes(date_int, output);
}

void EncodeRepeatedSymbolCStr(std::size_t cstr_len,
                              const std::vector<std::string>& symbols,
                              databento::IWritable* output) {
  const auto length = static_cast<std::uint32_t>(symbols.size());
  EncodeAsBytes(length, output);
  for (const auto& symbol : symbols) {
    EncodeFixedLenCStr(cstr_len, symbol, output);
  }
}

void EncodeSymbolMappings(
    std::size_t cstr_len,
    const std::vector<databento::SymbolMapping>& symbol_mappings,
    databento::IWritable* output) {
  const auto mappings_length =
      static_cast<std::uint32_t>(symbol_mappings.size());
  EncodeAsBytes(mappings_length, output);
  for (const auto& symbol_mapping : symbol_mappings) {
    EncodeFixedLenCStr(cstr_len, symbol_mapping.raw_symbol, output);
    const auto interval_length =
        static_cast<std::uint32_t>(symbol_mapping.intervals.size());
    EncodeAsBytes(interval_length, output);
    for (const auto& interval : symbol_mapping.intervals) {
      EncodeDate(interval.start_date, output);
      EncodeDate(interval.end_date, output);
      EncodeFixedLenCStr(cstr_len, interval.symbol, output);
    }
  }
}
}  // namespace

DbnEncoder::DbnEncoder(const Metadata& metadata, IWritable* output)
    : output_{output} {
  EncodeMetadata(metadata, output_);
}

void DbnEncoder::EncodeMetadata(const Metadata& metadata, IWritable* output) {
  const auto version = std::min<std::uint8_t>(
      std::max<std::uint8_t>(1, metadata.version), kDbnVersion);
  EncodeChars(kDbnPrefix, kMagicSize - 1, output);
  EncodeAsBytes(version, output);
  const std::uint32_t length = CalcLength(metadata);
  EncodeAsBytes(length, output);
  EncodeFixedLenCStr(kDatasetCstrLen, metadata.dataset, output);
  if (metadata.has_mixed_schema) {
    EncodeAsBytes(kNullSchema, output);
  } else {
    EncodeAsBytes(metadata.schema, output);
  }
  EncodeAsBytes(metadata.start, output);
  EncodeAsBytes(metadata.end, output);
  EncodeAsBytes(metadata.limit, output);
  if (version == 1) {
    // backwards compatibility for record_count
    EncodeAsBytes(kNullRecordCount, output);
  }
  if (metadata.has_mixed_stype_in) {
    EncodeAsBytes(kNullSType, output);
  } else {
    EncodeAsBytes(metadata.stype_in, output);
  }
  EncodeAsBytes(metadata.stype_out, output);
  EncodeAsBytes(static_cast<std::uint8_t>(metadata.ts_out), output);
  if (version > 1) {
    const auto symbol_cstr_len =
        static_cast<std::uint16_t>(metadata.symbol_cstr_len);
    EncodeAsBytes(symbol_cstr_len, output);
  }
  // padding + schema definition length
  auto reserved_length =
      version == 1 ? kMetadataReservedLenV1 : kMetadataReservedLen;
  const std::vector<std::byte> padding(reserved_length + sizeof(std::uint32_t));
  output->WriteAll(padding.data(), padding.size());

  // variable-length data
  EncodeRepeatedSymbolCStr(metadata.symbol_cstr_len, metadata.symbols, output);
  EncodeRepeatedSymbolCStr(metadata.symbol_cstr_len, metadata.partial, output);
  EncodeRepeatedSymbolCStr(metadata.symbol_cstr_len, metadata.not_found,
                           output);
  EncodeSymbolMappings(metadata.symbol_cstr_len, metadata.mappings, output);
}

void DbnEncoder::EncodeRecord(const Record& record, IWritable* output) {
  output->WriteAll(reinterpret_cast<const std::byte*>(&record.Header()),
                   record.Size());
}

void DbnEncoder::EncodeRecord(const Record& record) {
  EncodeRecord(record, output_);
}

std::uint32_t DbnEncoder::CalcLength(const Metadata& metadata) {
  const auto symbol_cstr_len = metadata.symbol_cstr_len;
  const auto mapping_interval_len = sizeof(std::uint32_t) * 2 + symbol_cstr_len;
  // schema_definition_length, symbols_count, partial_count, not_found_count,
  // mappings_count
  const auto var_len_counts_size = sizeof(std::uint32_t) * 5;

  const auto c_str_count = metadata.symbols.size() + metadata.partial.size() +
                           metadata.not_found.size();
  const auto mappings_len = std::accumulate(
      metadata.mappings.begin(), metadata.mappings.end(), std::size_t{0},
      [symbol_cstr_len, mapping_interval_len](std::size_t acc,
                                              const SymbolMapping& m) {
        return acc + symbol_cstr_len + sizeof(std::uint32_t) +
               m.intervals.size() * mapping_interval_len;
      });
  return static_cast<std::uint32_t>(kFixedMetadataLen + var_len_counts_size +
                                    c_str_count * symbol_cstr_len +
                                    mappings_len);
}
