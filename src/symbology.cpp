#include "databento/symbology.hpp"

#include <numeric>  // accumulate
#include <sstream>

#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "stream_op_helper.hpp"      // StreamOpBuilder

namespace databento {
std::string JoinSymbolStrings(const std::string& method_name,
                              const std::vector<std::string>& symbols) {
  if (symbols.empty()) {
    throw InvalidArgumentError{method_name, "symbols", "Cannot be empty"};
  }
  return std::accumulate(symbols.begin(), symbols.end(), std::string{},
                         [](std::string acc, const std::string& sym) {
                           return acc.empty() ? sym
                                              : std::move(acc) + ',' + sym;
                         });
}

std::string ToString(const StrMappingInterval& mapping_interval) {
  return MakeString(mapping_interval);
}

std::string ToString(const SymbologyResolution& sym_res) {
  return MakeString(sym_res);
}

std::ostream& operator<<(std::ostream& stream,
                         const StrMappingInterval& mapping_interval) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("StrMappingInterval")
      .Build()
      .AddField("start_date", mapping_interval.start_date)
      .AddField("end_date", mapping_interval.end_date)
      .AddField("symbol", mapping_interval.symbol)
      .Finish();
}

std::ostream& operator<<(std::ostream& stream,
                         const SymbologyResolution& sym_res) {
  auto stream_helper = StreamOpBuilder{stream}
                           .SetSpacer("\n    ")
                           .SetTypeName("SymbologyResolution")
                           .Build();
  std::ostringstream intermediate;
  std::ostringstream key_value;
  auto intermediate_builder =
      StreamOpBuilder{intermediate}.SetIndent("    ").SetSpacer("\n    ");
  auto mappings_helper = intermediate_builder.Build();
  auto key_value_builder = StreamOpBuilder{key_value}.SetSpacer(" ");
  for (const auto& mapping : sym_res.mappings) {
    // empty stream
    key_value.str("");
    std::ostringstream intervals;
    auto interval_helper = StreamOpBuilder{intervals}.SetSpacer(" ").Build();
    for (const auto& interval : mapping.second) {
      interval_helper.AddItem(interval);
    }
    mappings_helper.AddItem(static_cast<std::ostringstream&>(
        key_value_builder.Build()
            .AddItem(mapping.first)
            .AddItem(static_cast<std::ostringstream&>(interval_helper.Finish()))
            .Finish()));
  }
  stream_helper.AddField(
      "mappings", static_cast<std::ostringstream&>(mappings_helper.Finish()));

  intermediate.str("");
  auto partial_helper =
      intermediate_builder.SetIndent("").SetSpacer(" ").Build();
  for (const auto& symbol : sym_res.partial) {
    partial_helper.AddItem(symbol);
  }
  stream_helper.AddField(
      "partial", static_cast<std::ostringstream&>(partial_helper.Finish()));

  intermediate.str("");
  auto not_found_helper = intermediate_builder.Build();
  for (const auto& symbol : sym_res.not_found) {
    not_found_helper.AddItem(symbol);
  }
  stream_helper.AddField(
      "not_found", static_cast<std::ostringstream&>(not_found_helper.Finish()));

  return stream_helper.Finish();
}
}  // namespace databento
