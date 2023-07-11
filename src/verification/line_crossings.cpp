#include "line_crossings.hpp"
#include "gd_types.hpp"
#include "verification/verification_utils.hpp"


#include <common/assignment.hpp>
#include <iostream>
#include <memory>

using namespace gd;

/*
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Surface_sweep_2_algorithms.h>

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
*/

size_t gd::countCrossings(const instance_t& instance, const VertexAssignment& assignment)
{
  const auto& graph = instance.m_graph;
  const auto& points = instance.m_points;

  size_t num_crossings = 0;
  for (auto edge : graph)
  {
    if (edge.first > edge.second) continue;
    for (vertex_t x = edge.first + 1; x < instance.m_graph.getNbVertices(); ++x)
    {
      if (x == edge.second) continue;
      auto neighbors = instance.m_graph.getNeighborIterator(x);
      for (auto neighbor = neighbors.first; neighbor != neighbors.second; ++neighbor)
      {
        if (*neighbor < x || *neighbor == edge.second) continue;
        // edge and x-neighbor are edges
        num_crossings += gd::intersect(
            points.getPoint(assignment.getAssigned(edge.first)),
            points.getPoint(assignment.getAssigned(edge.second)),
            points.getPoint(assignment.getAssigned(x)),
            points.getPoint(assignment.getAssigned(*neighbor))) ? 1 : 0;
      }
    }
  }

  return num_crossings;
}

void gd::enumerateCrossingPairs(const instance_t& instance, const VertexAssignment& assignment,
    VertexSet& res, vertex_t u, vertex_t v, size_t max_size)
{
  const auto& graph = instance.m_graph;
  const auto& pset = instance.m_points;
  const auto& pU = pset.getPoint(assignment.getAssigned(u));
  const auto& pV = pset.getPoint(assignment.getAssigned(v));
  for (vertex_t x = 0; x < graph.getNbVertices(); ++x)
  {
    if (x == u || x == v || !assignment.isAssigned(x)) continue;
    auto nRange = graph.getNeighborIterator(x);

    const auto pX = pset.getPoint(assignment.getAssigned(x));
    for (auto y = nRange.first; y != nRange.second; ++y)
    {
      if (*y == u || *y == v || !assignment.isAssigned(*y)
        || (res.contains(x) && res.contains(*y))) continue;

      const auto pY = pset.getPoint(assignment.getAssigned(*y));
      if (gd::intersect(pU, pV, pX, pY))
      {
        res.insert(x);
        res.insert(*y);
        if (res.size() >= max_size) return;
      }
    }
  }
}
