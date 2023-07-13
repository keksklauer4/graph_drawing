#ifndef __GD_POINTSET_HPP__
#define __GD_POINTSET_HPP__

#include <gd_types.hpp>

namespace gd
{

  class PointSet
  {
    public:
      using point_iterator_t = Vector<point_t>::const_iterator;
      using point_iterator_pair_t = std::pair<point_iterator_t, point_iterator_t>;
    public:
      PointSet(){}
      PointSet(Vector<point_t>&& points): m_points(points)
      {
        std::sort(m_points.begin(), m_points.end(),
          [](const Point& p1, const Point& p2) -> bool{
            return p1.id < p2.id;
        });
      }

      point_iterator_pair_t getPointIterator() const
      { return std::make_pair(m_points.begin(), m_points.end()); }
      size_t getNumPoints() const { return m_points.size(); }
      const point_t& getPoint(point_id_t id) const { return m_points.at(id); }

      point_iterator_t begin() const { return m_points.begin(); }
      point_iterator_t end() const { return m_points.end(); }

    private:
      Vector<point_t> m_points;
  };

}

#endif
