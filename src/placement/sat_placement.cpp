#include "sat_placement.hpp"
#include "EvalMaxSAT.h"
#include "common/instance.hpp"
#include "gd_types.hpp"
#include "glucose/core/Solver.h"
#include "verification/verification_utils.hpp"
#include <utility>
#include <verification/line_crossings.hpp>
#include <iostream>
#include <vector>
#include <verification/trivial_collinear.cpp>

using namespace gd;

SATPlacement::SATPlacement(const Instance& instance, std::vector<std::vector<vertex_t>>& partitions, 
      std::vector<std::vector<point_id_t>>& clusters, std::vector<int>& node_2_partition, std::vector<int>& point_2_cluster,
      std::vector<int>& partition_2_cluster, std::vector<int>& cluster_2_partition, const int& max_iterations)
  : m_instance(instance),
    m_assignment(instance),
    m_max_iterations(max_iterations),
    m_num_points(instance.m_points.getNumPoints()),
    m_num_nodes(instance.m_graph.getNbVertices()),
    m_partitions(partitions),
    m_clusters(clusters),
    m_node_2_partition(node_2_partition),
    m_point_2_cluster(point_2_cluster),
    m_partition_2_cluster(partition_2_cluster),
    m_cluster_2_partition(cluster_2_partition),
    solver(new EvalMaxSAT(0))
    {
        build_local_clauses();
        iterative_solving();
    }

void SATPlacement::reserve_variables(){
    point_is_used = std::vector<int>(m_num_points, 0);
    for (int i = 0; i < m_num_points; i++) {
        point_is_used[i] = SATPlacement::solver->newVar();
    }

    map_node_to_point = std::vector<int>(m_num_points * m_num_nodes, 0);
    for (int i = 0; i < m_num_nodes; i++) {
        for(int j = 0; j < m_num_points; j++){
            map_node_to_point[i * m_num_points + j] = SATPlacement::solver->newVar();
        }
    }
}
void SATPlacement::reserve_variables_mapped_neighbors(){
    mapped_neighbors = std::vector<int>(m_num_points * m_num_points, 0);
    for (int i = 0; i < m_num_points; i++) {
        for(int j = 0; j < m_num_points; j++){
            mapped_neighbors[i * m_num_points + j] = SATPlacement::solver->newVar();
        }
    }

}

void SATPlacement::print_point_is_used(){
    for (int i = 0; i < m_num_points; i++) {
        std::cout << point_is_used[i] << " ";
    }
    std::cout << std::endl;
}

void SATPlacement::print_map_node_to_point(){
    for (int i = 0; i < m_num_nodes; i++) {
        for(int j = 0; j < m_num_points; j++){
            std::cout << map_node_to_point[i * m_num_points + j] << " ";
        }
        std::cout << std::endl;
    }
}

void SATPlacement::print_mapped_neighbors(){
    for (int i = 0; i < m_num_points; i++) {
        for(int j = 0; j < m_num_points; j++){
            std::cout << mapped_neighbors[i * m_num_points + j] << " ";
        }
        std::cout << std::endl;
    }
}

void SATPlacement::build_local_clauses(){
    std::cout << "reserving variables" << std::endl;
    reserve_variables();
    //print_point_is_used();
    //print_map_node_to_point();
    //print_mapped_neighbors();
    std::cout << "build mapped to one clauses" << std::endl;
    build_clauses_mapped_to_one();
    std::cout << "atmost one per point clauses" << std::endl;
    //build_clauses_at_most_one_node_per_point();
    build_clauses_at_most_one_node_per_point_local();
    std::cout << "build one mapped equiv used clauses" << std::endl;
    build_clauses_mapped_equiv_used();
    std::cout << "build no collinearity clauses" << std::endl;
    build_clauses_no_collinearity();

    //reserve_variables_mapped_neighbors();
    //build_clauses_mapped_neighbors();
    //build_clauses_crossing();
    std::cout << "build local crossing clauses" << std::endl;
    build_clauses_local_crossing();
}

void SATPlacement::build_clauses_mapped_to_one(){
    for(int i = 0; i < m_num_nodes; i++){
        int cluster = m_partition_2_cluster[m_node_2_partition[i]];
        int n_points = m_clusters[cluster].size();
        std::vector<int> at_least_one(n_points, 0);
        for(int j = 0; j < n_points; j++){
            int point = 
            at_least_one[j] = map_node_to_point[i * m_num_points + m_clusters[cluster][j]];
        }
        solver->addClause(at_least_one);
        //solver->addClause({*solver->newCard(at_least_one) <= 1});
        solver->AtMostOne(at_least_one);
    }
}

void SATPlacement::build_clauses_at_most_one_node_per_point(){
    for(int j = 0; j < m_num_points; j++){
        std::vector<int> at_most_one(m_num_nodes);
        for(int i = 0; i < m_num_nodes; i++){
            at_most_one[i] = map_node_to_point[i * m_num_points + j];
        }
        solver->AtMostOne(at_most_one);
        //solver->addClause({*solver->newCard(at_most_one) <= 1});
    }
}
void SATPlacement::build_clauses_at_most_one_node_per_point_local(){
    for(int j = 0; j < m_num_points; j++){
        int part = m_cluster_2_partition[m_point_2_cluster[j]];
        int num_nodes = m_partitions[part].size();
        std::vector<int> at_most_one(num_nodes);
        for(int i = 0; i < num_nodes; i++){
            at_most_one[i] = map_node_to_point[m_partitions[part][i] * m_num_points + j];
        }
        solver->AtMostOne(at_most_one);
    }
}

void SATPlacement::build_clauses_mapped_equiv_used(){
    for (int j = 0; j < m_num_points; j++) {
        int partition = m_cluster_2_partition[m_point_2_cluster[j]];
        int n_nodes = m_partitions[partition].size();
        std::vector<int> equiv(n_nodes + 1, 0);
        for(int i = 0; i < n_nodes; i++){
            equiv[i] = map_node_to_point[m_partitions[partition][i] * m_num_points + j];
            solver->addClause({-equiv[i], point_is_used[j]});
        }
        equiv[equiv.size() - 1] = -point_is_used[j];
        solver->addClause(equiv);
    }
}

void SATPlacement::build_clauses_no_collinearity(){
    for (auto edge : m_instance.m_graph){
        if(m_node_2_partition[edge.first] == m_node_2_partition[edge.second]){
            int cluster = m_partition_2_cluster[m_node_2_partition[edge.first]];
            int n_points = m_clusters[cluster].size();
            for(int j = 0; j < n_points; j++){
                for(int l = 0; l < n_points; l++){
                    if(j != l){
                        Point p  = m_instance.m_points.getPoint(m_clusters[cluster][j]);
                        Point q  = m_instance.m_points.getPoint(m_clusters[cluster][l]);
                        line_2d_t line_coords = std::make_pair(p.getCoordPair(), q.getCoordPair());
                        for (int t = 0; t < n_points; t++) {
                            if(t != j && t != l){
                                Point r = m_instance.m_points.getPoint(m_clusters[cluster][t]);
                                if (gd::isOnLine(line_coords, r)) {
                                    int mij = map_node_to_point[edge.first * m_num_points + m_clusters[cluster][j]];
                                    int mkl = map_node_to_point[edge.second * m_num_points + m_clusters[cluster][l]];
                                    solver->addClause({-mij, -mkl, -point_is_used[m_clusters[cluster][t]]});
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void SATPlacement::build_clauses_local_crossing(){
    for(int c = 0; c < m_clusters.size(); c++){
        int n_points = m_clusters[c].size();

        std::vector<int> mapped_neighbors_local(n_points * n_points, 0);

        for(int j = 0; j < n_points; j++){
            for(int l = 0; l < n_points; l++){
                mapped_neighbors_local[j * n_points + l] = solver->newVar();
            }
        }

        for (auto edge : m_instance.m_graph){
            if(m_node_2_partition[edge.first] == m_node_2_partition[edge.second] && m_partition_2_cluster[m_node_2_partition[edge.first]] == c){
                for(int j = 0; j < n_points; j++){
                    for(int l = 0; l < n_points; l++){
                        if(j != l){
                            int mij = map_node_to_point[edge.first * m_num_points + m_clusters[c][j]];
                            int mkl = map_node_to_point[edge.second * m_num_points + m_clusters[c][l]];
                            solver->addClause({-mij, -mkl, mapped_neighbors_local[j * n_points + l]});
                        }
                    }
                }
            }
        }
        for(int j = 0; j < n_points; j++){
            for(int l = 0; l < n_points; l++){
                if(j != l){
                    for(int n = 0; n < n_points; n++){
                        for(int o = 0; o < n_points; o++){
                            if(j != n && j != o && l != n && l != o && n != o){
                                Point pj = m_instance.m_points.getPoint(m_clusters[c][j]);
                                Point pl = m_instance.m_points.getPoint(m_clusters[c][l]);
                                Point pn = m_instance.m_points.getPoint(m_clusters[c][n]);
                                Point po = m_instance.m_points.getPoint(m_clusters[c][o]);
                                if(gd::intersect(pj, pl, pn, po)){
                                    solver->addWeightedClause({-mapped_neighbors_local[j * n_points + l], -mapped_neighbors_local[n * n_points + o]}, 1);
                                    //solver->addClause({-mapped_neighbors_local[j * n_points + l], -mapped_neighbors_local[n * n_points + o]});
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
void SATPlacement::build_clauses_mapped_neighbors(){
    for (auto edge : m_instance.m_graph){
        if(m_node_2_partition[edge.first] == m_node_2_partition[edge.second]){
            int cluster = m_partition_2_cluster[m_node_2_partition[edge.first]];
            int n_points = m_clusters[cluster].size();
            for(int j = 0; j < n_points; j++){
                for(int l = 0; l < n_points; l++){
                    if(j != l){
                        int mij = map_node_to_point[edge.first * m_num_points + m_clusters[cluster][j]];
                        int mkl = map_node_to_point[edge.second * m_num_points + m_clusters[cluster][l]];
                        int mnjl = mapped_neighbors[j * m_num_points + l];
                        solver->addClause({-mij, -mkl, mnjl});
                    }
                }
            }
        }
    }
}

void SATPlacement::build_clauses_crossing(){
    for(int c = 0; c < m_clusters.size(); c++){
        int n_points = m_clusters[c].size();
        for(int j = 0; j < n_points; j++){
            for(int l = 0; l < n_points; l++){
                if(j != l){
                    for(int n = 0; n < n_points; n++){
                        for(int o = 0; o < n_points; o++){
                            if(j != n && j != o && l != n && l != o && n != o){
                                Point pj = m_instance.m_points.getPoint(m_clusters[c][j]);
                                Point pl = m_instance.m_points.getPoint(m_clusters[c][l]);
                                Point pn = m_instance.m_points.getPoint(m_clusters[c][n]);
                                Point po = m_instance.m_points.getPoint(m_clusters[c][o]);
                                if(gd::intersect(pj, pl, pn, po)){
                                    solver->addWeightedClause({-mapped_neighbors[j * m_num_points +l], -mapped_neighbors[n * m_num_points + o]}, 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool SATPlacement::solve(){
    return solver->solve();
}

void SATPlacement::assign_all(){
    for(int i = 0; i < m_num_nodes; i++){
        for(int j = 0; j < m_num_points; j++){
            if(solver->getValue(map_node_to_point[i * m_num_points + j])){
                m_assignment.assign(i, j);
            }
        }
    }
}

void SATPlacement::unassign_all(){
    for(int i = 0; i < m_num_nodes; i++){
        m_assignment.unassign(i);
    }

}

void SATPlacement::print_mapping(){
    for(int i = 0; i < m_num_nodes; i++){
        for(int j = 0; j < m_num_points; j++){
            if(solver->getValue(map_node_to_point[i * m_num_points + j])){
                std::cout << "node " << i << " is mapped to point " << j << std::endl;
            }
        }
    }
}

void SATPlacement::copy_assignment(std::vector<int>& current_best_assignment){
    for(int i = 0; i < m_num_nodes; i++){
        for(int j = 0; j < m_num_points; j++){
            if(solver->getValue(map_node_to_point[i * m_num_points + j])){
                current_best_assignment[i] = j;
                //std::cout << "node " << i << " is mapped to point " << j << std::endl;
            }
        }
    }

}

void SATPlacement::count_crossings(){
    crossings = gd::countCrossings(m_instance, m_assignment);
    std::cout << "crossings " << crossings << std::endl;
}

bool SATPlacement::is_valid(){
    bool failed = false;
    for(auto edge : m_instance.m_graph){
        if(edge.first < edge.second){
            Point p  = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.first));
            Point q  = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.second));
            line_2d_t line_coords = std::make_pair(p.getCoordPair(), q.getCoordPair());

            auto point_range = m_instance.m_points.getPointIterator();
            for (auto it = point_range.first; it != point_range.second; ++it){
                point_id_t rid = it->id;
                if(m_assignment.isPointUsed(rid) && rid != edge.first && rid != edge.second){
                    Point r = m_instance.m_points.getPoint(rid);
                    if(gd::isOnLine(line_coords, r)){
                        //std::cout << "the point " << rid << " is used by the node " << m_assignment.getAssignedVertex(rid) <<" and the edge from node " << edge.first << " to node " << edge.second << " exists" << std::endl;
                        failed = true;
                    }
                }
            }
        }
    }
    return !failed;
}

void SATPlacement::print_used_points(){
    for(int j = 0; j < m_num_points; j++){
        if(solver->getValue(point_is_used[j])){
            std::cout << "point " << j << " is used"<< std::endl;
        }else{
            std::cout << "point " << j << " is not used"<< std::endl;
        }
    }
}

void SATPlacement::add_collinearity_clauses(){
    for(int j = 0; j < m_num_points - 1; j++){
        for(int l = j + 1; l < m_num_points; l++){
            if( m_assignment.isPointUsed(j) && m_assignment.isPointUsed(l)){
                for(int n = 0; n < m_num_points; n++){
                    if(j != n && l != n && m_assignment.isPointUsed(n)){
                        Point p = m_instance.m_points.getPoint(j);
                        Point q = m_instance.m_points.getPoint(l);
                        Point r = m_instance.m_points.getPoint(n);
                        line_2d_t line_coords = std::make_pair(p.getCoordPair(), q.getCoordPair());
                        if(isOnLine(line_coords, r)){
                            int mij = map_node_to_point[m_assignment.getAssignedVertex(j) * m_num_points + j];
                            int mkl = map_node_to_point[m_assignment.getAssignedVertex(l) * m_num_points + l];
                            //std::cout << m_assignment.getAssignedVertex(j) * m_num_points + j << "\t" << m_assignment.getAssignedVertex(l) * m_num_points + l << "\t" << point_is_used[n] << std::endl;
                            solver->addClause({-mij, -mkl, -point_is_used[n]});
                        }
                    }
                }
            }
        }
    }
}

void SATPlacement::add_collinearity_clauses1(){
    bool failed = false;
    for(auto edge : m_instance.m_graph){
        if(edge.first < edge.second){
            Point p  = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.first));
            Point q  = m_instance.m_points.getPoint(m_assignment.getAssigned(edge.second));
            line_2d_t line_coords = std::make_pair(p.getCoordPair(), q.getCoordPair());
            auto point_range = m_instance.m_points.getPointIterator();
            for (auto it = point_range.first; it != point_range.second; ++it){
                point_id_t rid = it->id;
                if(m_assignment.isPointUsed(rid) && rid != edge.first && rid != edge.second){
                    Point r = m_instance.m_points.getPoint(rid);
                    if(gd::isOnLine(line_coords, r)){
                        //std::cout << "the point " << rid << " is used by the node " << m_assignment.getAssignedVertex(rid) <<" and the edge from node " << edge.first << " to node " << edge.second << " exists" << std::endl;
                        failed = true;
                        int mij = map_node_to_point[m_assignment.getAssignedVertex(p.id) * m_num_points + p.id];
                        int mkl = map_node_to_point[m_assignment.getAssignedVertex(q.id) * m_num_points + q.id];
                        solver->addClause({-mij, -mkl, -point_is_used[rid]});
                    }
                }
            }
        }
    }
}
void SATPlacement::add_crossing_clauses(){
    for(auto edge1 : m_instance.m_graph){
        for (auto edge2 :  m_instance.m_graph) {
            if(edge1.first != edge2.first && edge1.first != edge2.second && edge1.second != edge2.first && edge1.second != edge2.second){
                int j = m_assignment.getAssigned(edge1.first);
                int l = m_assignment.getAssigned(edge1.second);
                int n = m_assignment.getAssigned(edge2.first);
                int p = m_assignment.getAssigned(edge2.second);
                Point p1 = m_instance.m_points.getPoint(j);
                Point q1 = m_instance.m_points.getPoint(l);
                Point p2 = m_instance.m_points.getPoint(n);
                Point q2 = m_instance.m_points.getPoint(p);
                int mij = map_node_to_point[edge1.first * m_num_points + j];
                int mkl = map_node_to_point[edge1.second * m_num_points + l];
                int mmn = map_node_to_point[edge2.first * m_num_points + n];
                int mop = map_node_to_point[edge2.second * m_num_points + p];
                if(gd::intersect(p1, q1, p2, q2)){
                    //solver->addWeightedClause({-mij, -mkl, -mmn, -mop}, 1);
                    solver->addClause({-mij, -mkl, -mmn, -mop});
                }

            }
        }
    }
}

vertex_pair_t SATPlacement::get_edge_with_most_crossings(){
    gd::UnorderedMap<vertex_pair_t, int, PairHashFunc<vertex_t>> edgeMap{};
    for(auto edge : m_instance.m_graph){
        if(edge.first < edge.second){
            edgeMap.insert({edge, 0});
        }
    }

    for(auto edge1 : m_instance.m_graph){
        if(edge1.first > edge1.second) continue;
        for (auto edge2 :  m_instance.m_graph) {
            if(edge2.first > edge2.second) continue;
            if(edge1.first != edge2.first && edge1.first != edge2.second && edge1.second != edge2.first && edge1.second != edge2.second){
                int j = m_assignment.getAssigned(edge1.first);
                int l = m_assignment.getAssigned(edge1.second);
                int n = m_assignment.getAssigned(edge2.first);
                int p = m_assignment.getAssigned(edge2.second);
                Point p1 = m_instance.m_points.getPoint(j);
                Point q1 = m_instance.m_points.getPoint(l);
                Point p2 = m_instance.m_points.getPoint(n);
                Point q2 = m_instance.m_points.getPoint(p);
                if(gd::intersect(p1, q1, p2, q2)){
                    edgeMap[edge1] ++;
                    edgeMap[edge2] ++;
                }
            }
        }
    }
    vertex_pair_t edge_with_most_crossings;
    int num_of_crossings = 0;
    for(const auto& [key, value] : edgeMap){
        if(value > num_of_crossings){
            num_of_crossings = value;
            edge_with_most_crossings = key;
        }
    }
    int i = edge_with_most_crossings.first;
    int k = edge_with_most_crossings.second;
    int j = m_assignment.getAssigned(i);
    int l = m_assignment.getAssigned(k);
    int mij = map_node_to_point[i * m_num_points + j];
    int mkl = map_node_to_point[k * m_num_points + l];
    solver->addClause({-mij, -mkl});
    std::cout << "edge " << edge_with_most_crossings.first << ", " << edge_with_most_crossings.second << " has " << edgeMap[edge_with_most_crossings] << " crossings" << std::endl;
    return edge_with_most_crossings;
}

void SATPlacement::iterative_solving(){
    int current_iteration = 0;
    int min_crossings = 100000000;
    std::vector<int> current_best_assignment(m_num_nodes, -1);
    std::vector<int> assumptions(m_num_nodes - 2, -1);
    bool solve_with_assumptions = false;
    int pre_iteration = 0;
    while (!solve_with_assumptions) {
        std::cout << "--------------------------" << std::endl;
        std::cout << "---pre_iteration " << pre_iteration << "---" << std::endl;
        if(solve()){
            assign_all();
            if(is_valid()){
                std::cout << "valid" << std::endl;
                count_crossings();
                if(min_crossings > crossings){
                    min_crossings = crossings;
                    copy_assignment(current_best_assignment);
                }
                vertex_pair_t edge_with_most_crossings = get_edge_with_most_crossings();
                int last_index = 0;
                for(int i = 0; i < m_num_nodes; i++){
                    if(i == edge_with_most_crossings.first || i == edge_with_most_crossings.second) continue;
                    assumptions[last_index] = map_node_to_point[i * m_num_points + m_assignment.getAssigned(i)];
                    last_index++;
                }
                std::cout << "first valid assignment is found" << std::endl;
                solve_with_assumptions = true;
            }else{
                std::cout << "not valid" << std::endl;
                add_collinearity_clauses1();
                std::cout << "add collinearity clauses" << std::endl;
            }
            unassign_all();
        }else{
            std::cout << "unsat" << std::endl;
            current_iteration = m_max_iterations;
        }
        std::cout << "--------------------------" << std::endl;
        pre_iteration++;
    }
    while (crossings > 0 && current_iteration < m_max_iterations) {
        std::cout << "--------------------------" << std::endl;
        //if(solver->solve(assumptions)){
        if(solve()){
            std::cout << "---iteration " << current_iteration << "---" << std::endl;
            assign_all();
            if(is_valid()){
                std::cout << "valid" << std::endl;
                count_crossings();
                if(min_crossings > crossings){
                    min_crossings = crossings;
                    copy_assignment(current_best_assignment);
                }
                std::cout << "cost: " << solver->getCost() << std::endl;
                std::cout << "min cr: " << min_crossings << std::endl;
                vertex_pair_t edge_with_most_crossings = get_edge_with_most_crossings();
                int last_index = 0;
                for(int i = 0; i < m_num_nodes; i++){
                    if(i == edge_with_most_crossings.first || i == edge_with_most_crossings.second) continue;
                    assumptions[last_index] = map_node_to_point[i * m_num_points + m_assignment.getAssigned(i)];
                    last_index++;
                }
            }else{
                std::cout << "not valid" << std::endl;
                add_collinearity_clauses1();
                std::cout << "add collinearity clauses" << std::endl;
            }
            unassign_all();
            std::cout << "--------------------------" << std::endl;
        }else{
            std::cout << "unsat" << std::endl;
            current_iteration = m_max_iterations;
        }
        current_iteration++;
    }
    for(int i = 0; i < current_best_assignment.size(); i++){
        std::cout << "map node " << i << " to point " << current_best_assignment[i] << std::endl;
    }
}

