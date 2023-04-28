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
      .AddField("last_modified_date", condition_detail.last_modified_date)
      .Finish();
}

std::string ToString(const DatasetRange& dataset_range) {
  return MakeString(dataset_range);
}
std::ostream& operator<<(std::ostream& stream,
                         const DatasetRange& dataset_range) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("DatasetRange")
      .Build()
      .AddField("start_date", dataset_range.start_date)
      .AddField("end_date", dataset_range.end_date)
      .Finish();
}
}  // namespace databento
