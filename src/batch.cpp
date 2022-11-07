#include "databento/batch.hpp"

#include <sstream>

#include "stream_op_helper.hpp"

namespace databento {
std::string ToString(const BatchJob& batch_job) {
  return MakeString(batch_job);
}

std::ostream& operator<<(std::ostream& stream, const BatchJob& batch_job) {
  std::ostringstream symbol_stream;
  auto symbol_helper = StreamOpBuilder{symbol_stream}.SetSpacer(" ").Build();
  for (const auto& symbol : batch_job.symbols) {
    symbol_helper.AddItem(symbol);
  }
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("BatchJob")
      .Build()
      .AddField("id", batch_job.id)
      .AddField("user_id", batch_job.user_id)
      .AddField("bill_id", batch_job.bill_id)
      .AddField("cost", batch_job.cost)
      .AddField("dataset", batch_job.dataset)
      .AddField("symbols",
                static_cast<std::ostringstream&>(symbol_helper.Finish()))
      .AddField("stype_in", batch_job.stype_in)
      .AddField("stype_out", batch_job.stype_out)
      .AddField("schema", batch_job.schema)
      .AddField("start", batch_job.start)
      .AddField("end", batch_job.end)
      .AddField("limit", batch_job.limit)
      .AddField("encoding", batch_job.encoding)
      .AddField("compression", batch_job.compression)
      .AddField("split_duration", batch_job.split_duration)
      .AddField("split_size", batch_job.split_size)
      .AddField("split_symbols", batch_job.split_symbols)
      .AddField("packaging", batch_job.packaging)
      .AddField("delivery", batch_job.delivery)
      .AddField("is_full_book", batch_job.is_full_book)
      .AddField("is_example", batch_job.is_example)
      .AddField("record_count", batch_job.record_count)
      .AddField("billed_size", batch_job.billed_size)
      .AddField("actual_size", batch_job.actual_size)
      .AddField("package_size", batch_job.package_size)
      .AddField("state", batch_job.state)
      .AddField("ts_received", batch_job.ts_received)
      .AddField("ts_queued", batch_job.ts_queued)
      .AddField("ts_process_start", batch_job.ts_process_start)
      .AddField("ts_process_done", batch_job.ts_process_done)
      .AddField("ts_expiration", batch_job.ts_expiration)
      .Finish();
}
}  // namespace databento
