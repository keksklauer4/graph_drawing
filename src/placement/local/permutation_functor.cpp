#include "permutation_functor.hpp"
#include "common/misc.hpp"
#include "common/random_gen.hpp"
#include "gd_types.hpp"
#include "verification/line_crossings.hpp"

#include <cstddef>
#include <verification/incremental_collinear.hpp>

using namespace gd;

void PermutationFunctor::set_points()
{
  for (vertex_t v : m_vertices)
  {
    for (point_id_t p : m_pointIds)
    {
      m_vertexToPoint.insert(std::make_pair(v, p));
      m_pointToVertex.insert(std::make_pair(p, v));
    }
  }
}

void CollinearFunctor::initialize(vertex_t vertex, point_id_t point)
{
  auto range = m_collinear.getCollinearTriplets(point);
  VertexSet vset{};
  for (auto it = range.first; it != range.second; ++it)
  {
    vertex_t v1 = m_assignment.getAssignedVertex(it->second.first);
    vertex_t v2 = m_assignment.getAssignedVertex(it->second.second);
    if (isDefined(v1) && isDefined(v2) && m_instance.m_graph.connected(v1, v2))
    {
      vset.insert(v1);
      vset.insert(v2);
    }
  }
  if (vset.empty())
  {
    m_valid = false;
    return;
  }
  VertexVector neighbors{};
  auto neighborRange = m_instance.m_graph.getNeighborIterator(vertex);
  for (auto it = neighborRange.first; it != neighborRange.second; ++it)
  { if (m_assignment.isAssigned(*it)) neighbors.push_back(*it); }

  RandomGen random;
  random.shuffle(neighbors);
  size_t idx = 0;
  while(vset.size() < 8 && idx < neighbors.size())
  {
    vset.insert(neighbors[idx++]);
  }

  idx = 0;
  m_vertices.insert(m_vertices.begin(), vset.begin(), vset.end());
  m_previousMapping.resize(m_vertices.size() + 1);
  for (vertex_t v : m_vertices)
  {
    m_pointIds.push_back(m_assignment.getAssigned(v));
    m_previousMapping[idx++] = m_assignment.getAssigned(v);
  }

  m_vertices.push_back(vertex);
  m_pointIds.push_back(point);
  m_previousMapping[idx++] = UINT_UNDEF;
  m_valid = true;
}

#define NUM_SWAP_CANDIDATES 10

void PlacedPermutationFunctorCrater::initialize(vertex_t vertex, point_id_t)
{
  VertexSet vset{};
  m_kdtree.k_nearest_neighbors(m_instance.m_points.getPoint(m_assignment.getAssigned(vertex)),
                              NUM_SWAP_CANDIDATES,
    [&](point_id_t point) -> bool{
      if (m_assignment.isPointUsed(point) && !vset.contains(m_assignment.getAssignedVertex(point)))
      {
        vset.insert(m_assignment.getAssignedVertex(point));
        return true;
      }
      return false;
  });

  vset.insert(vertex);

  size_t idx = 0;
  m_vertices.insert(m_vertices.begin(), vset.begin(), vset.end());
  m_previousMapping.resize(m_vertices.size());
  for (vertex_t v : m_vertices)
  {
    m_pointIds.push_back(m_assignment.getAssigned(v));
    m_previousMapping[idx++] = m_assignment.getAssigned(v);
  }

  m_valid = true;
}

void PlacedPermutationFunctorCrossing::initialize(vertex_t vertex, point_id_t)
{
  const auto& graph = m_instance.m_graph;
  VertexVector neighbors{};
  auto neighborRange = graph.getNeighborIterator(vertex);
  neighbors.reserve(graph.getDegree(vertex));
  for (auto it = neighborRange.first; it != neighborRange.second; ++it)
  {
    if (m_assignment.isAssigned(*it)) neighbors.push_back(*it);
  }

  VertexSet vset{};
  size_t remaining_neighbors = neighbors.size();
  VertexSet crossing{};
  VertexVector crossing_vec{};
  while(vset.size() < NUM_SWAP_CANDIDATES && remaining_neighbors > 0)
  {
    size_t neighborIdx = m_random.getRandomUint(remaining_neighbors);
    enumerateCrossingPairs(m_instance, m_assignment, crossing,
      vertex, neighbors[neighborIdx], NUM_SWAP_CANDIDATES * NUM_SWAP_CANDIDATES);
    std::swap(neighbors[neighborIdx], neighbors[remaining_neighbors - 1]);
    remaining_neighbors--;
    if (crossing.empty()) continue;
    if (vset.size() + crossing.size() <= NUM_SWAP_CANDIDATES)
    {
      vset.insert(crossing.begin(), crossing.end());
    }
    else
    {
      crossing_vec.clear();
      crossing_vec.insert(crossing_vec.begin(), crossing.begin(), crossing.end());
      m_random.shuffle(crossing_vec);
      for (vertex_t crossing_v : crossing_vec)
      {
        vset.insert(crossing_v);
        if (vset.size() >= NUM_SWAP_CANDIDATES) break;
      }
    }
    crossing.clear();
  }

  size_t idx = 0;
  m_vertices.insert(m_vertices.begin(), vset.begin(), vset.end());
  m_previousMapping.resize(m_vertices.size());
  for (vertex_t v : m_vertices)
  {
    m_pointIds.push_back(m_assignment.getAssigned(v));
    m_previousMapping[idx++] = m_assignment.getAssigned(v);
  }

  m_valid = true;
}
