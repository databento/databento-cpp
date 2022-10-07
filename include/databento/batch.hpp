#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "databento/datetime.hpp"  // EpochNanos
#include "databento/enums.hpp"  // BatchState, Delivery, Packaging, Schema, SType

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
  EpochNanos start;
  EpochNanos end;
  std::size_t limit;
  Compression compression;
  DurationInterval split_duration;
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
  BatchState state;
  EpochNanos ts_received;
  EpochNanos ts_queued;
  EpochNanos ts_process_start;
  EpochNanos ts_process_done;
  EpochNanos ts_expiration;
};
}  // namespace databento
