#include <cmath>
#include <placement/divide/tree_clustering.hpp>
#include "common/instance.hpp"
#include "gd_types.hpp"
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>
#include <iostream>
#include <math.h>

using namespace gd;
using namespace clustering;

void Cluster::sort_x_direction(){
    std::sort(m_points.begin(), m_points.end());
}
void Cluster::sort_y_direction(){
    std::sort(m_points.begin(), m_points.end(), [](auto &left, auto &right){
        return !(left.second > right.second or (left.second == right.second and left.first > right.first));});
}
void Cluster::build_children(bool split_x){
    print_points();
    m_split_x = split_x;
    if(m_split_x){
        sort_y_direction();
    }else{
        sort_x_direction();
    }
    bool divisible_by_two = m_size % 2 == 0;
    int half_range = 0;
    if(divisible_by_two){
        half_range = m_size / 2;
    }else{
        half_range = (m_size - 1) / 2;
    }
    std::vector<coordinate_2d_t> left_half(m_points.begin(), m_points.begin() + half_range);
    Cluster left_clr = Cluster(left_half);
    std::vector<coordinate_2d_t> right_half(m_points.begin() + half_range, m_points.begin() + m_size);
    Cluster right_clr = Cluster(left_half);
    left_child = &left_clr;
    right_child = &right_clr;
    hasChildren = true;
    std::cout << "children are build" << std::endl;
}
Cluster* Cluster::get_left_child(){
    return left_child;
}
Cluster* Cluster::get_right_child(){
    return right_child;
}
void Cluster::print_points(){
    for(int i = 0; i < m_size; i++){
        std:: cout << "(" << m_points[i].first << ", " << m_points[i].second << "); ";
    }
    std::cout << std::endl;
}
Cluster& ClusterTree::initialize_root_cluster(){
    std::vector<coordinate_2d_t> point_list(m_instance.m_points.getNumPoints());
    for(int i = 0; i < m_instance.m_points.getNumPoints(); i++){
        point_list[i] = m_instance.m_points.getPoint(i).getCoordPair();
    }
    return *(new Cluster(point_list));
}

void ClusterTree::print_root_cluster(){
    root->print_points();
}

void ClusterTree::build_clusters(){
    std::cout << "building clusters" << std::endl;
    std::queue<Cluster*> queue{};
    queue.push(root);
    int level = 0;
    while (level < m_depth) {
        int clusters_on_level = std::exp2(level);
        bool split_x = (level + 1) % 2 == 0;
        for(int i = 0; i < clusters_on_level; i++){
            if(!queue.empty()){
                Cluster* c = queue.front();
                std:: cout << c->size() << std::endl;
                //c->build_children(split_x);
                //Cluster* left = c->get_left_child();
                //std:: cout << (c->get_left_child())->size() << std::endl;
                //queue.push(left);
                //queue.push(c->get_right_child());
                queue.pop();
            }
        }
        level++;
    }
}

void ClusterTree::print_leave_clusters(){
    std::cout << "print clusters" << std::endl;

}