#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <utility>
#include <vector>
#include "EvalMaxSAT.h"

namespace gd
{

  class Hier_SATPlacement
  {
    public:
        Hier_SATPlacement(const Instance& instance);
    private:
        const Instance& m_instance;
  };

}