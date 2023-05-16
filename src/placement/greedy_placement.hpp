#ifndef __GD__GREEDY_PLACEMENT_HPP__
#define __GD__GREEDY_PLACEMENT_HPP__

#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <verification/incremental_collinear.hpp>
#include <verification/incremental_crossing.hpp>

namespace gd
{

  class GreedyPlacement
  {
    public:
      GreedyPlacement(const Instance& instance);

      const VertexAssignment& findPlacement();
      size_t getNumCrossings() const;

    private:
      point_id_t findEligiblePoint(vertex_t vertex);

    private:
      const Instance& m_instance;
      VertexAssignment m_assignment;

      CollinearCheck m_collChecker;
      IncrementalCollinear m_incrementalCollinearity;
      IncrementalCrossing m_incrementalCrossing;

      Set<point_id_t> m_unused;
      Vector<Point> m_mappedNeighbors;
  };

}



#endif
