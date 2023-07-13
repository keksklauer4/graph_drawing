#include "greedy_placement.hpp"
#include "gd_types.hpp"
#include "placement/local/local_reopt.hpp"
#include "placement/local/permutation_functor.hpp"

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
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <placement/local/local_gurobi.hpp>
#include <placement/local/local_functors.hpp>
#include <utility>

#include <placement/local/local_toolset.hpp>

using namespace gd;


GreedyPlacement::GreedyPlacement(const Instance& instance, VertexOrder& order, PlacementVisualizer* vis)
  : m_instance(instance),
    m_order(order),
    m_assignment(instance),
    m_crossingHierarchy(m_instance, m_assignment),
    m_visualizer(vis),
    m_collChecker(), m_incrementalCollinearity(instance, m_assignment, m_collChecker),
    m_incrementalCrossing(instance, m_assignment, m_crossingHierarchy),
    m_localImprovementToolset(nullptr)
{
  if (m_visualizer != nullptr) m_visualizer->setAssignment(m_assignment);
}

GreedyPlacement::~GreedyPlacement()
{
  if (m_localImprovementToolset != nullptr) delete m_localImprovementToolset;
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
                        findEligiblePoint(vertex)
                      : m_random.getRandomUint(pset.getNumPoints()));
    if (!isDefined(target))
    {
      bool successful_rebuild = rebuild_collinear(vertex);
      if (!successful_rebuild)
      {
        STATS(m_instance.m_stats.set_timestamp_crossings(
          m_incrementalCrossing.getTotalNumCrossings(),
          WorkType::DEATH);)
        throw std::runtime_error("Can't find a point to map to... :(");
      }
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::COLLINEARITY_REBUILD);)
      continue;
    }
    else
    {
      m_instance.m_timer.timer_initial_placement();
      placeInitial(vertex, target);
      m_instance.m_timer.timer_initial_placement();
      embedded.push_back(vertex);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::INIT_PLACING);)
    }
    ++num_placed;
    if (m_visualizer != nullptr)
    {
      m_visualizer->draw([&](std::ostream& stream){
        stream << "Placing vertex " << vertex << " onto point "
          << pset.getPoint(target).getCoordPair()
          << " with id " << target <<  ". [total: "
          << m_incrementalCrossing.getTotalNumCrossings() << "]";
      });
    }
    std::cout << "Improving\n";

    if (m_incrementalCrossing.getTotalNumCrossings() == 0) continue;

    m_instance.m_timer.timer_move_op();
    for (int i = 0; i < 50; ++i)
    {
      vertex_t v = random.getRandom(embedded);
      if (tried.contains(v)) continue;
      tryImprove(v);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::MOVE_OP);)
    }
    m_instance.m_timer.timer_move_op();

    std::cout << "Num crossings: " << m_incrementalCrossing.getTotalNumCrossings() << std::endl;
    std::cout << m_instance.m_timer << std::endl;
  }

  return m_assignment;
}

bool GreedyPlacement::improve_locally(
    LocalGurobi& optimizer, LocalImprovementFunctor& functor,
    vertex_t vertex, point_id_t point)
{
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
      assert(isDefined(p) && "Point is undefined!");
      if (v != vertex || valid_start)
      {
        m_incrementalCollinearity.place(v, p);
        m_incrementalCrossing.place(v, p);
        m_assignment.assign(v, p);
      }
    });
    return false;
  }
  bool valid = optimizer.optimize(*reinterpret_cast<LocalImprovementFunctor*>(&functor));
  functor.get_mapping([&](vertex_t v, point_id_t p){
    assert(isDefined(p) && "Point is undefined!");
    if (v != vertex || valid_start || valid)
    {
      assert(m_incrementalCollinearity.isValidCandidate(v, p) && "Point is actually invalid!");
      m_incrementalCollinearity.place(v, p);
      m_incrementalCrossing.place(v, p);
      m_assignment.assign(v, p);
    }
  });
  if (m_visualizer != nullptr)
  {
    m_visualizer->draw([&](std::ostream& stream){
      stream << "Gurobi local opt [total: "
            << m_incrementalCrossing.getTotalNumCrossings() << "]";
    });
  }
  return valid;
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

size_t GreedyPlacement::getNumCrossings() const
{
  return m_incrementalCrossing.getTotalNumCrossings();
}

constexpr float DESTRUC_P = 0.1;

bool GreedyPlacement::improve(size_t num_tries)
{
  ShortTermVertexSet tried{};
  RandomGen random{};

  if (m_localImprovementToolset == nullptr)
  {
    m_localImprovementToolset = new LocalImprovementToolset{
      m_instance, m_assignment, m_incrementalCollinearity
    };
  }
  size_t previous_crossings = m_incrementalCrossing.getTotalNumCrossings();

  for (size_t iter = 0; iter < num_tries && !m_instance.m_timer.time_limit(); ++iter)
  {
    vertex_t candidate = m_localImprovementToolset->m_random.getRandomUint(m_instance.m_graph.getNbVertices());
   STATS( size_t before = m_incrementalCrossing.getTotalNumCrossings();)
    auto& functor = m_localImprovementToolset->getRandomFunctor();
    improve_locally(m_localImprovementToolset->optimizer,
        functor, candidate,
        m_assignment.getAssigned(candidate));
    STATS(size_t now = m_incrementalCrossing.getTotalNumCrossings();)
    STATS(m_instance.m_stats.set_timestamp_crossings(
        now,
        WorkType::REOPT);)
    STATS(m_instance.m_stats.set_local_reopt_result(
        functor.type(),
        ((int64_t)now) - ((int64_t)before));)

    for (size_t i = 0; i < 100 && !m_instance.m_timer.time_limit(); ++i)
    {
      candidate = VERTEX_UNDEF;
      candidate = random.getRandomUint(m_instance.m_graph.getNbVertices());
      if (tried.contains(candidate)) continue;
      tried.insert(candidate);
      tryImprove(candidate);
      STATS(m_instance.m_stats.set_timestamp_crossings(
        m_incrementalCrossing.getTotalNumCrossings(),
        WorkType::MOVE_OP);)
    }
  }
  return m_incrementalCrossing.getTotalNumCrossings() < previous_crossings;
}

bool GreedyPlacement::tryImprove(vertex_t vertex)
{
  bool success = false;
  m_incrementalCollinearity.deplace(vertex);
  size_t num_crossings = m_incrementalCrossing.getNumCrossings(vertex);
  point_id_t best = m_assignment.getAssigned(vertex);
  auto previous = m_assignment.getAssigned(vertex);
  m_incrementalCrossing.deplace(vertex, best);
  m_assignment.unassign(vertex);

  size_t previous_crossings = num_crossings;

  for (const auto& point : m_instance.m_points)
  {
    if (m_assignment.isPointUsed(point.id)
      || !m_incrementalCollinearity.isValidCandidate(vertex, point.id)) continue;

    size_t crossings = m_incrementalCrossing.calculateCrossing(vertex, point.id, num_crossings);

    if (crossings < num_crossings)
    {
      best = point.id;
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
      // std::cout << "Success " << success << std::endl;
      if (success) visualize_rebuild();
      if (success) return true;
    }
  }


  // std::cout << "Resetting\n";
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
    // std::cout << "Trying with point " << bad_point << std::endl;
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
  // std::cout << "Num before " << m_assignment.getUnusedPoints().size() << std::endl;
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
        // std::cout << "Adding " << v1 << std::endl;
      }
      if (!vset.contains(v2))
      {
        vset.insert(v2);
        vec.push_back(v2);
        mapping.insert(std::make_pair(v2, it->second.second));
        m_incrementalCollinearity.deplace(v2);
        m_incrementalCrossing.deplace(v2, m_assignment.getAssigned(v2));
        m_assignment.unassign(v2);
        // std::cout << "Adding " << v2 << std::endl;
      }
    }
    // std::cout << "Num " << m_assignment.getUnusedPoints().size() << std::endl;
  }
}

bool GreedyPlacement::rebuild(VertexVector& rebuild_vec)
{
  std::cout << "Retrying rebuild_collinear" << std::endl;
  bool success = false;
  size_t idx = 0;
  for (vertex_t next : rebuild_vec)
  {
    // std::cout << "Vertex " << next << std::endl;
    point_id_t target = findEligiblePoint(next);
    if (!isDefined(target)) break;
    m_incrementalCollinearity.place(next, target);
    m_incrementalCrossing.place(next, target);
    m_assignment.assign(next, target);
    idx++;
  }
  // std::cout << "rebuild size " << rebuild_vec.size() << "  " << idx << std::endl;
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
