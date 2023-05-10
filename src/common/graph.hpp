#ifndef __GD_GRAPH_HPP__
#define __GD_GRAPH_HPP__

#include <cstddef>
#include <gd_types.hpp>
#include <rapidjson/rapidjson.h>

namespace gd
{

  class AdjacencyArrayIterator
  {
    public:
      AdjacencyArrayIterator(size_t numEdges)
        : m_edgeIdx(numEdges) {}

      AdjacencyArrayIterator(const Vector<vertex_t>& vertices, const Vector<vertex_t>& edges)
        : m_vertices(&vertices), m_edges(&edges), m_vertexIdx(0), m_edgeIdx(0)
          { setAppropriateVertexIdx(); }

      vertex_pair_t operator*() const
      { return vertex_pair_t { m_vertexIdx, m_edges->at(m_edgeIdx) }; }
      void operator++();
      friend bool operator!=(const AdjacencyArrayIterator& it1, const AdjacencyArrayIterator& it2)
      { return it1.m_edgeIdx != it2.m_edgeIdx; }

    private:
      void setAppropriateVertexIdx();

    private:
      const Vector<vertex_t>* m_vertices;
      const Vector<vertex_t>* m_edges;

      size_t m_vertexIdx;
      size_t m_edgeIdx;
  };

  class Graph
  {
    public:
      typedef std::pair<Vector<vertex_t>::const_iterator,Vector<vertex_t>::const_iterator> neighbor_iterator_t;

    public:
      Graph() {}
      Graph(Vector<vertex_pair_t>& edges);

      size_t getNbVertices() const { return m_vertices.size(); }
      size_t getNbEdges() const { return m_edges.size() / 2; }
      size_t getDegree(vertex_t vertex) const;

      neighbor_iterator_t getNeighborIterator(vertex_t vertex) const;
      AdjacencyArrayIterator begin() const { return AdjacencyArrayIterator(m_vertices, m_edges); }
      AdjacencyArrayIterator end() const { return AdjacencyArrayIterator(m_edges.size()); }

    private:
      Vector<vertex_t> m_vertices;
      Vector<vertex_t> m_edges;

  };


}






#endif
