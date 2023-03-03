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

std::string ToString(const DatasetConditionInfo& condition) {
  return MakeString(condition);
}
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditionInfo& condition) {
  std::ostringstream details_stream;
  auto details_helper = StreamOpBuilder{details_stream}
                            .SetSpacer("\n    ")
                            .SetIndent("    ")
                            .Build();
  for (const auto& detail : condition.details) {
    details_helper.AddItem(detail);
  }
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("DatasetConditionInfo")
      .Build()
      .AddField("condition", condition.condition)
      .AddField("details",
                static_cast<std::ostringstream&>(details_helper.Finish()))
      .AddField("adjusted_start_date", condition.adjusted_start_date)
      .AddField("adjusted_end_date", condition.adjusted_end_date)
      .AddField("available_start_date", condition.available_start_date)
      .AddField("available_end_date", condition.available_end_date)
      .Finish();
}
}  // namespace databento
