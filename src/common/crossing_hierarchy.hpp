#ifndef __GD_CROSSING_HIERARCHY_HPP__
#define __GD_CROSSING_HIERARCHY_HPP__

#include <gd_types.hpp>
namespace aabb{ class Tree; }

namespace gd
{

  class CrossingHierarchy
  {
  public:
    typedef std::vector<double> CoordinateVector;
    typedef std::pair<CoordinateVector, CoordinateVector> EdgeBox;
  public:
    CrossingHierarchy(const Instance& instance, const VertexAssignment& assignment);

    void place(vertex_t v, point_id_t p);
    void deplace(vertex_t v);

     // assumes vertex is not placed anywhere!
    size_t count_crossings(vertex_t v, point_id_t p, size_t ub = 1e9);

  private:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;

    Map<vertex_pair_t, unsigned int> m_edgeIndices;
    Vector<vertex_pair_t> m_edges;
    Vector<EdgeBox> m_edgeBoxes;
    aabb::Tree* m_bvhTree;

    EdgeBox m_queryBox;

    size_t m_numEdgesPlaced;
  };

}



#endif
