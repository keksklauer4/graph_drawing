#include "graph.hpp"

using namespace gd;

size_t Graph::getDegree(vertex_t vertex) const
{
  return vertex + 1 < m_vertices.size() ?
      m_vertices.at(vertex+1) - m_vertices.at(vertex) :
      m_edges.size() - m_vertices.at(vertex);
}


