#ifndef __GD_COLLINEAR_FUNCTOR_HPP__
#define __GD_COLLINEAR_FUNCTOR_HPP__

#include "gd_types.hpp"
#include <placement/local/local_functors.hpp>

namespace gd
{

  class PermutationFunctor : public LocalImprovementFunctor
  {
  public:
    PermutationFunctor(const Instance& instance, const VertexAssignment& assignment,
                       IncrementalCollinear& collinear)
      : m_instance(instance), m_assignment(assignment),
        m_collinear(collinear) {}

    void set_points() override;

    void reset() override { reset_base(); }

  protected:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;
    IncrementalCollinear& m_collinear;
  };

  class CollinearFunctor : public PermutationFunctor
  {
  public:
    CollinearFunctor(const Instance& instance, const VertexAssignment& assignment,
                      IncrementalCollinear& collinear)
      : PermutationFunctor(instance, assignment, collinear){}

    void initialize(vertex_t vertex, point_id_t point) override;
  };

}


#endif
