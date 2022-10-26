#pragma once

#include <cstddef>
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
  UnixNanos start;
  UnixNanos end;
  std::size_t limit;
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
  UnixNanos ts_received;
  UnixNanos ts_queued;
  UnixNanos ts_process_start;
  UnixNanos ts_process_done;
  UnixNanos ts_expiration;
};
}  // namespace databento
