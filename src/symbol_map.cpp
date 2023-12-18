#include "databento/symbol_map.hpp"

#include "databento/compat.hpp"

using databento::PitSymbolMap;

template <typename SymbolMappingRec>
void PitSymbolMap::OnSymbolMapping(const SymbolMappingRec& symbol_mapping) {
  const auto it = map_.find(symbol_mapping.hd.instrument_id);
  if (it == map_.end()) {
    map_.emplace(symbol_mapping.hd.instrument_id,
                 symbol_mapping.STypeOutSymbol());
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
template void PitSymbolMap::OnSymbolMapping(
    const SymbolMappingMsgV1& symbol_mapping);
template void PitSymbolMap::OnSymbolMapping(
    const SymbolMappingMsgV2& symbol_mapping);
