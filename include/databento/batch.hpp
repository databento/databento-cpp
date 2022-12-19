#pragma once

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/enums.hpp"  // JobState, Delivery, Packaging, Schema, SType

namespace databento {
struct BatchJob {
  std::string id;
  std::string user_id;
  std::string bill_id;
  // Price in cents
  double cost;
  std::string dataset;
  std::vector<std::string> symbols;
  SType stype_in;
  SType stype_out;
  Schema schema;
  std::string start;
  std::string end;
  std::size_t limit;
  Encoding encoding;
  Compression compression;
  SplitDuration split_duration;
  std::size_t split_size;
  bool split_symbols;
  Packaging packaging;
  Delivery delivery;
  // If the entire universe of instruments for the dataset is selected.
  bool is_full_universe;
  // If the batch job is a tutorial example.
  bool is_example;
  std::size_t record_count;
  // Size in bytes.
  std::size_t billed_size;
  // Size in bytes.
  std::size_t actual_size;
  // Size in bytes.
  std::size_t package_size;
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

std::string ToString(const BatchJob& batch_job);
std::ostream& operator<<(std::ostream& stream, const BatchJob& batch_job);
}  // namespace databento
