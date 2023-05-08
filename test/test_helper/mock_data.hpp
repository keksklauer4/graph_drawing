#ifndef __GD_MOCK_DATA_HPP__
#define __GD_MOCK_DATA_HPP__

#include <common/assignment.hpp>
#include <common/instance.hpp>
#include <common/graph.hpp>
#include <common/pointset.hpp>
#include <gd_types.hpp>

#include <initializer_list>

#define PAIR(a,b) (std::make_pair(a,b))

namespace gd
{

  inline Graph create_mock_graph(std::initializer_list<vertex_pair_t> edgelist)
  {
    Vector<vertex_pair_t> edges{};
    edges.insert(edges.begin(), edgelist.begin(), edgelist.end());
    for (const auto& edge : edgelist) edges.push_back(reversePair(edge));
    return Graph(edges);
  }

  inline PointSet create_mock_pointset(std::initializer_list<Point> pointlist)
  {
    Vector<Point> points{};
    points.insert(points.begin(), pointlist.begin(), pointlist.end());
    return PointSet(std::move(points));
  }

  inline VertexAssignment create_mock_assignment(size_t num_vertices, std::initializer_list<std::pair<vertex_t, point_id_t>> as)
  {
    VertexAssignment assignment {num_vertices};
    for (const auto& assigned : as) assignment.assign(assigned.first, assigned.second);
    return assignment;
  }

  inline instance_t create_mock_instance(std::initializer_list<vertex_pair_t> edgelist, std::initializer_list<Point> pointlist)
  {
    return instance_t{create_mock_graph(edgelist), create_mock_pointset(pointlist)};
  }
}



#endif
