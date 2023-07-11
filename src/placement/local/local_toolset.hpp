#ifndef __GD_LOCAL_TOOLSET_HPP__
#define __GD_LOCAL_TOOLSET_HPP__

#include "common/kd_tree.hpp"
#include "common/random_gen.hpp"
#include "gd_types.hpp"
#include <common/assignment.hpp>
#include <verification/incremental_collinear.hpp>
#include <placement/local/local_functors.hpp>
#include <placement/local/local_gurobi.hpp>
#include <placement/local/permutation_functor.hpp>

namespace gd
{
  struct LocalImprovementToolset
  {
    LocalImprovementToolset(const Instance& instance,
      const VertexAssignment& assignment,IncrementalCollinear& collinearity)
      : m_kdtree(KdTree{instance.m_points}), optimizer(instance, assignment),
        m_bombFunctor(instance, assignment, m_kdtree, collinearity),
        m_vertexNeighborFunctor(instance, assignment, m_kdtree, collinearity),
        m_permutationCrater(instance, assignment, collinearity, m_kdtree),
        m_permutationCrossing(instance, assignment, collinearity) {}

    LocalImprovementFunctor& getRandomFunctor()
    {
      switch(2)//m_random.getRandomUint(3))
      {
        case 0: return *reinterpret_cast<LocalImprovementFunctor*>(&m_bombFunctor);
        case 1: return *reinterpret_cast<LocalImprovementFunctor*>(&m_vertexNeighborFunctor);
        case 2: return *reinterpret_cast<LocalImprovementFunctor*>(&m_permutationCrossing);
        default: return *reinterpret_cast<LocalImprovementFunctor*>(&m_permutationCrater);
      }
    }

    LocalGurobi optimizer;
    LocalImprovementBomb m_bombFunctor;
    LocalImprovementVertexNeighbors m_vertexNeighborFunctor;
    PlacedPermutationFunctorCrater m_permutationCrater;
    PlacedPermutationFunctorCrossing m_permutationCrossing;
    RandomGen m_random;
    KdTree m_kdtree;
  };
}

#endif
