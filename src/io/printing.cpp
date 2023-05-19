#include "printing.hpp"

#include <common/instance.hpp>
#include <common/assignment.hpp>

using namespace gd;

void gd::printAssignment(std::ostream& os, const Instance& instance, const VertexAssignment& assignment)
{
  for (vertex_t v = 0; v < instance.m_graph.getNbVertices(); ++v)
  {
    os << "Vertex " << v;
    if (assignment.isAssigned(v)) os << " assigned to " << instance.m_points.getPoint(assignment.getAssigned(v));
    else os << " unassigned.";
    os << std::endl;
  }
}
