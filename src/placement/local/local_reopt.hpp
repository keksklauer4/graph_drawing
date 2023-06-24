#ifndef __GD_LOCAL_REOPT_HPP__
#define __GD_LOCAL_REOPT_HPP__

#include <gd_types.hpp>
#include <common/instance.hpp>
#include <common/assignment.hpp>
#include <verification/verification_utils.hpp>

namespace gd
{

  struct LocalImprovementFunctor
  {
    typedef MultiMap<vertex_t, point_id_t>::const_iterator size_t_mm_iterator;
    typedef RangeIterator<size_t_mm_iterator> range_iterator_t;
  public:
    virtual ~LocalImprovementFunctor() {}

    range_iterator_t get_points(vertex_t v) const { return m_vertexToPoint.equal_range(v); }
    range_iterator_t get_vertices(point_id_t p) const { return m_pointToVertex.equal_range(p); }

    RangeIterator<VertexVector::const_iterator> get_vertex_range() const
    { return std::make_pair(m_vertices.begin(), m_vertices.end()); }
    RangeIterator<PointIdVector::const_iterator> get_point_range() const
    { return std::make_pair(m_pointIds.begin(), m_pointIds.end()); }

    size_t get_num_vars() const { return m_vertexToPoint.size(); }

    bool is_valid() const { return m_valid; }

    template<typename Functor>
    void get_mapping(Functor func) const
    {
      assert(m_vertices.size() == m_previousMapping.size() && "Bad mapping!");
      for (size_t idx = 0; idx < m_vertices.size(); ++idx)
      {
        func(m_vertices.at(idx), m_previousMapping.at(idx));
      }
    }

    void set_mapped(size_t idx, point_id_t p) { m_previousMapping[idx] = p; }

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
    virtual void optimize(LocalImprovementFunctor& functor) = 0;

  protected:
    void build_problem()
    {
      create_variables();
      create_vertex_mapped_csts();
      create_at_most_one_vertex_mapped_to_cst();
      create_pair_collinear_csts();
      create_collinear_triples_csts();
      create_pair_crossings();
    }

    virtual void create_variables() = 0;
    virtual void create_vertex_mapped_csts() = 0;
    virtual void create_at_most_one_vertex_mapped_to_cst() = 0;

    virtual void create_pair_collinear_csts() = 0;
    virtual void create_collinear_triples_csts() = 0;
    virtual void create_single_crossings() = 0;
    virtual void create_pair_crossings() = 0;

    template<typename Functor>
    void enumerate_pair_collinearities(Functor func) const
    {
      const auto& pset = m_instance.m_points;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
          auto vRange = m_functor->get_points(*v);
          line_2d_t line {};
          for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
          {
            line.first = pset.getPoint(pointU->second).getCoordPair();
            for (auto pointV = vRange.first; pointV != vRange.second; ++pointV)
            { // all "constant" time (if constant number of vertices and points to map to)
              if (pointU->second == pointV->second) continue;
              line.second = pset.getPoint(pointV->second).getCoordPair();
              for (vertex_t mappedVertex : m_assignment)
              {
                if (gd::isOnLine(line, pset.getPoint(m_assignment.getAssigned(mappedVertex))))
                {
                  func(*u, pointU->second, *v, pointV->second);
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
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
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
      Vector<pointid_pair_t> existing_edges{};
      existing_edges.reserve(m_instance.m_graph.getNbEdges());
      for (const auto& edge : m_instance.m_graph)
      {
        if (m_assignment.isAssigned(edge.first) && m_assignment.isAssigned(edge.second))
        {
          existing_edges.push_back(pointid_pair_t{
            m_assignment.getAssigned(edge.first),
            m_assignment.getAssigned(edge.second)
          });
        }
      }

      const auto& pset = m_instance.m_points;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
        {
          const auto& p1 = pset.getPoint(pointU->second);
          auto neighbors = m_instance.m_graph.getNeighborIterator(*u);
          size_t num_crossings = 0;
          for (auto neighbor = neighbors.first; neighbor != neighbors.second; ++neighbor)
          {
            if (!m_assignment.isAssigned(*neighbor)) continue;
            const auto& q1 = pset.getPoint(m_assignment.getAssigned(*neighbor));
            for (const auto& edge : existing_edges)
            {
              if (gd::intersect(p1, q1, pset.getPoint(edge.first), pset.getPoint(edge.second))) num_crossings++;
            }
          }
          if (num_crossings != 0) func(*u, pointU->second, num_crossings);
        }
      }
    }

    template<typename Functor>
    void enumerate_pair_crossings(Functor func) const
    {
      Vector<pointid_pair_t> existing_edges{};
      existing_edges.reserve(m_instance.m_graph.getNbEdges());
      for (const auto& edge : m_instance.m_graph)
      {
        if (m_assignment.isAssigned(edge.first) && m_assignment.isAssigned(edge.second))
        {
          existing_edges.push_back(pointid_pair_t{
            m_assignment.getAssigned(edge.first),
            m_assignment.getAssigned(edge.second)
          });
        }
      }

      const auto& pset = m_instance.m_points;
      auto vertex_range = m_functor->get_vertex_range();
      for (auto u = vertex_range.first; u != vertex_range.second; ++u)
      {
        auto uRange = m_functor->get_points(*u);
        for (auto v = u + 1; v != vertex_range.second; ++v)
        {
          auto vRange = m_functor->get_points(*v);
          line_2d_t line {};
          for (auto pointU = uRange.first; pointU != uRange.second; ++pointU)
          {
            const auto& p1 = pset.getPoint(pointU->second);
            for (auto pointV = vRange.first; pointV != vRange.second; ++pointV)
            {
              const auto& q1 = pset.getPoint(pointV->second);
              size_t num_crossings = 0;
              for (const auto& edge : existing_edges)
              {
                if (gd::intersect(p1, q1, pset.getPoint(edge.first), pset.getPoint(edge.second))) num_crossings++;
              }

              if (num_crossings != 0) func(*u, pointU->second, *v, pointV->second, num_crossings);
            }
          }
        }
      }
    }

  protected:
    const Instance& m_instance;
    const VertexAssignment& m_assignment;


    // Initialize in deriving class
    const LocalImprovementFunctor* m_functor;


  };

}

#endif
