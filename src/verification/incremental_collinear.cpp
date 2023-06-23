#include "incremental_collinear.hpp"

#include<verification/verification_utils.hpp>
#include<common/assignment.hpp>

using namespace gd;

IncrementalCollinear::IncrementalCollinear(const Instance& instance,
  const VertexAssignment& assignment, CollinearFunction& collFunc)
  : m_assignment(assignment), m_instance(instance), m_collFunc(collFunc) {}

void IncrementalCollinear::findCollinear(const line_2d_t& line, bool fix_edge)
{
  for (const auto& p : m_instance.m_points)
  {
    if (gd::isOnLine(line, p))
    {
      if(fix_edge) m_invalidPoints.insert(p.id);
      if (m_assignment.isPointUsed(p.id) && !m_collFunc(p)) return;
    }
  }
}
