
#include "common/instance.hpp"
#include <common/assignment.hpp>
#include <algorithm>
#include <cstddef>
#include <gd_types.hpp>
#include <ostream>
#include <unordered_map>
#include <utility>
#include <vector>
#include "EvalMaxSAT.h"

namespace gd
{
    class ClusterMapBuilder{
        public:
            ClusterMapBuilder(const Instance& instance, HierarchicalGraph& hierarchy) : m_instance(instance), 
            m_hierarchy(hierarchy), m_assignment(instance), solver(new EvalMaxSAT(0)){
            //print_hierachies();
            init_coord_2_id();
            calculate_subgraphs();
            //print_partitions();
            //print_clusters();
            //print_p_2_c_mapping();
            iterative_sat();
        };
        public:
            void print_hierachies();
            void init_coord_2_id();
            void calculate_subgraphs();
            void print_partitions();
            void print_clusters();
            void print_p_2_c_mapping();
            void iterative_sat();
            bool is_valid();
            void map_partition_2_cluster(std::vector<vertex_t> &nodes, std::vector<point_id_t> &points, UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> &e_internal, UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> &e_external);
        private:
            const Instance& m_instance;
            VertexAssignment m_assignment;
            HierarchicalGraph& m_hierarchy;
            EvalMaxSAT* solver;
            UnorderedMap<coordinate_2d_t, point_id_t, PairHashFunc<coordinate_t>> coord_2_id{};
            UnorderedMap< point_id_t, coordinate_2d_t> id_2_coord{};
            std::vector<std::vector<vertex_t>> final_partitions{};
            std::vector<std::vector<point_id_t>> final_clusters{};
            std::vector<int> node_2_partition_id{};
            UnorderedSet<vertex_t> mapped_nodes{};
            UnorderedSet<vertex_t> available_nodes{};
            UnorderedMap<vertex_t, point_id_t> current_mapping{};
            UnorderedMap<point_id_t, vertex_t> current_mapping_reverse{};
            UnorderedSet<point_id_t> mapped_points{};
            UnorderedSet<point_id_t> available_points{};
            UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> current_edges{};

    };
}