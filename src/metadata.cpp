#include "databento/metadata.hpp"

#include <ostream>

#include "stream_op_helper.hpp"

namespace databento {
std::string ToString(const PublisherDetail& publisher_detail) {
  return MakeString(publisher_detail);
}
std::ostream& operator<<(std::ostream& stream,
                         const PublisherDetail& publisher_detail) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("PublisherDetail")
      .Build()
      .AddField("publisher_id", publisher_detail.publisher_id)
      .AddField("dataset", publisher_detail.dataset)
      .AddField("venue", publisher_detail.venue)
      .AddField("description", publisher_detail.description)
      .Finish();
}

std::string ToString(const FieldDetail& field_detail) {
  return MakeString(field_detail);
}
std::ostream& operator<<(std::ostream& stream,
                         const FieldDetail& field_detail) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("FieldDetail")
      .Build()
      .AddField("name", field_detail.name)
      .AddField("type", field_detail.type)
      .Finish();
}

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
      .AddField("start", dataset_range.start)
      .AddField("end", dataset_range.end)
      .Finish();
}
}  // namespace databento
