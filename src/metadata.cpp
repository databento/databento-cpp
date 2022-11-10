#include "databento/metadata.hpp"

#include <sstream>

#include "stream_op_helper.hpp"

namespace databento {
std::string ToString(const DatasetConditionDetail& condition_detail) {
  return MakeString(condition_detail);
}
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditionDetail& condition_detail) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("DatasetConditionDetail")
      .Build()
      .AddField("date", condition_detail.date)
      .AddField("condition", condition_detail.condition)
      .Finish();
}

std::string ToString(const DatasetConditions& conditions) {
  return MakeString(conditions);
}
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditions& conditions) {
  std::ostringstream details_stream;
  auto details_helper = StreamOpBuilder{details_stream}
                            .SetSpacer("\n    ")
                            .SetIndent("    ")
                            .Build();
  for (const auto& detail : conditions.details) {
    details_helper.AddItem(detail);
  }
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("DatasetConditions")
      .Build()
      .AddField("condition", conditions.condition)
      .AddField("details",
                static_cast<std::ostringstream&>(details_helper.Finish()))
      .AddField("adjusted_start_date", conditions.adjusted_start_date)
      .AddField("adjusted_end_date", conditions.adjusted_end_date)
      .Finish();
}
}  // namespace databento
