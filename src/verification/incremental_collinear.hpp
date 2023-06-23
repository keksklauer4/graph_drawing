#ifndef __GD_INCREMENTAL_COLLINEAR_HPP__
#define __GD_INCREMENTAL_COLLINEAR_HPP__

#include "gd_types.hpp"
#include <common/instance.hpp>

namespace gd
{
  // function object to keep the incremental collinear class generic
  struct CollinearFunction
  {
    virtual bool operator()(const Point&) = 0;
  };

  struct CollinearCheck : CollinearFunction
  {
    CollinearCheck(): collinear(false) {}

    bool operator()(const Point&) override
    {
      collinear = true;
      return false; // continue? return false to stop
    }

    bool collinear;
  };



  class IncrementalCollinear
  {
    public:
      IncrementalCollinear(const Instance& instance,
        const VertexAssignment& assignment, CollinearFunction& collFunc);

      // add check whether point itself is colinear with already set point
      void findCollinear(const line_2d_t& line, bool fix_edge = false);
      bool isPointInvalid(point_id_t p) const { return m_invalidPoints.contains(p); }

    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;
      CollinearFunction& m_collFunc;

      Set<point_id_t> m_invalidPoints;
  };

}

#endif
