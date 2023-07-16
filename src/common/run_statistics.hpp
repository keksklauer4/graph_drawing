#ifndef __GD_STATISTICS_HPP__
#define __GD_STATISTICS_HPP__

#include "common/time_tracker.hpp"
#include <cstddef>
#include <gd_types.hpp>

#define USE_STATISTICS

#define COMMA ,
#ifdef USE_STATISTICS
#define STATS(code) code
#else
#define STATS(code)
#endif

namespace gd
{
  enum LocalReoptFunctors : int
  {
    LOCAL_VERTEX_NEIGHBORS = 1,
    LOCAL_IMPROVEMENT_BOMB = 2,
    PERMUTATION_CROSSING = 3,
    PERMUTATION_CRATER = 4,
    COLLINEAR_REBUILD = 5
  };

  enum WorkType : int
  {
    INIT_PLACING = 0,
    MOVE_OP = 1,
    REOPT = 2,
    COLLINEARITY_REBUILD = 3,
    DEATH = 4
  };

  struct CurrentProgress
  {
    CurrentProgress() {}
    CurrentProgress(size_t placement_idx, size_t crossings, double time, WorkType work)
      : curr_placement_idx(placement_idx), curr_crossings(crossings), time_stamp(time), type(work) {}

    size_t curr_placement_idx;
    size_t curr_crossings;
    double time_stamp; // decimal in [0,1] (maybe slightly larger than 1)
    WorkType type;
  };

  class RunStatistics
  {
  public:
    RunStatistics(const TimeTracker& timer, const size_t num_nodes, const size_t num_edges, const size_t num_points);

    void new_run();
    void set_local_reopt_result(LocalReoptFunctors functor, int64_t delta);
    void set_timestamp_crossings(size_t num_crossings, WorkType type);
    void set_current_run(size_t idx);
    void set_initial_placement_quality(bool valid, size_t num_crossing);

    size_t get_num_m_reopt_results() const;
    size_t get_num_m_init_placement_results() const;
    size_t get_num_m_progress() const;
    const size_t& get_m_num_runs() const;
    const std::pair<bool, size_t>& get_m_init_placement_results_i(int i) const;
    const CurrentProgress& get_m_progress_i(int i) const;
    const std::pair<LocalReoptFunctors, int64_t>& get_m_reopt_results_i(int i) const;
    size_t get_time_CR() const;
    size_t get_time_COLL() const;
    size_t get_time_LOCAL_SAT() const;
    size_t get_time_MOVE_OP() const;
    size_t get_time_INIT() const;
    size_t get_time_COLL_RE() const;
    size_t get_time_GUROBI() const;
    size_t get_time_GUROBI_BUILD() const;
    size_t get_num_nodes() const;
    size_t get_num_edges() const;
    size_t get_num_points() const;

  private:
    const TimeTracker& m_timer;
    const size_t m_num_nodes;
    const size_t m_num_edges;
    const size_t m_num_points;
    Vector<std::pair<LocalReoptFunctors, int64_t>> m_reopt_results;
    Vector<std::pair<bool, size_t>> m_init_placement_results; // (valid, num_crossings);

    Vector<CurrentProgress> m_progress;

    size_t m_curr_placement_idx;
    size_t m_num_runs;
  };

}


#endif

