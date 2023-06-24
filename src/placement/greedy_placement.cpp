#include "greedy_placement.hpp"
#include "gd_types.hpp"
#include "placement/local/local_reopt.hpp"

#include <common/random_gen.hpp>
#include <placement/placement_metrics.hpp>

#include <cassert>
#include <common/misc.hpp>
#include <cstddef>
#include <io/placement_visualizer.hpp>

#include <common/instance.hpp>
#include <common/short_term_set.hpp>
#include <common/kd_tree.hpp>
#include <io/printing.hpp>
#include <placement/greedy_placement.hpp>
#include <placement/vertex_order.hpp>

#include <ostream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <placement/local/local_gurobi.hpp>
#include <placement/local/local_functors.hpp>

using namespace gd;


GreedyPlacement::GreedyPlacement(const Instance& instance, VertexOrder& order, PlacementVisualizer* vis)
  : m_instance(instance),
    m_order(order),
    m_assignment(instance),
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

  Vector<vertex_t> embedded{};
  embedded.reserve(m_instance.m_graph.getNbVertices());
  vertex_t vertex = VERTEX_UNDEF;

  RandomGen random{};
  ShortTermVertexSet tried{};
  LocalImprovementVertexNeighbors improvementFunctor{m_instance, m_assignment};
  LocalGurobi gurobi{m_instance, m_assignment};
  KdTree kdtree {m_instance.m_points};

  while (isDefined((vertex = m_order.getNext())))
  {
    point_id_t target = findEligiblePoint(vertex);
    if (!isDefined(target)) throw std::runtime_error("Can't find a point to map to... :(");
    placeInitial(vertex, target);
    embedded.push_back(vertex);
    if (m_visualizer != nullptr)
    {
      m_visualizer->draw([&](std::ostream& stream){
        stream << "Placing vertex " << vertex << " onto point "
          << pset.getPoint(target).getCoordPair()
          << " with id " << target <<  ". [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
      });
    }

    for (int i = 0; i < 5; ++i)
    {
      vertex_t v = random.getRandom(embedded);
      // if (tried.contains(v)) continue;
      // tryImprove(v);

      improvementFunctor.reset();
      improvementFunctor.initialize(v, m_assignment.getAssigned(v));
      if (!improvementFunctor.is_valid()) continue;
      auto wRange = improvementFunctor.get_vertex_range();
      for (auto w = wRange.first; w != wRange.second; ++w)
      {
        if (!m_assignment.isAssigned(*w)) continue;
        m_incrementalCollinearity.deplace(*w);
        m_incrementalCrossing.deplace(*w, m_assignment.getAssigned(*w));
        m_assignment.unassign(*w);
      }
      improvementFunctor.set_points(kdtree, m_incrementalCollinearity);
      if (!improvementFunctor.is_valid())
      {
        improvementFunctor.get_mapping([&](vertex_t v, point_id_t p){
          assert(isDefined(p) && "Point is undefined!");
          m_incrementalCollinearity.place(v, p);
          m_incrementalCrossing.place(v, p);
          m_assignment.assign(v, p);
        });
      }
      gurobi.optimize(*reinterpret_cast<LocalImprovementFunctor*>(&improvementFunctor));
      improvementFunctor.get_mapping([&](vertex_t v, point_id_t p){
        assert(isDefined(p) && "Point is undefined!");
        assert(m_incrementalCollinearity.isValidCandidate(v, p) && "Point is actually invalid!");
        m_incrementalCollinearity.place(v, p);
        m_incrementalCrossing.place(v, p);
        m_assignment.assign(v, p);
      });

      m_visualizer->draw([&](std::ostream& stream){
        stream << "Gurobi local opt [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
      });
    }
    tried.clear();
  }

  return m_assignment;
}

size_t GreedyPlacement::placeInitial(vertex_t vertex, point_id_t target)
{ // assumes vertex and target are defined!
  size_t crossings = m_incrementalCrossing.place(vertex, target);
  m_incrementalCollinearity.place(vertex, target);

  m_assignment.assign(vertex, target);
  m_unused.erase(target);
  return crossings;
}

point_id_t GreedyPlacement::findEligiblePoint(vertex_t vertex)
{
  point_pair_t line {};
  std::pair<size_t, double> quality = std::make_pair(UINT_UNDEF, DOUBLE_MAX);;
  point_id_t best = POINT_UNDEF;
  double distance = DOUBLE_MAX;
  for (point_id_t pointid : m_unused)
  {
    if (!m_assignment.isPointUsed(pointid) && m_incrementalCollinearity.isValidCandidate(vertex, pointid))
    {
      size_t crossings = m_incrementalCrossing.calculateCrossing(vertex, pointid);
      if (crossings < quality.first || (crossings == quality.first &&
        (distance = squared_neighbor_distance_metric(m_instance, m_assignment, vertex, pointid)) < quality.second))
      {
        quality.first = crossings;
        quality.second = distance;
        best = pointid;
      }
    }
  }
  return best;
}

size_t GreedyPlacement::getNumCrossings() const
{
  return m_incrementalCrossing.getTotalNumCrossings();
}

constexpr float DESTRUC_P = 0.1;

bool GreedyPlacement::improve(size_t num_tries)
{
  ShortTermVertexSet tried{};
  bool improved = false;
  RandomGen random{};

  KdTree kdtree {m_instance.m_points};
  Vector<VertexPointPair> temp{};

  ProbabilisticDecision choice{};

  for (size_t i = 0; i < num_tries; ++i)
  {
    vertex_t candidate = VERTEX_UNDEF;
    /*size_t num_crossings = 0;

    for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
    {
      if (m_incrementalCrossing.getNumCrossings(v) > num_crossings && !tried.contains(v))
      {
        candidate = v;
        num_crossings = m_incrementalCrossing.getNumCrossings(v);
      }
    }*/
    candidate = random.getRandomUint(m_instance.m_graph.getNbVertices());
    if (tried.contains(candidate)) continue;

    tried.insert(candidate);
    bool success;
    if (!choice(DESTRUC_P)) success = tryImprove(candidate);
    else success = circularRebuild(kdtree, temp, candidate);
    improved |= success;
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
    m_incrementalCrossing.deplace(vertex, previous);

    m_incrementalCrossing.place(vertex, best);
    m_assignment.assign(vertex, best);

    if (m_visualizer != nullptr)
    {
      m_visualizer->draw([&](std::ostream& s){
        s << "Moving vertex " << vertex << " from point "
          << previous << " (" << previous_crossings
          << " crossings)  to " << best << " ("
          << num_crossings << " crossings). [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
      });
    }
  }

  return success;
}

constexpr size_t DESTRUC_SIZE = 3;
bool GreedyPlacement::circularRebuild(const KdTree& kdtree, Vector<VertexPointPair>& destructed, vertex_t candidate)
{
  assert(m_assignment.isAssigned(candidate) && "Candidate should be assigned.");
  destructed.resize(DESTRUC_SIZE);
  size_t size = 0;
  const Point& p = m_instance.m_points.getPoint(m_assignment.getAssigned(candidate));
  kdtree.k_nearest_neighbors(p, DESTRUC_SIZE,
    [&](point_id_t neighbor){
      if (m_assignment.isPointUsed(neighbor))
      {
        destructed[size++] = VertexPointPair(m_assignment.getAssignedVertex(neighbor), neighbor);
        return true;
      }
      return false;
  });

  size_t crossings = 0;
  for (size_t idx = 0; idx < size; ++idx)
  {
    crossings += m_incrementalCrossing.getNumCrossings(destructed[idx].vertex);
  }

  // deplace vertices
  for (size_t idx = 0; idx < size; ++idx)
  {
    m_incrementalCollinearity.deplace(destructed[idx].vertex);
    m_incrementalCrossing.deplace(destructed[idx].vertex, destructed[idx].point);
    m_assignment.unassign(destructed[idx].vertex);
  }

  // place vertices
  size_t newCrossings = 0;
  for (size_t idx = 0; idx < size; ++idx)
  {
    point_id_t target = findEligiblePoint(destructed[idx].vertex);
    if (!isDefined(target))
    {
      newCrossings = UINT_UNDEF;
      break;
    }
    newCrossings += placeInitial(destructed[idx].vertex, target);
  }

  if (crossings < newCrossings)
  {
    for (size_t idx = 0; idx < size; ++idx)
    {
      if (!m_assignment.isAssigned(destructed[idx].vertex)) continue;
      m_incrementalCollinearity.deplace(destructed[idx].vertex);
      m_incrementalCrossing.deplace(destructed[idx].vertex,
        m_assignment.getAssigned(destructed[idx].vertex));
      m_assignment.unassign(destructed[idx].vertex);
    }
    for (size_t idx = 0; idx < size; ++idx) placeInitial(destructed[idx].vertex, destructed[idx].point);
    return false;
  }
  else if (m_visualizer != nullptr)
  {
    m_visualizer->draw([&](std::ostream& s){
      s << "Replacing " << size << " vertices: ";
      for (size_t idx = 0; idx < size; ++idx)
      {
        s << destructed[idx].vertex;
        if (idx + 1 != size) s << ", ";
      }
      s << " [crossings: " << crossings
        << " to " << newCrossings << "] [total: "
        << m_incrementalCrossing.getTotalNumCrossings() << "]";
    });
  }
  return true;
}
