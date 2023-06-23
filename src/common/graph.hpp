#ifndef __GD_GRAPH_HPP__
#define __GD_GRAPH_HPP__

#include <gd_types.hpp>
#include <rapidjson.h>

namespace gd
{

  class Graph
  {
    public:
      Graph() {}

      size_t getNbVertices() const { return m_edges.size(); }
      size_t getNbEdges() const { return m_edges.size(); }
      size_t getDegree(vertex_t vertex) const;

    private:
      Vector<vertex_t> m_vertices;
      Vector<vertex_t> m_edges;

  };

}






#endif
