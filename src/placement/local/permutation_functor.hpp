#ifndef __GD_COLLINEAR_FUNCTOR_HPP__
#define __GD_COLLINEAR_FUNCTOR_HPP__

#include "common/kd_tree.hpp"
#include "common/random_gen.hpp"
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
    void name(std::ostream& os) const override { os << "CollinearFunctor"; }
    LocalReoptFunctors type() const override { return LocalReoptFunctors::COLLINEAR_REBUILD; }

  };

  class PlacedPermutationFunctorCrater : public PermutationFunctor
  {
  public:
    PlacedPermutationFunctorCrater(const Instance& instance, const VertexAssignment& assignment,
                      IncrementalCollinear& collinear, const KdTree& kdtree)
      : PermutationFunctor(instance, assignment, collinear), m_kdtree(kdtree) {}

    void initialize(vertex_t vertex, point_id_t point) override;
    bool has_start() const override { return true; }
    void name(std::ostream& os) const override { os << "PlacedPermutationFunctorCrater"; }
    LocalReoptFunctors type() const override { return LocalReoptFunctors::PERMUTATION_CRATER; }

  private:
    const KdTree& m_kdtree;
  };


  class PlacedPermutationFunctorCrossing : public PermutationFunctor
  {
  public:
    PlacedPermutationFunctorCrossing(const Instance& instance, const VertexAssignment& assignment,
                      IncrementalCollinear& collinear)
      : PermutationFunctor(instance, assignment, collinear){}

    void initialize(vertex_t vertex, point_id_t point) override;
    bool has_start() const override { return true; }
    void name(std::ostream& os) const override { os << "PlacedPermutationFunctorCrossing"; }
    LocalReoptFunctors type() const override { return LocalReoptFunctors::PERMUTATION_CROSSING; }

  private:
    RandomGen m_random;
  };
}


#endif
