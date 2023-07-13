#include "local_reopt.hpp"
#include "gd_types.hpp"

#include <common/assignment.hpp>

using namespace gd;

void LocalImprovementFunctor::record_previous_mapping(const VertexAssignment& assignment)
{
  m_previousMapping.resize(m_vertices.size());
  auto* write_ptr = m_previousMapping.data();
  for (auto vertex : m_vertices)
  {
    *write_ptr++ = (assignment.isAssigned(vertex) ?
                      assignment.getAssigned(vertex) : UINT_UNDEF);
  }
}

void LocalImprovementFunctor::reset_base()
{
  m_vertices.clear();
  m_pointIds.clear();
  m_vertexToPoint.clear();
  m_pointToVertex.clear();
  m_previousMapping.clear();
  m_valid = false;
}

void LocalImprovementFunctor::build_datastructures()
{
  Set<point_id_t> points{};
  for (const auto& pair : m_vertexToPoint)
  {
    m_pointToVertex.insert(reversePair(pair));
    points.insert(pair.second);
  }
  if (points.size() < m_vertices.size()) m_valid = false;
  else
  {
    m_pointIds.insert(m_pointIds.begin(), points.begin(), points.end());
  }
}

void LocalReOpt::build_datastructures()
{
  // build subgraph
  const auto& pset = m_instance.m_points;
  const auto& graph = m_instance.m_graph;

  auto vertex_range = m_functor->get_vertex_range();
  for (auto it1 = vertex_range.first; it1 != vertex_range.second; ++it1)
  {
    for (auto it2 = it1 + 1; it2 != vertex_range.second; ++it2)
    {
      if (*it1 != *it2 && graph.connected(*it1, *it2))
      {
        m_subgraph.insert(getOrderedPair(*it1, *it2));
      }
    }
  }

  // prepare existing edges
  m_existing_edges.reserve(m_instance.m_graph.getNbEdges());
  for (const auto& edge : m_instance.m_graph)
  {
    if (edge.first < edge.second &&
      m_assignment.isAssigned(edge.first) && m_assignment.isAssigned(edge.second))
    {
      m_existing_edges.push_back(pointid_pair_t{
        m_assignment.getAssigned(edge.first),
        m_assignment.getAssigned(edge.second)
      });
    }
  }
}
