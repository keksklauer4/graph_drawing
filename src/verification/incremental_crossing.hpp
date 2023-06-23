#ifndef __GD_INCREMENTAL_CROSSING_HPP__
#define __GD_INCREMENTAL_CROSSING_HPP__

#include <gd_types.hpp>

namespace gd
{

  class IncrementalCrossing
  {
    public:
      IncrementalCrossing(const VertexAssignment& assignment,const Instance& instance);

      // size_t calculateCrossing();

    private:
      const VertexAssignment& m_assignment;
      const Instance& m_instance;


  };


}



#endif
