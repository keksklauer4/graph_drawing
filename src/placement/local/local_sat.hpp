#ifndef __GD_LOCAL_SAT_HPP__
#define __GD_LOCAL_SAT_HPP__

#include <placement/local/local_reopt.hpp>
#include <gd_types.hpp>

#include <EvalMaxSAT.h>

namespace gd
{

  class LocalSAT : public LocalReOpt
  {
    typedef int SatVar;
    typedef std::pair<point_id_t, vertex_t> vertex_point_pair_t;
    typedef Map<vertex_point_pair_t, SatVar, PairLexicographicOrdering<size_t>> SatVariableMap;
  public:
    LocalSAT(const Instance& instance, const VertexAssignment& assignment)
      : LocalReOpt(instance, assignment) {}
    ~LocalSAT();
    void optimize(LocalImprovementFunctor& functor) override;


  private:
    void create_variables() override;
    void create_vertex_mapped_csts() override;
    void create_at_most_one_vertex_mapped_to_cst() override;

    void create_pair_collinear_csts() override;
    void create_collinear_triples_csts() override;
    void create_single_crossings() override;
    void create_pair_crossings() override;

  private:
    EvalMaxSAT* m_solver;

    SatVariableMap m_satVariables;
    Vector<SatVar> m_clauseVec;
  };
}


#endif
