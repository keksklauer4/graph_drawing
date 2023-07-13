#ifndef __GD_TIME_TRACKER_HPP__
#define __GD_TIME_TRACKER_HPP__

#include <gd_types.hpp>
#include <ostream>

namespace gd
{
  enum TIMER_TYPES : int
  {
    ETIMER_CROSSING = 0,
    ETIMER_COLLINEAR = 1,
    ETIMER_LOCAL_SAT = 2,
    ETIMER_MOVE_OP = 3,
    ETIMER_INITIAL_PLACEMENT = 4,
    ETIMER_COLLINEAR_REBUILD = 5,
    ETIMER_GUROBI = 6,
    ETIMER_GUROBI_BUILD_MODEL = 7
  };

  class TimeTracker
  {
    public:
      TimeTracker(size_t limit_ms = 1e12);

      void set_time_limit(size_t limit_ms) const;

      double get_fraction_time_limit() const;

      void timer_crossing() const;
      void timer_collinear() const;
      void timer_local_sat() const;
      void timer_move_op() const;
      void timer_initial_placement() const;
      void timer_collinear_rebuild() const;
      void timer_gurobi() const;
      void timer_gurobi_model() const;

      bool time_limit() const;
      size_t get_timer_val(size_t time_idx) const;
      double get_timer_val_frac(size_t time_idx) const;

      void end_all_timers() const;

    private:
      void toggle_timer(size_t time_idx) const;
      void start_timer(size_t time_idx) const;
      void end_timer(size_t time_idx) const;

    private:
      mutable Vector<size_t> m_timerStart;
      mutable Vector<size_t> m_timerAccumulated;

      mutable size_t m_time_limit_ms;
      mutable std::chrono::time_point<std::chrono::system_clock> m_start;
  };

  std::ostream& operator<<(std::ostream& os, const TimeTracker& timer);

}


#endif
