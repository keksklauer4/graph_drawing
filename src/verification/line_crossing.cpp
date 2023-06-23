#include "line_crossings.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>
#include <memory>

using namespace gd;

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits;
typedef Traits::Curve_2 Segment;
typedef Traits::Point_2 Point;

#define MAGIC_NUM 1e9

struct CounterFakeIterator
{
  CounterFakeIterator(): m_point(Point(MAGIC_NUM, MAGIC_NUM)), m_counter(0) {}
  Point& operator*() { return m_point; }
  void operator++() { m_counter++; }

  size_t getCounter() const { return m_counter + (m_point.x() != MAGIC_NUM ? 1 : 0); }

  private:
    Point m_point;
    size_t m_counter;
};

size_t gd::countCrossings(const instance_t& instance, const VertexAssignment& assignment)
{
  const auto& graph = instance.m_graph;
  const auto& points = instance.m_points;

  auto segments = std::make_unique<Segment[]>(graph.getNbEdges());
  auto* segmentptr = segments.get();
  for (auto edge : graph)
  {
    if (edge.first < edge.second)
    {
      const auto& p1 = points.getPoint(assignment.getAssigned(edge.first));
      const auto& p2 = points.getPoint(assignment.getAssigned(edge.second));
      *segmentptr++ = Segment(Point(p1.x, p1.y), Point(p2.x, p2.y));
    }
  }

  CounterFakeIterator it{};
  CGAL::compute_intersection_points(segments.get(), 
      segments.get() + graph.getNbEdges(), it);

  return it.getCounter();
}
