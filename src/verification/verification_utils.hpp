#ifndef __GD_VERIFICATION_UTILS_HPP__
#define __GD_VERIFICATION_UTILS_HPP__

#include <cstddef>
#include <gd_types.hpp>

namespace
{
  inline bool within_intervall(gd::coordinate_t a, gd::coordinate_t b, gd::coordinate_t c)
  {
    return (a <= b && b <= c) || (c <= b && b <= a);
  }

  inline int orientation(const gd::Point& p1, const gd::Point& p2, const gd::Point& p3)
  {
    int value = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
    return value > 0 ? 1 : ( value < 0 ? -1 : 0);
  }

}

namespace gd
{

  inline bool isCollinear(const line_2d_t& line, const Point& p)
  {
    #define B1 (line.second.first - line.first.first)
    #define B2 (line.second.second - line.first.second)
    #define C1 (p.x - line.first.first)
    #define C2 (p.y - line.first.second)

    return (B1 * C2 - B2 * C1) == 0;

    #undef B1
    #undef B2
    #undef C1
    #undef C2
  }

  inline bool operator==(const coordinate_2d_t& p1, const Point& p2)
  {
    return (p1.first == p2.x && p1.second == p2.y);
  }

  inline bool isLineEndpoint(const line_2d_t& line, const Point& p)
  {
    return line.first == p || line.second == p;
  }

  inline bool isOnLine(const line_2d_t& line, const Point& p)
  { // faster?
    return isCollinear(line, p) && !isLineEndpoint(line, p)
      && (line.first.first != line.second.first ?
        within_intervall(line.first.first, p.x, line.second.first)
      : within_intervall(line.first.second, p.y, line.second.second));
  }

  // line p1-q1 , line p2-q2
  inline bool intersect(const Point& p1, const Point& q1, const Point& p2, const Point& q2)
  {
    return (orientation(p1, q1, p2) != orientation(p1, q1, q2))
      && (orientation(p2, q2, p1) != orientation(p2, q2, q1));
  }


}

#endif
