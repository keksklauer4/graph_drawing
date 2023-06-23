#ifndef __GD_LINE_CROSSINGS_HPP__
#define __GD_LINE_CROSSINGS_HPP__

#include <gd_types.hpp>
#include <common/instance.hpp>
#include <common/assignment.hpp>

namespace gd
{

  // check that every vertex is mapped before calling this!
  size_t countCrossings(const instance_t& instance, const VertexAssignment& assignment);

}

#endif
