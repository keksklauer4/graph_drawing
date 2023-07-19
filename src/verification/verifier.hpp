#ifndef __GD_VERIFIER_HPP__
#define __GD_VERIFIER_HPP__

#include <gd_types.hpp>

namespace gd
{

  class Verifier
  {
    public:
      Verifier(const Instance& instance, const VertexAssignment& assignment)
        : m_instance(instance), m_assignment(assignment) {}

      bool verify(size_t& num_crossings) const;
      bool distinct() const;

    private:
      const Instance& m_instance;
      const VertexAssignment& m_assignment;
  };

}

#endif
