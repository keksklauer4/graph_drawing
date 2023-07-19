#ifndef __GD__GREEDY_PLACEMENT_HPP__
#define __GD__GREEDY_PLACEMENT_HPP__

#include <common/crossing_hierarchy.hpp>
#include <cstddef>
#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <verification/incremental_collinear.hpp>
#include <verification/incremental_crossing.hpp>
#include <placement/vertex_order.hpp>

#include <common/random_gen.hpp>

namespace gd
{
  struct LocalImprovementToolset;

  class GreedyPlacement
  {
    public:
      GreedyPlacement(const Instance& instance, VertexOrder& order, bool fasterImprove = true, PlacementVisualizer* vis = nullptr);
      ~GreedyPlacement();

      void start_placement(const VertexAssignment& assignment);
      const VertexAssignment& findPlacement();
      bool improve(size_t num_tries);
      size_t getNumCrossings() const;
      const VertexAssignment& getAssignment() const { return m_assignment; }

    private:
      size_t placeInitial(vertex_t vertex, point_id_t target);
      point_id_t findEligiblePoint(vertex_t vertex);
      bool tryImprove(vertex_t candidate);
      bool fasterImprove(vertex_t candidate);
      void checkNearestNeighborMoves(vertex_t candidate, Point around, size_t& best_crossings, point_id_t& best_point);
      bool circularRebuild(const KdTree& kdtree, Vector<VertexPointPair>& destructed, vertex_t candidate);

      bool improve_locally(LocalGurobi& optimizer, LocalImprovementFunctor& functor,
                           vertex_t v, point_id_t p);


      bool rebuild_collinear(vertex_t v);
      void collinear_neighbors(vertex_t v, VertexVector& vec, MultiMap<vertex_t, point_id_t>& mapping);
      void collinear_tedious(point_id_t p, VertexVector& vec, MultiMap<vertex_t, point_id_t>& mapping, VertexSet& vset);
      void find_collinear_points(PointIdVector& points, vertex_t v);
      bool rebuild(VertexVector& vec);

      void visualize_rebuild();

      void calibrate_improve_iterations(vertex_t candidate);

      void rollback_reopt_bug(bool infeasible);

      void run_reopt(vertex_t candidate);

    private:
      const Instance& m_instance;
      VertexOrder& m_order;
      VertexAssignment m_assignment;
      CrossingHierarchy m_crossingHierarchy;
      PlacementVisualizer* m_visualizer;

      CollinearCheck m_collChecker;
      IncrementalCollinear m_incrementalCollinearity;
      IncrementalCrossing m_incrementalCrossing;

      Vector<Point> m_mappedNeighbors;

      RandomGen m_random;

      LocalImprovementToolset* m_localImprovementToolset;

      size_t m_numImprovementIters;

      KdTree* m_kdtree;
      PointIdSet m_checkedPoints;
      Vector<VertexPointPair> m_temporaryMapping;

      bool m_fasterImprove;
  };

}



#endif
