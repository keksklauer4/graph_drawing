#include "crossing_hierarchy.hpp"
#include "gd_types.hpp"
#include "verification/verification_utils.hpp"

#include <common/assignment.hpp>
#include <common/instance.hpp>

#include <AABBCC/AABBCC.h>
#include <limits>
#include <io/printing.hpp>

using namespace gd;
using namespace aabb;

namespace
{
  void setBoundingBoxCoordinate(double& lb, double& ub, double val1, double val2)
  {
    if (val1 <= val2)
    {
      lb = val1;
      ub = val2;
    }
    else
    {
      lb = val2;
      ub = val1;
    }
  }

  void setBox(CrossingHierarchy::EdgeBox& box, const Point& p1, const Point& p2)
  {
    setBoundingBoxCoordinate(box.first[0], box.second[0], p1.x, p2.x);
    setBoundingBoxCoordinate(box.first[1], box.second[1], p1.y, p2.y);
  }
}


CrossingHierarchy::CrossingHierarchy(const Instance& instance, const VertexAssignment& assignment)
  : m_instance(instance), m_assignment(assignment), m_numEdgesPlaced(0)
{
  const auto& graph = m_instance.m_graph;
  unsigned int edgeIndex = 0;
  m_edgeBoxes.resize(graph.getNbEdges());
  m_edges.reserve(graph.getNbEdges());
  for (const auto& edge : graph)
  {
    if (edge.first < edge.second)
    {
      m_edgeBoxes[edgeIndex].first.resize(2);
      m_edgeBoxes[edgeIndex].second.resize(2);
      m_edgeIndices[edge] = edgeIndex++;
      m_edges.push_back(edge);
    }
  }

  m_bvhTree = new Tree{2, 0, (unsigned int)m_edgeIndices.size(), false};

  m_queryBox.first.resize(2);
  m_queryBox.second.resize(2);
}

void CrossingHierarchy::place(vertex_t v, point_id_t p)
{
  auto neighbors = m_instance.m_graph.getNeighborIterator(v);
  const auto& vP = m_instance.m_points.getPoint(p);
  for (auto it = neighbors.first; it != neighbors.second; ++it)
  {
    if (m_assignment.isAssigned(*it))
    { // add edge to tree
      auto ordered = getOrderedPair(v, *it);
      const auto& wP = m_instance.m_points.getPoint(m_assignment.getAssigned(*it));
      size_t particleIndex = m_edgeIndices[ordered];
      auto& edgeBox = m_edgeBoxes[particleIndex];
      setBox(edgeBox, vP, wP);
      m_bvhTree->insertParticle(particleIndex, edgeBox.first, edgeBox.second);
      m_numEdgesPlaced++;
    }
  }
}

void CrossingHierarchy::deplace(vertex_t v)
{
  auto neighbors = m_instance.m_graph.getNeighborIterator(v);
  for (auto it = neighbors.first; it != neighbors.second; ++it)
  {
    if (m_assignment.isAssigned(*it))
    { // remove edge from tree
      auto ordered = getOrderedPair(v, *it);
      size_t particleIndex = m_edgeIndices[ordered];
      m_bvhTree->removeParticle(particleIndex);
      m_numEdgesPlaced--;
    }
  }
}

size_t CrossingHierarchy::count_crossings(vertex_t v, point_id_t p, size_t ub)
{
  if (m_numEdgesPlaced == 0) return 0;
  size_t num_crossings = 0;
  const auto& pV = m_instance.m_points.getPoint(p);
  auto neighbors = m_instance.m_graph.getNeighborIterator(v);

  for (auto it = neighbors.first; it != neighbors.second; ++it)
  {
    if (m_assignment.isAssigned(*it))
    { // remove edge from tree
      auto ordered = getOrderedPair(v, *it);
      size_t particleIndex = m_edgeIndices[ordered];
      const auto& pW = m_instance.m_points.getPoint(
          m_assignment.getAssigned(*it)
      );
      setBox(m_queryBox, pV, pW);
      AABB box{m_queryBox.first, m_queryBox.second};

      m_bvhTree->query(std::numeric_limits<unsigned int>::max(), box,
        [&](unsigned int collidingIndex){
          const auto& edge = m_edges[collidingIndex];
          if (edge.first != *it && edge.second != *it)
          { // no need to check edge.first != v as v is not placed by assumption!
            const auto& pX = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.first));
            const auto& pY = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.second));
            if (gd::intersect(pV, pW, pX, pY)) num_crossings++;
          }
          return num_crossings < (ub + 1); // stop if too many crossings
      });
    }
    if (num_crossings >= (ub + 1)) break; // we are too bad anyways, so stop
  }
  return num_crossings;
}
