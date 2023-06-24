#ifndef __GD_LOCAL_FUNCTORS_HPP__
#define __GD_LOCAL_FUNCTORS_HPP__

#include <gd_types.hpp>
#include <placement/local/local_reopt.hpp>

namespace gd
{

  // choose a (not necessarily mapped) vertex
  struct LocalImprovementVertexNeighbors : public LocalImprovementFunctor
  {
  public:
    LocalImprovementVertexNeighbors(
        const Instance& instance, const VertexAssignment& assignment);

    void initialize(vertex_t vertex, point_id_t point);
    void set_points(const KdTree& kdtree, IncrementalCollinear& collinear);
    void reset();

  private:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;

    vertex_t m_chosen;
    point_id_t m_center;
    bool m_wasAssignedToCenter = false;
  };


  size_t get_viable_neighbors(
    const VertexAssignment& assignment, const KdTree& kdtree,
    IncrementalCollinear& collinear,
    MultiMap<vertex_t, point_id_t>& options,
    vertex_t v, const Point& center, bool wasAssigned
  );
}




#endif
