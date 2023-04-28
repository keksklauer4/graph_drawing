#ifndef __GD_GRAPH_HPP__
#define __GD_GRAPH_HPP__

#include <gd_types.hpp>
#include <rapidjson/rapidjson.h>

namespace gd
{

  class Graph
  {
    public:
      Graph() {}
      Graph(Vector<vertex_pair_t>& edges);

      size_t getNbVertices() const { return m_vertices.size(); }
      size_t getNbEdges() const { return m_edges.size() / 2; }
      size_t getDegree(vertex_t vertex) const;

    private:
      Vector<vertex_t> m_vertices;
      Vector<vertex_t> m_edges;

  };

}






#endif
