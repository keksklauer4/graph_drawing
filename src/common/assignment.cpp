#include "assignment.hpp"

#include <cassert>
#include <gd_types.hpp>
#include <common/misc.hpp>
#include <common/instance.hpp>

using namespace gd;

VertexAssignment::VertexAssignment(const Instance& instance)
{
  m_assignment.resize(instance.m_graph.getNbVertices());
  m_pointToVertex.resize(instance.m_points.getNumPoints());
  std::fill(m_assignment.begin(), m_assignment.end(), VERTEX_UNDEF);
  std::fill(m_pointToVertex.begin(), m_pointToVertex.end(), POINT_UNDEF);
  for (const auto& point : instance.m_points) m_remainingPoints.insert(point.id);
}

void VertexAssignment::assign(vertex_t vertex, point_id_t to)
{
  assert(!isDefined(m_pointToVertex[to]) && "Point is already in use!");
  if (isDefined(m_assignment[vertex]))
  {
    m_remainingPoints.insert(m_assignment[vertex]);
    m_pointToVertex[m_assignment[vertex]] = POINT_UNDEF;
  }
  m_pointToVertex[to] = vertex;
  m_assignment[vertex] = to;
  m_unassigned.erase(vertex);
  m_assignedVertices.insert(vertex);
  m_remainingPoints.erase(to);
}

void VertexAssignment::unassign(vertex_t vertex)
{
  m_unassigned.insert(vertex);
  m_assignedVertices.erase(vertex);
  assert(isDefined(m_assignment[vertex]) && "Vertex wasn't assigned, i. e. can't unassign!");
  m_remainingPoints.insert(m_assignment[vertex]);
  m_pointToVertex[m_assignment[vertex]] = POINT_UNDEF;
  m_assignment[vertex] = UINT_UNDEF;
}

bool VertexAssignment::isAssigned(vertex_t vertex) const
{
  return isDefined(m_assignment.at(vertex));
}

point_id_t VertexAssignment::getAssigned(vertex_t vertex) const
{
  return m_assignment.at(vertex);
}

bool VertexAssignment::isPointUsed(point_id_t p) const
{
  return isDefined(m_pointToVertex[p]);
}

vertex_t VertexAssignment::getAssignedVertex(point_id_t p) const
{
  return m_pointToVertex.at(p);
}

