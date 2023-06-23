#include "incremental_crossing.hpp"
#include "gd_types.hpp"
#include "verification/verification_utils.hpp"

#include<common/instance.hpp>
#include<common/assignment.hpp>
#include <numeric>

using namespace gd;

IncrementalCrossing::IncrementalCrossing(const Instance& instance,
    const VertexAssignment& assignment)
  : m_assignment(assignment), m_instance(instance)
{
  m_numCrossings.resize(m_instance.m_graph.getNbVertices());
  std::fill(m_numCrossings.begin(), m_numCrossings.end(), 0);
}

void IncrementalCrossing::initialPlacement(vertex_t newVertex, point_id_t p)
{
  const auto& graph = m_instance.m_graph;
  const auto& pset = m_instance.m_points;
  const Point& targetPoint = pset.getPoint(p);

  auto newNeighbors = graph.getNeighborIterator(newVertex);
  m_mappedNeighbors.clear();
  for (auto it = newNeighbors.first; it != newNeighbors.second; ++it)
  {
    if (m_assignment.isAssigned(*it)) m_mappedNeighbors.push_back(*it);
  }

  for (vertex_t v = 0; v < graph.getNbVertices(); ++v)
  {
    if (!m_assignment.isAssigned(v)) continue;
    const Point& vPoint = pset.getPoint(m_assignment.getAssigned(v));
    auto neighbors = graph.getNeighborIterator(v);
    for (auto w = neighbors.first; w != neighbors.second; ++w)
    {
      if (v < (*w) && m_assignment.isAssigned(*w))
      { // both v,w (v < w) are assigned and there is an edge between them (check whether that crosses with any of newVertex)
        // now iterate over newVertex's neighbors
        const Point& wPoint = pset.getPoint(m_assignment.getAssigned(*w));

        for (vertex_t neighbor : m_mappedNeighbors)
        {
          const Point& neighborPoint = pset.getPoint(m_assignment.getAssigned(neighbor));
          if (gd::intersect(vPoint, wPoint, targetPoint, neighborPoint))
          {
            m_numCrossings[v]++;
            m_numCrossings[*w]++;
            m_numCrossings[newVertex]++;
            m_numCrossings[neighbor]++;
          }
        }
      }
    }
  }
}

size_t IncrementalCrossing::getTotalNumCrossings() const
{
  return std::accumulate(m_numCrossings.begin(), m_numCrossings.end(), 0);
}
