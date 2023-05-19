#include "assignment.hpp"

#include <gd_types.hpp>
#include <common/misc.hpp>

using namespace gd;

VertexAssignment::VertexAssignment(size_t num_vertices)
{
  m_assignment.resize(num_vertices);
  std::fill(m_assignment.begin(), m_assignment.end(), UINT_UNDEF);
}

void VertexAssignment::assign(vertex_t vertex, point_id_t to)
{
  if (isDefined(m_assignment[vertex])) m_usedPoints.erase(m_assignment[vertex]);
  m_assignment[vertex] = to;
  m_unassigned.erase(vertex);
  m_usedPoints.insert(to);
  m_assignedVertices.insert(vertex);
}

void VertexAssignment::unassign(vertex_t vertex)
{
  m_usedPoints.erase(m_assignment[vertex]);
  m_unassigned.insert(vertex);
  m_assignedVertices.erase(vertex);
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
  return m_usedPoints.contains(p);
}
