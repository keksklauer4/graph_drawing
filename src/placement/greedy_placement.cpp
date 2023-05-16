#include "greedy_placement.hpp"
#include "common/misc.hpp"
#include "gd_types.hpp"

#include <common/instance.hpp>
#include <stdexcept>
#include <iostream>

using namespace gd;


GreedyPlacement::GreedyPlacement(const Instance& instance)
  : m_instance(instance),
    m_assignment(instance.m_graph.getNbVertices()),
    m_collChecker(), m_incrementalCollinearity(instance, m_assignment, m_collChecker),
    m_incrementalCrossing(instance, m_assignment)
{ }

const VertexAssignment& GreedyPlacement::findPlacement()
{
  auto pointsRange = m_instance.m_points.getPointIterator();
  for (auto it = pointsRange.first; it != pointsRange.second; ++it)
  {
    m_unused.insert(it->id);
  }

  for (vertex_t v = 0; v != m_instance.m_graph.getNbVertices(); ++v)
  {
    point_id_t target = findEligiblePoint(v);
    if (!isDefined(target)) throw std::runtime_error("Cant find a point to map to... :(");

    m_incrementalCrossing.initialPlacement(v, target);
    m_assignment.assign(v, target);
    m_unused.erase(target);
    std::cout << "Mapped vertex " << v << " point " << target << std::endl;
  }
  return m_assignment;
}

point_id_t GreedyPlacement::findEligiblePoint(vertex_t vertex)
{
  const auto& pset = m_instance.m_points;
  auto neighbors = m_instance.m_graph.getNeighborIterator(vertex);
  m_mappedNeighbors.clear();
  for (auto w = neighbors.first; w != neighbors.second; ++w)
  {
    if (m_assignment.isAssigned(*w))
    {
      m_mappedNeighbors.push_back(pset.getPoint(m_assignment.getAssigned(*w)));
    }
  }

  line_2d_t line {};
  for (point_id_t pointid : m_unused)
  {
    m_collChecker.collinear = false;
    const Point& p1 = m_instance.m_points.getPoint(pointid);
    line.first = p1.getCoordPair();
    for (const Point& neighborPoint : m_mappedNeighbors)
    {
      line.second = neighborPoint.getCoordPair();
      m_incrementalCollinearity.findCollinear(line);
      if (m_collChecker.collinear) break;
    }
    if (!m_collChecker.collinear) return pointid;
  }
  return POINT_UNDEF;
}

size_t GreedyPlacement::getNumCrossings() const
{
  return m_incrementalCrossing.getTotalNumCrossings();
}
