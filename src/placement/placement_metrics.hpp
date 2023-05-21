#ifndef __GD_PLACEMENT_METRIC_HPP__
#define __GD_PLACEMENT_METRIC_HPP__

#include "common/instance.hpp"
#include <gd_types.hpp>

namespace gd
{
  double squared_distance(const Point& p1, const Point& p2);

  double squared_neighbor_distance_metric(const Instance& instance,
    const VertexAssignment& assignment, vertex_t v, point_id_t p);

}


#endif
