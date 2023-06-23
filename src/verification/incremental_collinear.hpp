#ifndef __GD_INCREMENTAL_COLLINEAR_HPP__
#define __GD_INCREMENTAL_COLLINEAR_HPP__

#include <common/instance.hpp>

namespace gd
{
  // function object to keep the incremental collinear class generic
  struct CollinearFunction
  {
    virtual bool operator()(const Point&) = 0;
  };

  struct CollinearCheck
  {
    CollinearCheck(): collinear(false) {}

    bool operator()(const Point&)
    {
      collinear = true;
      return false; // return false to stop
    }

    bool collinear;
  };



  class IncrementalCollinear
  {
    public:
      IncrementalCollinear(const VertexAssignment& assignment, const Instance& instance, CollinearFunction& collFunc);

    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;
      CollinearFunction& m_collFunc;
  };

}

#endif
