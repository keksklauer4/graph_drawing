#include "incremental_crossing.hpp"
#include "gd_types.hpp"
#include "verification/verification_utils.hpp"

#include <cassert>
#include<common/instance.hpp>
#include<common/assignment.hpp>
#include <cstdio>
#include <numeric>

#include<iostream>

#include <common/crossing_hierarchy.hpp>
#include <stdexcept>

using namespace gd;

IncrementalCrossing::IncrementalCrossing(const Instance& instance,
    const VertexAssignment& assignment, CrossingHierarchy& crossing_hierarchy)
  : m_assignment(assignment), m_instance(instance), m_crossing_hierarchy(crossing_hierarchy)
{
  m_numCrossings.resize(m_instance.m_graph.getNbVertices());
  std::fill(m_numCrossings.begin(), m_numCrossings.end(), 0);
}

size_t IncrementalCrossing::checkPlacement(vertex_t vertex, point_id_t point, bool fix, int delta, size_t ub)
{
  m_instance.m_timer.timer_crossing();
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
  for (auto pair : m_instance.m_graph)
  {
    if (pair.first > pair.second || pair.first == vertex
      || pair.second == vertex || !m_assignment.isAssigned(pair.first)
      || !m_assignment.isAssigned(pair.second)) continue;
    const auto& vPoint = pset.getPoint(m_assignment.getAssigned(pair.first));
    const auto& wPoint = pset.getPoint(m_assignment.getAssigned(pair.second));
    for (vertex_t neighbor : m_mappedNeighbors)
    {
      if (neighbor == pair.first || neighbor == pair.second
        || !gd::intersect(vPoint, wPoint, targetPoint, pset.getPoint(m_assignment.getAssigned(neighbor))))
      {
        continue;
      }
      num_crossings++;
      if (fix)
      {
        m_numCrossings[pair.first] += delta;
        m_numCrossings[pair.second] += delta;
        m_numCrossings[vertex] += delta;
        m_numCrossings[neighbor] += delta;
      }
      else if (num_crossings > ub) break;
    }
    if (!fix && num_crossings > ub) break;
  }
  m_instance.m_timer.timer_crossing();
  return num_crossings;
}

size_t IncrementalCrossing::calculateCrossing(vertex_t vertex, point_id_t point, size_t ub)
{
  /*m_instance.m_timer.timer_crossing();
  size_t crossings = m_crossing_hierarchy.count_crossings(vertex, point, ub);
  m_instance.m_timer.timer_crossing();*/
  return checkPlacement(vertex, point, false, 0, ub);
}

size_t IncrementalCrossing::place(vertex_t vertex, point_id_t point)
{
  /*m_instance.m_timer.timer_crossing();
  m_crossing_hierarchy.place(vertex, point);
  m_instance.m_timer.timer_crossing();*/
  return checkPlacement(vertex, point, true, +1);
}
void IncrementalCrossing::deplace(vertex_t vertex, point_id_t point)
{
  /*m_instance.m_timer.timer_crossing();
  m_crossing_hierarchy.deplace(vertex);
  m_instance.m_timer.timer_crossing();*/
  checkPlacement(vertex, point, true, -1);
  assert(getNumCrossings(vertex) == 0 && "Vertex has been deplaced, so there should be no crossings!");
}


size_t IncrementalCrossing::getTotalNumCrossings() const
{// every intersection is counted 4 times (for every vertex counted)
  return std::accumulate(m_numCrossings.begin(), m_numCrossings.end(), 0) / 4;
}
