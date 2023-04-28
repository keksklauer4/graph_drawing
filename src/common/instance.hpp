#ifndef __GD_INSTANCE_HPP__
#define __GD_INSTANCE_HPP__

#include <common/graph.hpp>
#include <common/pointset.hpp>

namespace gd
{

  typedef struct Instance
  {
    Instance(Graph&& graph, PointSet&& points)
      : m_graph(graph), m_points(points) {}

    Graph m_graph;
    PointSet m_points;
  } instance_t;


}


#endif
