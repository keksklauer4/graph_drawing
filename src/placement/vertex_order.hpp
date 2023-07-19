#ifndef __GD_VERTEX_ORDER_HPP__
#define __GD_VERTEX_ORDER_HPP__

#include <cstddef>
#include <gd_types.hpp>
#include <common/instance.hpp>
#include <common/misc.hpp>

namespace gd
{
  class VertexOrder
  {
    public:
      virtual vertex_t getNext() = 0;
  };

  // Order: Degree of a vertex that has not been placed yet.
  class MaxEmbeddedVertexOrder : public VertexOrder
  {
    public:
      MaxEmbeddedVertexOrder(const Instance& instance, vertex_t start = UINT_UNDEF)
        : m_instance(instance)
      {
        m_degree.resize(m_instance.m_graph.getNbVertices());
        std::fill(m_degree.begin(), m_degree.end(), 0);
        for (vertex_t v = 0; v < m_degree.size(); ++v)
        {
          m_degreeQueue.push(VertexDegreePair(v, v == start ? 1 : 0));
        }
      }

      // returns VERTEX_UNDEF if no next vertex
      vertex_t getNext() override
      {
        vertex_t v = VERTEX_UNDEF;
        while(!m_degreeQueue.empty())
        {
          const VertexDegreePair& p = m_degreeQueue.top();
          if (isDefined(m_degree[p.vertex])) { v = p.vertex; m_degreeQueue.pop(); break; }
          else { m_degreeQueue.pop(); continue; }
        }
        if (isDefined(v))
        {
          auto neighbors = m_instance.m_graph.getNeighborIterator(v);
          for (auto neighbor = neighbors.first; neighbor != neighbors.second; ++neighbor)
          {
            if (!isDefined(m_degree[*neighbor])) continue;
            m_degreeQueue.push(VertexDegreePair{*neighbor, ++m_degree[*neighbor]});
          }
          m_degree[v] = UINT_UNDEF;
        }

        return v;
      }

    private:
      const Instance& m_instance;
      Vector<size_t> m_degree;
      PriorityQueue<VertexDegreePair> m_degreeQueue;
  };

  class TrivialCountingVertexOrder : public VertexOrder
  {
    public:
      TrivialCountingVertexOrder(const Instance& instance)
        : m_size(instance.m_graph.getNbVertices()), m_next(0) {}

      // returns VERTEX_UNDEF if no next vertex
      vertex_t getNext() override
      {
        if (m_next == m_size) return UINT_UNDEF;
        return m_next++;
      }

    private:
      size_t m_size;
      size_t m_next;
  };

  class PartitioningVertexOrder : public VertexOrder
  {
    public:
      typedef Vector<vertex_t>::const_iterator vertex_iterator_t;
      typedef std::pair<vertex_iterator_t, vertex_iterator_t> vertex_range_pair_t;
      PartitioningVertexOrder(vertex_range_pair_t range): m_range(range) {}

      vertex_t getNext() override
      {
        if (m_range.first == m_range.second) return UINT_UNDEF;
        return *m_range.first++;
      }

    private:
      vertex_range_pair_t m_range;
  };

}


#endif
