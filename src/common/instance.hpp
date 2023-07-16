#ifndef __GD_INSTANCE_HPP__
#define __GD_INSTANCE_HPP__

#include <common/graph.hpp>
#include <common/pointset.hpp>
#include <common/time_tracker.hpp>
#include <common/run_statistics.hpp>

namespace gd
{
  typedef struct Instance
  {
    Instance(Graph&& graph, PointSet&& points)
      : m_graph(graph), m_points(points),
        m_timer()
        STATS(COMMA m_stats(m_timer, graph.getNbVertices(), graph.getNbEdges(), points.getNumPoints()))
        {}

    Graph m_graph;
    PointSet m_points;
    TimeTracker m_timer;
    STATS(mutable RunStatistics m_stats;)
  } instance_t;


}


#endif
