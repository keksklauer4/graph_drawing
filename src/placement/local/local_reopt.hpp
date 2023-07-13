#ifndef __GD_LOCAL_REOPT_HPP__
#define __GD_LOCAL_REOPT_HPP__

#include "common/run_statistics.hpp"
#include <gd_types.hpp>
#include <common/instance.hpp>
#include <common/assignment.hpp>

#include <ostream>
#include <verification/verification_utils.hpp>
#include <io/printing.hpp>

#include <iostream>

namespace gd
{

  struct LocalImprovementFunctor
  {
    typedef MultiMap<vertex_t, point_id_t>::const_iterator size_t_mm_iterator;
    typedef RangeIterator<size_t_mm_iterator> range_iterator_t;
  public:
    virtual ~LocalImprovementFunctor() {}

    virtual void initialize(vertex_t vertex, point_id_t point) = 0;
    virtual void set_points() = 0;

    range_iterator_t get_points(vertex_t v) const { return m_vertexToPoint.equal_range(v); }
    range_iterator_t get_vertices(point_id_t p) const { return m_pointToVertex.equal_range(p); }

    RangeIterator<VertexVector::const_iterator> get_vertex_range() const
    { return std::make_pair(m_vertices.begin(), m_vertices.end()); }
    RangeIterator<PointIdVector::const_iterator> get_point_range() const
    { return std::make_pair(m_pointIds.begin(), m_pointIds.end()); }

    size_t get_num_vars() const { return m_vertexToPoint.size(); }

    bool is_valid() const { return m_valid; }

    virtual void reset() = 0;

    template<typename Functor>
    void get_mapping(Functor func) const
    {
      assert(m_vertices.size() == m_previousMapping.size() && "Bad mapping!");
      for (size_t idx = 0; idx < m_vertices.size(); ++idx)
      {
        func(m_vertices.at(idx), m_previousMapping.at(idx));
      }
    }

    virtual bool has_start() const { return false; }

    void set_mapped(size_t idx, point_id_t p) { m_previousMapping[idx] = p; }

    virtual void name(std::ostream& os) const = 0;
    virtual LocalReoptFunctors type() const = 0;

  protected:
    void record_previous_mapping(const VertexAssignment& assignment);
    void reset_base();
    void build_datastructures();


  protected:
    VertexVector m_vertices;
    PointIdVector m_pointIds;
    MultiMap<vertex_t, point_id_t> m_vertexToPoint;
    MultiMap<point_id_t, vertex_t> m_pointToVertex;

    PointIdVector m_previousMapping;

    bool m_valid = false;
  };


  class LocalReOpt
  {
  protected:
    LocalReOpt(const Instance& instance, const VertexAssignment& assignment)
      : m_instance(instance), m_assignment(assignment), m_functor(nullptr) {}
    virtual ~LocalReOpt() { }

  public:
    virtual bool optimize(LocalImprovementFunctor& functor) = 0;

  protected:
    void build_problem()
    {
      build_datastructures();

      create_variables();
      create_vertex_mapped_csts();
      create_at_most_one_vertex_mapped_to_cst();

      // collinearity
      create_pair_collinear_csts();
      create_collinear_triples_csts();

      // crossings
      create_single_crossings();
      create_pair_crossings();
      create_semi_internal_crossings();
      create_internal_crossings();
      create_pair_neighbor_crossings();

      m_subgraph.clear();
      m_existing_edges.clear();
    }

    virtual void create_variables() = 0;
    virtual void create_vertex_mapped_csts() = 0;
    virtual void create_at_most_one_vertex_mapped_to_cst() = 0;

    virtual void create_pair_collinear_csts() = 0;
    virtual void create_collinear_triples_csts() = 0;
    virtual void create_single_crossings() = 0;
    virtual void create_pair_crossings() = 0;
    virtual void create_semi_internal_crossings() = 0;
    virtual void create_internal_crossings() = 0;
    virtual void create_pair_neighbor_crossings() = 0;

    void build_datastructures();

    template<typename Functor>
    void enumerate_pair_collinearities(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
          if (!graph.connected(*u, *v)) continue; // TODO: only subgraph
          auto vRange = m_functor->get_points(*v);
          line_2d_t line {};
          for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
          {
            const auto& pU = pset.getPoint(pointU->second);
            line.first = pU.getCoordPair();
            for (auto pointV = vRange.first; pointV != vRange.second; ++pointV)
            { // all "constant" time (if constant number of vertices and points to map to)
              if (pointU->second == pointV->second) continue;
              const auto& pV = pset.getPoint(pointV->second);
              line.second = pV.getCoordPair();
              for (vertex_t mappedVertex : m_assignment)
              {
                const auto& p = pset.getPoint(m_assignment.getAssigned(mappedVertex));
                if (gd::isOnLine(line, p))
                {
                  func(*u, pointU->second, *v, pointV->second);
                }
                else if (graph.connected(*u, mappedVertex))
                {
                  line_2d_t helper = line;
                  helper.second = p.getCoordPair();
                  if (gd::isOnLine(helper, pV)) func(*u, pointU->second, *v, pointV->second);
                }
                else if (graph.connected(*v, mappedVertex))
                {
                  line_2d_t helper = line;
                  helper.first = p.getCoordPair();
                  if (gd::isOnLine(helper, pU)) func(*u, pointU->second, *v, pointV->second);
                }
              }
            }
          }
        }
      }
    }

    template<typename Functor>
    void enumerate_triplet_collinearities(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
          if (!graph.connected(*u, *v)) continue; // TODO: only subgraph
          auto vRange = m_functor->get_points(*v);
          line_2d_t line {};
          for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
          {
            line.first = pset.getPoint(pointU->second).getCoordPair();
            for (auto pointV = vRange.first; pointV != vRange.second; ++pointV)
            { // all "constant" time (if constant number of vertices and points to map to)
              if (pointU->second == pointV->second) continue;
              line.second = pset.getPoint(pointV->second).getCoordPair();

              for (auto w = vertex_range.first; w != vertex_range.second; ++w)
              {
                if (*w == *u || *w == *v) continue;
                auto wRange = m_functor->get_points(*w);

                for (auto pointW = wRange.first; pointW != wRange.second; ++pointW)
                {
                  if (pointU->second == pointW->second || pointV->second == pointW->second) continue;

                  if (gd::isOnLine(line, pset.getPoint(pointW->second)))
                  {
                    func(*u, pointU->second, *v, pointV->second, *w, pointW->second);
                  }
                }
              }
            }
          }
        }
      }
    }

    template<typename Functor>
    void enumerate_single_crossings(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        auto neighbors = graph.getNeighborIterator(*u);
        for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
        {
          const auto& p1 = pset.getPoint(pointU->second);
          size_t num_crossings = 0;
          for (auto neighbor = neighbors.first; neighbor != neighbors.second; ++neighbor)
          {
            if (!m_assignment.isAssigned(*neighbor)) continue;
            const auto& q1 = pset.getPoint(m_assignment.getAssigned(*neighbor));
            for (const auto& edge : m_existing_edges)
            {
              if (edge.first != q1.id && edge.second != q1.id &&
                gd::intersect(p1, q1, pset.getPoint(edge.first), pset.getPoint(edge.second)))
              {
                num_crossings++;
              }
            }
          }
          if (num_crossings != 0) func(*u, pointU->second, num_crossings);
        }
      }
    }

    template<typename Functor>
    void enumerate_pair_crossings(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
          if (!graph.connected(*u, *v)) continue; // TODO: only subgraph
          auto vRange = m_functor->get_points(*v);
          line_2d_t line {};
          for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
          {
            const auto& p1 = pset.getPoint(pointU->second);
            for (auto pointV = vRange.first; pointV != vRange.second; ++pointV)
            {
              if (pointU->second == pointV->second) continue;
              const auto& q1 = pset.getPoint(pointV->second);
              size_t num_crossings = 0;
              for (const auto& edge : m_existing_edges)
              {
                if (gd::intersect(p1, q1, pset.getPoint(edge.first), pset.getPoint(edge.second))) num_crossings++;
              }

              if (num_crossings != 0) func(*u, pointU->second, *v, pointV->second, num_crossings);
            }
          }
        }
      }
    }

#define FOR_LOOP_POINTS(pIt, range, pointName, cond, block)        \
  for (auto pIt = range.first; pIt != range.second; ++pIt)  \
  {                                                         \
    if (cond) continue;                                     \
    const auto& pointName = pset.getPoint(pIt->second);     \
    block                                                   \
  }

    template<typename Functor>
    void enumerate_internal_crossings(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      auto vertex_range = m_functor->get_vertex_range();


      // awful code block follows. For each tuple of 4 distinct vertices
      // on distinct points, check whether they cross and if so, add some weight
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto u_point_range = m_functor->get_points(*u);
        auto v_range = m_subgraph.equal_range(*u);
        for (auto x = vertex_range.first; x != vertex_range.second; ++x)
        {
          if (*u == *x) continue;
          auto x_point_range = m_functor->get_points(*x);
          auto y_range = m_subgraph.equal_range(*x);
          for (auto v = v_range.first; v != v_range.second; ++v)
          {
            if (v->second == *x) continue;
            auto v_point_range = m_functor->get_points(v->second);
            for (auto y = y_range.first; y != y_range.second; ++y)
            {
              if (y->second == v->second || y->second == *u) continue;

              // now we have 4 distinct vertices with edges uv and xy
              // now iterate over all valid points for each of those
              auto y_point_range = m_functor->get_points(y->second);

              FOR_LOOP_POINTS(pU, u_point_range, pointU, false, {
                FOR_LOOP_POINTS(pV, v_point_range, pointV, pU->second == pV->second, {
                  FOR_LOOP_POINTS(pX, x_point_range, pointX, pX->second == pU->second || pX->second == pV->second, {
                    FOR_LOOP_POINTS(pY, y_point_range, pointY,
                            pY->second == pU->second || pY->second == pV->second || pY->second == pX->second, {
                      if (gd::intersect(pointU, pointV, pointX, pointY))
                      {
                        func(*u, pointU.id, v->second, pointV.id,
                             *x, pointX.id, y->second, pointY.id);
                      }

                    })

                  })
                })
              })
            }
          }
        }
      }
    }

    template<typename Functor>
    void enumerate_semi_internal_crossings(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();

      // awful code block follows. For each tuple of 3 distinct vertices
      // on distinct points, check whether an edge of the third vertex to a neighbor
      // crosses the uv edge
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto u_point_range = m_functor->get_points(*u);
        auto v_range = m_subgraph.equal_range(*u);
        for (auto x = vertex_range.first; x != vertex_range.second; ++x)
        {
          if (*u == *x) continue;
          auto x_point_range = m_functor->get_points(*x);
          for (auto v = v_range.first; v != v_range.second; ++v)
          {
            if (v->second == *x) continue;
            auto v_point_range = m_functor->get_points(v->second);
            auto neighborRange = graph.getNeighborIterator(*x);

            FOR_LOOP_POINTS(pU, u_point_range, pointU, false, {
              FOR_LOOP_POINTS(pV, v_point_range, pointV, pU->second == pV->second, {
                FOR_LOOP_POINTS(pX, x_point_range, pointX, pX->second == pU->second || pX->second == pV->second, {
                  size_t num_crossings = 0;
                  for (auto neighbor = neighborRange.first; neighbor != neighborRange.second; ++neighbor)
                  {
                    if (*neighbor == *u || *neighbor == v->second
                        || !m_assignment.isAssigned(*neighbor)) continue;
                    const auto& neighborPoint = pset.getPoint(m_assignment.getAssigned(*neighbor));
                    if (gd::intersect(pointU, pointV, pointX, neighborPoint))
                    {
                      num_crossings++;
                    }
                  }
                  if (num_crossings == 0) continue;
                  func(*u, pointU.id, v->second, pointV.id,
                       *x, pointX.id, num_crossings);

                })
              })
            })

          }
        }
      }
    }

    template<typename Functor>
    void enumerate_pair_neighbor_crossings(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      const auto& graph = m_instance.m_graph;
      auto vertex_range = m_functor->get_vertex_range();

      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto u_point_range = m_functor->get_points(*u);
        auto neighborURange = graph.getNeighborIterator(*u);
        for (auto x = u + 1; x != vertex_range.second; ++x)
        {
          auto x_point_range = m_functor->get_points(*x);

          auto neighborXRange = graph.getNeighborIterator(*x);

          FOR_LOOP_POINTS(pU, u_point_range, pointU, false, {
            FOR_LOOP_POINTS(pX, x_point_range, pointX, pX->second == pU->second, {
              size_t num_crossings = 0;

              for (auto neighborU = neighborURange.first; neighborU != neighborURange.second; ++neighborU)
              {
                if (!m_assignment.isAssigned(*neighborU)) continue;
                const auto& uNeighborPoint = pset.getPoint(m_assignment.getAssigned(*neighborU));
                for (auto neighborX = neighborXRange.first; neighborX != neighborXRange.second; ++neighborX)
                {
                  if (*neighborU == *neighborX || !m_assignment.isAssigned(*neighborX)) continue;
                  if (gd::intersect(pointU, uNeighborPoint, pointX, pset.getPoint(m_assignment.getAssigned(*neighborX))))
                  {
                      num_crossings++;
                  }
                }
              }
              if (num_crossings == 0) continue;
              func(*u, pointU.id, *x, pointX.id, num_crossings);
            })
          })
        }
      }
    }
#undef FOR_LOOP



  protected:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;

    MultiMap<vertex_t, vertex_t> m_subgraph;
    Vector<pointid_pair_t> m_existing_edges;

    // Initialize in deriving class
    const LocalImprovementFunctor* m_functor;


  };


  struct TwoVertexPointPair
  {
    TwoVertexPointPair(vertex_t uvertex, point_id_t upoint,
                       vertex_t vvertex, point_id_t vpoint)
    {
      if (uvertex < vvertex) initialize(uvertex, upoint, vvertex, vpoint);
      else initialize(vvertex, vpoint, uvertex, upoint);
    }

    void initialize(vertex_t uvertex, point_id_t upoint,
                    vertex_t vvertex, point_id_t vpoint)
    {
      u = uvertex; pU = upoint;
      v = vvertex; pV = vpoint;
    }

    friend bool operator<(const TwoVertexPointPair& p1, const TwoVertexPointPair& p2)
    {
      if (p1.u < p2.u) return true;
      else if (p1.u > p2.u) return false;
      if (p1.pU < p2.pU) return true;
      else if (p1.pU > p2.pU) return false;
      if (p1.v < p2.v) return true;
      else if (p1.v > p2.v) return false;
      if (p1.pV < p2.pV) return true;
      return false;
    }

    vertex_t u;
    point_id_t pU;
    vertex_t v;
    point_id_t pV;
  };
}

#endif
