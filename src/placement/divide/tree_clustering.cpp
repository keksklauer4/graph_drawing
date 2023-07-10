#include <cctype>
#include "common/instance.hpp"
#include <cmath>
#include <exception>
#include <functional>
#include <ostream>
#include "EvalMaxSAT.h"
#include "verification/verifier.hpp"
#include <placement/divide/graph_partitioning.hpp>
#include <placement/divide/tree_clustering.hpp>
#include "gd_types.hpp"
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>
#include <iostream>
#include <math.h>

#include "verification/verification_utils.hpp"
#include <verification/line_crossings.hpp>
#include <verification/trivial_collinear.cpp>
#include <cmath>


#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Boolean_set_operations_2.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 CGALP;
typedef CGAL::Polygon_2<K> Polygon_2;

typedef std::pair<coordinate_2d_t, coordinate_2d_t> rect_t;
using namespace gd;
using namespace partitioning;
struct sort_x{
    bool operator()(const coordinate_2d_t &left, const coordinate_2d_t &right){
        return left.first < right.first || (left.first == right.first && left.second < right.second);
    }
};
struct sort_y{
    bool operator()(const coordinate_2d_t &left, const coordinate_2d_t &right){
        return left.second < right.second || (left.second == right.second && left.first < right.first);
    }
};
void ClusterMapBuilder::init_coord_2_id(){
    auto point_range = m_instance.m_points.getPointIterator();
    for (auto it = point_range.first; it != point_range.second; ++it){
        point_id_t rid = it->id;
        coordinate_2d_t rc = it->getCoordPair();
        coord_2_id.insert({rc, rid});
        id_2_coord.insert({rid, rc});
    }
}
void ClusterMapBuilder::calculate_subgraphs(){
    //std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "+++++++++calculate subgraphs++++++++++" << std::endl;
    //std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << m_instance.m_points.getNumPoints() << " points and " << m_instance.m_graph.getNbVertices() << " nodes" << std::endl; 
    double splitting_factor = (m_instance.m_points.getNumPoints() / m_instance.m_graph.getNbVertices());
    //std::cout <<"splitting factor " << splitting_factor << std::endl; 

    Partition root_partition = m_hierarchy.get_root_partition();
    Interval inter = root_partition.interval;
    Queue<size_t> queue{};
    queue.push(root_partition.left_child);
    queue.push(root_partition.right_child);
    
    int level = 1;
    int nodes_done = 0;
    int nodes_on_level = 2;
    int nodes_on_next_level = 0;
    int total_leafs = 0;
    int proccessed_leafs = 0;
    std::vector<std::vector<vertex_t>> partitions;
    partitions.push_back(std::vector<vertex_t>(0));
    partitions.push_back(std::vector<vertex_t>(0));

    final_partitions.push_back(std::vector<vertex_t>(0));
    final_partitions.push_back(std::vector<vertex_t>(0));
    std::vector<bool> splitted(2, true);
    std::vector<bool> splitted_prev(2, true);
    int current_partition = 0;
    //std::vector<int> current_mapping;
    UnorderedMap<int, int> current_mapping{};
    current_mapping.insert({0,0});
    current_mapping.insert({1,1});
    std::vector<int> remap;
    for(int i = 0; i < splitted.size(); i++){
        if(!splitted[i]) continue;
            remap.push_back(i);
    }
    std::vector<int> number_of_part_per_quadrant = {1,1,1,1};
    std::vector<int> part_2_quadrant(4, -1);
    int next_index_final = 0;

    std::vector<coordinate_2d_t> points{};
    for(auto p: m_instance.m_points){
        points.push_back(p.getCoordPair());
    }
    /*
    for(auto coord : points){
        std::cout << coord << " ";
    }
    std::cout << std::endl;
    */
    std::vector<std::vector<coordinate_2d_t>> old_clusters{};
    std::vector<std::vector<coordinate_2d_t>> new_clusters{};

    int l_size = m_hierarchy.get_partition(root_partition.left_child).size();
    double d_l_size = l_size * splitting_factor;
    int floor_d_l_size = std::floor(d_l_size);
    //std::cout << "floor_d_l_size "<< floor_d_l_size << std::endl;
    std::sort(points.begin(), points.end(), sort_x());
    old_clusters.push_back(std::vector<coordinate_2d_t>(points.begin(), points.begin() + floor_d_l_size));
    old_clusters.push_back(std::vector<coordinate_2d_t>(points.begin() + floor_d_l_size, points.end()));
    /*
    for(int i = 0; i < current_clusters.size(); i++){
        current_clusters[i]->print_points();
    }
    */
    while (!queue.empty()) {
        Partition part = m_hierarchy.get_partition(queue.front());
        Vector<vertex_t> vertecies = m_hierarchy.get_partition_vertecies(part.interval.start, part.interval.end);
        std::vector<vertex_t> v(vertecies.size(), 0);
        for(int i = 0; i < vertecies.size(); i++){
            v[i] = vertecies[i];
        }

        bool is_leaf = !part.has_children() || (v.size() < 15);
        if(!is_leaf){//not a leaf node
            queue.push(part.left_child);
            queue.push(part.right_child);
            nodes_on_next_level+=2;
        }else{
            total_leafs++;
        }
        current_partition++;

        int next_0 = 0;
        for(int i = 0; i < partitions.size();i++){
            if(partitions[i].empty()){
                next_0 = i;
                break;
            }
        }
        int next_0_final = -1;
        for(int i = next_index_final; i < final_partitions.size(); i++){
            if (!final_partitions[i].empty()) continue;
            next_0_final = i;
            break;
        }
        //std::cout << "next_final_index is currently " << next_index_final << std::endl;
        //std::cout << "next_0 after it is " << next_0_final << std::endl;
            
        next_index_final = next_0_final;
        if(is_leaf){
            /*
            std::cout << "replacing a [ ] with a final array" << std::endl;
            std::cout << "insert [ ";
            for(int i = 0; i < v.size(); i++){
                std::cout << v[i] << " ";
            }
            std::cout << "] into ";
            */
            if(next_0_final == -1){
                std::cout << "push_back, since no {0}" << std::endl; 
                final_partitions.push_back(v);
            }else{
                /*
                std::cout << "insert vector at " << next_0_final;

                std::cout << "\t[ ";
                for(int i = 0; i < v.size(); i++){
                    std::cout << v[i] << " ";
                }
                std::cout << "]" << std::endl; 
                */
                /*
                std::cout << "\t[ ";
                for(int i = 0; i < final_partitions[next_0_final].size(); i++){
                    std::cout << final_partitions[next_0_final][i] << " ";
                }
                std::cout << "]" << std::endl; 
                */
                final_partitions[next_0_final] = v;
                next_index_final++;

            }
        }else{
            //std::cout << "replacing a vector with an additional [ ] while next_index_final is "<< next_index_final + 1 << std::endl;
            final_partitions.insert(final_partitions.begin() + next_index_final + 1, std::vector<vertex_t>(0));
            next_index_final+=2;
        }
        /*
        std::cout << "insert [ ";
        for(int i = 0; i < v.size(); i++){
            std::cout << v[i] << " ";
        }
        std::cout << "] into partitions["<<next_0<<"]"<<std::endl; 
        
        for (int i = 0; i < partitions.size(); i++) {
            std::cout << "partition " << i << "\t";
            for(int j = 0; j < partitions[i].size(); j++){
                std::cout << partitions[i][j] << " ";
            }
            std::cout << std::endl;
        }
        */
        partitions[next_0] = v;
        if(!is_leaf) splitted[next_0] = true;
        
        nodes_done++;
        queue.pop();

        if(nodes_done == nodes_on_level){

            //copy new partitions if they where splitted

            //std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
            level++;
            /*
            std::cout << "-----------splitted--------------\n" << "[ " ;
            for(int i = 0; i< splitted.size(); i++){
                std::cout << splitted[i] << " ";
            }
            std::cout << "]" << std::endl;
            for (int i = 0; i < final_partitions.size(); i++) {
                std::cout << "partition " << i << "\t [";
                for(int j = 0; j < final_partitions[i].size(); j++){
                    std::cout << final_partitions[i][j] << " ";
                }
                std::cout << "]" << std::endl;
            }
            std::cout << "number of partitions = " << partitions.size() << " at level = " << level << std::endl;
            for (int i = 0; i < partitions.size(); i++) {
                std::cout << "partition " << i << "\t";
                for(int j = 0; j < partitions[i].size(); j++){
                    std::cout << partitions[i][j] << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "--------------------------------" << std::endl;
            */
            //calculate contracted graph




            /*
            UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> edges;
            for(auto e : m_instance.m_graph){
                if(e.first > e.second) continue;
                int i = e.first, k = e.second;
                int part_i = -1, part_k = -1;
                for(int j = 0; j < partitions.size(); j++){
                    if(std::find(partitions[j].begin(), partitions[j].end(), i) != partitions[j].end()) part_i = j;
                    if(std::find(partitions[j].begin(), partitions[j].end(), k) != partitions[j].end()) part_k = j;
                }
                if(part_i == part_k) continue;
                edges.insert(std::make_pair(part_i, part_k));
            }
            */





            /*
            for(auto e : edges){
                if(e.first > e.second) continue;
                std::cout << "(" << e.first << " -> " << e.second << ")\t ([ ";
                for (int i = 0; i< partitions[e.first].size(); i++) {
                    std::cout << partitions[e.first][i] << " ";
                }
                std::cout << "] -> [ ";
                for(int i = 0; i< partitions[e.second].size(); i++) {
                    std::cout << partitions[e.second][i] << " ";
                }
                std::cout << "])" << std::endl;
            }
            */
            if(partitions.size() > 2){
                delete solver;
                solver = new EvalMaxSAT(0);
                /*
                std::cout << "splitted_prev \t[ " ;
                 for(int i = 0; i< splitted_prev.size(); i++){
                    std::cout << splitted_prev[i] << " ";
                }
                std::cout << "]" << std::endl;
                
                //std::cout << "map " << partitions.size() << " partitions" << std::endl;
                for(int i = 0; i < partitions.size(); i++){
                    std::cout << "partition " << i << "\t[ ";
                    for(int j = 0; j < partitions[i].size(); j++){
                        std::cout << partitions[i][j] << " ";
                    }
                    std::cout << "]\t" << partitions[i].size() << std::endl;
                }
                
                for(int i = 0; i < final_partitions.size(); i++){
                    std::cout << "final_partitions " << i << "\t[ ";
                    for(int j = 0; j < final_partitions[i].size(); j++){
                        std::cout << final_partitions[i][j] << " ";
                    }
                    std::cout << "]\t" << final_partitions[i].size() << std::endl;
                }
                */

                std::vector<int> n_2_p(m_instance.m_graph.getNbVertices(), -1);
                for(int i = 0; i < partitions.size(); i++){
                    for(auto id:partitions[i]){
                        n_2_p[id] = i;
                    }
                }
                /*
                //build contracted graph
                UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> edges;
                for(auto p : partitions){
                    for(auto n : p){
                        auto it = m_instance.m_graph.getNeighborIterator(n);
                        for (auto neighbor = it.first; neighbor != it.second; ++neighbor){
                            if(n_2_p[n] == n_2_p[*neighbor]) continue;
                            edges.insert(std::make_pair(n_2_p[n],n_2_p[*neighbor]));
                        }
                    }
                }
                
                std::vector<rect_t> rectangles(old_clusters.size());
                //std::cout << rectangles.size() << "  " << old_clusters.size() << std::endl;
                //build partition rectangles
                for(int i = 0; i < old_clusters.size(); i++){
                    int x0 = 10000;
                    int x1 = 0;
                    int y0 = 10000;
                    int y1 = 0;
                    for(int j = 0; j < old_clusters[i].size(); j++){
                        coordinate_2d_t c = old_clusters[i][j];
                        if(c.first < x0) x0 = c.first;
                        if(c.first > x1) x1 = c.first;
                        if(c.second < y0) y0 = c.second;
                        if(c.second > y1) y1 = c.second;
                    }
                    //std::cout << x0 << ", " << y0 << ", " << x1-x0 << ", " << y1-y0 << std::endl;
                    //since x0 < x1 && y0 < y1 this shouldnt be an issue
                    rect_t r = std::make_pair(std::make_pair(x0, y0), std::make_pair(x1, y1));
                    rectangles[i] = r;
                }
                
                for(auto e0 : edges){
                    if(e0.first > e0.second) continue;
                    for(auto e1 : edges){
                        if(e0.first == e1.first && e0.second == e1.second) continue;
                        if(e1.first > e1.second) continue;
                        std::cout << "check " << e0 << " and " << e1 << std::endl;
                        
                        std::cout << "edge0 [ ";
                        for(auto c0 : old_clusters[e0.first]){
                            std::cout << c0 << " ";
                        }
                        std::cout << "] ---> [ ";
                        for(auto c1 : old_clusters[e0.second]){
                           std::cout << c1<< " ";
                        }
                        std::cout << "]" << std::endl;

                        std::cout << "edge1 [ ";
                        for(auto c2 : old_clusters[e1.first]){
                            std::cout << c2 << " ";
                        }
                        std::cout << "] ---> [ ";
                        for(auto c3 : old_clusters[e1.second]){
                            std::cout << c3<< " ";
                        }
                        std::cout << "]" << std::endl;
                        
                        bool inter = CGAL::do_intersect(e_2_pgn[e0], e_2_pgn[e1]);
                        if(inter){
                            std::cout << "intersection" << std::endl;
                            std::cout << "edge0 " << e0 << std::endl;
                            std::cout << "edge0 " << e1 << std::endl;
                            
                        }
                    }
                }
                */

                /*
                std::cout << "splitted \t[ " ;
                 for(int i = 0; i< splitted.size(); i++){
                    std::cout << splitted[i] << " ";
                }
                std::cout << "]" << std::endl;
                */

                //in each iteration there are at most 2 times as many partitions
                //for example each of the 4 initial clusters gets splitted
                // 2|3---6|7
                //  |     |
                // 0|1---4|5
                //if only one partition gets spliited, for example partition 3 which is mapped to cluster 1
                // 1|2--4
                //  |   |
                //  0---3
                //the indices get shifted by one for each split prior to the cluster index
                std::sort(remap.begin(), remap.end(), std::greater<int>());
                int prefix = 0;
                for(int i = 0; i < splitted_prev.size(); i++){
                    if(splitted_prev[i]) prefix++;
                }
                //std::cout << splitted_prev.size() << "<>" << prefix << std::endl;
                std::vector<std::pair<int,int>> double_for_each_split(prefix + splitted_prev.size());
                std::vector<int> old_id_to_new_id(splitted_prev.size(), -1);
                int next_id = 0;
                /*
                for(auto mapped : current_mapping){
                    std::cout << mapped.first << " -> " << mapped.second << std::endl;
                }
                */
                for(int i = 0; i < splitted_prev.size(); i++){
                    if(splitted_prev[i]){
                        int m2 = current_mapping[i];
                        //std::cout << i << "_a is mapped to " << m2 << " with the new id " << next_id + 0 << std::endl;
                        //std::cout << i << "_b is mapped to " << m2 << " with the new id " << next_id + 1 << std::endl;
                        double_for_each_split[next_id + 0] = std::make_pair(m2, next_id + 0);
                        double_for_each_split[next_id + 1] = std::make_pair(m2, next_id + 1);
                        old_id_to_new_id[i] = next_id;
                        next_id+=2;
                    }else{
                        int m2 = current_mapping[i];
                        //std::cout << i << " is mapped to " << m2 << " with the new id " << next_id << std::endl;
                        double_for_each_split[next_id] = std::make_pair(m2, next_id);
                        old_id_to_new_id[i] = next_id;
                        next_id++;
                    }
                }
                for(int i = 0; i < splitted_prev.size(); i++){
                    if (splitted_prev[i]) {
                        //reserves 2 slots for the newly splitted partitionS
                        new_clusters.push_back(old_clusters[i]);
                        new_clusters.push_back(old_clusters[i]);
                    }else{
                        new_clusters.push_back(old_clusters[i]);
                    }
                }
                /*
                for(int i = 0; i < old_id_to_new_id.size(); i++){
                    if(splitted_prev[i]){
                        std::cout << i << "~>" << old_id_to_new_id[i] << "/" << old_id_to_new_id[i] + 1 << std::endl;
                    }else{
                        std::cout << i << "~>" << old_id_to_new_id[i] << std::endl;
                    }
                }
                */
                
                /*
                std::cout << "[ ";
                for(int i = 0; i < double_for_each_split.size(); i++){
                    std::cout << "<" << double_for_each_split[i].first << ", " << double_for_each_split[i].second << "> ";
                }
                std::cout << "]" << std::endl;
                */
                /*
                std::sort(double_for_each_split.begin(), double_for_each_split.end(),
                [](const std::pair<int, int> &left, const std::pair<int, int> &right){
                    return left.first > right.first || (left.first == right.first && left.second < right.second);
                });*/
                std::sort(double_for_each_split.begin(), double_for_each_split.end(),
                [](const std::pair<int, int> &left, const std::pair<int, int> &right){
                    return left.first < right.first || (left.first == right.first && left.second < right.second);
                });
                /*
                std::cout << "[ ";
                for(int i = 0; i < double_for_each_split.size(); i++){
                    std::cout << "<" << double_for_each_split[i].first << ", " << double_for_each_split[i].second << "> ";
                }
                std::cout << "]" << std::endl;
                */
                
                for(int i = 0; i < double_for_each_split.size() - 1; i++){
                    if(double_for_each_split[i].first != double_for_each_split[i +1].first) continue;
                    for(int j = i + 2; j < double_for_each_split.size(); j++){
                        //std::cout << "increase " << double_for_each_split[j].first << std::endl;
                        double_for_each_split[j].first++;
                    }
                }

                UnorderedMap<int, int> new_mapping;
                for(int i = 0; i < double_for_each_split.size(); i++){
                    auto p = double_for_each_split[i];
                    new_mapping[p.second] = p.first;
                }
                std::vector<int> new_mapping_vec(partitions.size(), -1);

                for(auto mapped : new_mapping){
                    new_mapping_vec[mapped.first] = mapped.second;
                }
                std::vector<std::vector<int>> remap_partitions_var(remap.size());
                for(int i = 0; i < remap_partitions_var.size(); i++){
                    std::vector<int> remap_vars{};
                    for(int j = 0; j < 4; j++){
                        remap_vars.push_back(solver->newVar());
                    }
                    remap_partitions_var[i] = remap_vars;
                }
                /*
                //std::cout << remap_partitions_var.size() << " partitions to remap" << std::endl;
                for(int i = 0; i < remap_partitions_var.size(); i++){
                    //always size 4 sinze 2 partitions get remapped
                    //use variables [m_00, m_01, m_10, m_11]
                    //add clauses (m_00, m_01), (-m_00, -m_01), (m_10, m_11), (-m_10, -m_11) and (-m_00, -m_10), (-m_01, -m_11)
                    solver->addClause({remap_partitions_var[i][0], remap_partitions_var[i][1]});
                    solver->addClause({-remap_partitions_var[i][0], -remap_partitions_var[i][1]});
                    solver->addClause({remap_partitions_var[i][2], remap_partitions_var[i][3]});
                    solver->addClause({-remap_partitions_var[i][2], -remap_partitions_var[i][3]});
                    solver->addClause({-remap_partitions_var[i][0], -remap_partitions_var[i][2]});
                    solver->addClause({-remap_partitions_var[i][1], -remap_partitions_var[i][3]});
                }
                if(solver->solve()){
                    //std::cout << "found mapping" << std::endl;
                    for(int i = 0; i < remap_partitions_var.size(); i++){
                        bool m_00 = solver->getValue(remap_partitions_var[i][0]);
                        bool m_01 = solver->getValue(remap_partitions_var[i][1]);
                        bool m_10 = solver->getValue(remap_partitions_var[i][2]);
                        bool m_11 = solver->getValue(remap_partitions_var[i][3]);
                        //std::cout << m_00 << ", " << m_01 << ", " << m_10 << ", " << m_11 << std::endl;
                        int m_0 = -1;
                        int m_1 = -1;
                        if(m_00){
                            m_0 = 0;
                        }else if (m_01) {
                            m_0 = 1;
                        }else{
                            throw std::exception();
                        }
                        if(m_10){
                            m_1 = 0;
                        }else if (m_11) {
                            m_1 = 1;
                        }else{
                            throw std::exception();
                        }
                        int part = remap[i];
                        int cluster = current_mapping[part];
                        int splits_before = 0;
                        int new_id = old_id_to_new_id[part];
                        for(int j = 0; j < current_mapping.size(); j++){
                            if(splitted_prev[j] && current_mapping[j] < cluster) splits_before++;
                        }
                        int id = splits_before+cluster;
                        new_mapping[new_id] += m_0;
                        new_mapping[new_id + 1] += m_1;

                        int num_elements_a = partitions[new_id].size();
                        int num_elements_b = partitions[new_id + 1].size();
                        l_size = 0;

                        if(level%2 == 0){
                            std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_y());
                        }else{
                            std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_x());
                        }
                        if(m_0 == 0){
                            //std::cout << "partition " << new_id << " is the left/lower part" << std::endl;
                            l_size = num_elements_a;
                            d_l_size = l_size * splitting_factor;
                            floor_d_l_size = std::floor(d_l_size);
                            new_clusters[new_id] = std::vector<coordinate_2d_t>(old_clusters[part].begin(), old_clusters[part].begin() + floor_d_l_size);
                            new_clusters[new_id + 1] = std::vector<coordinate_2d_t>(old_clusters[part].begin() + floor_d_l_size, old_clusters[part].end());
                        }else{
                            //std::cout << "partition " << new_id + 1 << " is the left/lower part" << std::endl;
                            l_size = num_elements_b;
                            d_l_size = l_size * splitting_factor;
                            floor_d_l_size = std::floor(d_l_size);
                            new_clusters[new_id] = std::vector<coordinate_2d_t>(old_clusters[part].begin() + floor_d_l_size, old_clusters[part].end());
                            new_clusters[new_id + 1] = std::vector<coordinate_2d_t>(old_clusters[part].begin(), old_clusters[part].begin() + floor_d_l_size);
                        }
                    }
                    
                    for(int i = 0; i < remap_partitions_var.size(); i++){
                        int part = remap[i];
                        int cluster = current_mapping[part];
                        int splits_before = 0;
                        int new_id = old_id_to_new_id[part];
                        for(int j = 0; j < current_mapping.size(); j++){
                            if(splitted_prev[j] && current_mapping[j] < cluster) splits_before++;
                        }
                        int id = splits_before+cluster;
                        new_mapping[new_id] += 0;
                        new_mapping[new_id + 1] += 1;
                        int num_elements_a = partitions[new_id].size();
                        int num_elements_b = partitions[new_id + 1].size();
                        if(level%2 == 0){
                            std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_y());
                        }else{
                            std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_x());
                        }
                        l_size = num_elements_a;
                        d_l_size = l_size * splitting_factor;
                        floor_d_l_size = std::floor(d_l_size);
                        new_clusters[new_id] = std::vector<coordinate_2d_t>(old_clusters[part].begin(), old_clusters[part].begin() + floor_d_l_size);
                        new_clusters[new_id + 1] = std::vector<coordinate_2d_t>(old_clusters[part].begin() + floor_d_l_size, old_clusters[part].end());
                    }
                    old_clusters.clear();
                    for(int j = 0; j < new_clusters.size(); j++){
                        if(new_clusters[j].empty()) std::cout << "!!!fail!!!" << std::endl;
                        old_clusters.push_back(new_clusters[j]);
                    }
                    new_clusters.clear();
                    current_mapping.clear();
                    for(auto mapped : new_mapping){
                        current_mapping.insert(mapped);
                    }
                    new_mapping.clear();
                    remap.clear();
                    for(int i = 0; i < splitted.size(); i++){
                        if(!splitted[i]) continue;
                        remap.push_back(i);
                    }
                }else{
                    std::cout << "unsat" << std::endl; 
                }*/
                for(int i = 0; i < remap_partitions_var.size(); i++){
                    int part = remap[i];
                    int cluster = current_mapping[part];
                    int splits_before = 0;
                    int new_id = old_id_to_new_id[part];
                    for(int j = 0; j < current_mapping.size(); j++){
                        if(splitted_prev[j] && current_mapping[j] < cluster) splits_before++;
                    }
                    int id = splits_before+cluster;
                    new_mapping[new_id] += 0;
                    new_mapping[new_id + 1] += 1;
                    int num_elements_a = partitions[new_id].size();
                    int num_elements_b = partitions[new_id + 1].size();
                    if(level%2 == 0){
                        std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_y());
                    }else{
                        std::sort(old_clusters[part].begin(), old_clusters[part].end(), sort_x());
                    }
                    l_size = num_elements_a;
                    d_l_size = l_size * splitting_factor;
                    floor_d_l_size = std::floor(d_l_size);
                    new_clusters[new_id] = std::vector<coordinate_2d_t>(old_clusters[part].begin(), old_clusters[part].begin() + floor_d_l_size);
                    new_clusters[new_id + 1] = std::vector<coordinate_2d_t>(old_clusters[part].begin() + floor_d_l_size, old_clusters[part].end());
                }
                old_clusters.clear();
                for(int j = 0; j < new_clusters.size(); j++){
                    if(new_clusters[j].empty()) std::cout << "!!!fail!!!" << std::endl;
                    old_clusters.push_back(new_clusters[j]);
                }
                new_clusters.clear();
                current_mapping.clear();
                for(auto mapped : new_mapping){
                    current_mapping.insert(mapped);
                }
                new_mapping.clear();
                remap.clear();
                for(int i = 0; i < splitted.size(); i++){
                    if(!splitted[i]) continue;
                    remap.push_back(i);
                }
            }
            //std::cout << "copy partitions" << std::endl;
            partitions.clear();
            partitions = std::vector<std::vector<vertex_t>>(final_partitions.begin(), final_partitions.end());
            /*
            for(int i = 0; i < partitions.size(); i++){
                std::cout << "[ ";
                for(int j = 0; j < partitions[i].size(); j++){
                    std::cout << partitions[i][j] << " ";
                }
                std::cout << "]" << std::endl;
            }
            */
            next_index_final = 0;
            
            for(int i = 0; i < final_partitions.size(); i++){
                if(final_partitions[i].empty()) break;
                next_index_final++;
            }
            //std::cout << "start next iteration with next_index_final = "<< next_index_final <<std::endl;
            
            nodes_done = 0;
            nodes_on_level = nodes_on_next_level;
            nodes_on_next_level = 0;
            int prefixsum = 0;
            for(int i = 0; i < splitted.size(); i++){
                if(splitted[i]) prefixsum++;
            }
            //std::cout << "there were a total of " << prefixsum << " splitted partitions" << std::endl; 
            splitted_prev = std::vector<bool>(splitted.begin(), splitted.end());

            splitted = std::vector<bool>(partitions.size(), false);
            //std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
        }
    }

    for(int i = 0; i < old_clusters.size(); i++){
        std::vector<point_id_t> final_cluster{};
        for(auto c : old_clusters[i]){
            final_cluster.push_back(coord_2_id[c]);
        }
        final_clusters.push_back(final_cluster);
    }
    /*
    for(int i = 0; i < current_clusters.size(); i++){
        std::cout << "cluster " << i << "\t[ ";
        for(auto c : *current_clusters[i]->get_points()){
            std::cout << c << " ";
        }
        std::cout << "]" << std::endl;
    }
    */
    if(final_partitions.size() != final_clusters.size()) throw std::exception(/*different size*/);
    for (int i = 0; i < final_partitions.size(); i++) {
        if(final_partitions[i].size() > final_clusters[i].size()){
             std::cout << final_partitions[i].size() << " > " << final_clusters[i].size() << std::endl;
        }
    }
}

void ClusterMapBuilder::print_partitions(){
    for (int i = 0; i < final_partitions.size(); i++) {
        std::cout << "[ ";
            for(int j = 0; j < final_partitions[i].size(); j++){
                std::cout << final_partitions[i][j] << " ";
            }
        std::cout << "]";
    }
    std::cout << std::endl;
}

void ClusterMapBuilder::print_clusters(){
    for (int i = 0; i < final_clusters.size(); i++) {
        std::cout << "[ ";
            for(int j = 0; j < final_clusters[i].size(); j++){
                std::cout << final_clusters[i][j] << " ";
            }
        std::cout << "]";
    }
    std::cout << std::endl;
}

void ClusterMapBuilder::print_p_2_c_mapping(){
    if(final_partitions.size() != final_clusters.size()) throw std::exception(/*different size*/);
    for (int i = 0; i < final_partitions.size(); i++) {
        if(final_partitions[i].size() > final_clusters[i].size()){
             std::cout << final_partitions[i].size() << " > " << final_clusters[i].size() << std::endl;
             throw std::exception(/*different size*/);
        }else{
            std::cout << "[ ";
            for(int j = 0; j < final_partitions[i].size(); j++){
                std::cout << final_partitions[i][j] << " ";
            }
            std::cout << "] ----> [ ";
            for(int j = 0; j < final_clusters[i].size(); j++){
                std::cout << final_clusters[i][j] << " ";
            }
            std::cout << "];";
        }
        std::cout << std::endl;
    }
}
void ClusterMapBuilder::map_partition_2_cluster(std::vector<vertex_t> &nodes, std::vector<point_id_t> &points, UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> &e_internal, UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> &e_external){
    solver = new EvalMaxSAT(0);
    int num_points_i = points.size();
    int num_nodes_i = nodes.size();
    //generate variables for point is used
    std::vector<int> point_is_used(num_points_i, -1);
    for(int l = 0; l < num_points_i; l++){
        point_is_used[l] = solver->newVar();
    }
    //generate variables for node gets mapped to point in the partition
    std::vector<int> map_n_2_p_var(num_nodes_i * num_points_i, -1);
    for(int k = 0; k < num_nodes_i; k++){
        for(int l = 0; l < num_points_i; l++){
            map_n_2_p_var[k * num_points_i + l] = solver->newVar();
        }
    }
    //each node should be mapped to one point
    for(int j = 0; j < num_nodes_i; j++){
        std::vector<int> at_least_one = std::vector<int>(map_n_2_p_var.begin() + j * num_points_i, map_n_2_p_var.begin() + (j + 1) * num_points_i);
        solver->AtMostOne(at_least_one);
        solver->addClause(at_least_one);
    }
    //each point should be used by at_most_one node
    for(int l = 0; l < num_points_i; l++){
        std::vector<int> at_most_one(num_nodes_i, -1);
        for(int k = 0; k < num_nodes_i; k++){
            at_most_one[k] = map_n_2_p_var[k * num_points_i + l];
        }
        solver->AtMostOne(at_most_one);
    }
    //if one node is mapped to a point, the point is used
    for(int k = 0; k < num_points_i; k++){
        std::vector<int> equiv(num_nodes_i + 1, 0);
        for(int l = 0; l < num_nodes_i; l++){
            equiv[l] = map_n_2_p_var[l * num_points_i + k];
            solver->addClause({-equiv[l], point_is_used[k]});
        }
        equiv[equiv.size() - 1] = -point_is_used[k];
        solver->addClause(equiv);
    }
    //deny collinearity inside the cluster
    for(auto e : e_internal){
        for(int j = 0; j < num_points_i; j++){
            for(int k = 0; k < num_points_i; k++){
                if(j == k) continue;
                line_2d_t line_coords = std::make_pair(id_2_coord[points[j]], id_2_coord[points[k]]);
                for(int l = 0; l < num_points_i; l++){
                    if(l == j || l == k) continue;
                    Point point = m_instance.m_points.getPoint(points[l]);
                    if(!gd::isOnLine(line_coords, point)) continue;
                    int index0 = node_2_partition_id[e.first] * num_points_i + j;
                    int index1 = node_2_partition_id[e.second] * num_points_i + k;
                    solver->addClause({-map_n_2_p_var[index0], -map_n_2_p_var[index1], -point_is_used[l]});
                }
            }
        }
    }

    for(auto e : e_external){
        int internal;
        int external;
        if(mapped_nodes.contains(e.first)){
            internal = e.second;
            external = e.first;
        }else{
            internal = e.first;
            external = e.second;
        }
        for(int j = 0; j < num_points_i; j++){
            auto line_coords = std::make_pair(id_2_coord[current_mapping[external]], id_2_coord[points[j]]);
            for(auto pid : mapped_points){
                Point p = m_instance.m_points.getPoint(pid);
                if(!gd::isOnLine(line_coords, p)) continue;
                solver->addClause({-map_n_2_p_var[node_2_partition_id[internal] * num_points_i + j]});
            }
            for(int k = 0; k < num_points_i; k++){
                Point p = m_instance.m_points.getPoint(points[k]);
                if(!gd::isOnLine(line_coords, p)) continue;
                solver->addClause({-map_n_2_p_var[node_2_partition_id[internal] * num_points_i + j], -point_is_used[k]});
            }
        }
    }
    for(auto e : current_edges){
        //only looking for edges that have nothing to do with the partition
        if (!(mapped_nodes.contains(e.first) && mapped_nodes.contains(e.second))) continue; 
        auto line_coords = std::make_pair(id_2_coord[current_mapping[e.first]], id_2_coord[current_mapping[e.second]]);
        for(int j = 0; j < num_points_i; j++){
            Point p = m_instance.m_points.getPoint(points[j]);
            if(!gd::isOnLine(line_coords, p)) continue;
            //std::cout << "deny " << id_2_coord[points[j]] << " if edge " << id_2_coord[current_mapping[e.first]] <<  " --- > " << id_2_coord[current_mapping[e.second]] << " exists "<<std::endl;
            solver->addClause({-point_is_used[j]});
        }
    }

    std::vector<int> mapped_neighbors_local(num_points_i * num_points_i, 0);
    for(int j = 0; j < num_points_i; j++){
        for(int l = 0; l < num_points_i; l++){
            mapped_neighbors_local[j * num_points_i + l] = solver->newVar();
        }
    }

    for (auto edge : e_internal){
        for(int j = 0; j < num_points_i; j++){
            for(int l = 0; l < num_points_i; l++){
                if(j == l) continue;
                solver->addClause({-map_n_2_p_var[node_2_partition_id[edge.first] * num_points_i + j], -map_n_2_p_var[node_2_partition_id[edge.second] * num_points_i + l], mapped_neighbors_local[j * num_points_i + l]});
            }
        }
    }

    for(int j = 0; j < num_points_i; j++){
        for(int l = 0; l < num_points_i; l++){
            if(j == l)continue;
            for(int n = 0; n < num_points_i; n++){
                for(int o = 0; o < num_points_i; o++){
                    if(j == n || j == o || l == n || l == o || n == o) continue;
                    Point pj = m_instance.m_points.getPoint(points[j]);
                    Point pl = m_instance.m_points.getPoint(points[l]);
                    Point pn = m_instance.m_points.getPoint(points[n]);
                    Point po = m_instance.m_points.getPoint(points[o]);
                    if(!gd::intersect(pj, pl, pn, po)) continue;
                    solver->addWeightedClause({-mapped_neighbors_local[j * num_points_i + l], -mapped_neighbors_local[n * num_points_i + o]}, 1);
                }
            }
        }
    }
    std::vector<point_id_t> new_mapping_i{};
    if(solver->solve()){
        for(int k = 0; k < num_nodes_i; k++){
            for(int l = 0; l < num_points_i; l++){
                if(!solver->getValue(map_n_2_p_var[k * num_points_i + l])) continue;
                vertex_t node = nodes[k];
                point_id_t point = points[l];
                available_nodes.erase(node);
                available_points.erase(point);
                mapped_nodes.insert(node);
                mapped_points.insert(point);
                current_mapping.insert({node, point});
                current_mapping_reverse.insert({point, node});
                //std::cout << "point " << id_2_coord[point] << " gets used" << std::endl;
                }
            }
            for(auto edge : e_external){
                current_edges.insert(edge);
            }
            for(auto edge : e_internal){
                current_edges.insert(edge);
            }
            e_external.clear();
            e_internal.clear();

    }else{
        std::cout << "unsat" << std::endl;
    }
}

void ClusterMapBuilder::iterative_sat(){
    UnorderedSet<vertex_t> unmapped_nodes{}; //edges in already mapped partitions
    UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> new_edges_internal{}; //edges inside the new partition
    UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> new_edges_external{}; //edges from the new partion into mapped ones
    UnorderedSet<vertex_pair_t, PairHashFunc<vertex_t>> unmapped_edges{}; //edges from unmapped nodes

    for(auto e : m_instance.m_graph){
        std::cout << e << ", ";
    }
    std::cout << std::endl;
    
    int num_partitions = final_partitions.size();
    int num_nodes = m_instance.m_graph.getNbVertices();
    node_2_partition_id = std::vector<int>(num_nodes, -1);
    for(int i = 0; i < num_partitions; i++){
        int num_nodes_i = final_partitions[i].size();
        for(int j = 0; j < num_nodes_i; j++){
            node_2_partition_id[final_partitions[i][j]] = j;
        }
    }

    for(int i = 0; i < num_partitions; i++){
        int num_nodes_i = final_partitions[i].size();
        for(int j = 0; j < num_nodes_i; j++){
            if(j != node_2_partition_id[final_partitions[i][j]]) throw std::exception();
        }
    }

    for(int i = 0; i < num_partitions; i++){
        int num_nodes_i = final_partitions[i].size();
        int num_points_i = final_clusters[i].size();

        for(int j = 0; j < num_nodes_i; j++){
            available_nodes.insert(final_partitions[i][j]);
        }
        for(int j = 0; j < num_points_i; j++){
            available_points.insert(final_clusters[i][j]);
        }

        for(auto n : final_partitions[i]){
            auto it = m_instance.m_graph.getNeighborIterator(n);
            for (auto neighbor = it.first; neighbor != it.second; ++neighbor){
                if(!(mapped_nodes.contains(*neighbor) || available_nodes.contains(*neighbor))) continue;
                vertex_pair_t edge;
                if(n > *neighbor){
                    edge = std::make_pair(*neighbor, n);
                }else{
                    edge = std::make_pair(n,*neighbor);
                }
                if(mapped_nodes.contains(*neighbor)) new_edges_external.insert(edge);
                if(available_nodes.contains(*neighbor)) new_edges_internal.insert(edge);
            }
        }
        map_partition_2_cluster(final_partitions[i], final_clusters[i], new_edges_internal, new_edges_external);

        for(auto n : mapped_nodes){//retarded...
            auto it = m_instance.m_graph.getNeighborIterator(n);
            for (auto neighbor = it.first; neighbor != it.second; ++neighbor){
                if(!mapped_nodes.contains(*neighbor)) continue;
                current_edges.insert(std::make_pair(n, *neighbor));
                current_edges.insert(std::make_pair(*neighbor, n));
            }
        }

    }
    if(unmapped_nodes.empty()){
        std::cout << current_mapping.size() << std::endl;
        for(auto &[key, value] : current_mapping){
            std::cout << key << " ---> " << value << std::endl;
            m_assignment.assign(key, value);
        }
        if(!is_valid()) return;
        Verifier verifier{m_instance, m_assignment};
        size_t num_crossings = 0;
        bool valid = verifier.verify(num_crossings);
        std::cout << "Verification result: " << (valid? "Valid" : "Invalid") << std::endl;
        if (valid) std::cout << "Number of crossings " << num_crossings << std::endl;
        std::cout << "Number of edges " << m_instance.m_graph.getNbEdges() << std::endl;
    }else{
        std::cout << "there are unmapped nodes " << unmapped_nodes.size() << std::endl;
        std::cout << "there are unused points " << available_points.size() << std::endl;

    }
}
bool ClusterMapBuilder::is_valid(){
    bool failed = false;
    for(auto edge : m_instance.m_graph){
        if(edge.first > edge.second) continue;
        line_2d_t line_coords = std::make_pair(id_2_coord[m_assignment.getAssigned(edge.first)], id_2_coord[m_assignment.getAssigned(edge.second)]);
        auto point_range = m_instance.m_points.getPointIterator();
        for (auto it = point_range.first; it != point_range.second; ++it){
            point_id_t rid = it->id;
            if(!m_assignment.isPointUsed(rid)) continue;
            Point r = m_instance.m_points.getPoint(rid);
            if(gd::isOnLine(line_coords, r)){
                failed = true;
                //std::cout << edge << std::endl;
                std::cout << id_2_coord[m_assignment.getAssigned(edge.first)] << " --> " << id_2_coord[m_assignment.getAssigned(edge.second)] << std::endl;
                std::cout << id_2_coord[rid] << " " << rid << std::endl;
            }
        }
    }
    return !failed;
}