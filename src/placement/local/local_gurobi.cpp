#include "local_gurobi.hpp"
#include "gd_types.hpp"
#include "placement/local/local_reopt.hpp"

#include <common/misc.hpp>
#include <cstddef>
#include <io/printing.hpp>


using namespace gd;

LocalGurobi::~LocalGurobi()
{
}

void LocalGurobi::optimize(LocalImprovementFunctor& functor)
{
  m_functor = &functor;
  m_vars.clear();
  m_edgeVars.clear();
  m_edgeVarMap.clear();

  m_env = new GRBEnv();
  m_env->set(GRB_DoubleParam_TimeLimit, 10);
  m_model = new GRBModel(*m_env);
  m_model->set(GRB_IntParam_Threads, 1);
  m_model->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
  m_objective = new GRBQuadExpr();
  build_problem();
  m_model->setObjective(*m_objective);
  m_model->set("NonConvex", "2.0");

  m_model->write("out.lp");
  m_model->write("out.mps");
  m_model->write("out.mst");
  m_model->write("out.rew");

  m_model->optimize();
  if (m_model->get(GRB_IntAttr_SolCount) > 0)
  {
    size_t idx = 0;
    auto vertexRange = m_functor->get_vertex_range();
    for (auto v = vertexRange.first; v != vertexRange.second; ++v)
    {
      auto pointRange = m_functor->get_points(*v);
      for (auto p = pointRange.first; p != pointRange.second; ++p)
      {
        double val = m_vars[std::make_pair(*v, p->second)]->get(GRB_DoubleAttr_X);
        if (val > 0.99) functor.set_mapped(idx++, p->second);
      }
    }
  }

  delete[] m_rawVariables;
  delete m_objective;
  delete m_model;
  delete m_env;
}

void LocalGurobi::create_variables()
{
  m_rawVariables = m_model->addVars(m_functor->get_num_vars(), GRB_BINARY);
  GRBVar* nextVar = m_rawVariables;
  auto vertex_range = m_functor->get_vertex_range();
  for (auto v = vertex_range.first; v != vertex_range.second; ++v)
  {
    auto pointRange = m_functor->get_points(*v);
    for (auto p = pointRange.first; p != pointRange.second; ++p)
    {
      m_vars.insert(std::make_pair(vertex_point_pair_t{*v, p->second}, nextVar++));
    }
  }
  m_functor->get_mapping([&](vertex_t v, point_id_t p){
    if (!isDefined(p) || !m_vars.contains(vertex_point_pair_t{v,p})) return;

    m_vars[vertex_point_pair_t{v,p}]->set(GRB_DoubleAttr_Start, 1.0);
  });
}

void LocalGurobi::create_vertex_mapped_csts()
{
  auto vertex_range = m_functor->get_vertex_range();
  for (auto v = vertex_range.first; v != vertex_range.second; ++v)
  {
    GRBLinExpr exactOne = 0;
    auto pointRange = m_functor->get_points(*v);
    for (auto p = pointRange.first; p != pointRange.second; ++p)
    {
      exactOne += *m_vars[vertex_point_pair_t{*v, p->second}];
    }
    m_model->addConstr(exactOne == 1);
  }

}

void LocalGurobi::create_at_most_one_vertex_mapped_to_cst()
{
  auto point_range = m_functor->get_point_range();
  for (auto p = point_range.first; p != point_range.second; ++p)
  {
    auto vertices = m_functor->get_vertices(*p);
    auto next = vertices.first; ++next;
    if (next == vertices.second) continue;
    GRBLinExpr atMostOne = 0;
    for (auto v = vertices.first; v != vertices.second; ++v)
    {
      atMostOne += *m_vars[vertex_point_pair_t{v->second, *p}];
    }
    m_model->addConstr(atMostOne <= 1);
  }
}


void LocalGurobi::create_pair_collinear_csts()
{
  enumerate_pair_collinearities(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV){

        m_model->addConstr(
          (*m_vars[vertex_point_pair_t{u, pointU}]
          + *m_vars[vertex_point_pair_t{v, pointV}]) <= 1
        );

  });
}

void LocalGurobi::create_collinear_triples_csts()
{
  enumerate_triplet_collinearities(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV,
      vertex_t w, point_id_t pointW){

        m_model->addConstr(
          (*m_vars[vertex_point_pair_t{u, pointU}]
          + *m_vars[vertex_point_pair_t{v, pointV}]
          + *m_vars[vertex_point_pair_t{w, pointW}]) <= 2
        );

  });
}


void LocalGurobi::create_pair_crossings()
{
  enumerate_pair_crossings(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV,
      size_t num_crossings){

      *m_objective += num_crossings
          * (*m_vars[vertex_point_pair_t{u, pointU}])
          * (*m_vars[vertex_point_pair_t{v, pointV}]);

  });

}

void LocalGurobi::create_single_crossings()
{
  enumerate_single_crossings(
    [&](vertex_t u, point_id_t pointU, size_t num_crossings){

      *m_objective += num_crossings * (*m_vars[vertex_point_pair_t{u, pointU}]);
  });
}

void LocalGurobi::create_internal_crossings()
{
  enumerate_internal_crossings(
    [&](vertex_t u, point_id_t pU, vertex_t v, point_id_t pV,
        vertex_t x, point_id_t pX, vertex_t y, point_id_t pY) {

      GRBVar& e1 = get_edge_var(u, pU, v, pV);
      GRBVar& e2 = get_edge_var(x, pX, y, pY);
      *m_objective += e1 * e2;
  });
}

GRBVar& LocalGurobi::get_edge_var(vertex_t u, point_id_t pU,
                                  vertex_t v, point_id_t pV)
{
  TwoVertexPointPair p { u, pU, v, pV };
  GRBVar*& var = m_edgeVarMap[p];
  if (var == nullptr)
  {
    auto it = m_edgeVars.emplace(m_edgeVars.begin(), m_model->addVar(0,1,0,GRB_BINARY));
    var = &(*it);

    // create constraints to enforce the relation between the edge and the atual vars
    GRBVar pointvars[2] = {
      *m_vars[vertex_point_pair_t{u, pU}],
      *m_vars[vertex_point_pair_t{v, pV}]
    };

    m_model->addGenConstrAnd(*var, pointvars, 2);
  }
  return *var;
}

void LocalGurobi::create_semi_internal_crossings()
{
  enumerate_semi_internal_crossings(
    [&](vertex_t u, point_id_t pU, vertex_t v, point_id_t pV,
        vertex_t x, point_id_t pX, size_t num_crossings) {

      auto vars = get_best_triplet(u, pU, v, pV, x, pX);
      *m_objective += num_crossings * (*vars.first) * (*vars.second);
  });
}

std::pair<GRBVar*, GRBVar*> LocalGurobi::get_best_triplet(
    vertex_t u, point_id_t pU, vertex_t v,
    point_id_t pV, vertex_t x, point_id_t pX)
{
  auto findIt = m_edgeVarMap.find(TwoVertexPointPair(u,pU,v,pV));
  if (findIt != m_edgeVarMap.end())
  { return std::make_pair(findIt->second, m_vars[vertex_point_pair_t{x, pX}]); }

  findIt = m_edgeVarMap.find(TwoVertexPointPair(u,pU,x,pX));
  if (findIt != m_edgeVarMap.end())
  { return std::make_pair(findIt->second, m_vars[vertex_point_pair_t{v, pV}]); }

  findIt = m_edgeVarMap.find(TwoVertexPointPair(v,pV,x,pX));
  if (findIt != m_edgeVarMap.end())
  { return std::make_pair(findIt->second, m_vars[vertex_point_pair_t{u, pU}]); }

  return std::make_pair(&get_edge_var(u, pU, v, pV), m_vars[vertex_point_pair_t{x, pX}]);
}

void LocalGurobi::create_pair_neighbor_crossings()
{
  enumerate_pair_neighbor_crossings(
    [&](vertex_t u, point_id_t pU, vertex_t x, point_id_t pX, size_t num_crossings) {

      *m_objective += num_crossings
          * (*m_vars[vertex_point_pair_t{u, pU}])
          * (*m_vars[vertex_point_pair_t{x, pX}]);
  });
}
