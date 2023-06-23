#ifndef __GD_POINTSET_HPP__
#define __GD_POINTSET_HPP__

#include <gd_types.hpp>

namespace gd
{

  class PointSet
  {
    public:
      using point_iterator_t = Vector<coordinate_t>::const_iterator;
      using point_iterator_pair_t = std::pair<point_iterator_t, point_iterator_t>;
    public:
      PointSet(){}
      PointSet(Vector<coordinate_t>&& points): m_points(points) {}

      point_iterator_pair_t getPointIterator() const
      { return std::make_pair(m_points.begin(), m_points.end()); }
      size_t getNumPoints() const { return m_points.size(); }

    private:
      Vector<coordinate_t> m_points;
  };

}

#endif
