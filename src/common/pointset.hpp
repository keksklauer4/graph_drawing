#ifndef __GD_POINTSET_HPP__
#define __GD_POINTSET_HPP__

#include <gd_types.hpp>

namespace gd
{

  class PointSet
  {
    public:
      PointSet(){}
      PointSet(Vector<coordinate_t>&& points): m_points(points) {}

      size_t getNumPoints() const { return m_points.size(); }

    private:
      Vector<coordinate_t> m_points;
  };

}

#endif
