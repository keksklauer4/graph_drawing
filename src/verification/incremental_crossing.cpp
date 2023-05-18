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

size_t IncrementalCrossing::checkPlacement(vertex_t vertex, point_id_t point, bool fix, size_t delta)
{
  const auto& graph = m_instance.m_graph;
  const auto& pset = m_instance.m_points;
  const Point& targetPoint = pset.getPoint(point);

  auto newNeighbors = graph.getNeighborIterator(vertex);
  m_mappedNeighbors.clear();
  for (auto it = newNeighbors.first; it != newNeighbors.second; ++it)
  {
    if (m_assignment.isAssigned(*it)) m_mappedNeighbors.push_back(*it);
  }

  size_t num_crossings = 0;
  for (vertex_t v = 0; v < graph.getNbVertices(); ++v)
  {
    if (v == vertex || !m_assignment.isAssigned(v)) continue;
    const Point& vPoint = pset.getPoint(m_assignment.getAssigned(v));
    auto neighbors = graph.getNeighborIterator(v);
    for (auto w = neighbors.first; w != neighbors.second; ++w)
    {
      if (v < (*w) && (*w) != vertex && m_assignment.isAssigned(*w))
      { // both v,w (v < w) are assigned and there is an edge between them (check whether that crosses with any of newVertex)
        // now iterate over newVertex's neighbors
        const Point& wPoint = pset.getPoint(m_assignment.getAssigned(*w));

        for (vertex_t neighbor : m_mappedNeighbors)
        {
          const Point& neighborPoint = pset.getPoint(m_assignment.getAssigned(neighbor));
          if (gd::intersect(vPoint, wPoint, targetPoint, neighborPoint))
          {
            num_crossings++;
            if (fix)
            {
              m_numCrossings[v]         += delta;
              m_numCrossings[*w]        += delta;
              m_numCrossings[vertex]    += delta;
              m_numCrossings[neighbor]  += delta;
            }
          }
        }
      }
    }
  }
  return num_crossings;
}

size_t IncrementalCrossing::calculateCrossing(vertex_t vertex, point_id_t point)
{
  return checkPlacement(vertex, point, false, 0);
}

void IncrementalCrossing::place(vertex_t vertex, point_id_t point)
{
  checkPlacement(vertex, point, true, +1);
}

void IncrementalCrossing::deplace(vertex_t vertex)
{
  checkPlacement(vertex, m_assignment.getAssigned(vertex), true, -1);
}


size_t IncrementalCrossing::getTotalNumCrossings() const
{// every intersection is counted 4 times (for every vertex counted)
  return std::accumulate(m_numCrossings.begin(), m_numCrossings.end(), 0) / 4;
}
