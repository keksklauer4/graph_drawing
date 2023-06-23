#ifndef __GD_GRAPH_PARTITIONING_HPP__
#define __GD_GRAPH_PARTITIONING_HPP__

#include "common/instance.hpp"
#include "common/misc.hpp"
#include <gd_types.hpp>
#include <memory>
#include <ostream>
#include <utility>

namespace gd
{
  namespace partitioning
  {
    struct Interval
    {
      Interval(): start(0), end(0) {}
      Interval(size_t s, size_t e): start(s), end(e) {}

      size_t size() const { return (end - start); }
      bool contains(size_t idx) const
      { return start <= idx && idx < end; }

      size_t start;
      size_t end; // exclusive
    };

    struct Partition
    {
      Partition(): interval(), left_child(UINT_UNDEF),
        right_child(UINT_UNDEF), parent(UINT_UNDEF) {}
      Partition(Interval inter, size_t par = UINT_UNDEF)
        : interval(inter), left_child(UINT_UNDEF),
          right_child(UINT_UNDEF), parent(par) {}

      size_t size() const { return interval.size(); }
      bool empty() const { return size() == 0; }

      bool has_children() const { return isDefined(left_child); }

      Interval interval;
      size_t left_child;
      size_t right_child;
      size_t parent;
    };

    struct PartitioningConfig
    {
      PartitioningConfig(const Graph& graph, const Vector<int>& vertices,
                         const Vector<int>& inverseVertices);

      void rebuild(const Interval& interval);
      void call_partitioner();
      bool isInPartition(size_t idx) const;

    private:
      const Vector<int>& m_vertices;
      const Vector<int>& m_inverseVertices;
      const Graph& m_graph;

      std::unique_ptr<int[]> m_partition;
      std::unique_ptr<int[]> m_lengths;
      std::unique_ptr<int[]> m_adjacency;

      int n;
      double imbalance;
      int edge_cut;
      int nparts;
    };
  }

  class HierarchicalGraph
  {
    public:
      typedef Vector<vertex_t>::const_iterator vertex_iterator_t;
      typedef std::pair<vertex_iterator_t, vertex_iterator_t> vertex_range_iterator_t;
      HierarchicalGraph(const Graph& graph, const Vector<int>& vertices,
        const Vector<partitioning::Partition>& partitions);

      vertex_range_iterator_t get_range() const
      { return std::make_pair(m_vertices.begin(), m_vertices.end()); }

      friend std::ostream& operator<<(std::ostream& os, const HierarchicalGraph& g)
      {
        if (g.m_vertices.size() == 0) { os << "(0)"; return os; }
        os << g.m_partitions.size() << std::endl;
        /*
        Stack<size_t> stack{};
        stack.push(0);
        while(!stack.empty())
        {
          os << "("
        }*/
        return os;
      }

    private:
      const Graph& m_graph;
      Vector<vertex_t> m_vertices;
      Vector<partitioning::Partition> m_partitions;
  };

  class HierarchicalGraphBuilder
  {
    public:
      HierarchicalGraphBuilder(const instance_t& instance);
      HierarchicalGraph partition();

    private:
      size_t dfs(const partitioning::Partition& partition);
      void add_partition(Stack<size_t>& remaining_parts, size_t parentIdx,
        size_t partition_size, bool left_connected = false);
      size_t find_partition(partitioning::PartitioningConfig& partitioner, partitioning::Partition& parent);
      void adjust_inverses(const partitioning::Interval& interval);

    private:
      const instance_t& m_instance;
      Vector<int> m_vertices;
      Vector<int> m_inverseVertices; // index of v in m_vertices
      Vector<partitioning::Partition> m_partitions;
      Vector<bool> m_connected;
      Vector<bool> m_isInCurrentPartition;

      // for the dfs
      Stack<Graph::neighbor_iterator_t> m_dfsStack;
      Vector<vertex_t> m_currPartition;
      Vector<bool> m_visited;
  };


}



#endif
