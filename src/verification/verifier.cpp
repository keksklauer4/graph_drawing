#include "verifier.hpp"


#include <common/instance.hpp>
#include <common/assignment.hpp>
#include <verification/collinear.hpp>
#include <verification/line_crossings.hpp>
#include <cstddef>

using namespace gd;

bool Verifier::verify(size_t& num_crossings) const
{
  num_crossings = UINT_UNDEF;
  // Order is important
  // 1. check every vertex assigned to distinct vertices
  // 2. check collinearity
  // 3. count crossings
  if (distinct() && !gd::trivialCollinearityCheck(m_instance, m_assignment))
  {
    num_crossings = gd::countCrossings(m_instance, m_assignment);
    return true;
  }
  return false;
}

bool Verifier::distinct() const
{
  if (m_assignment.getNumUnassigned() != 0) return false;
  Set<point_id_t> points {};
  for (vertex_t i = 0; i != m_instance.m_graph.getNbVertices(); ++i)
  {
    points.insert(m_assignment.getAssigned(i));
  }
  return points.size() == m_instance.m_graph.getNbVertices();
}


