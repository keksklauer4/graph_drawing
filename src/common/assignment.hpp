#ifndef __GD_ASSIGNMENT_HPP__
#define __GD_ASSIGNMENT_HPP__

#include <gd_types.hpp>

namespace gd
{

  class VertexAssignment
  {
    public:
      VertexAssignment(int num_vertices);

      void assign(vertex_t vertex, point_id_t to);
      bool isAssigned(vertex_t vertex) const;
      point_id_t getAssigned(vertex_t vertex) const;

    private:
      Vector<point_id_t> m_assignment;
      Set<vertex_t> m_unassigned;
  };

}

#endif
