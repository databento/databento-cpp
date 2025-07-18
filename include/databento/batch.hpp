#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "databento/enums.hpp"  // JobState, Delivery, Schema, SType

namespace databento {
// Description of a batch job.
struct BatchJob {
  std::string id;
  std::string user_id;
  // Cost in US dollars
  double cost_usd;
  std::string dataset;
  std::vector<std::string> symbols;
  SType stype_in;
  SType stype_out;
  Schema schema;
  std::string start;
  std::string end;
  std::uint64_t limit;
  Encoding encoding;
  Compression compression;
  bool pretty_px;
  bool pretty_ts;
  bool map_symbols;
  SplitDuration split_duration;
  std::uint64_t split_size;
  bool split_symbols;
  Delivery delivery;
  std::uint64_t record_count;
  // Size in bytes.
  std::uint64_t billed_size;
  // Size in bytes.
  std::uint64_t actual_size;
  // Size in bytes.
  std::uint64_t package_size;
  JobState state;
  std::string ts_received;
  // Empty if it hasn't been queued.
  std::string ts_queued;
  // Empty if processing hasn't started.
  std::string ts_process_start;
  // Empty if it hasn't finished processing.
  std::string ts_process_done;
  // Empty if it hasn't finished processing. The expiration is set based on when
  // the job finishes processing, not when it was requested.
  std::string ts_expiration;
};

// Description of a batch file.
struct BatchFileDesc {
  std::string filename;
  std::uint64_t size;
  std::string hash;
  std::string https_url;
  std::string ftp_url;
};

std::string ToString(const BatchJob& batch_job);
std::ostream& operator<<(std::ostream& stream, const BatchJob& batch_job);
std::string ToString(const BatchFileDesc& file_desc);
std::ostream& operator<<(std::ostream& stream, const BatchFileDesc& file_desc);
}  // namespace databento
