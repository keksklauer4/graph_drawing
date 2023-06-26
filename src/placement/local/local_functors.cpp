#include "local_functors.hpp"
#include "common/misc.hpp"
#include "gd_types.hpp"

#include <common/instance.hpp>
#include <common/assignment.hpp>
#include <common/random_gen.hpp>
#include <common/kd_tree.hpp>
#include <verification/incremental_collinear.hpp>

using namespace gd;

#define MAX_NUM_VERTICES 10
#define MIN_NUM_VERTICES 3
#define MAX_NUM_POINTS 8

void LocalImprovementNN::set_points()
{
  const auto& pset = m_instance.m_points;
  for(size_t idx = 0; idx < m_vertices.size(); ++idx)
  {
    size_t num_found;
    m_temp_points.clear();
    if (m_vertices[idx] != m_chosen)
    {
      num_found = gd::get_viable_neighbors(
        m_assignment, m_kdtree, m_collinear,
        m_vertexToPoint, m_vertices[idx],
        pset.getPoint(m_previousMapping[idx]),
        m_temp_points
      );
    }
    else
    {
      num_found = gd::get_viable_neighbors(
        m_assignment, m_kdtree, m_collinear,
        m_vertexToPoint, m_chosen,
        pset.getPoint(m_center),
        m_temp_points
      );
    }

    if (num_found == 0)
    {
      m_valid = false;
      return;
    }

    /*for (point_id_t p : m_previousMapping)
    {
      if (isDefined(p) && !m_temp_points.contains(p)
          && !m_assignment.isPointUsed(p)
          && !m_collinear.isPointInvalid(p)
          && m_collinear.isValidCandidate(m_vertices[idx], p))
      {
        m_vertexToPoint.insert(std::make_pair(m_vertices[idx], p));
      }
    }*/
  }
  m_valid = true;
  build_datastructures();
}

void LocalImprovementNN::reset()
{
  reset_base();
  m_chosen = UINT_UNDEF;
  m_center = UINT_UNDEF;
  m_start = false;
}

void LocalImprovementNN::all_assigned()
{
  bool start = true;
  for (vertex_t v : m_vertices) start &= m_assignment.isAssigned(v);
  m_start = start;
}


void LocalImprovementVertexNeighbors::initialize(vertex_t vertex, point_id_t point)
{
  const auto& graph = m_instance.m_graph;
  auto neighborRange = graph.getNeighborIterator(vertex);
  VertexVector neighbors{};
  neighbors.reserve(graph.getNbEdges() / graph.getNbVertices() + 1);
  for (auto it = neighborRange.first; it != neighborRange.second; ++it)
  {
    if (m_assignment.isAssigned(*it)) neighbors.push_back(*it);
  }

  if(neighbors.size() < MIN_NUM_VERTICES)
  {
    m_valid = false;
    return;
  }

  RandomGen random{};
  if (neighbors.size() < MAX_NUM_VERTICES)
  {
    m_vertices.insert(m_vertices.begin(), neighbors.begin(), neighbors.end());
  }
  else
  {
    m_vertices.reserve(MAX_NUM_VERTICES);
    while(m_vertices.size() < (MAX_NUM_VERTICES - 1))
    {
      size_t idx = random.getRandomUint(neighbors.size());
      m_vertices.push_back(neighbors[idx]);
      neighbors[idx] = neighbors.back();
      neighbors.resize(neighbors.size() - 1);
    }
  }
  m_vertices.push_back(vertex);
  record_previous_mapping(m_assignment);

  m_valid = true;
  m_chosen = vertex;
  m_center = point;

  all_assigned();
}


void LocalImprovementBomb::initialize(vertex_t vertex, point_id_t point)
{
  const auto& pset = m_instance.m_points;

  VertexSet included{};
  m_kdtree.k_nearest_neighbors(pset.getPoint(point), MAX_NUM_POINTS,
    [&](point_id_t p) -> bool {
      if(!included.contains(p) && m_assignment.isPointUsed(p))
      {
        m_vertices.push_back(m_assignment.getAssignedVertex(p));
        included.insert(p);
        return true;
      }
      return false;
  });
  if (m_vertices.size() < MIN_NUM_VERTICES)
  {
    m_valid = false;
    return;
  }


  record_previous_mapping(m_assignment);

  m_valid = true;
  m_chosen = vertex;
  m_center = point;

  all_assigned();
}


size_t gd::get_viable_neighbors(
    const VertexAssignment& assignment, const KdTree& kdtree,
    IncrementalCollinear& collinear,
    MultiMap<vertex_t, point_id_t>& options,
    vertex_t v, const Point& center,
    Set<point_id_t>& points_included)
{
  size_t num_options = 0;

  kdtree.k_nearest_neighbors(center, MAX_NUM_POINTS,
    [&](point_id_t p) -> bool {
      if(!points_included.contains(p)
        && !assignment.isPointUsed(p)
        && !collinear.isPointInvalid(p)
        && collinear.isValidCandidate(v, p))
      {
        options.insert(std::make_pair(v, p));
        num_options++;
        points_included.insert(p);
        return true;
      }
      return false;
  });

  return num_options;
}
