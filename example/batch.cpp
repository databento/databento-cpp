#include "databento/batch.hpp"

#include <algorithm>  // find_if
#include <iostream>

#include "databento/historical.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.keyFromEnv().Build();

  const auto job = client.BatchSubmitJob("GLBX.MDP3", databento::Schema::Trades,
                                         {"GE"}, "2022-08-26", "2022-09-27");
  const auto all_jobs = client.BatchListJobs();

  const auto all_job_it = std::find_if(
      all_jobs.begin(), all_jobs.end(),
      [&job](const databento::BatchJob& a_job) { return job.id == a_job.id; });

  if (all_job_it == all_jobs.end()) {
    std::cout << "Couldn't find submitted job" << std::endl;
  } else {
    std::cout << "Found submitted job with ID " << all_job_it->id << std::endl;
  }

  return 0;
}
