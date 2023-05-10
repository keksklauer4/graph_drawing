#ifndef __GD_COLLINEAR_HPP__
#define __GD_COLLINEAR_HPP__

#include <gd_types.hpp>
#include <common/instance.hpp>

namespace gd
{

  // false if no collinearities
  bool trivialCollinearityCheck(const instance_t& instance,
    const VertexAssignment& assignment);

}

#endif
