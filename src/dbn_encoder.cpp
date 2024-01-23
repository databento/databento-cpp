#include "databento/dbn_encoder.hpp"

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

using databento::DbnEncoder;

namespace {
constexpr std::size_t kMagicSize = 4;
constexpr std::uint32_t kZstdMagicNumber = 0xFD2FB528;
constexpr auto kDbnPrefix = "DBN";
constexpr std::size_t kFixedMetadataLen = 100;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kReservedLen = 53;
constexpr std::size_t kReservedLenV1 = 47;
constexpr std::size_t kBufferCapacity = 8UL * 1024;



constexpr std::uint32_t kSymbolCstrLenV1 = 22;
constexpr std::uint64_t NULL_RECORD_COUNT = std::numeric_limits<std::uint64_t>::max();
constexpr std::uint64_t NULL_LIMIT = 0;
constexpr std::uint16_t NULL_SCHEMA = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint8_t NULL_STYPE = std::numeric_limits<std::uint8_t>::max();

}  // namespace


static
std::uint32_t version_symbol_cstr_len(uint8_t version) {
  if (version < 2) {
    return kSymbolCstrLenV1;
  } else {
    return databento::kSymbolCstrLen;
  }
}

static
std::uint32_t calc_length(const databento::Metadata& metadata) {
  std::uint32_t symbol_cstr_len = version_symbol_cstr_len(metadata.version);
  std::uint32_t mapping_interval_len = sizeof(uint32_t) * 2 + symbol_cstr_len;
  // schema_definition_length, symbols_count, partial_count, not_found_count, mappings_count
  std::uint32_t var_len_counts_size = sizeof(uint32_t) * 5;

  std::uint32_t c_str_count = metadata.symbols.size() + metadata.partial.size() + metadata.not_found.size();

  std::uint32_t mappings_len = 0;
  for (const databento::SymbolMapping& item: metadata.mappings) {
    mappings_len += symbol_cstr_len + sizeof(uint32_t) + item.intervals.size() * mapping_interval_len;
  }

  return kFixedMetadataLen + var_len_counts_size + c_str_count * symbol_cstr_len + mappings_len;
}

static
void encode_date(std::uint32_t date, databento::IWritable& writer) {
  // in c++ the date is already represented as a uint32, so this function doesn't really do anything compared to rust
  writer.Write(reinterpret_cast<const std::uint8_t *>(&date), sizeof(date));
}

static
void encode_fixed_len_cstr(const std::size_t symbol_cstr_len, const std::string & str, databento::IWritable& writer) {
  // check if string is printable (rust checks if it is ascii)
  if (!std::all_of(str.begin(), str.end(), [](unsigned char ch) {return std::isprint(ch);})) {
    throw databento::InvalidArgumentError("encode_fixed_len_cstr", "str", "must only contain printable characters");
  }

  if (str.size() > symbol_cstr_len) {
    std::string details("value too large to fit, can be at most ");
    details.append(std::to_string(symbol_cstr_len)).append(", was ").append(std::to_string(str.size()));
    throw databento::InvalidArgumentError("encode_fixed_len_cstr", "str", std::move(details));
  }

  // write the string
  writer.Write(reinterpret_cast<const std::uint8_t *>(str.data()), str.size());

  // write padding
  for (std::size_t i = str.size(); i < symbol_cstr_len; ++i) {
    const std::uint8_t zero = 0;
    writer.Write(&zero, sizeof(zero));
  }
}

static
void encode_symbol_mapping(const std::size_t symbol_cstr_len, const databento::SymbolMapping & symbol_mapping, databento::IWritable& writer) {
  encode_fixed_len_cstr(symbol_cstr_len, symbol_mapping.raw_symbol, writer);
  // encode interval_count
  const std::uint32_t length = symbol_mapping.intervals.size(); // assume that this will not overflow
  // assuming little endian, it is currently required for the cmake stage to pass
  writer.Write(reinterpret_cast<const std::uint8_t *>(&length), sizeof(length));

  for (const databento::MappingInterval& interval: symbol_mapping.intervals) {
    encode_date(interval.start_date, writer);
    encode_date(interval.end_date, writer);
    encode_fixed_len_cstr(symbol_cstr_len, interval.symbol, writer);
  }
}

static
std::uint64_t deref_or(const std::uint64_t * ptr, std::uint64_t otherwise) {
  if (ptr) {
    return *ptr;
  } else {
    return otherwise;
  }
}

// future, could use std::optional here if it is OK to depend on it.
// can the maybe fields ever be not set tho? seems like its not possible in the C++ version
static
void encode_range_and_counts(std::uint8_t version,
                             std::uint64_t start,
                             const std::uint64_t* maybe_end,
                             const std::uint64_t* maybe_limit,
                             databento::IWritable& writer) {
  // assuming little endian, it is currently required for the cmake stage to pass
  writer.Write(reinterpret_cast<const std::uint8_t *>(&start), sizeof(start));

  std::uint64_t tmp_uint64 = deref_or(maybe_end, databento::kUndefTimestamp);
  // assuming little endian, it is currently required for the cmake stage to pass
  writer.Write(reinterpret_cast<const std::uint8_t *>(&tmp_uint64), sizeof(tmp_uint64));

  tmp_uint64 = deref_or(maybe_limit, NULL_LIMIT);
  // assuming little endian, it is currently required for the cmake stage to pass
  writer.Write(reinterpret_cast<const std::uint8_t *>(&tmp_uint64), sizeof(tmp_uint64));

  if (version == 1) {
    tmp_uint64 = NULL_RECORD_COUNT;
    // assuming little endian, it is currently required for the cmake stage to pass
    writer.Write(reinterpret_cast<const std::uint8_t *>(&tmp_uint64), sizeof(tmp_uint64));
  }
}

static
void encode_repeated_symbol_cstr(const std::size_t symbol_cstr_len, const std::vector<std::string> & symbols, databento::IWritable& writer) {
  // write number of symbols (length)
  const std::uint32_t length = symbols.size(); // assume that this will never overflow, who even has more than 4 billion symbols anyway
  writer.Write(reinterpret_cast<const std::uint8_t *>(&length), sizeof(length));

  for (const auto& symbol: symbols) {
    encode_fixed_len_cstr(symbol_cstr_len, symbol, writer);
  }
}

static
void encode_symbol_mappings(const std::size_t symbol_cstr_len, const std::vector<databento::SymbolMapping> & symbol_mappings, databento::IWritable& writer) {
  // encode mappings_count
  const std::uint32_t length = symbol_mappings.size(); // assume that this will never overflow, who even has more than 4 billion mappings anyway
  writer.Write(reinterpret_cast<const std::uint8_t *>(&length), sizeof(length));

  for (const auto& mapping: symbol_mappings) {
    encode_symbol_mapping(symbol_cstr_len, mapping, writer);
  }
}

void DbnEncoder::EncodeMetadata(const Metadata& metadata, IWritable& writer) {
  // maybe one should serialize to a internal buffer and then write after everything is done?
  // the writer might be in a strange state if anything fails in the current design

  std::uint8_t tmp[std::max<std::uint32_t>({8, kReservedLen, kReservedLenV1})]{};

  // write the magic number and version
  std::memcpy(tmp, kDbnPrefix, 3);
  tmp[3] = std::min<uint8_t>(std::max<uint8_t>(metadata.version, 1), kDbnVersion);
  writer.Write(tmp, kMagicSize);

  std::uint32_t length = calc_length(metadata);
  // assuming little endian, it is currently required for the cmake stage to pass
  writer.Write(reinterpret_cast<const std::uint8_t *>(&length), sizeof(length));

  encode_fixed_len_cstr(kDatasetCstrLen, metadata.dataset, writer);

  // assuming little endian, it is currently required for the cmake stage to pass
  std::uint16_t raw_schema;
  if (metadata.has_mixed_schema) {
    raw_schema = NULL_SCHEMA;
  } else {
    raw_schema = static_cast<std::uint16_t>(metadata.schema);
  }
  writer.Write(reinterpret_cast<const std::uint8_t *>(&raw_schema), sizeof(raw_schema));

  std::uint64_t end_count = metadata.end.time_since_epoch().count();
  encode_range_and_counts(metadata.version, metadata.start.time_since_epoch().count(), &end_count, &metadata.limit, writer);

  std::uint8_t stype_in;
  if (metadata.has_mixed_stype_in) {
    stype_in = NULL_STYPE;
  } else {
    stype_in = static_cast<std::uint8_t>(metadata.stype_in);
  }
  writer.Write(&stype_in, sizeof(stype_in));

  auto stype_out = static_cast<std::uint8_t>(metadata.stype_out);
  writer.Write(&stype_out, sizeof(stype_out));

  auto ts_out = static_cast<std::uint8_t>(metadata.ts_out);
  writer.Write(&ts_out, sizeof(ts_out));

  if (metadata.version > 1) {
    if (metadata.symbol_cstr_len > std::numeric_limits<std::uint16_t>::max()) {
      throw databento::InvalidArgumentError("EncodeMetadata", "symbol_cstr_len", "value too large to fit in uint16");
    }

    std::uint16_t symbol_cstr_len = metadata.symbol_cstr_len;
    // assuming little endian, it is currently required for the cmake stage to pass
    writer.Write(reinterpret_cast<const std::uint8_t *>(&symbol_cstr_len), sizeof(symbol_cstr_len));
  }

  // padding
  if (metadata.version == 1) {
    std::memset(tmp, 0, kReservedLenV1);
    writer.Write(tmp, kReservedLenV1);
  } else {
    std::memset(tmp, 0, kReservedLen);
    writer.Write(tmp, kReservedLen);
  }

  // schema_definition_length, not supported by DBN v1 nor v2
  const std::uint32_t schema_definition_length = 0;
  writer.Write(reinterpret_cast<const std::uint8_t *>(&schema_definition_length), sizeof(schema_definition_length));


  encode_repeated_symbol_cstr(metadata.symbol_cstr_len, metadata.symbols, writer);
  encode_repeated_symbol_cstr(metadata.symbol_cstr_len, metadata.partial, writer);
  encode_repeated_symbol_cstr(metadata.symbol_cstr_len, metadata.not_found, writer);
  encode_symbol_mappings(metadata.symbol_cstr_len, metadata.mappings, writer);
}



