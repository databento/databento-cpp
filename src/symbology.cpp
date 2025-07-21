#include "databento/symbology.hpp"

#include <cstdint>
#include <memory>
#include <numeric>  // accumulate
#include <sstream>
#include <string>
#include <string_view>

#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "stream_op_helper.hpp"      // StreamOpBuilder

namespace databento {
TsSymbolMap SymbologyResolution::CreateSymbolMap() const {
  TsSymbolMap res;
  if (stype_in == SType::InstrumentId) {
    for (const auto& [iid_str, intervals] : mappings) {
      const auto iid = static_cast<std::uint32_t>(std::stoul(iid_str));
      for (const auto& interval : intervals) {
        res.Insert(iid, interval.start_date, interval.end_date,
                   std::make_shared<std::string>(interval.symbol));
      }
    }
  } else {
    for (const auto& [orig_symbol, intervals] : mappings) {
      auto symbol = std::make_shared<std::string>(orig_symbol);
      for (const auto& interval : intervals) {
        const auto iid = static_cast<std::uint32_t>(std::stoul(interval.symbol));
        res.Insert(iid, interval.start_date, interval.end_date, symbol);
      }
    }
  }
  return res;
}

std::string JoinSymbolStrings(const std::string& method_name,
                              std::vector<std::string>::const_iterator symbols_begin,
                              std::vector<std::string>::const_iterator symbols_end) {
  if (symbols_begin == symbols_end) {
    throw InvalidArgumentError{method_name, "symbols", "Cannot be empty"};
  }
  return std::accumulate(symbols_begin, symbols_end, std::string{},
                         [](std::string acc, const std::string& sym) {
                           return acc.empty() ? sym : std::move(acc) + ',' + sym;
                         });
}

std::string JoinSymbolStrings(const std::string& method_name,
                              const std::vector<std::string>& symbols) {
  if (symbols.empty()) {
    throw InvalidArgumentError{method_name, "symbols", "Cannot be empty"};
  }
  return JoinSymbolStrings(method_name, symbols.begin(), symbols.end());
}

std::string ToString(const SymbologyResolution& sym_res) { return MakeString(sym_res); }

std::ostream& operator<<(std::ostream& stream, const SymbologyResolution& sym_res) {
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
  for (const auto& [symbol, intervals] : sym_res.mappings) {
    // empty stream
    key_value.str("");
    std::ostringstream interval_ss;
    auto interval_helper = StreamOpBuilder{interval_ss}.SetSpacer(" ").Build();
    for (const auto& interval : intervals) {
      interval_helper.AddItem(interval);
    }
    mappings_helper.AddItem(static_cast<std::ostringstream&>(
        key_value_builder.Build()
            .AddItem(symbol)
            .AddItem(static_cast<std::ostringstream&>(interval_helper.Finish()))
            .Finish()));
  }
  stream_helper.AddField("mappings",
                         static_cast<std::ostringstream&>(mappings_helper.Finish()));

  intermediate.str("");
  auto partial_helper = intermediate_builder.SetIndent("").SetSpacer(" ").Build();
  for (const auto& symbol : sym_res.partial) {
    partial_helper.AddItem(symbol);
  }
  stream_helper.AddField("partial",
                         static_cast<std::ostringstream&>(partial_helper.Finish()));

  intermediate.str("");
  auto not_found_helper = intermediate_builder.Build();
  for (const auto& symbol : sym_res.not_found) {
    not_found_helper.AddItem(symbol);
  }
  return stream_helper
      .AddField("not_found",
                static_cast<std::ostringstream&>(not_found_helper.Finish()))
      .AddField("stype_in", sym_res.stype_in)
      .AddField("stype_out", sym_res.stype_out)
      .Finish();
}
}  // namespace databento
