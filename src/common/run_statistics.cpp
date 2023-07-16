#include "run_statistics.hpp"
#include <cstddef>
#include <utility>

using namespace gd;

#include <common/time_tracker.hpp>


#define MTIMER_CROSSING 0ull
#define MTIMER_COLLINEAR 1ull
#define MTIMER_LOCAL_SAT 2ull
#define MTIMER_MOVE_OP 3ull
#define MTIMER_INITIAL_PLACEMENT 4ull
#define MTIMER_COLLINEAR_REBUILD 5ull
#define MTIMER_GUROBI 6ull
#define MTIMER_GUROBI_BUILD_MODEL 7ull

RunStatistics::RunStatistics(const TimeTracker& timer, const size_t num_nodes, const size_t num_edges, const size_t num_points)
  : m_timer(timer), m_curr_placement_idx(0), m_num_runs(0), m_num_nodes(num_nodes), m_num_edges(num_edges), m_num_points(num_points)
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


const size_t& RunStatistics::get_m_num_runs() const
{
  return m_num_runs;
}

size_t RunStatistics::get_num_m_init_placement_results() const
{
  return m_init_placement_results.size();
}

const std::pair<bool, size_t>& RunStatistics::get_m_init_placement_results_i(int i) const
{
  return m_init_placement_results[i];
}

size_t RunStatistics::get_num_m_progress() const
{
  return m_progress.size();
}

const CurrentProgress& RunStatistics::get_m_progress_i(int i) const
{
  return m_progress[i];
}

size_t RunStatistics::get_num_m_reopt_results() const
{
  return m_reopt_results.size();
}

const std::pair<LocalReoptFunctors, int64_t>& RunStatistics::get_m_reopt_results_i(int i) const
{
  return m_reopt_results[i];
}

size_t RunStatistics::get_time_CR() const
{
  return m_timer.get_timer_val(MTIMER_CROSSING);
}

size_t RunStatistics::get_time_COLL() const
{
  return m_timer.get_timer_val(MTIMER_COLLINEAR);
}

size_t RunStatistics::get_time_LOCAL_SAT() const
{
  return m_timer.get_timer_val(MTIMER_LOCAL_SAT);
}

size_t RunStatistics::get_time_MOVE_OP() const
{
  return m_timer.get_timer_val(MTIMER_MOVE_OP);
}

size_t RunStatistics::get_time_INIT() const
{
  return m_timer.get_timer_val(MTIMER_INITIAL_PLACEMENT);
}

size_t RunStatistics::get_time_COLL_RE() const
{
  return m_timer.get_timer_val(MTIMER_COLLINEAR_REBUILD);
}

size_t RunStatistics::get_time_GUROBI() const
{
  return m_timer.get_timer_val(MTIMER_GUROBI);
}

size_t RunStatistics::get_time_GUROBI_BUILD() const
{
  return m_timer.get_timer_val(MTIMER_GUROBI_BUILD_MODEL);
}

size_t RunStatistics::get_num_nodes() const
{
  return m_num_nodes;
}

size_t RunStatistics::get_num_edges() const
{
  return m_num_edges;
}

size_t RunStatistics::get_num_points() const
{
  return m_num_points;
}