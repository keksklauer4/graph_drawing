#include "incremental_collinear.hpp"

#include<verification/verification_utils.hpp>
#include<common/assignment.hpp>

using namespace gd;

IncrementalCollinear::IncrementalCollinear(const Instance& instance,
  const VertexAssignment& assignment, CollinearFunction& collFunc)
  : m_assignment(assignment), m_instance(instance), m_collFunc(collFunc) {}

void IncrementalCollinear::findCollinear(const line_2d_t& line) const
{
  for (const auto& p : m_instance.m_points)
  {
    if (m_assignment.isPointUsed(p.id) && gd::isOnLine(line, p)
        && !m_collFunc(p)) return;
  }
}
