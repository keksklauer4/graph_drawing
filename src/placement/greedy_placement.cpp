#include "greedy_placement.hpp"
#include "gd_types.hpp"

#include <common/misc.hpp>
#include <io/placement_visualizer.hpp>

#include <common/instance.hpp>
#include <common/short_term_set.hpp>
#include <io/printing.hpp>

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace gd;


GreedyPlacement::GreedyPlacement(const Instance& instance, PlacementVisualizer* vis)
  : m_instance(instance),
    m_assignment(instance.m_graph.getNbVertices()),
    m_visualizer(vis),
    m_collChecker(), m_incrementalCollinearity(instance, m_assignment, m_collChecker),
    m_incrementalCrossing(instance, m_assignment)
{
  if (m_visualizer != nullptr) m_visualizer->setAssignment(m_assignment);
}

const VertexAssignment& GreedyPlacement::findPlacement()
{
  const auto& pset = m_instance.m_points;
  auto pointsRange = pset.getPointIterator();
  for (auto it = pointsRange.first; it != pointsRange.second; ++it)
  {
    m_unused.insert(it->id);
  }

  for (vertex_t v = 0; v != m_instance.m_graph.getNbVertices(); ++v)
  {
    point_id_t target = findEligiblePoint(v);
    if (!isDefined(target)) throw std::runtime_error("Cant find a point to map to... :(");

    m_incrementalCrossing.place(v, target);
    m_incrementalCollinearity.place(v, target);

    m_assignment.assign(v, target);
    m_unused.erase(target);
    if (m_visualizer != nullptr)
    {
      m_visualizer->draw([&](std::ostream& stream){
        stream << "Placing vertex " << v << " onto point "
          << pset.getPoint(target).getCoordPair()
          << " with id " << target << std::endl;
      });
    }
  }
  return m_assignment;
}

point_id_t GreedyPlacement::findEligiblePoint(vertex_t vertex)
{
  point_pair_t line {};
  for (point_id_t pointid : m_unused)
  {
    if (m_incrementalCollinearity.isValidCandidate(vertex, pointid)) return pointid;
  }
  return POINT_UNDEF;
}

size_t GreedyPlacement::getNumCrossings() const
{
  return m_incrementalCrossing.getTotalNumCrossings();
}

bool GreedyPlacement::improve(size_t num_tries)
{
  ShortTermVertexSet tried{};
  size_t n = m_instance.m_graph.getNbVertices();
  bool improved = false;
  for (size_t i = 0; i < num_tries; ++i)
  {
    vertex_t candidate = VERTEX_UNDEF;
    size_t num_crossings = 0;

    for (vertex_t v = 0; v < n; ++v)
    {
      if (m_incrementalCrossing.getNumCrossings(v) > num_crossings && !tried.contains(v))
      {
        candidate = v;
        num_crossings = m_incrementalCrossing.getNumCrossings(v);
      }
    }

    if (!isDefined(candidate)) return false;

    tried.insert(candidate);
    improved |= tryImprove(candidate);
  }
  return improved;
}

bool GreedyPlacement::tryImprove(vertex_t vertex)
{
  bool success = false;
  m_incrementalCollinearity.deplace(vertex);
  size_t num_crossings = m_incrementalCrossing.getNumCrossings(vertex);
  point_id_t best = m_assignment.getAssigned(vertex);

  size_t previous_crossings = num_crossings;

  for (const auto& point : m_instance.m_points)
  {
    if (m_assignment.isPointUsed(point.id)
      || !m_incrementalCollinearity.isValidCandidate(vertex, point.id)) continue;

    size_t crossings = m_incrementalCrossing.calculateCrossing(vertex, point.id);

    if (crossings < num_crossings)
    {
      best = point.id;
      num_crossings = crossings;
      success = true;
      if (num_crossings == 0) break;
    }
  }
  m_incrementalCollinearity.place(vertex, best);

  if (success)
  {
    point_id_t previous = m_assignment.getAssigned(vertex);
    m_incrementalCrossing.deplace(vertex);
    m_incrementalCrossing.place(vertex, best);
    m_assignment.assign(vertex, best);

    if (m_visualizer != nullptr)
    {
      m_visualizer->draw([&](std::ostream& s){
        s << "Moving vertex " << vertex << " from point "
          << previous << " (" << previous_crossings
          << " crossings)  to " << best << " ( "
          << num_crossings << " crossings).";
      });
    }
  }

  return success;
}
