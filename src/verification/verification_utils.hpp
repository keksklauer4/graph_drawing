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

}

#endif
