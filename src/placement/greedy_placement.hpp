#ifndef __GD__GREEDY_PLACEMENT_HPP__
#define __GD__GREEDY_PLACEMENT_HPP__

#include <common/kd_tree.hpp>
#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <verification/incremental_collinear.hpp>
#include <verification/incremental_crossing.hpp>
#include <placement/vertex_order.hpp>

#include <common/random_gen.hpp>

namespace gd
{

  class GreedyPlacement
  {
    public:
      GreedyPlacement(const Instance& instance, VertexOrder& order, PlacementVisualizer* vis = nullptr);

      const VertexAssignment& findPlacement();
      bool improve(size_t num_tries);
      size_t getNumCrossings() const;

    private:
      size_t placeInitial(vertex_t vertex, point_id_t target);
      point_id_t findEligiblePoint(vertex_t vertex);
      bool tryImprove(vertex_t candidate);
      bool circularRebuild(const KdTree& kdtree, Vector<VertexPointPair>& destructed, vertex_t candidate);

      bool improve_locally(LocalGurobi& optimizer, LocalImprovementFunctor& functor,
                           vertex_t v, point_id_t p);


      bool rebuild_collinear(vertex_t v);
      void collinear_neighbors(vertex_t v, VertexVector& vec, MultiMap<vertex_t, point_id_t>& mapping);
      void collinear_tedious(point_id_t p, VertexVector& vec, MultiMap<vertex_t, point_id_t>& mapping, VertexSet& vset);
      void find_collinear_points(PointIdVector& points, vertex_t v);
      bool rebuild(VertexVector& vec);

      void visualize_rebuild();

    private:
      const Instance& m_instance;
      VertexOrder& m_order;
      VertexAssignment m_assignment;
      PlacementVisualizer* m_visualizer;

      CollinearCheck m_collChecker;
      IncrementalCollinear m_incrementalCollinearity;
      IncrementalCrossing m_incrementalCrossing;

      Vector<Point> m_mappedNeighbors;

      RandomGen m_random;
  };

}



#endif
