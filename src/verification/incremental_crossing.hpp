#ifndef __GD_INCREMENTAL_CROSSING_HPP__
#define __GD_INCREMENTAL_CROSSING_HPP__

#include <gd_types.hpp>

namespace gd
{

  class IncrementalCrossing
  {
    public:
      IncrementalCrossing(const Instance& instance, const VertexAssignment& assignment);

      size_t calculateCrossing();
      void initialPlacement(vertex_t vertex, point_id_t point);
      size_t getTotalNumCrossings() const;

    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;

      Vector<size_t> m_numCrossings; // per vertex
      Vector<vertex_t> m_mappedNeighbors; // used as temporary memory
  };


}



#endif
