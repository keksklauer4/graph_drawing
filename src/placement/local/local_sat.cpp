#include "local_sat.hpp"
#include "gd_types.hpp"
#include <utility>


using namespace gd;


void LocalSAT::optimize(LocalImprovementFunctor& functor)
{
  m_solver = new EvalMaxSAT(0);
  m_functor = &functor;
  m_satVariables.clear();
  build_problem();
}

void LocalSAT::create_variables()
{
  auto vertex_range = m_functor->get_vertex_range();
  for (auto v = vertex_range.first; v != vertex_range.second; ++v)
  {
    auto pointRange = m_functor->get_points(*v);
    for (auto p = pointRange.first; p != pointRange.second; ++p)
    {
      m_satVariables.insert(std::make_pair(vertex_point_pair_t{*v, p->second}, m_solver->newVar()));
    }
  }
}

void LocalSAT::create_vertex_mapped_csts()
{
  auto vertex_range = m_functor->get_vertex_range();
  for (auto v = vertex_range.first; v != vertex_range.second; ++v)
  {
    m_clauseVec.clear();
    auto pointRange = m_functor->get_points(*v);
    for (auto p = pointRange.first; p != pointRange.second; ++p)
    {
      m_clauseVec.push_back(m_satVariables[vertex_point_pair_t{*v, p->second}]);
    }
    m_solver->addClause(m_clauseVec);
    m_solver->AtMostOne_Pairwise(m_clauseVec);
  }

  m_clauseVec.clear();
}

void LocalSAT::create_at_most_one_vertex_mapped_to_cst()
{
  auto point_range = m_functor->get_point_range();
  for (auto p = point_range.first; p != point_range.second; ++p)
  {
    auto vertices = m_functor->get_vertices(*p);
    auto next = vertices.first; ++next;
    if (next == vertices.second) continue;
    m_clauseVec.clear();

    for (auto v = vertices.first; v != vertices.second; ++v)
    {
      m_clauseVec.push_back(m_satVariables[vertex_point_pair_t{v->second, *p}]);
    }
    m_solver->AtMostOne(m_clauseVec);
  }
  m_clauseVec.clear();
}


void LocalSAT::create_pair_collinear_csts()
{
  enumerate_pair_collinearities(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV){
      m_clauseVec.clear();
      m_clauseVec.insert(m_clauseVec.begin(),
        {-m_satVariables[vertex_point_pair_t{u, pointU}],
            -m_satVariables[vertex_point_pair_t{v, pointV}]}
      );
      m_solver->addClause(m_clauseVec);
  });
  m_clauseVec.clear();
}

void LocalSAT::create_collinear_triples_csts()
{
  enumerate_triplet_collinearities(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV,
      vertex_t w, point_id_t pointW){
      m_clauseVec.clear();
      m_clauseVec.insert(m_clauseVec.begin(),
        {-m_satVariables[vertex_point_pair_t{u, pointU}],
           -m_satVariables[vertex_point_pair_t{v, pointV}],
           -m_satVariables[vertex_point_pair_t{w, pointW}]}
      );
      m_solver->addClause(m_clauseVec);
  });
  m_clauseVec.clear();
}

void LocalSAT::create_pair_crossings()
{
  enumerate_pair_crossings(
    [&](vertex_t u, point_id_t pointU,
      vertex_t v, point_id_t pointV,
      size_t num_crossings){
      m_clauseVec.clear();
      m_clauseVec.insert(m_clauseVec.begin(),
        {-m_satVariables[vertex_point_pair_t{u, pointU}],
           -m_satVariables[vertex_point_pair_t{v, pointV}]}
      );
      m_solver->addWeightedClause(m_clauseVec, -num_crossings);
  });
  m_clauseVec.clear();
}

void LocalSAT::create_single_crossings()
{
  enumerate_single_crossings(
    [&](vertex_t u, point_id_t pointU, size_t num_crossings){
      m_clauseVec.clear();
      m_clauseVec.insert(m_clauseVec.begin(),
        {-m_satVariables[vertex_point_pair_t{u, pointU}]}
      );
      m_solver->addWeightedClause(m_clauseVec, -num_crossings);
  });
  m_clauseVec.clear();
}
