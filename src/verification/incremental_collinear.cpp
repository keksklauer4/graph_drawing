#include "incremental_collinear.hpp"

using namespace gd;

IncrementalCollinear::IncrementalCollinear(const VertexAssignment& assignment,
  const Instance& instance, CollinearFunction& collFunc)
  : m_assignment(assignment), m_instance(instance), m_collFunc(collFunc) {}
