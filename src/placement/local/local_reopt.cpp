#include "local_reopt.hpp"
#include "gd_types.hpp"

#include <common/assignment.hpp>

using namespace gd;

void LocalImprovementFunctor::record_previous_mapping(const VertexAssignment& assignment)
{
  m_previousMapping.resize(m_vertices.size());
  auto* write_ptr = m_previousMapping.data();
  for (auto vertex : m_vertices)
  {
    *write_ptr++ = (assignment.isAssigned(vertex) ?
                      assignment.getAssigned(vertex) : UINT_UNDEF);
  }
}

void LocalImprovementFunctor::reset_base()
{
  m_vertices.clear();
  m_pointIds.clear();
  m_vertexToPoint.clear();
  m_pointToVertex.clear();
  m_previousMapping.clear();
  m_valid = false;
}

void LocalImprovementFunctor::build_datastructures()
{
  Set<point_id_t> points{};
  for (const auto& pair : m_vertexToPoint)
  {
    m_pointToVertex.insert(reversePair(pair));
    points.insert(pair.second);
  }
  if (points.size() < m_vertices.size()) m_valid = false;
  else
  {
    m_pointIds.insert(m_pointIds.begin(), points.begin(), points.end());
  }
}
