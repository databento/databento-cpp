#include "databento/dbz.hpp"

#include <array>
#include <sstream>  // ostringstream
#include <utility>  // pair

#include "stream_op_helper.hpp"

namespace databento {
std::string ToString(const Metadata& metadata) { return MakeString(metadata); }
std::ostream& operator<<(std::ostream& stream, const Metadata& metadata) {
  auto helper = StreamOpBuilder{stream}
                    .SetTypeName("Metadata")
                    .SetSpacer("\n    ")
                    .Build()
                    .AddField("version", metadata.version)
                    .AddField("dataset", metadata.dataset)
                    .AddField("schema", metadata.schema)
                    .AddField("start", metadata.start)
                    .AddField("end", metadata.end)
                    .AddField("limit", metadata.limit)
                    .AddField("record_count", metadata.record_count)
                    .AddField("compression", metadata.compression)
                    .AddField("stype_in", metadata.stype_in)
                    .AddField("stype_out", metadata.stype_out);

  // format symbols, partial, and not_found
  constexpr auto kVecCount = 3;
  constexpr std::array<std::vector<std::string> Metadata::*, kVecCount>
      kStrVecs{&Metadata::symbols, &Metadata::partial, &Metadata::not_found};
  constexpr std::array<char const*, kVecCount> kStrVecNames{
      "symbols", "partial", "not_found"};
  for (std::size_t i = 0; i < kVecCount; ++i) {
    std::ostringstream vec_stream;
    auto vec_helper = StreamOpBuilder{vec_stream}.SetSpacer(" ").Build();
    for (const auto& str : metadata.*(kStrVecs[i])) {
      vec_helper.AddItem(str);
    }
    helper.AddField(kStrVecNames[i],
                    static_cast<std::ostringstream&>(vec_helper.Finish()));
  }

  // format mappings
  std::ostringstream mappings;
  auto mappings_helper =
      StreamOpBuilder{mappings}.SetIndent("    ").SetSpacer("\n    ").Build();
  for (const auto& mapping : metadata.mappings) {
    mappings_helper.AddItem(mapping);
  }
  return helper
      .AddField("mappings",
                static_cast<std::ostringstream&>(mappings_helper.Finish()))
      .Finish();
}

std::string ToString(const SymbolMapping& mapping) {
  return MakeString(mapping);
}
std::ostream& operator<<(std::ostream& stream, const SymbolMapping& mapping) {
  std::ostringstream intervals;
  auto intervals_helper = StreamOpBuilder{intervals}.SetSpacer(" ").Build();
  for (const auto& interval : mapping.intervals) {
    intervals_helper.AddItem(interval);
  }
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("SymbolMapping")
      .Build()
      .AddField("native", mapping.native)
      .AddField("intervals",
                static_cast<std::ostringstream&>(intervals_helper.Finish()))
      .Finish();
}

std::string ToString(const MappingInterval& interval) {
  return MakeString(interval);
}
std::ostream& operator<<(std::ostream& stream,
                         const MappingInterval& interval) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("MappingInterval")
      .Build()
      .AddField("start_date", interval.start_date)
      .AddField("end_date", interval.end_date)
      .AddField("symbol", interval.symbol)
      .Finish();
}
}  // namespace databento
