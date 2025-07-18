#include "databento/batch.hpp"

#include <sstream>

#include "stream_op_helper.hpp"

namespace databento {
std::string ToString(const BatchJob& batch_job) { return MakeString(batch_job); }

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
      .AddField("cost_usd", batch_job.cost_usd)
      .AddField("dataset", batch_job.dataset)
      .AddField("symbols", static_cast<std::ostringstream&>(symbol_helper.Finish()))
      .AddField("stype_in", batch_job.stype_in)
      .AddField("stype_out", batch_job.stype_out)
      .AddField("schema", batch_job.schema)
      .AddField("start", batch_job.start)
      .AddField("end", batch_job.end)
      .AddField("limit", batch_job.limit)
      .AddField("encoding", batch_job.encoding)
      .AddField("compression", batch_job.compression)
      .AddField("pretty_px", batch_job.pretty_px)
      .AddField("pretty_ts", batch_job.pretty_ts)
      .AddField("map_symbols", batch_job.map_symbols)
      .AddField("split_duration", batch_job.split_duration)
      .AddField("split_size", batch_job.split_size)
      .AddField("split_symbols", batch_job.split_symbols)
      .AddField("delivery", batch_job.delivery)
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

std::string ToString(const BatchFileDesc& file_desc) { return MakeString(file_desc); }

std::ostream& operator<<(std::ostream& stream, const BatchFileDesc& file_desc) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("BatchFileDesc")
      .Build()
      .AddField("filename", file_desc.filename)
      .AddField("size", file_desc.size)
      .AddField("hash", file_desc.hash)
      .AddField("https_url", file_desc.https_url)
      .AddField("ftp_url", file_desc.ftp_url)
      .Finish();
}
}  // namespace databento
