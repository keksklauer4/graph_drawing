#include "graph.hpp"

#include <algorithm>

using namespace gd;

size_t Graph::getDegree(vertex_t vertex) const
{
  return vertex + 1 < m_vertices.size() ?
      m_vertices.at(vertex+1) - m_vertices.at(vertex) :
      m_edges.size() - m_vertices.at(vertex);
}


Graph::Graph(Vector<vertex_pair_t>& edges)
{
  if (edges.empty()) return;

  std::sort(edges.begin(), edges.end(), PairLexicographicOrdering<vertex_t>());
  auto largest_first = std::max_element(edges.begin(), edges.end(), 
      [](const auto& e1, const auto& e2){
        return e1.first < e2.first;
  });

  size_t num_vertices = largest_first->first + 1;
  m_vertices.resize(num_vertices);

  size_t edgeIdx = 0;
  m_edges.resize(edges.size());
  for (const auto& edge : edges)
  {
    m_edges[edgeIdx++] = edge.second;
    m_vertices[edge.first]++;
  }
}
