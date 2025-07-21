#pragma once
#include <date/date.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "databento/compat.hpp"
#include "databento/record.hpp"

namespace databento {
// Forward declare
struct Metadata;

/// A timeseries symbol map. Useful for working with historical
/// data.
class TsSymbolMap {
 public:
  // UTC date and instrument ID to text symbol.
  using Store = std::map<std::pair<date::year_month_day, std::uint32_t>,
                         std::shared_ptr<const std::string>>;

  TsSymbolMap() = default;
  explicit TsSymbolMap(const Metadata& metadata);

  bool IsEmpty() const { return map_.empty(); }
  std::size_t Size() const { return map_.size(); }
  const Store& Map() const { return map_; }
  Store& Map() { return map_; }
  Store::const_iterator Find(date::year_month_day date,
                             std::uint32_t instrument_id) const {
    return map_.find(std::make_pair(date, instrument_id));
  }
  template <typename R>
  Store::const_iterator Find(const R& rec) const {
    static_assert(has_header<R>::value,
                  "must be a DBN record struct with an `hd` RecordHeader field");
    date::year_month_day index_date{
        date::sys_days{date::floor<date::days>(rec.IndexTs())}};
    return map_.find(std::make_pair(index_date, rec.hd.instrument_id));
  }
  const std::string& At(date::year_month_day date, std::uint32_t instrument_id) const {
    return *map_.at(std::make_pair(date, instrument_id));
  }
  template <typename R>
  const std::string& At(const R& rec) const {
    static_assert(has_header<R>::value,
                  "must be a DBN record struct with an `hd` RecordHeader field");
    date::year_month_day index_date{
        date::sys_days{date::floor<date::days>(rec.IndexTs())}};
    return *map_.at(std::make_pair(index_date, rec.hd.instrument_id));
  }
  void Insert(std::uint32_t instrument_id, date::year_month_day start_date,
              date::year_month_day end_date,
              const std::shared_ptr<const std::string>& symbol);

 private:
  Store map_;
};

// A point-in-time symbol map. Useful for working with live
// symbology or a historical request over a single day or other
// situations where the symbol mappings are known not to change.
class PitSymbolMap {
 public:
  // Instrument ID to text symbol
  using Store = std::unordered_map<std::uint32_t, std::string>;

  PitSymbolMap() = default;
  PitSymbolMap(const Metadata& metadata, date::year_month_day date);

  bool IsEmpty() const { return map_.empty(); }
  std::size_t Size() const { return map_.size(); }
  const Store& Map() const { return map_; }
  Store& Map() { return map_; }
  Store::const_iterator Find(const Record& rec) const {
    return map_.find(rec.Header().instrument_id);
  }
  Store::const_iterator Find(std::uint32_t instrument_id) const {
    return map_.find(instrument_id);
  }
  template <typename R>
  const std::string& At(const R& rec) const {
    static_assert(has_header<R>::value,
                  "must be a DBN record struct with an `hd` RecordHeader field");
    return map_.at(rec.hd.instrument_id);
  }
  const std::string& At(const Record& rec) const {
    return map_.at(rec.Header().instrument_id);
  }
  const std::string& operator[](std::uint32_t instrument_id) {
    return map_[instrument_id];
  }
  void OnRecord(const Record& rec);
  template <typename SymbolMappingRec>
  void OnSymbolMapping(const SymbolMappingRec& symbol_mapping);

 private:
  Store map_;
};

// Forward declare explicit instantiation
extern template void PitSymbolMap::OnSymbolMapping(
    const SymbolMappingMsgV1& symbol_mapping);
extern template void PitSymbolMap::OnSymbolMapping(
    const SymbolMappingMsgV2& symbol_mapping);
}  // namespace databento
