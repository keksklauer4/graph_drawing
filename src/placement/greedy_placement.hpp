#ifndef __GD__GREEDY_PLACEMENT_HPP__
#define __GD__GREEDY_PLACEMENT_HPP__

#include "common/kd_tree.hpp"
#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <verification/incremental_collinear.hpp>
#include <verification/incremental_crossing.hpp>

namespace gd
{

  class GreedyPlacement
  {
    public:
      GreedyPlacement(const Instance& instance, PlacementVisualizer* vis = nullptr);

      const VertexAssignment& findPlacement();
      bool improve(size_t num_tries);
      size_t getNumCrossings() const;

    private:
      size_t placeInitial(vertex_t vertex, point_id_t target);
      point_id_t findEligiblePoint(vertex_t vertex);
      bool tryImprove(vertex_t candidate);
      bool circularRebuild(const KdTree& kdtree, Vector<VertexPointPair>& destructed, vertex_t candidate);

    private:
      const Instance& m_instance;
      VertexAssignment m_assignment;
      PlacementVisualizer* m_visualizer;

      CollinearCheck m_collChecker;
      IncrementalCollinear m_incrementalCollinearity;
      IncrementalCrossing m_incrementalCrossing;

      Set<point_id_t> m_unused;
      Vector<Point> m_mappedNeighbors;
  };

}



#endif
