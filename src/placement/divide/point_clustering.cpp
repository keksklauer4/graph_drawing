#include "point_clustering.hpp"

#include <common/instance.hpp>
#include <common/misc.hpp>
#include <cstddef>
#include <memory>

/*
#include <CGAL/cluster_point_set.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/compute_average_spacing.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef Kernel::Point_3 Point_3;
typedef CGAL::Point_set_3<Point_3> PointSet_3;
*/

#include <hclust/fastcluster.h>

#include <iostream>
#include <cassert>
#include <ostream>

#include <io/printing.hpp>

using namespace gd;

PointClustering::PointClustering(const Instance& instance)
  : m_instance(instance)
{ }

void PointClustering::cluster()
{
  const auto& pset = m_instance.m_points;
  size_t num_points = pset.getNumPoints();
  if (num_points == 0) return;
  auto distance = std::make_unique<double[]>(num_points * (num_points - 1) / 2);
  auto* write_dist = distance.get();
  double max_dist = 0;
  for (point_id_t i = 0; i < num_points; ++i)
  {
    for (point_id_t j = i + 1; j < num_points; ++j)
    {
      double dist = gd::distance(pset.getPoint(i), pset.getPoint(j));
      higherTo(max_dist, dist);
      *write_dist++ = dist;
    }
  }
  max_dist = 0;

  auto merge = std::make_unique<int[]>(2 * (num_points - 1));
  auto height = std::make_unique<double[]>(num_points - 1);

  hclust_fast(num_points, distance.get(), HCLUST_METHOD_COMPLETE, merge.get(), height.get());

  auto labels = std::make_unique<int[]>(num_points);

  size_t nclust = 5;

  cutree_k(num_points, merge.get(), nclust, labels.get());
  cutree_cdist(num_points, merge.get(), height.get(), max_dist, labels.get());

  m_pointToCluster.resize(num_points);
  for (size_t i = 0; i < (num_points-1); ++i)
  {
    int a = merge[2*i];
    int b = merge[2*i + 1];
    if (a < 0 && b < 0) m_clusters.push_back(Cluster{(size_t)-(a+1), (size_t)-(b+1), true, true});
    else if (a < 0) m_clusters.push_back(Cluster{(size_t)-(a+1), (size_t)b-1, true, false});
    else if (b < 0) m_clusters.push_back(Cluster{(size_t)(a-1), (size_t)-(b+1), false, true});

    if (a < 0) m_pointToCluster[-(a+1)] = m_clusters.size() - 1;
    if (b < 0) m_pointToCluster[-(b+1)] = m_clusters.size() - 1;
  }

/*
  PointSet_3 points;
  CoordinateToIdMap pointMap{};
  auto point_range = m_instance.m_points.getPointIterator();
  for (auto it = point_range.first; it != point_range.second; ++it)
  {
    points.insert(Point_3(it->x, it->y, 0));
    pointMap.insert(it->getCoordToId());
  }

  double spacing = CGAL::compute_average_spacing<CGAL::Parallel_if_available_tag> (points, 4);

  PointSet_3::Property_map<size_t> cluster_map = points.add_property_map<size_t>("cluster", UINT_UNDEF).first;

  size_t nb_clusters = CGAL::cluster_point_set(points, cluster_map,
                                             points.parameters().neighbor_radius(spacing));

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

  m_pointToCluster.resize(m_points.size());
  for(cluster_t cluster_idx = 0; cluster_idx < m_clusters.size(); ++cluster_idx)
  {
    const auto& cluster = m_clusters.at(cluster_idx);
    for (size_t idx = cluster.m_offset; idx < (cluster.m_offset + cluster.m_size); ++idx)
    {
      m_pointToCluster.at(m_points.at(idx)) = cluster_idx;
    }
  }

*/


}
