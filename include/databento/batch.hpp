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
  std::string dataset;
  double cost;
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
  bool is_full_book;
  bool is_example;
  std::size_t record_count;
  std::size_t billed_size;
  std::size_t actual_size;
  std::size_t package_size;
  JobState state;
  std::string ts_received;
  std::string ts_queued;
  std::string ts_process_start;
  std::string ts_process_done;
  std::string ts_expiration;
};

std::string ToString(const BatchJob& batch_job);
std::ostream& operator<<(std::ostream& stream, const BatchJob& batch_job);
}  // namespace databento
