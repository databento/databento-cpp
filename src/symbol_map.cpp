#include "databento/symbol_map.hpp"

#include <date/date.h>

#include <memory>
#include <string>

#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"

using databento::TsSymbolMap;

namespace {
bool IsInverse(const databento::Metadata& metadata) {
  if (metadata.stype_in.has_value()) {
    if (*metadata.stype_in == databento::SType::InstrumentId) {
      return true;
    }
    if (metadata.stype_out == databento::SType::InstrumentId) {
      return false;
    }
  }
  throw databento::InvalidArgumentError{
      "SymbolMap", "metadata",
      "Can only create symbol maps from metadata where InstrumentId is one "
      "of the stypes"};
}
}  // namespace

TsSymbolMap::TsSymbolMap(const Metadata& metadata) {
  if (::IsInverse(metadata)) {
    for (const auto& mapping : metadata.mappings) {
      const auto iid = static_cast<std::uint32_t>(std::stoul(mapping.raw_symbol));
      for (const auto& interval : mapping.intervals) {
        // Handle old symbology format
        if (interval.symbol.empty()) {
          continue;
        }
        Insert(iid, interval.start_date, interval.end_date,
               std::make_shared<std::string>(interval.symbol));
      }
    }
  } else {
    for (const auto& mapping : metadata.mappings) {
      const auto symbol = std::make_shared<std::string>(mapping.raw_symbol);
      for (const auto& interval : mapping.intervals) {
        // Handle old symbology format
        if (interval.symbol.empty()) {
          continue;
        }
        const auto iid = static_cast<std::uint32_t>(std::stoul(interval.symbol));
        Insert(iid, interval.start_date, interval.end_date, symbol);
      }
    }
  }
}

void TsSymbolMap::Insert(std::uint32_t instrument_id, date::year_month_day start_date,
                         date::year_month_day end_date,
                         const std::shared_ptr<const std::string>& symbol) {
  if (start_date > end_date) {
    throw InvalidArgumentError{"TsSymbolMap::Insert", "end_date",
                               "can't be before start_date"};
  }
  if (start_date == end_date) {
    // Ignore
    return;
  }
  for (date::sys_days day = start_date; day < end_date; day += date::days{1}) {
    map_.emplace(std::make_pair(date::year_month_day{day}, instrument_id), symbol);
  }
}

using databento::PitSymbolMap;

PitSymbolMap::PitSymbolMap(const Metadata& metadata, date::year_month_day date) {
  if (date::sys_days{date} < date::floor<date::days>(metadata.start) ||
      // need to compare with `end` as datetime to handle midnight case
      UnixNanos{date::sys_days{date}} >= metadata.end) {
    throw InvalidArgumentError{"PitSymbolMap::PitSymbolMap", "date",
                               "Outside query range"};
  }
  const auto is_inverse = IsInverse(metadata);
  for (const auto& mapping : metadata.mappings) {
    const auto interval_it =
        std::find_if(mapping.intervals.begin(), mapping.intervals.end(),
                     [date](const MappingInterval& interval) {
                       return date >= interval.start_date && date < interval.end_date;
                     });
    // Empty symbols in old symbology format
    if (interval_it == mapping.intervals.end() || interval_it->symbol.empty()) {
      continue;
    }
    if (is_inverse) {
      const auto iid = static_cast<std::uint32_t>(std::stoul(mapping.raw_symbol));
      map_.emplace(iid, interval_it->symbol);
    } else {
      const auto iid = static_cast<std::uint32_t>(std::stoul(interval_it->symbol));
      map_.emplace(iid, mapping.raw_symbol);
    }
  }
}

template <typename SymbolMappingRec>
void PitSymbolMap::OnSymbolMapping(const SymbolMappingRec& symbol_mapping) {
  const auto it = map_.find(symbol_mapping.hd.instrument_id);
  if (it == map_.end()) {
    map_.emplace(symbol_mapping.hd.instrument_id, symbol_mapping.STypeOutSymbol());
  } else {
    it->second = symbol_mapping.STypeOutSymbol();
  }
}

void PitSymbolMap::OnRecord(const Record& record) {
  if (record.RType() == RType::SymbolMapping) {
    // Version compat
    if (record.Header().Size() >= sizeof(SymbolMappingMsgV2)) {
      OnSymbolMapping(record.Get<SymbolMappingMsgV2>());
    } else {
      OnSymbolMapping(record.Get<SymbolMappingMsgV1>());
    }
  }
}

// Explicit instantiation
template void PitSymbolMap::OnSymbolMapping(const SymbolMappingMsgV1& symbol_mapping);
template void PitSymbolMap::OnSymbolMapping(const SymbolMappingMsgV2& symbol_mapping);
