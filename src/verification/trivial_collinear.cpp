#include "collinear.hpp"
#include "common/assignment.hpp"
#include "gd_types.hpp"
#include <gd_types.hpp>

#include <numeric>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_2.h>

using namespace gd;

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef CGAL::Point_set_2<Kernel>::Vertex_handle Vertex_handle;
typedef Kernel::Point_2 Point_2;

typedef UnorderedMap<coordinate_2d_t, point_id_t, PairHashFunc<coordinate_t>> CoordinateToIdMap;

struct FakeIterator
{
  public:
    FakeIterator(const VertexAssignment& as, const CoordinateToIdMap& m)
      : m_assignment(as), m_coordinateToId(m), m_failed(false) {}


    void reset() { m_handle = Vertex_handle(); m_failed = false; }
    void operator++() { if (!m_failed) checkMapped(); }
    Vertex_handle& operator*() { return m_handle; }

    bool isCollinear()
    {
      checkMapped();
      return m_failed;
    }

  private:
    void checkMapped()
    {
      const auto& p = m_handle->point();
      auto findIt = m_coordinateToId.find(coordinate_2d_t { p.x(), p.y() });
      if (findIt != m_coordinateToId.end() && m_assignment.isPointUsed(findIt->second))
      {
        m_failed = true;
      }
    }

  private:
    const VertexAssignment& m_assignment;
    const CoordinateToIdMap& m_coordinateToId;
    bool m_failed;

    Vertex_handle m_handle;
};

namespace
{
  Point_2 fromPoint(const Point& p)
  {
    return Point_2(p.x, p.y);
  }

  bool checkSingleLine(CGAL::Point_set_2<Kernel>& pointset, FakeIterator collinearityChecker, const point_pair_t& line)
  {
    collinearityChecker.reset();
    bool firstleft_or_up = (line.first.x < line.second.x ||
        (line.first.x == line.second.x && line.first.y > line.second.y));
    Point_2 p1 = firstleft_or_up ? fromPoint(line.first) : fromPoint(line.second);
    Point_2 p3 = firstleft_or_up ? fromPoint(line.second) : fromPoint(line.first);
    
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
    pointset.insert(Point_2(it->x, it->y));
    pointMap.insert(it->getCoordToId());
  }
  
  FakeIterator collinearityChecker(assignment, pointMap); 
  
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



