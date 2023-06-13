#ifndef __GD_MISC_HPP__
#define __GD_MISC_HPP__

#include <cmath>
#include <gd_types.hpp>

namespace gd
{

  inline bool isDefined(size_t id) { return id != UINT_UNDEF; }

  template<typename T>
  inline void higherTo(T& val, T otherval)
  {
    if (val < otherval) val = otherval;
  }

  template<typename T>
  inline void lowerTo(T& val, T otherval)
  {
    if (val > otherval) val = otherval;
  }

  inline double distance(const Point& p1, const Point& p2)
  {
    return std::sqrt(std::pow(static_cast<double>(p1.x - p2.x), 2)
                    + std::pow(static_cast<double>(p1.y - p2.y), 2));
  }
}

#endif
