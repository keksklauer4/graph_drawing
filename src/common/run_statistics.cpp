#include "run_statistics.hpp"
#include <utility>

using namespace gd;

#include <common/time_tracker.hpp>


RunStatistics::RunStatistics(const TimeTracker& timer)
  : m_timer(timer), m_curr_placement_idx(0), m_num_runs(0)
{
  m_reopt_results.reserve(256);
  m_init_placement_results.reserve(32);
  m_progress.reserve(66536);
}


void RunStatistics::new_run()
{
  m_curr_placement_idx = m_num_runs++;
}

void RunStatistics::set_local_reopt_result(LocalReoptFunctors functor, int64_t delta)
{
  m_reopt_results.push_back(std::make_pair(functor, delta));
}

void RunStatistics::set_timestamp_crossings(size_t num_crossings, WorkType type)
{
  m_progress.push_back(CurrentProgress{m_curr_placement_idx, num_crossings,
    m_timer.get_fraction_time_limit(), type});
}

void RunStatistics::set_current_run(size_t idx)
{ m_curr_placement_idx = idx; }

void RunStatistics::set_initial_placement_quality(bool valid, size_t num_crossing)
{
  m_init_placement_results.push_back(std::make_pair(valid, num_crossing));
}

