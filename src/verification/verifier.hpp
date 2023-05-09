#ifndef __GD_VERIFIER_HPP__
#define __GD_VERIFIER_HPP__

#include <common/instance.hpp>
#include <common/assignment.hpp>

namespace gd
{

  class Verifier
  {
    public:
      Verifier(const instance_t& instance, const VertexAssignment& assignment)
        : m_instance(instance), m_assignment(assignment) {}
      
      bool verify(size_t& num_crossings) const;

    private:
      bool distinct() const;

    private:
      const instance_t& m_instance;
      const VertexAssignment& m_assignment;
  };

}

#endif
