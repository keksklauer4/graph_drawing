#include "point_clustering.hpp"
#include "gd_types.hpp"

#include <common/instance.hpp>

#include <CGAL/cluster_point_set.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>

using namespace gd;

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Point_set_3<Point_3> PointSet_3;


PointClustering::PointClustering(const Instance& instance)
  : m_instance(instance)
{ }

void PointClustering::cluster()
{
  PointSet_3 points;
  CoordinateToIdMap pointMap{};
  auto point_range = m_instance.m_points.getPointIterator();
  for (auto it = point_range.first; it != point_range.second; ++it)
  {
    points.insert(Point_3(it->x, it->y, 0));
    pointMap.insert(it->getCoordToId());
  }

  PointSet_3::Property_map<size_t> cluster_map = points.add_property_map<size_t>("cluster", UINT_UNDEF).first;
std::vector<std::pair<size_t, size_t> > adjacencies;
  size_t nb_clusters = CGAL::cluster_point_set(points, cluster_map,
                                             points.parameters().neighbor_radius(1.0)
                                                 .adjacencies(std::back_inserter(adjacencies)));

  m_clusters.resize(nb_clusters);

  for(PointSet_3::Index idx : points)
  {
    m_clusters[cluster_map[idx]].m_size++;
  }
  size_t offset = 0;
  for (Cluster& c : m_clusters)
  {
    c.m_offset = offset;
    offset += c.m_size;
  }
  m_points.resize(offset);
  for (const PointSet_3::Index& idx : points)
  {
    const auto& point = points.point(idx);
    m_points[m_clusters[cluster_map[idx]].m_offset++] =
          pointMap[coordinate_2d_t{point.x(), point.y()}];
  }

  for (Cluster& c : m_clusters) c.m_offset -= c.m_size;
}

