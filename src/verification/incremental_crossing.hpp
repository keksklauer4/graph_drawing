#ifndef __GD_INCREMENTAL_CROSSING_HPP__
#define __GD_INCREMENTAL_CROSSING_HPP__

#include <gd_types.hpp>

namespace gd
{

  class IncrementalCrossing
  {
    public:
      IncrementalCrossing(const Instance& instance, const VertexAssignment& assignment);

      size_t calculateCrossing(vertex_t vertex, point_id_t point);
      void place(vertex_t vertex, point_id_t point);
      void deplace(vertex_t vertex, point_id_t point);

      size_t getTotalNumCrossings() const;
      size_t getNumCrossings(vertex_t vertex) const { return m_numCrossings.at(vertex); }

    private:
      size_t checkPlacement(vertex_t vertex, point_id_t point, bool fix, int delta);


    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;

      Vector<size_t> m_numCrossings; // per vertex
      Vector<vertex_t> m_mappedNeighbors; // used as temporary memory
  };


}



#endif
