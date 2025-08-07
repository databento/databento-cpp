#include "databento/batch.hpp"

#include <algorithm>  // find_if
#include <iostream>

#include "databento/historical.hpp"
#include "databento/publishers.hpp"  // Dataset

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKeyFromEnv().Build();

  const auto job =
      client.BatchSubmitJob(db::ToString(db::Dataset::GlbxMdp3), {"GEZ2"},
                            db::Schema::Trades, {"2022-08-26", "2022-09-27"});
  const auto all_jobs = client.BatchListJobs();

  const auto all_job_it =
      std::find_if(all_jobs.begin(), all_jobs.end(),
                   [&job](const db::BatchJob& a_job) { return job.id == a_job.id; });

  if (all_job_it == all_jobs.end()) {
    std::cout << "Couldn't find submitted job\n";
  } else {
    std::cout << "Found submitted job: " << *all_job_it << '\n';
  }

  return 0;
}
