
#include "common/instance.hpp"
#include <algorithm>
#include <cstddef>
#include <gd_types.hpp>
#include <ostream>
#include <utility>
#include <vector>

namespace gd
{
    namespace clustering {
        class Cluster{
            public:
                Cluster(std::vector<coordinate_2d_t> points) : m_size(points.size()), m_points(points){};
            public:
                int size(){return m_size;};
                coordinate_2d_t coord(int i){return m_points[i];};
                void split_x(bool x){m_split_x = x;};
                void sort_x_direction();
                void sort_y_direction();
                void build_children(bool split_x);
                void print_points();
                bool has_children(){return hasChildren;};
                Cluster* get_left_child();
                Cluster* get_right_child();
            private:
                const int m_size;
                std::vector<coordinate_2d_t> m_points;
                bool m_split_x = false;
                Cluster* left_child = nullptr;
                Cluster* right_child = nullptr;
                bool hasChildren = false;
        };
        class ClusterTree{
            public:
                ClusterTree(const int depth, const Instance& instance) : m_depth(depth), m_instance(instance){
                    initialize_root_cluster();
                    build_clusters();
                    print_leave_clusters();
                };
            private:
                Cluster& initialize_root_cluster();
                void build_clusters();
                void print_root_cluster();
                void print_leave_clusters();
            private:
                const int m_depth;
                const Instance& m_instance;
                Cluster* root = nullptr;
                int splitted = 0;
        };
    }
}