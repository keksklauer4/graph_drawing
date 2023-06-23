#include "graph_partitioning.hpp"
#include "gd_types.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>

#include <kaHIP_interface.h>
#include <io/printing.hpp>
#include <common/misc.hpp>

using namespace gd;
using namespace gd::partitioning;
#define MAX_PARTITION_SIZE 7


HierarchicalGraphBuilder::HierarchicalGraphBuilder(const instance_t& instance)
  : m_instance(instance)
{
  const auto& graph = m_instance.m_graph;

  m_visited.insert(m_visited.begin(), graph.getNbVertices(), false);
  m_isInCurrentPartition.insert(m_isInCurrentPartition.begin(), graph.getNbVertices(), false);
  m_partitions.reserve(graph.getNbVertices());
  m_connected.reserve(graph.getNbVertices());

  m_vertices.resize(graph.getNbVertices());
  m_inverseVertices.resize(graph.getNbVertices());
  for (vertex_t v = 0; v < graph.getNbVertices(); ++v)
  {
    m_vertices[v] = static_cast<int>(v);
    m_inverseVertices[v] = v;
  }
}

HierarchicalGraph HierarchicalGraphBuilder::partition()
{
  const auto& graph = m_instance.m_graph;
  PartitioningConfig partitioner{graph, m_vertices, m_inverseVertices};
  Stack<size_t> remaining_parts{};
  m_partitions.push_back(Partition{ Interval{0, m_vertices.size() } });
  m_connected.push_back(false);
  remaining_parts.push(0);

  while(!remaining_parts.empty())
  {
    size_t parentIdx = remaining_parts.top();
    Partition& parent = m_partitions[parentIdx];

    remaining_parts.pop();
    if (parent.size() <= MAX_PARTITION_SIZE) continue;
    size_t partition_size = (!m_connected[parentIdx] ? dfs(parent) : UINT_UNDEF);
    if(isDefined(partition_size))
    { // if partition is disconnected
      add_partition(remaining_parts, parentIdx, partition_size, true);
    }
    else
    { // partition is not disconnected, hence, partition
      size_t partition_size = find_partition(partitioner, parent);
      add_partition(remaining_parts, parentIdx, partition_size);
    }
  }

  return HierarchicalGraph{
    m_instance.m_graph,
    m_vertices,
    m_partitions
  };
}

size_t HierarchicalGraphBuilder::find_partition(PartitioningConfig& partitioner,
                                         Partition& parent)
{
  partitioner.rebuild(parent.interval);
  partitioner.call_partitioner();
  size_t partition_size = 0;
  for (size_t i = parent.interval.start; i < parent.interval.end; ++i)
  {
    if (partitioner.isInPartition(i - parent.interval.start))
    {
      m_isInCurrentPartition[m_vertices[i]] = true;
      partition_size++;
    }
  }

  std::partition(m_vertices.begin() + parent.interval.start,
                 m_vertices.begin() + parent.interval.end,
    [&](int vertex){
      return m_isInCurrentPartition.at(vertex);
  });

  for (size_t i = parent.interval.start; i < parent.interval.end; ++i)
  { m_isInCurrentPartition[m_vertices[i]] = false; }

  adjust_inverses(parent.interval);
  return partition_size;
}

void HierarchicalGraphBuilder::add_partition(
    Stack<size_t>& remaining_parts, size_t parentIdx,
    size_t partition_size, bool left_connected)
{
  Partition& parent = m_partitions.at(parentIdx);
  parent.left_child = m_partitions.size();
  parent.right_child = parent.left_child + 1;
  remaining_parts.push(parent.left_child);
  remaining_parts.push(parent.right_child);
  m_partitions.push_back(Partition{
    Interval{ parent.interval.start, parent.interval.start + partition_size }, parentIdx });
  m_partitions.push_back(Partition{
    Interval{ parent.interval.start + partition_size, parent.interval.end }, parentIdx });
  m_connected.push_back(left_connected);
  m_connected.push_back(false);
}


size_t HierarchicalGraphBuilder::dfs(const Partition& partition)
{
  if(partition.empty()) return UINT_UNDEF;
  vertex_t start = (vertex_t)m_vertices[partition.interval.start];
  m_dfsStack.push(m_instance.m_graph.getNeighborIterator(start));
  m_visited[start] = true;

  size_t psize = 1;
  while(!m_dfsStack.empty())
  {
    auto& top = m_dfsStack.top();
    if (top.first == top.second) m_dfsStack.pop();
    else if(!partition.interval.contains(m_inverseVertices[*(top.first)]))
    { top.first++; }
    else if (!m_visited[*(top.first)])
    {
      vertex_t next = *(top.first);
      psize++;
      m_visited[next] = true;
      top.first++;
      m_dfsStack.push(m_instance.m_graph.getNeighborIterator(next));
    }
    else top.first++;
  }

  size_t result = UINT_UNDEF;
  if (psize < partition.size())
  { // partition is disconnected
    std::partition(m_vertices.begin() + partition.interval.start,
                   m_vertices.begin() + partition.interval.end,
      [&](int vertex){
        return m_visited[vertex];
    });
    adjust_inverses(partition.interval);
    result = psize;
  }

  for (size_t idx = partition.interval.start; idx < partition.interval.end; ++idx)
  { m_visited[idx] = false; }
  return result;
}

void HierarchicalGraphBuilder::adjust_inverses(const Interval& interval)
{
  for (size_t i = interval.start; i < interval.end; ++i)
  {
    m_inverseVertices[m_vertices[i]] = i;
  }
}

#define IMBALANCE_CONFIG 0.4

PartitioningConfig::PartitioningConfig(const Graph& graph,
    const Vector<int>& vertices, const Vector<int>& inverseVertices)
  : m_vertices(vertices), m_inverseVertices(inverseVertices), m_graph(graph),
    m_partition(std::make_unique<int[]>(graph.getNbVertices())),
    m_lengths(std::make_unique<int[]>(graph.getNbVertices() + 1)),
    m_adjacency(std::make_unique<int[]>(graph.getNbEdges() * 2)),
    n((int)graph.getNbVertices()), imbalance(IMBALANCE_CONFIG),
    edge_cut(0), nparts(2)
{ }

void PartitioningConfig::rebuild(const Interval& interval)
{
  auto* write_ptr = m_adjacency.get();
  m_lengths[0] = 0;
  size_t offset = 0;

  size_t vertex_idx = 0;
  for (size_t idx = interval.start; idx < interval.end; ++idx)
  {
    auto range = m_graph.getNeighborIterator(m_vertices.at(idx));
    for (auto it = range.first; it != range.second; ++it)
    {
      if(interval.contains(m_inverseVertices.at(*it)))
      {
        *write_ptr++ = static_cast<int>(m_inverseVertices.at(*it) - interval.start);
        offset++;
      }
    }
    m_lengths[++vertex_idx] = offset;
  }
  n = static_cast<int>(interval.size());
  edge_cut = 0;
  nparts = 2;
  imbalance = IMBALANCE_CONFIG;
}

void PartitioningConfig::call_partitioner()
{
  kaffpa(&n, NULL, m_lengths.get(),
    NULL, m_adjacency.get(),
    &nparts, &imbalance,
    false, 0, ECO,
    &edge_cut, m_partition.get()
  );
}

bool PartitioningConfig::isInPartition(size_t idx) const
{
  return m_partition[idx] == 0;
}

HierarchicalGraph::HierarchicalGraph(const Graph& graph,
        const Vector<int>& vertices, const Vector<partitioning::Partition>& partitions)
  : m_graph(graph)
{
  m_vertices.reserve(vertices.size());
  m_vertices.insert(m_vertices.begin(), vertices.begin(), vertices.end());

  m_partitions.reserve(partitions.size());
  m_partitions.insert(m_partitions.begin(),
    partitions.begin(), partitions.end());
}
