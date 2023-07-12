#ifndef __GD_LOCAL_FUNCTORS_HPP__
#define __GD_LOCAL_FUNCTORS_HPP__

#include <gd_types.hpp>
#include <placement/local/local_reopt.hpp>

namespace gd
{

  struct LocalImprovementNN : public LocalImprovementFunctor
  {
  public:
    LocalImprovementNN(const Instance& instance, const VertexAssignment& assignment,
                      const KdTree& kdtree, IncrementalCollinear& collinear)
      : m_instance(instance), m_assignment(assignment),
        m_kdtree(kdtree), m_collinear(collinear),
        m_chosen(UINT_UNDEF), m_center(UINT_UNDEF) {}

    void set_points() override;
    bool has_start() const override { return m_start; }
    void reset() override;

  protected:
    void all_assigned();

  protected:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;
    const KdTree& m_kdtree;
    IncrementalCollinear& m_collinear;

    Set<point_id_t> m_temp_points;

    vertex_t m_chosen;
    point_id_t m_center;
    bool m_start = false;
  };

  // choose a (not necessarily mapped) vertex
  struct LocalImprovementVertexNeighbors : public LocalImprovementNN
  {
  public:
    LocalImprovementVertexNeighbors(
        const Instance& instance, const VertexAssignment& assignment,
        const KdTree& kdtree, IncrementalCollinear& collinear)
      : LocalImprovementNN(instance, assignment, kdtree, collinear) {}

    void initialize(vertex_t vertex, point_id_t point) override;
    void name(std::ostream& os) const override { os << "LocalImprovementVertexNeighbors"; }

  };

  struct LocalImprovementBomb : public LocalImprovementNN
  {
  public:
    LocalImprovementBomb(
        const Instance& instance, const VertexAssignment& assignment,
        const KdTree& kdtree, IncrementalCollinear& collinear)
      : LocalImprovementNN(instance, assignment, kdtree, collinear) {}

    void initialize(vertex_t vertex, point_id_t point) override;
    void name(std::ostream& os) const override { os << "LocalImprovementBomb"; }

  };


  size_t get_viable_neighbors(
    const VertexAssignment& assignment, const KdTree& kdtree,
    IncrementalCollinear& collinear,
    MultiMap<vertex_t, point_id_t>& options,
    vertex_t v, const Point& center,
    Set<point_id_t>& points_included
  );
}




#endif
