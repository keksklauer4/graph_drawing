#include "incremental_collinear.hpp"
#include "gd_types.hpp"

#include<verification/verification_utils.hpp>
#include<common/assignment.hpp>

using namespace gd;

IncrementalCollinear::IncrementalCollinear(const Instance& instance,
  const VertexAssignment& assignment, CollinearFunction& collFunc)
  : m_assignment(assignment), m_instance(instance), m_collFunc(collFunc) {}

bool IncrementalCollinear::findCollinear(const point_pair_t& line, bool fix_edge)
{
  bool found = false;
  line_2d_t line_coords = std::make_pair(line.first.getCoordPair(), line.second.getCoordPair());
  for (const auto& p : m_instance.m_points)
  {
    if (gd::isOnLine(line_coords, p))
    {
      if(fix_edge)
      {
        m_invalidPoints[p.id]++;
        m_invalidities.insert(std::make_pair(getOrderedPair(line.first.id, line.second.id), p.id));
      }
      if (m_assignment.isPointUsed(p.id) && (found = true) && !m_collFunc(p)) return true;
    }
  }
  return found;
}

bool IncrementalCollinear::isValidCandidate(vertex_t vertex, point_id_t p)
{
  if (isPointInvalid(p)) return false;
  point_pair_t line;
  line.first = m_instance.m_points.getPoint(p);
  auto neighborRange = m_instance.m_graph.getNeighborIterator(vertex);
  for (auto neighbor = neighborRange.first; neighbor != neighborRange.second; ++neighbor)
  {
    if (m_assignment.isAssigned(*neighbor))
    {
      line.second = m_instance.m_points.getPoint(m_assignment.getAssigned(*neighbor));
      if (findCollinear(line)) return false;
    }
  }
  return true;
}


void IncrementalCollinear::place(vertex_t vertex, point_id_t point)
{
  point_pair_t line;
  line.first = m_instance.m_points.getPoint(point);
  auto neighborRange = m_instance.m_graph.getNeighborIterator(vertex);
  for (auto neighbor = neighborRange.first; neighbor != neighborRange.second; ++neighbor)
  {
    if (m_assignment.isAssigned(*neighbor))
    {
      line.second = m_instance.m_points.getPoint(m_assignment.getAssigned(*neighbor));
      findCollinear(line, true);
    }
  }
}

void IncrementalCollinear::deplace(vertex_t vertex)
{
  point_id_t p1 = m_assignment.getAssigned(vertex);
  auto neighborRange = m_instance.m_graph.getNeighborIterator(vertex);
  for (auto neighbor = neighborRange.first; neighbor != neighborRange.second; ++neighbor)
  {
    if (!m_assignment.isAssigned(*neighbor)) continue;

    auto invalidRange = m_invalidities.equal_range(getOrderedPair(p1, m_assignment.getAssigned(*neighbor)));
    for (auto it = invalidRange.first; it != invalidRange.second; ++it)
    {
      m_invalidPoints[it->second]--;
    }
    m_invalidities.erase(invalidRange.first, invalidRange.second);
  }
}
