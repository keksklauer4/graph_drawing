#include "placement_metrics.hpp"

#include <gd_types.hpp>
#include <common/instance.hpp>
#include <common/assignment.hpp>
#include <cmath>

using namespace gd;

double gd::squared_distance(const Point& p1, const Point& p2)
{
  return std::pow((double)(p1.x - p2.x), 2) + std::pow((double)(p1.y - p2.y), 2);
}

double gd::squared_neighbor_distance_metric(const Instance& instance,
  const VertexAssignment& assignment, vertex_t v, point_id_t p)
{
  const Point& point = instance.m_points.getPoint(p);
  double dist = 0;
  auto neighbors = instance.m_graph.getNeighborIterator(v);
  for (auto w = neighbors.first; w != neighbors.second; ++w)
  {
    if (!assignment.isAssigned(*w)) continue;
    dist += squared_distance(point, instance.m_points.getPoint(assignment.getAssigned(*w)));
  }
  return dist;
}

