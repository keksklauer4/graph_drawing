
#include <gd_types.hpp>
#include <common/assignment.hpp>
#include <utility>
#include <vector>
#include <EvalMaxSAT.h>

namespace gd
{

  class SATPlacement
  {
    public:
      SATPlacement(const Instance& instance, std::vector<std::vector<vertex_t>>& partitions, 
      std::vector<std::vector<point_id_t>>& clusters, std::vector<int>& node_2_partition, std::vector<int>& point_2_cluster,
      std::vector<int>& partition_2_cluster, std::vector<int>& cluster_2_partition, const int& max_iterations);
    
    private:
      void reserve_variables();
      void print_point_is_used();
      void print_map_node_to_point();
      void print_mapped_neighbors();
      void build_clauses_mapped_to_one();
      void build_clauses_at_most_one_node_per_point();
      void build_clauses_mapped_equiv_used();
      void build_clauses_no_collinearity();
      void build_clauses_mapped_neighbors();
      void build_clauses_crossing();
      void build_local_clauses();
      bool solve();
      void assign_all();
      void unassign_all();
      void print_mapping();
      void print_used_points();
      void count_crossings();
      bool is_valid();
      void iterative_solving();
      void add_crossing_clauses();
      void add_collinearity_clauses();
      void build_clauses_local_crossing();
      void reserve_variables_mapped_neighbors();
      void copy_assignment(std::vector<int>& current_best_assignment);
      void build_clauses_at_most_one_node_per_point_local();
      void add_collinearity_clauses1();
      vertex_pair_t get_edge_with_most_crossings();
      int get_map_node_to_point(int, int);
      void reserve_variables_map_node_to_point_local();

    private:
      const Instance& m_instance;
      const int& m_max_iterations;
      const int m_num_points;
      const int m_num_nodes;
      const std::vector<std::vector<vertex_t>>& m_partitions;
      const std::vector<std::vector<point_id_t>>& m_clusters;
      const std::vector<int>& m_node_2_partition;
      const std::vector<int>& m_point_2_cluster;
      const std::vector<int>& m_partition_2_cluster;
      const std::vector<int>& m_cluster_2_partition;
      EvalMaxSAT* solver;
      std::vector<int> point_is_used;
      std::vector<int> map_node_to_point;
      std::vector<int> mapped_neighbors;
      VertexAssignment m_assignment;
      std::vector<std::vector<int>> map_node_to_point_local;
      int crossings = 10000000;
  };

}