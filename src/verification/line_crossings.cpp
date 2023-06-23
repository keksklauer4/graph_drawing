#include "line_crossings.hpp"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>
#include <iostream>
#include <memory>

using namespace gd;

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel> Traits;
typedef Traits::Curve_2 Segment;
typedef Traits::Point_2 Point_2;

struct CounterFakeIterator
{
  CounterFakeIterator(size_t& counter)
  : m_point(Point_2(0, 0)), m_counter(counter) {}
  Point_2& operator*() { return m_point; }
  void operator++() { m_counter++; }

  size_t getCounter() const { return m_counter; }

  private:
    Point_2 m_point;
    size_t& m_counter;
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
      *segmentptr++ = Segment(Point_2(p1.x, p1.y), Point_2(p2.x, p2.y));
    }
  }

  size_t num_crossings = 0;
  CounterFakeIterator it{num_crossings};
  CGAL::compute_intersection_points(segments.get(), 
      segments.get() + graph.getNbEdges(), it);

  return num_crossings;
}
