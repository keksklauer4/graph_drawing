#ifndef __GD_INCREMENTAL_COLLINEAR_HPP__
#define __GD_INCREMENTAL_COLLINEAR_HPP__

#include <gd_types.hpp>
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
    typedef MultiMap<point_id_t, pointid_pair_t> CollinearTripletMMap;
    public:
      IncrementalCollinear(const Instance& instance,
        const VertexAssignment& assignment, CollinearFunction& collFunc);

      void place(vertex_t vertex, point_id_t point);
      void deplace(vertex_t vertex);

      bool isValidCandidate(vertex_t vertex, point_id_t p, bool ignore_point_coll = false);
      // add check whether point itself is colinear with already set point

      bool isPointInvalid(point_id_t p) const
      { return m_invalidPoints.contains(p) && m_invalidPoints.find(p)->second > 0; }

      bool findCollinear(const point_pair_t& line, bool fix_edge = false);

      RangeIterator<CollinearTripletMMap::const_iterator> getCollinearTriplets(point_id_t p)
      { return m_collinearLines.equal_range(p); }

    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;
      CollinearFunction& m_collFunc;

      Map<point_id_t, size_t> m_invalidPoints;
      MultiMap<pointid_pair_t, point_id_t> m_invalidities; // from line to points

      CollinearTripletMMap m_collinearLines;
      Set<std::pair<point_id_t, pointid_pair_t>> m_collTriplet;
  };

}

#endif
