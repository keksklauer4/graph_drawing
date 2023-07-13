#include "kd_tree.hpp"

#include <boost/tuple/tuple.hpp>
#include <common/pointset.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <memory>

using namespace gd;

KdTree::KdTree(const PointSet& pset)
  : m_pset(pset)
{
  m_points.resize(pset.getNumPoints());
  m_pointIds.resize(pset.getNumPoints());

  auto* points = &m_points.at(0);
  auto* ids = &m_pointIds.at(0);
  for (const auto& point : m_pset)
  {
    *points++ = Point_2(point.x, point.y);
    *ids++ = point.id;
  }

  m_tree = std::unique_ptr<Neighbor_search::Tree>(
      new Neighbor_search::Tree (
          boost::make_zip_iterator(boost::make_tuple( m_points.begin(),m_pointIds.begin())),
          boost::make_zip_iterator(boost::make_tuple( m_points.end(),m_pointIds.end())))
  );
}


void KdTree::k_nearest_neighbors(const Point& p, size_t k,
  std::function<bool(size_t)> func) const
{
  if (k == 0) return;
  size_t remaining = k;
  Neighbor_search search(*m_tree, Point_2(p.x, p.y), m_points.size());
  for(Neighbor_search::iterator it = search.begin(); it != search.end(); it++)
  {
    if (func(boost::get<1>(it->first)) && (--remaining) == 0) return;
  }
}
