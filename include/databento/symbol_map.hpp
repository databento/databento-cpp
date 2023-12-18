#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "databento/compat.hpp"
#include "databento/record.hpp"

namespace databento {
// A point-in-time symbol map. Useful for working with live symbology or
// a historical request over a single day or other situations where the
// symbol mappings are known not to change.
class PitSymbolMap {
 public:
  using Store = std::unordered_map<std::uint32_t, std::string>;

  PitSymbolMap() = default;

  bool IsEmpty() const { return map_.empty(); }
  std::size_t Size() const { return map_.size(); }
  const Store& Map() const { return map_; }
  Store& Map() { return map_; }
  Store::const_iterator Find(std::uint32_t instrument_id) const {
    return map_.find(instrument_id);
  }
  std::string& operator[](std::uint32_t instrument_id) {
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
