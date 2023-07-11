#include "time_tracker.hpp"
#include "gd_types.hpp"

#include <cassert>
#include <chrono>
#include <common/misc.hpp>

using namespace gd;

#define MTIMER_CROSSING 0ull
#define MTIMER_COLLINEAR 1ull
#define MTIMER_LOCAL_SAT 2ull
#define MTIMER_MOVE_OP 3ull
#define MTIMER_INITIAL_PLACEMENT 4ull
#define MTIMER_COLLINEAR_REBUILD 5ull
#define MTIMER_GUROBI 6ull
#define MTIMER_GUROBI_BUILD_MODEL 7ull

#define TOTAL_NUM_TIMERS 8ull

namespace
{
  size_t get_nano_seconds()
  {
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now())
      .time_since_epoch().count();
  }
}

TimeTracker::TimeTracker(size_t limit_ms)
  : m_time_limit_ms(limit_ms), m_start(std::chrono::system_clock::now())
{
  m_timerStart.insert(m_timerStart.begin(),
    TOTAL_NUM_TIMERS, UINT_UNDEF);
  m_timerAccumulated.resize(TOTAL_NUM_TIMERS);
}

void TimeTracker::set_time_limit(size_t limit_ms) const
{
  m_time_limit_ms = limit_ms;
  m_start = std::chrono::system_clock::now();
}

double TimeTracker::get_fraction_time_limit() const
{
  if (m_time_limit_ms == 0) return 1;
  return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_start).count())
    / static_cast<double>(m_time_limit_ms);
}

bool TimeTracker::time_limit() const
{
  return m_time_limit_ms <= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_start).count();
}

void TimeTracker::start_timer(size_t time_idx) const
{
  assert(!isDefined(m_timerStart[time_idx]) && "Timer was already started...");
  m_timerStart[time_idx] = get_nano_seconds();
}


void TimeTracker::end_timer(size_t time_idx) const
{
  assert(isDefined(m_timerStart[time_idx]) && "Timer was not yet started...");
  m_timerAccumulated[time_idx] += get_nano_seconds() - m_timerStart[time_idx];
  m_timerStart[time_idx] = UINT_UNDEF;
}

void TimeTracker::toggle_timer(size_t time_idx) const
{
  if (!isDefined(m_timerStart.at(time_idx))) start_timer(time_idx);
  else end_timer(time_idx);
}

size_t TimeTracker::get_timer_val(size_t time_idx) const
{
  return m_timerAccumulated.at(time_idx);
}

void TimeTracker::timer_crossing() const { toggle_timer(MTIMER_CROSSING); }
void TimeTracker::timer_collinear() const { toggle_timer(MTIMER_COLLINEAR); }
void TimeTracker::timer_local_sat() const { toggle_timer(MTIMER_LOCAL_SAT); }
void TimeTracker::timer_move_op() const { toggle_timer(MTIMER_MOVE_OP); }
void TimeTracker::timer_initial_placement() const { toggle_timer(MTIMER_INITIAL_PLACEMENT); }
void TimeTracker::timer_collinear_rebuild() const { toggle_timer(MTIMER_COLLINEAR_REBUILD); }
void TimeTracker::timer_gurobi() const { toggle_timer(MTIMER_GUROBI); }
void TimeTracker::timer_gurobi_model() const { toggle_timer(MTIMER_GUROBI_BUILD_MODEL); }

#define OUTPUT_TIMER(name, val) os << name << static_cast<double>(val)/1e6f << std::endl;

std::ostream& gd::operator<<(std::ostream& os, const TimeTracker& timer)
{
  os << "===========================\n";
  os << "Timer stats:\n";
  OUTPUT_TIMER("Crossing timer:     ", timer.get_timer_val(MTIMER_CROSSING));
  OUTPUT_TIMER("Collinear timer:    ", timer.get_timer_val(MTIMER_COLLINEAR));
  OUTPUT_TIMER("Local SAT timer:    ", timer.get_timer_val(MTIMER_LOCAL_SAT));
  OUTPUT_TIMER("Move Op timer:      ", timer.get_timer_val(MTIMER_MOVE_OP));
  OUTPUT_TIMER("Initial pl timer:   ", timer.get_timer_val(MTIMER_INITIAL_PLACEMENT));
  OUTPUT_TIMER("Coll-rebuild timer: ", timer.get_timer_val(MTIMER_COLLINEAR_REBUILD));
  OUTPUT_TIMER("Gurobi timer:       ", timer.get_timer_val(MTIMER_GUROBI));
  OUTPUT_TIMER("Gurobi model timer: ", timer.get_timer_val(MTIMER_GUROBI_BUILD_MODEL));
  os << "===========================\n";
  return os;
}