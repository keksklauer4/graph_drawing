#ifndef __GD_ASSIGNMENT_HPP__
#define __GD_ASSIGNMENT_HPP__

#include <cstddef>
#include <gd_types.hpp>

namespace gd
{

  class VertexAssignment
  {
    public:
      VertexAssignment(size_t num_vertices);

      void assign(vertex_t vertex, point_id_t to);
      bool isAssigned(vertex_t vertex) const;
      point_id_t getAssigned(vertex_t vertex) const;
      bool isPointUsed(point_id_t p) const;
      size_t getNumUnassigned() const { return m_unassigned.size(); }

    private:
      Vector<point_id_t> m_assignment;
      Set<vertex_t> m_unassigned;
      Set<point_id_t> m_usedPoints;
  };

}

#endif
