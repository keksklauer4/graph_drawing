#ifndef __GD_ASSIGNMENT_HPP__
#define __GD_ASSIGNMENT_HPP__

#include "common/instance.hpp"
#include <gd_types.hpp>

namespace gd
{

  class VertexAssignment
  {
    public:
      VertexAssignment(const Instance& instance);

      void assign(vertex_t vertex, point_id_t to);
      void unassign(vertex_t vertex);
      bool isAssigned(vertex_t vertex) const;
      point_id_t getAssigned(vertex_t vertex) const;
      bool isPointUsed(point_id_t p) const;
      size_t getNumUnassigned() const { return m_unassigned.size(); }
      vertex_t getAssignedVertex(point_id_t p) const;

    private:
      Vector<point_id_t> m_assignment;  // m_assignment[vertex] = pointid on which vertex is;
      Vector<vertex_t> m_pointToVertex; // m_pointToVertex[point] = vertex on point;

      Set<vertex_t> m_unassigned;
      Set<vertex_t> m_assignedVertices;
  };

}

#endif
