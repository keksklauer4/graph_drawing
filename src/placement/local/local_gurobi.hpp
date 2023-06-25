#ifndef __GD_LOCAL_GUROBI_HPP__
#define __GD_LOCAL_GUROBI_HPP__

#include "gd_types.hpp"
#include <placement/local/local_reopt.hpp>

#include <gurobi_c.h>
#include <gurobi_c++.h>

class GRBEnv;
class GRBModel;
class GRBVar;
class GRBQuadExpr;

namespace gd
{

  class LocalGurobi : public LocalReOpt
  {
    typedef std::pair<point_id_t, vertex_t> vertex_point_pair_t;
    typedef Map<vertex_point_pair_t, GRBVar*, PairLexicographicOrdering<size_t>> GrbVariableMap;
  public:
    LocalGurobi(const Instance& instance, const VertexAssignment& assignment)
      : LocalReOpt(instance, assignment) {}
    ~LocalGurobi();

    void optimize(LocalImprovementFunctor& functor) override;


  private:
    void create_variables() override;
    void create_vertex_mapped_csts() override;
    void create_at_most_one_vertex_mapped_to_cst() override;

    void create_pair_collinear_csts() override;
    void create_collinear_triples_csts() override;
    void create_single_crossings() override;
    void create_pair_crossings() override;
    void create_internal_crossings() override;
    void create_semi_internal_crossings() override;
    void create_pair_neighbor_crossings() override;

    GRBVar& get_edge_var(vertex_t u, point_id_t pU, vertex_t v, point_id_t pV);
    std::pair<GRBVar*, GRBVar*> get_best_triplet(vertex_t u, point_id_t pU, vertex_t v,
                            point_id_t pV, vertex_t x, point_id_t pX);

  private:
    GRBEnv* m_env;
    GRBModel* m_model;

    GrbVariableMap m_vars;
    GRBVar* m_rawVariables; // just a pointer to delete afterwards
    GRBQuadExpr* m_objective;

    List<GRBVar> m_edgeVars;
    Map<TwoVertexPointPair, GRBVar*> m_edgeVarMap;
  };

}

#endif
