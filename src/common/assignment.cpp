#include "assignment.hpp"

#include <gd_types.hpp>
#include <common/misc.hpp>

using namespace gd;

VertexAssignment::VertexAssignment(int num_vertices)
{
  m_assignment.resize(num_vertices);
  std::fill(m_assignment.begin(), m_assignment.end(), UINT_UNDEF);
}

void VertexAssignment::assign(vertex_t vertex, point_id_t to)
{
  m_assignment[vertex] = to;
  m_unassigned.erase(vertex);
}

bool VertexAssignment::isAssigned(vertex_t vertex) const
{
  return isDefined(m_assignment.at(vertex));
}

point_id_t VertexAssignment::getAssigned(vertex_t vertex)
{
  return m_assignment[vertex];
}
