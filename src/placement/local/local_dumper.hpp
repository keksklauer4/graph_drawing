#ifndef __GD_LOCAL_DUMPER_HPP__
#define __GD_LOCAL_DUMPER_HPP__


#include "gd_types.hpp"
#include "placement/local/local_reopt.hpp"
#include <algorithm>
#include <cstddef>
namespace gd
{
  typedef std::pair<vertex_t, point_id_t> vertex_point_pair_t;
  constexpr static inline vertex_point_pair_t VP_PAIR_UNDEF = {UINT_UNDEF, UINT_UNDEF};

  struct CrossingTerm
  {
    typedef std::array<vertex_point_pair_t, 4> crossing_var_array_t;
    CrossingTerm() {}
    CrossingTerm(vertex_pair_t first,
                 vertex_pair_t second = VP_PAIR_UNDEF,
                 vertex_pair_t third = VP_PAIR_UNDEF,
                 vertex_pair_t fourth = VP_PAIR_UNDEF)
    {
      m_terms[0] = first; m_terms[1] = second; m_terms[2] = third; m_terms[3] = fourth;
      std::sort(m_terms.begin(), m_terms.end());
    }

    crossing_var_array_t::const_iterator begin() const { return m_terms.begin(); }
    crossing_var_array_t::const_iterator end() const { return m_terms.end(); }

    friend bool operator<(const CrossingTerm& c1, const CrossingTerm& c2)
    { return std::lexicographical_compare(c1.begin(), c1.end(), c2.begin(), c2.end()); }

    crossing_var_array_t m_terms;
  };

  class LocalDumper : LocalReOpt
  {
  public:
    LocalDumper(const Instance& instance, const VertexAssignment& assignment)
      : LocalReOpt(instance, assignment) {}
    ~LocalDumper();

    bool optimize(LocalImprovementFunctor& functor) override;


  private:
    void create_variables() override;
    void create_vertex_mapped_csts() override;
    void create_at_most_one_vertex_mapped_to_cst() override;

    void create_pair_collinear_csts() override;
    void create_collinear_triples_csts() override;
    void create_single_crossings() override;
    void create_pair_crossings() override;
    void create_internal_crossings() override;
    void create_semi_internal_crossings() override;
    void create_pair_neighbor_crossings() override;

  private:
    Map<CrossingTerm, size_t> m_linear;
  };

}


#endif
