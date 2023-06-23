#ifndef __GD_MISC_HPP__
#define __GD_MISC_HPP__

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

}

#endif
