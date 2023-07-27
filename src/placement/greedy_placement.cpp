#include "greedy_placement.hpp"
#include "common/assignment.hpp"
#include "gd_types.hpp"
#include "placement/local/local_reopt.hpp"
#include "placement/local/permutation_functor.hpp"
#include "verification/collinear.hpp"
#include "verification/verifier.hpp"

#include <chrono>
#include <common/random_gen.hpp>
#include <cstdint>
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
#include <ratio>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <placement/local/local_gurobi.hpp>
#include <placement/local/local_functors.hpp>
#include <utility>

#include <placement/local/local_toolset.hpp>

using namespace gd;

namespace
{
  Point getInterpolatedPlacedNeighbors(const instance_t& instance, const VertexAssignment& assignment, vertex_t candidate)
  {
    double x = 0; double y = 0; size_t num = 0;
    auto neighborIt = instance.m_graph.getNeighborIterator(candidate);
    for (auto it = neighborIt.first; it != neighborIt.second; ++it)
    {
      if (assignment.isAssigned(*it))
      {
        const auto& npoint = instance.m_points.getPoint(assignment.getAssigned(*it));
        x += npoint.x;
        y += npoint.y;
        num++;
      }
    }
    x /= num; y /= num;
    return Point{UINT_UNDEF, (coordinate_t)x, (coordinate_t)y};
  }
}


GreedyPlacement::GreedyPlacement(const Instance& instance, VertexOrder& order, bool fasterImprove, PlacementVisualizer* vis)
  : m_instance(instance),
    m_order(order),
    m_assignment(instance),
    m_crossingHierarchy(m_instance, m_assignment),
    m_visualizer(vis),
    m_collChecker(), m_incrementalCollinearity(instance, m_assignment, m_collChecker),
    m_incrementalCrossing(instance, m_assignment, m_crossingHierarchy),
    m_localImprovementToolset(nullptr), m_numImprovementIters(1),
    m_kdtree(new KdTree{m_instance.m_points}),
    m_fasterImprove(fasterImprove)
{
  if (m_visualizer != nullptr) m_visualizer->setAssignment(m_assignment);
}

GreedyPlacement::~GreedyPlacement()
{
  if (m_localImprovementToolset != nullptr) delete m_localImprovementToolset;
  delete m_kdtree;
}

void GreedyPlacement::start_placement(const VertexAssignment& assignment)
{
  for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
  {
    if (m_assignment.isAssigned(v))
    {
      point_id_t p = m_assignment.getAssigned(v);
      m_assignment.unassign(v);
      m_incrementalCollinearity.deplace(v);
      m_incrementalCrossing.deplace(v, p);
    }
  }

  for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
  {
    if (assignment.isAssigned(v))
    {
      point_id_t p = assignment.getAssigned(v);
      assert(m_incrementalCollinearity.isValidCandidate(v, p) && "Start solution is invalid!");
      m_incrementalCollinearity.place(v, p);
      m_incrementalCrossing.place(v, p);
      m_assignment.assign(v, p);
    }
  }
}

const VertexAssignment& GreedyPlacement::findPlacement()
{
  m_instance.m_timer.timer_initial_placement();
  const auto& pset = m_instance.m_points;
  auto pointsRange = pset.getPointIterator();

  Vector<vertex_t> embedded{};
  embedded.reserve(m_instance.m_graph.getNbVertices());
  vertex_t vertex = VERTEX_UNDEF;

  RandomGen random{};
  ShortTermVertexSet tried{};

  size_t num_placed = 0;
  while (isDefined((vertex = m_order.getNext())))
  {
    std::cout << "#placed " << num_placed << " (of " << m_instance.m_graph.getNbVertices() << ")" << std::endl;
    point_id_t target = (num_placed > 0 ?
                        fasterFindEligiblePoint(vertex)
                      : m_random.getRandomUint(pset.getNumPoints()));
    if (!isDefined(target))
    {
      bool successful_rebuild = rebuild_collinear(vertex);
      if (!successful_rebuild)
      {
        STATS(m_instance.m_stats.set_timestamp_crossings(
          m_incrementalCrossing.getTotalNumCrossings(),
          WorkType::DEATH);)
        m_instance.m_timer.timer_initial_placement();
        throw std::runtime_error("Can't find a point to map to... :(");
      }
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::COLLINEARITY_REBUILD);)
      embedded.push_back(vertex);
    }
    else
    {
      placeInitial(vertex, target);
      embedded.push_back(vertex);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::INIT_PLACING);)
    }
    ++num_placed;
    if (m_visualizer != nullptr && isDefined(target))
    {
      m_visualizer->draw([&](std::ostream& stream){
        stream << "Placing vertex " << vertex << " onto point "
          << pset.getPoint(target).getCoordPair()
          << " with id " << target <<  ". [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
      });
    }

    if (m_incrementalCrossing.getTotalNumCrossings() == 0) continue;
    std::cout << "Improving\n";

    size_t current_crossings = m_incrementalCrossing.getTotalNumCrossings();
    m_instance.m_timer.timer_move_op();
    for (int i = 0; i < 50; ++i)
    {
      vertex_t v = random.getRandom(embedded);
      if (tried.contains(v)) continue;
      if (i == 0) calibrate_improve_iterations(v);
      else if (m_fasterImprove) fasterImprove(v);
      else tryImprove(v);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::MOVE_OP);)
    }
    m_instance.m_timer.timer_move_op();

    std::cout << "Num crossings: " << m_incrementalCrossing.getTotalNumCrossings()
              << " (before improvement " << current_crossings << ")\n";
    std::cout << m_instance.m_timer << std::endl;
  }
  m_instance.m_timer.timer_initial_placement();

  return m_assignment;
}

void GreedyPlacement::calibrate_improve_iterations(vertex_t candidate)
{
  typedef std::chrono::steady_clock::time_point time_point;
  time_point begin = std::chrono::steady_clock::now();
  if (m_fasterImprove) fasterImprove(candidate);
  else tryImprove(candidate);
  time_point end = std::chrono::steady_clock::now();
  double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
  double limit = static_cast<double>(m_instance.m_timer.get_time_limit_ms() * 1e6);

  m_numImprovementIters = std::max(1ul, std::min(50ul, static_cast<size_t>(
    (0.04 /
    (duration / limit)) // <- percentage of time limit
    / static_cast<double>(m_instance.m_graph.getNbVertices())
  )));
  std::cout << "m_numImprovementIters is " << m_numImprovementIters << std::endl;
}

bool GreedyPlacement::improve_locally(
    LocalGurobi& optimizer, LocalImprovementFunctor& functor,
    vertex_t vertex, point_id_t point)
{
  size_t crossings_before = m_incrementalCrossing.getTotalNumCrossings();
  bool valid_start = m_assignment.getAssigned(vertex) == point;
  functor.reset();
  functor.initialize(vertex, point);
  if (!functor.is_valid()) return false;
  auto wRange = functor.get_vertex_range();
  for (auto w = wRange.first; w != wRange.second; ++w)
  {
    if (!m_assignment.isAssigned(*w)) continue;
    m_incrementalCollinearity.deplace(*w);
    m_incrementalCrossing.deplace(*w, m_assignment.getAssigned(*w));
    m_assignment.unassign(*w);
  }
  functor.set_points();
  if (!functor.is_valid())
  {
    functor.get_mapping([&](vertex_t v, point_id_t p){
      if(valid_start && !isDefined(p)) throw std::runtime_error("Point is undefined!");
      if (v != vertex || valid_start)
      {
        m_incrementalCollinearity.place(v, p);
        m_incrementalCrossing.place(v, p);
        m_assignment.assign(v, p);
      }
    });
    return false;
  }
  else
  {
    m_temporaryMapping.clear();
    functor.get_mapping([&](vertex_t v, point_id_t p){
      m_temporaryMapping.push_back(VertexPointPair{v, p});
    });
  }
  bool valid = optimizer.optimize(*reinterpret_cast<LocalImprovementFunctor*>(&functor));
  bool feasible = true;

  functor.get_mapping([&](vertex_t v, point_id_t p){
    if (!isDefined(p)) feasible = false;
    if ((v != vertex || valid_start || valid) && feasible)
    {
      if(m_assignment.isPointUsed(p) || m_incrementalCollinearity.isPointInvalid(p)
        || !m_incrementalCollinearity.isValidCandidate(v, p))
      {
        std::cout << "ERROR: Bad mapping from " << v << " to " << p << std::endl;
        std::cout << "point used: " << m_assignment.isPointUsed(p)
                  << " invalid collinear " << m_incrementalCollinearity.isPointInvalid(p)
                  << " invalid candidate " << !m_incrementalCollinearity.isValidCandidate(v, p)
                  << std::endl;
        feasible = false;
      }
      if (!feasible) return;
      m_incrementalCollinearity.place(v, p);
      m_incrementalCrossing.place(v, p);
      m_assignment.assign(v, p);
    }
  });

  if (!feasible || m_incrementalCrossing.getTotalNumCrossings() > (crossings_before+3))
  { // not a bug - its a "feature..." :D
    rollback_reopt_bug(!feasible);
  }

  if (feasible && m_visualizer != nullptr)
  {
    m_visualizer->draw([&](std::ostream& stream){
      stream << "Gurobi local opt [total: "
            << m_incrementalCrossing.getTotalNumCrossings() << "]";
    });
  }
  return valid;
}

void GreedyPlacement::rollback_reopt_bug(bool infeasible)
{
  if (infeasible) std::cout << "ERROR: Infeasibility bug occurred!\n";
  else std::cout << "ERROR: Another weird bug occurred where Gurobi did not accept the start solution (my fault :D).\n";

  for (const auto& mapped : m_temporaryMapping)
  {
    if (m_assignment.isAssigned(mapped.vertex))
    {
      m_incrementalCollinearity.deplace(mapped.vertex);
      m_incrementalCrossing.deplace(mapped.vertex, m_assignment.getAssigned(mapped.vertex));
      m_assignment.unassign(mapped.vertex);
    }
  }

  for (const auto& mapped : m_temporaryMapping)
  {
    m_incrementalCollinearity.place(mapped.vertex, mapped.point);
    m_incrementalCrossing.place(mapped.vertex, mapped.point);
    m_assignment.assign(mapped.vertex, mapped.point);
  }

  if (infeasible)
  {
    Verifier verifier{m_instance, m_assignment};
    size_t num_crossings = UINT_UNDEF;
    bool valid = verifier.verify(num_crossings);
    if (!valid)
    {
      std::cout << "ERROR: We are still invalid! Distinct (and all mapped): "
                << verifier.distinct()
                << " collinearity " << !gd::trivialCollinearityCheck(m_instance, m_assignment)
                << std::endl;
      throw std::runtime_error("Some bug killed us :(");
    }
    else std::cout << "NOTE: We are valid again.\n";
  }
  std::cout << "NOTE: Finished rollback.\n";
}

size_t GreedyPlacement::placeInitial(vertex_t vertex, point_id_t target)
{ // assumes vertex and target are defined!
  size_t crossings = m_incrementalCrossing.place(vertex, target);
  m_incrementalCollinearity.place(vertex, target);

  m_assignment.assign(vertex, target);
  return crossings;
}

point_id_t GreedyPlacement::findEligiblePoint(vertex_t vertex)
{
  point_pair_t line {};
  std::pair<size_t, double> quality = std::make_pair(UINT_MAX - 1, DOUBLE_MAX);;
  point_id_t best = POINT_UNDEF;
  double distance = DOUBLE_MAX;
  for (point_id_t pointid : m_assignment.getUnusedPoints())
  {
    if (!m_assignment.isPointUsed(pointid))
    {
      distance = squared_neighbor_distance_metric(m_instance, m_assignment, vertex, pointid);
      if (quality.first == 0
      && distance >= quality.second) continue;

      if (m_incrementalCollinearity.isValidCandidate(vertex, pointid))
      {
        size_t crossings = m_incrementalCrossing.calculateCrossing(vertex, pointid, quality.first);
        if (crossings < quality.first || (crossings == quality.first && distance < quality.second))
        {
          quality.first = crossings;
          quality.second = distance;
          best = pointid;
        }
      }
    }
  }
  return best;
}

point_id_t GreedyPlacement::fasterFindEligiblePoint(vertex_t candidate)
{
  m_checkedPoints.clear();
  size_t num_crossings = UINT_MAX;
  point_id_t best = UINT_UNDEF;

  checkNearestNeighborMoves(candidate,
    getInterpolatedPlacedNeighbors(m_instance, m_assignment, candidate),
                            num_crossings, best);

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
  RandomGen random{};

  size_t previous_crossings = m_incrementalCrossing.getTotalNumCrossings();

  for (size_t iter = 0; iter < num_tries && !m_instance.m_timer.time_limit(); ++iter)
  {
    run_reopt(random.getRandomUint(m_instance.m_graph.getNbVertices()));

    for (size_t i = 0; i < 100 && !m_instance.m_timer.time_limit(); ++i)
    {
      size_t candidate = VERTEX_UNDEF;
      candidate = random.getRandomUint(m_instance.m_graph.getNbVertices());
      if (tried.contains(candidate)) continue;
      tried.insert(candidate);
      fasterImprove(candidate);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::MOVE_OP);)
    }
  }
  return m_incrementalCrossing.getTotalNumCrossings() < previous_crossings;
}

void GreedyPlacement::run_reopt(vertex_t candidate)
{
  if (m_localImprovementToolset == nullptr)
  {
    m_localImprovementToolset = new LocalImprovementToolset{
      m_instance, m_assignment, m_incrementalCollinearity
    };
  }
  auto& functor = m_localImprovementToolset->getRandomFunctor();
  STATS(size_t before = m_incrementalCrossing.getTotalNumCrossings();)
  improve_locally(m_localImprovementToolset->optimizer,
    functor, candidate,
    m_assignment.getAssigned(candidate));
  STATS(size_t now = m_incrementalCrossing.getTotalNumCrossings();)
  STATS(m_instance.m_stats.set_local_reopt_result(
      functor.type(),
      ((int64_t)now) - ((int64_t)before));)
  STATS(m_instance.m_stats.set_timestamp_crossings(now, WorkType::REOPT);)
}

bool GreedyPlacement::tryImprove(vertex_t vertex)
{
  bool success = false;
  size_t num_crossings = m_incrementalCrossing.getNumCrossings(vertex);
  if (num_crossings == 0) return false;
  m_incrementalCollinearity.deplace(vertex);
  point_id_t best = m_assignment.getAssigned(vertex);
  auto previous = m_assignment.getAssigned(vertex);
  m_incrementalCrossing.deplace(vertex, best);
  m_assignment.unassign(vertex);

  size_t previous_crossings = num_crossings;

  for (point_id_t point_id : m_assignment.getUnusedPoints())
  {
    if (m_assignment.isPointUsed(point_id)
      || !m_incrementalCollinearity.isValidCandidate(vertex, point_id)) continue;

    size_t crossings = m_incrementalCrossing.calculateCrossing(vertex, point_id, num_crossings);

    if (crossings < num_crossings)
    {
      best = point_id;
      num_crossings = crossings;
      success = true;
      if (num_crossings == 0) break;
    }
  }
  m_incrementalCollinearity.place(vertex, best);
  m_incrementalCrossing.place(vertex, best);
  m_assignment.assign(vertex, best);

  if (success)
  {

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

bool GreedyPlacement::fasterImprove(vertex_t candidate)
{
  size_t num_crossings = m_incrementalCrossing.getNumCrossings(candidate);
  if (num_crossings == 0) return false;
  m_incrementalCollinearity.deplace(candidate);
  point_id_t best = m_assignment.getAssigned(candidate);
  point_id_t previous_point = best;
  auto previous = m_assignment.getAssigned(candidate);
  m_incrementalCrossing.deplace(candidate, best);
  m_assignment.unassign(candidate);

  size_t previous_crossings = num_crossings;

  m_checkedPoints.clear();

  checkNearestNeighborMoves(candidate, getInterpolatedPlacedNeighbors(m_instance, m_assignment, candidate),
                            num_crossings, best);
  checkNearestNeighborMoves(candidate, m_instance.m_points.getPoint(previous_point),
                            num_crossings, best);
  m_checkedPoints.clear();

  m_incrementalCollinearity.place(candidate, best);
  m_incrementalCrossing.place(candidate, best);
  m_assignment.assign(candidate, best);

  if (previous_crossings > num_crossings && m_visualizer != nullptr)
  {
    m_visualizer->draw([&](std::ostream& s){
        s << "FastMoving vertex " << candidate << " from point "
          << previous << " (" << previous_crossings
          << " crossings)  to " << best << " ("
          << num_crossings << " crossings). [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
    });
  }
  return previous_crossings < num_crossings;
}

#define NUM_VALID_CHECK 30
void GreedyPlacement::checkNearestNeighborMoves(
    vertex_t candidate, Point around, size_t& best_crossings,
    point_id_t& best_point)
{
  m_kdtree->k_nearest_neighbors(around, NUM_VALID_CHECK,
    [&](point_id_t p) -> bool {
      if (m_assignment.isPointUsed(p) || m_checkedPoints.contains(p)
        || m_incrementalCollinearity.isPointInvalid(p)) return false;
      m_checkedPoints.insert(p);
      if (!m_incrementalCollinearity.isValidCandidate(candidate, p)) return false;
      size_t crossings = m_incrementalCrossing.calculateCrossing(candidate, p, best_crossings);
      if (crossings >= best_crossings) return true;
      best_crossings = crossings;
      best_point = p;
      return true;
  });
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

bool GreedyPlacement::rebuild_collinear(vertex_t v)
{
  std::cout << "Rebuild collinear." << std::endl;
  VertexVector rebuild_vec{};
  MultiMap<vertex_t, point_id_t> mapping{};
  PointIdVector points{};

  find_collinear_points(points, v);
  collinear_neighbors(v, rebuild_vec, mapping);
  rebuild_vec.push_back(v);

  bool success = false;
  for (size_t i = 0; i < 50 && !success; ++i)
  {
    m_random.shuffle(rebuild_vec);
    success = rebuild(rebuild_vec);
  }

  if (success) return true;
  VertexSet chosen{};
  for (auto neighbor : rebuild_vec) chosen.insert(neighbor);

  for (point_id_t p : points)
  {
    collinear_tedious(p, rebuild_vec, mapping, chosen);
    for (size_t i = 0; i < 10 && !success; ++i)
    {
      m_random.shuffle(rebuild_vec);
      success = rebuild(rebuild_vec);
      if (success) visualize_rebuild();
      if (success) return true;
    }
  }


  // Reset to original setting again
  for (const auto& mapped : mapping)
  {
    assert(m_incrementalCollinearity.isValidCandidate(mapped.first, mapped.second) && "Mapping is invalid - weird bug!");
    m_incrementalCollinearity.place(mapped.first, mapped.second);
    m_incrementalCrossing.place(mapped.first, mapped.second);
    m_assignment.assign(mapped.first, mapped.second);
  }

  LocalGurobi optimizer{m_instance, m_assignment};
  for (point_id_t bad_point : points)
  {
    for (size_t i = 0; i != 10; ++i)
    {
      CollinearFunctor func {m_instance, m_assignment, m_incrementalCollinearity};
      success = improve_locally(optimizer, func, v, bad_point);
      if (success) return true;
    }
  }

  return false;
}


void GreedyPlacement::collinear_neighbors(vertex_t v, VertexVector& vec, MultiMap<vertex_t, point_id_t>& mapping)
{
  vec.reserve(m_instance.m_graph.getDegree(v) + 1);
  auto neighbors = m_instance.m_graph.getNeighborIterator(v);
  for (auto it = neighbors.first; it != neighbors.second; ++it)
  { if (m_assignment.isAssigned(*it)) vec.push_back(*it); }

  for (vertex_t neighbor : vec)
  {
    point_id_t p = m_assignment.getAssigned(neighbor);
    mapping.insert(std::make_pair(neighbor, p));

    m_assignment.unassign(neighbor);
    m_incrementalCollinearity.deplace(neighbor);
    m_incrementalCrossing.deplace(neighbor, p);
  }
}

void GreedyPlacement::collinear_tedious(
  point_id_t p, VertexVector& vec,
  MultiMap<vertex_t, point_id_t>& mapping, VertexSet& vset)
{
  auto range = m_incrementalCollinearity.getCollinearTriplets(p);
  for(auto it = range.first; it != range.second; ++it)
  {
    auto v1 = m_assignment.getAssignedVertex(it->second.first);
    auto v2 = m_assignment.getAssignedVertex(it->second.second);
    if (isDefined(v1) && isDefined(v2) && m_instance.m_graph.connected(v1, v2))
    {
      if (!vset.contains(v1))
      {
        vset.insert(v1);
        vec.push_back(v1);
        mapping.insert(std::make_pair(v1, it->second.first));
        m_incrementalCollinearity.deplace(v1);
        m_incrementalCrossing.deplace(v1, m_assignment.getAssigned(v1));
        m_assignment.unassign(v1);
      }
      if (!vset.contains(v2))
      {
        vset.insert(v2);
        vec.push_back(v2);
        mapping.insert(std::make_pair(v2, it->second.second));
        m_incrementalCollinearity.deplace(v2);
        m_incrementalCrossing.deplace(v2, m_assignment.getAssigned(v2));
        m_assignment.unassign(v2);
      }
    }
  }
}

bool GreedyPlacement::rebuild(VertexVector& rebuild_vec)
{
  std::cout << "Retrying rebuild_collinear" << std::endl;
  bool success = false;
  size_t idx = 0;
  for (vertex_t next : rebuild_vec)
  {
    point_id_t target = findEligiblePoint(next);
    if (!isDefined(target)) break;
    m_incrementalCollinearity.place(next, target);
    m_incrementalCrossing.place(next, target);
    m_assignment.assign(next, target);
    idx++;
  }
  if (idx == rebuild_vec.size()) success = true;
  else
  {
    for (size_t v_idx = 0; v_idx < idx; ++v_idx)
    {
      auto vertex = rebuild_vec[v_idx];
      auto p = m_assignment.getAssigned(vertex);
      m_assignment.unassign(vertex);
      m_incrementalCollinearity.deplace(vertex);
      m_incrementalCrossing.deplace(vertex, p);
    }
  }
  return success;
}

void GreedyPlacement::find_collinear_points(PointIdVector& points, vertex_t v)
{
  for (point_id_t p : m_assignment.getUnusedPoints())
  {
    if (m_incrementalCollinearity.isValidCandidate(v, p, true))
    {
      points.push_back(p);
    }
  }
}

void GreedyPlacement::visualize_rebuild()
{
  if (m_visualizer != nullptr)
  {
    std::cout << "Visualizing success\n";
    m_visualizer->draw([](std::ostream& s){
      s << "Rebuild worked.";
    });
  }
}
