#ifndef __GD_JSON_HANDLING_HPP__
#define __GD_JSON_HANDLING_HPP__

#include <common/run_statistics.hpp>
#include <common/instance.hpp>
#include <memory>

namespace gd
{

  instance_t parseInstanceFromFile(const std::string& filename);

  void dump_assignment(std::ostream& out, const Instance& instance,
                       const VertexAssignment& assignment);

  void dump_statistics(std::ostream& out, const RunStatistics& statistics);
}


#endif
