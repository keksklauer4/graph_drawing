#ifndef __GD_TIME_TRACKER_HPP__
#define __GD_TIME_TRACKER_HPP__

#include <gd_types.hpp>
#include <ostream>

namespace gd
{

  class TimeTracker
  {
    public:
      TimeTracker(size_t limit_ms = 1e9);

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

    private:
      void toggle_timer(size_t time_idx) const;
      void start_timer(size_t time_idx) const;
      void end_timer(size_t time_idx) const;

    private:
      mutable Vector<size_t> m_timerStart;
      mutable Vector<size_t> m_timerAccumulated;

      size_t m_time_limit_ms;
      std::chrono::time_point<std::chrono::system_clock> m_start;
  };

  std::ostream& operator<<(std::ostream& os, const TimeTracker& timer);

}


#endif
