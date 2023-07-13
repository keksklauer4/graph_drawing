#include "sampling_solver.hpp"

#include "common/assignment.hpp"
#include "common/misc.hpp"
#include "gd_types.hpp"
#include "io/placement_visualizer.hpp"
#include "placement/vertex_order.hpp"
#include "verification/verifier.hpp"
#include <math.h>
#include <placement/greedy_placement.hpp>

#include <climits>
#include <memory>
#include <sstream>
#include <iostream>
#include <stdexcept>



using namespace gd;

VertexAssignment SamplingSolver::solve(std::string vis_path)
{
  std::cout << "===========================\n";
  std::cout << "Instance statistics\n";
  std::cout << "#vertices: " << m_instance.m_graph.getNbVertices() << std::endl;
  std::cout << "#edges:    " << m_instance.m_graph.getNbEdges() << std::endl;
  std::cout << "#poins:    " << m_instance.m_points.getNumPoints() << std::endl;

  std::cout << "Expecting at least " << std::max(0l, (((int64_t)m_instance.m_graph.getNbEdges())
                                                      - 3 * (int64_t)m_instance.m_graph.getNbVertices() - 6))
            << " crossings." << std::endl;
  std::cout << "===========================\n";

  Vector<std::unique_ptr<point_id_t[]>> assignments{};

  size_t best_crossings = UINT_MAX;
  size_t best_idx = UINT_UNDEF;
  size_t num_valid = 0;

  size_t idx = 0;
  do{
    std::unique_ptr<PlacementVisualizer> visualizer;
    if (!vis_path.empty())
    {
      std::stringstream ss;
      ss << vis_path << idx << "/sample_"<< idx << "_";
      visualizer = std::make_unique<PlacementVisualizer>(m_instance, ss.str());
    }
    bool valid = false;
    size_t num_crossings = UINT_UNDEF;
    assignments.push_back(solve_instance(visualizer.get(), valid, num_crossings));

    if (valid)
    {
      std::cout << "NOTE: Nice, found a new valid initial solution with "
                << num_crossings << " crossings (best: " << best_crossings << ")\n";
      num_valid++;
      if (num_crossings < best_crossings || !isDefined(best_crossings))
      {
        best_crossings = num_crossings;
        best_idx = idx;
      }
    }
    idx++;
    std::cout << "NOTE: Timelimit at " << (m_instance.m_timer.get_fraction_time_limit() * 100) << "%.\n";
  } while(m_instance.m_timer.get_fraction_time_limit() < 0.2
        || (!isDefined(best_idx) && !m_instance.m_timer.time_limit()));

  if (num_valid == 0)
  {
    std::cout << "CRITICAL: All initial solutions invalid! Bad algorithm :(." << std::endl;
    return VertexAssignment{m_instance};
  }
  else if (m_instance.m_timer.time_limit())
  {
    std::cout << "WARNING: Wasted all the time finding an initial solution!" << std::endl;
    return make_assignment(assignments[best_idx].get());
  }

  std::cout << "NOTE: Found " << num_valid << " valid initial solution.\n";
  std::cout << "NOTE: Starting to improve best solution with " << best_crossings
            << " crossings." << std::endl;

  VertexAssignment assignment = make_assignment(assignments[best_idx].get());
  STATS(m_instance.m_stats.set_current_run(best_idx);)

  MaxEmbeddedVertexOrder order{m_instance, m_random.getRandomUint(m_instance.m_graph.getNbVertices())};
  GreedyPlacement placement{m_instance, order, nullptr};
  placement.start_placement(assignment);
  size_t iter = 0;
  while(!m_instance.m_timer.time_limit())
  {
    std::cout << "Trying to improve." << std::endl;
    size_t previous = placement.getNumCrossings();
    if (previous == 0) break;
    bool success = placement.improve(5);
    std::cout << "===========================\n";
    std::cout << "Improvement statistics\n";
    std::cout << "Iteration " << 5 * (++iter) << std::endl;
    std::cout << "Previous " << previous << " crossings" << std::endl;
    std::cout << "Now      " << placement.getNumCrossings() << " crossings" << std::endl;
    std::cout << "Time passed: " << round(m_instance.m_timer.get_fraction_time_limit() * 100) << "%" << std::endl;
    std::cout << "===========================\n";
  }

  const auto& greedy_sol = placement.getAssignment();
  VertexAssignment res{m_instance};
  for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
  {
    assert(greedy_sol.isAssigned(v) && "Vertex should be assigned!");
    res.assign(v, greedy_sol.getAssigned(v));
  }
  STATS(m_instance.m_timer.end_all_timers();)
  return res;
}

std::unique_ptr<point_id_t[]> SamplingSolver::solve_instance(
          PlacementVisualizer* visualizer,
          bool& valid, size_t& num_crossings)
{
  STATS(m_instance.m_stats.new_run();)
  MaxEmbeddedVertexOrder order{m_instance, m_random.getRandomUint(m_instance.m_graph.getNbVertices())};
  GreedyPlacement placement{m_instance, order, visualizer};
  const auto& res = placement.findPlacement();

  Verifier verifier{m_instance, res};
  valid = verifier.verify(num_crossings);
  STATS(m_instance.m_stats.set_initial_placement_quality(valid, num_crossings);)

  auto assignment = std::make_unique<point_id_t[]>(m_instance.m_graph.getNbVertices());
  for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
  {
    if (res.isAssigned(v)) assignment[v] = res.getAssigned(v);
  }
  return assignment;
}

VertexAssignment SamplingSolver::make_assignment(point_id_t* assigned)
{
  VertexAssignment assignment{m_instance};
  for (vertex_t v = 0; v < m_instance.m_graph.getNbVertices(); ++v)
  {
    if (isDefined(assigned[v])) assignment.assign(v, assigned[v]);
  }
  return assignment;
}