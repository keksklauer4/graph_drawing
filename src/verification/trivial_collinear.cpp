#include "collinear.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/enum.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_2.h>

#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <common/instance.hpp>

#include <numeric>

#define SCALE 2

using namespace gd;

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef CGAL::Point_set_2<Kernel>::Vertex_handle Vertex_handle;
typedef Kernel::Point_2 Point_2;

typedef UnorderedMap<coordinate_2d_t, point_id_t, PairHashFunc<coordinate_t>> CoordinateToIdMap;

typedef std::pair<Point_2, Point_2> point2_pair_t;

struct FakeIterator
{
  public:
    FakeIterator(const VertexAssignment& as, const CoordinateToIdMap& m, bool& failed)
      : m_assignment(as), m_coordinateToId(m), m_failed(failed) {}


    void reset(const point2_pair_t& line)
    {
      m_handle = Vertex_handle();
      m_failed = false;
      m_line = &line;
    }

    void operator++(int) { if (!m_failed) checkMapped(); }
    Vertex_handle& operator*() { return m_handle; }

    bool isCollinear()
    {
      return m_failed;
    }

  private:
    bool checkIsOwnLinesPoint(Point_2 p) const
    {
      return (p.x() == m_line->first.x() && p.y() == m_line->first.y())
        || (p.x() == m_line->second.x() && p.y() == m_line->second.y());
    }
    void checkMapped()
    {
      const auto& p = m_handle->point();
      if (!checkIsOwnLinesPoint(p)
        && CGAL::orientation(m_line->first, m_line->second, p) == CGAL::COLLINEAR)
        {
        auto findIt = m_coordinateToId.find(coordinate_2d_t { p.x() / SCALE, p.y() / SCALE });
        if (findIt != m_coordinateToId.end() && m_assignment.isPointUsed(findIt->second))
        {
          m_failed = true;
        }
      }
    }

  private:
    const VertexAssignment& m_assignment;
    const CoordinateToIdMap& m_coordinateToId;
    const point2_pair_t* m_line;
    bool& m_failed;

    Vertex_handle m_handle;
};

namespace
{
  Point_2 fromPoint(const Point& p)
  {
    return Point_2(SCALE * p.x, SCALE * p.y);
  }

  void adjustIfCollinear(Point_2& p1, Point_2& p3)
  { // assumption p1(x1,y1), p3(x3,y3) with x1 != x3 or y1 != y3 (not the same point)
    if (p1.x() == p3.x()) p1 = Point_2(p1.x() - 1, p1.y());
    else if (p1.y() == p3.y()) p1 = Point_2(p1.x(), p3.y() + 1);
  }

  bool checkSingleLine(CGAL::Point_set_2<Kernel>& pointset, FakeIterator collinearityChecker, const point_pair_t& line)
  {
    bool firstleft_or_up = (line.first.x < line.second.x ||
        (line.first.x == line.second.x && line.first.y > line.second.y));
    Point_2 p1 = firstleft_or_up ? fromPoint(line.first) : fromPoint(line.second);
    Point_2 p3 = firstleft_or_up ? fromPoint(line.second) : fromPoint(line.first);
    point2_pair_t p2_line = std::make_pair(p1, p3);
    collinearityChecker.reset(p2_line);
    adjustIfCollinear(p1, p3);
    pointset.range_search(p1, Point_2(p3.x(), p1.y()), p3, Point_2(p1.x(), p3.y()), collinearityChecker);

    return collinearityChecker.isCollinear();
  }
}


bool gd::trivialCollinearityCheck(const instance_t& instance,
    const VertexAssignment& assignment)
{
  CoordinateToIdMap pointMap{};
  CGAL::Point_set_2<Kernel> pointset;


  auto point_range = instance.m_points.getPointIterator();
  for (auto it = point_range.first; it != point_range.second; ++it)
  {
    pointset.insert(Point_2(SCALE * it->x, SCALE * it->y));
    pointMap.insert(it->getCoordToId());
  }
  bool failed = false;
  FakeIterator collinearityChecker(assignment, pointMap, failed);

  vertex_t lastFromVertex = VERTEX_UNDEF;
  Point from;
  for (const auto& edge : instance.m_graph)
  {
    if (edge.first < edge.second)
    {
     if (edge.first != lastFromVertex)
      {
        lastFromVertex = edge.first;
        from = instance.m_points.getPoint(assignment.getAssigned(lastFromVertex));
      }
      Point to = instance.m_points.getPoint(assignment.getAssigned(edge.second));

      if (checkSingleLine(pointset, collinearityChecker, std::make_pair(from, to))) return true;
    }
  }

  return false;
}



