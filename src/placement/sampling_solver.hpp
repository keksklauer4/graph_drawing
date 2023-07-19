#ifndef __GD_SAMPLING_SOLVER_HPP__
#define __GD_SAMPLING_SOLVER_HPP__

#include "common/assignment.hpp"
#include "common/random_gen.hpp"
#include <gd_types.hpp>
#include <common/instance.hpp>

namespace gd
{

  class SamplingSolver
  {
    public:
      SamplingSolver(const Instance& instance)
        : m_instance(instance) {}

      VertexAssignment solve(std::string vis_path, double fraction_initial_placement = 0.2, bool fasterImprove = true);

    private:
      std::unique_ptr<point_id_t[]> solve_instance(
          PlacementVisualizer* visualizer,
          bool& valid, size_t& num_crossings);

      VertexAssignment make_assignment(point_id_t* assigned);

    private:
      const Instance& m_instance;
      RandomGen m_random;

      bool m_fasterImprove;
  };

}


#endif
