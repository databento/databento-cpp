#include "databento/batch.hpp"

#include <algorithm>  // find_if
#include <iostream>

#include "databento/constants.hpp"
#include "databento/historical.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();

  const auto job =
      client.BatchSubmitJob(databento::dataset::kGlbxMdp3, {"GEZ2"},
                            databento::Schema::Trades, {"2022-08-26", "2022-09-27"});
  const auto all_jobs = client.BatchListJobs();

  const auto all_job_it = std::find_if(
      all_jobs.begin(), all_jobs.end(),
      [&job](const databento::BatchJob& a_job) { return job.id == a_job.id; });

  if (all_job_it == all_jobs.end()) {
    std::cout << "Couldn't find submitted job\n";
  } else {
    std::cout << "Found submitted job: " << *all_job_it << '\n';
  }

  return 0;
}
