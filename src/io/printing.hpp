#ifndef __GD_PRINTING_HPP__
#define __GD_PRINTING_HPP__

#include "common/instance.hpp"
#include <iostream>
#include <gd_types.hpp>
#include <ostream>

namespace gd
{
  template<typename T>
  std::ostream& operator<<(std::ostream& os, const Vector<T>& v)
  {
    for (const auto& val : v)
    {
      os << val << "\n";
    }
    return os;
  }


  template<typename T>
  std::ostream& operator<<(std::ostream& os, const Set<T>& v)
  {
    for (const auto& val : v)
    {
      os << val << "\n";
    }
    return os;
  }


  template<typename K, typename V>
  std::ostream& operator<<(std::ostream& os, const std::pair<K,V>& p)
  {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
  }

  inline std::ostream& operator<<(std::ostream& os, const Point& p)
  {
    os << "point " << p.id << " at " << p.getCoordPair();
    return os;
  }

  void printAssignment(std::ostream& os, const Instance& instance, const VertexAssignment& assignment);
}


#endif
