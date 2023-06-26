#include "gd_types.hpp"
#include "placement/greedy_placement.hpp"
#include "verification/verifier.hpp"
#include <algorithm>
#include <iostream>
#include <CLI/CLI.hpp>

#include <common/instance.hpp>
#include <io/json_handling.hpp>
#include <memory>

#include <io/placement_visualizer.hpp>

#include <EvalMaxSAT.h>
#include <iterator>
#include <iostream>
#include <placement/sat_placement.cpp>

#include <placement/divide/graph_partitioning.hpp>
#include <placement/divide/point_clustering.hpp>
#include <utility>
#include <vector>


#include <placement/divide/tree_clustering.hpp>


using namespace gd;

int main(int argc, const char** argv)
{
  CLI::App app("Graph drawing algorithms");
  // add version output
  app.set_version_flag("--version", "3.14159265");
  std::string file;
  app.add_option("-f,--file,file", file, "File name")->required();

  std::string visualization_file_prefix = "";
  app.add_option("--visualize", visualization_file_prefix, "Visualize result into folder");

  size_t improvement_iters = 100;
  app.add_option("--improvement_iters", improvement_iters, "Number of iterations to improve afterwards");


  CLI11_PARSE(app, argc, argv);
  instance_t instance = parseInstanceFromFile(file);

  
  HierarchicalGraphBuilder hierarchyBuilder{instance};
  HierarchicalGraph hierarchy = hierarchyBuilder.partition();

  
  std::cout << hierarchy.getNbPartitions() << std::endl;

  partitioning::Partition root_partition = hierarchy.get_root_partition();
  partitioning::Interval inter = root_partition.interval;
  Queue<size_t> queue{};
  queue.push(root_partition.left_child);
  queue.push(root_partition.right_child);
  while (!queue.empty()) {
    size_t index = queue.front();
    partitioning::Partition part = hierarchy.get_partition(index);
    if(part.has_children()){
      queue.push(part.left_child);
      queue.push(part.right_child);
    }else{
      Vector<vertex_t> vertecies = hierarchy.get_partition_vertecies(part.interval.start, part.interval.end);
      for(int i = 0; i < vertecies.size(); i++){
        std::cout << vertecies[i] << " ";
      }
      std::cout << std::endl;
    }
    queue.pop();
  }
  /*
  clustering::ClusterTree clusterTree(4, instance);
  
  PointClustering points{instance};
  points.cluster();
  std::vector<std::vector<int>> clusters(points.getSize());
  for(int j = 0; j < points.getSize(); j++){
    clusters[j] = std::vector<int>{};
  }

  for(int k= 0; k < instance.m_points.getNumPoints(); k++){
    int c = points.getCluster(k);
    clusters[c].push_back(k);
  }
  for(int l = 0; l < clusters.size(); l++){
    for(int m = 0; m < clusters[l].size(); m++){
      std::cout << clusters[l][m] << " ";
    }
    std::cout << std::endl;
  }
  */
  /*
  std::vector<coordinate_2d_t> point_list(instance.m_points.getNumPoints());
  for(int i = 0; i < instance.m_points.getNumPoints(); i++){
    point_list[i] = instance.m_points.getPoint(i).getCoordPair();
  }*/

  //sort x1 < x2
  //std::sort(point_list.begin(), point_list.end());
  /*
  for(int i = 0; i < point_list.size(); i++){
    std::cout << point_list[i].first << ", " << point_list[i].second << " ";
  }
  std::cout << std::endl;
  */
  //sort y1 < y2
  /*
  std::sort(point_list.begin(), point_list.end(), [](auto &left, auto &right){
    return !(left.second > right.second or (left.second == right.second and left.first > right.first));
  });
  */
  /*
  int num_points = point_list.size();
  bool divisible_by_two = num_points % 2 == 0;
  int half_range = 0;
  if(divisible_by_two){
    half_range = num_points / 2;
  }else{
    half_range = (num_points - 1) / 2;
  }
  std::vector<coordinate_2d_t> left_half(point_list.begin(), point_list.begin() + half_range);
  for(int i = 0; i < left_half.size(); i++){
    std::cout << left_half[i].first << ", " << left_half[i].second << ";   ";
  }
  std::cout << std::endl;
  std::cout << "++++++++++++++++" << std::endl;
  std::vector<coordinate_2d_t> right_half(point_list.begin() + half_range, point_list.begin() + num_points);
  for(int i = 0; i < right_half.size(); i++){
    std::cout << right_half[i].first << ", " << right_half[i].second << ";   ";
  }
  std::cout << std::endl;
  */
  /*
  for(int i = 0; i < point_list.size(); i++){
    std::cout << point_list[i].first << ", " << point_list[i].second << " ";
  }
  std::cout << std::endl;
  */
  std::cout << "Done parsing file." << std::endl;


  //GreedyPlacement placement{instance, order};
  //const auto& assignment = placement.findPlacement();
  //placement.improve(improvement_iters);

  //std::vector<std::vector<vertex_t>> partitions = {{0, 1, 3, 4}, {2, 5}, {6, 7}, {8}};
  //std::vector<std::vector<point_id_t>> clusters = {{0, 1, 2, 3, 7, 8, 9, 10, 14, 15, 16, 17}, {21, 22, 23, 24, 28, 29, 30, 31}, {4, 5, 6, 11, 12, 13, 18,19, 20}, {25, 26, 27, 32, 33, 34}};
  //std::vector<int> n2p{0, 0, 1, 0, 0, 1, 2, 2, 3};
  //std::vector<int> po2c{0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 1, 1, 1, 1, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3};
  //std::vector<int> p2c{0, 1, 2, 3};
  //std::vector<int> c2p{0, 1, 2, 3};

  /*
  int width = 64;
  int height = 64;
  int num_points = width * height;
  int nodes_x = 32;
  int nodes_y = 32;
  int num_nodes = nodes_x * nodes_y;
  int clusters_x = 16;
  int clusters_y = 16;
  int num_clusters = clusters_x * clusters_y;
  int points_per_cluster_x = 4;
  int points_per_cluster_y = 4;
  int points_per_cluster = points_per_cluster_x * points_per_cluster_y;
  int partitions_x = 16;
  int partitions_y = 16;
  int num_partitions = partitions_x * partitions_y;
  int points_per_partition = 4;
  std::vector<std::vector<vertex_t>> partitions(num_partitions, std::vector<vertex_t>(points_per_partition, 0));
  std::vector<int> n2p(num_nodes, 0);
  for(int i = 0; i < partitions_x; i++){
    for(int j = 0; j < partitions_y; j++){
      partitions[j * partitions_x + i][0] = i * 2 + j * 2 * nodes_x;
      partitions[j * partitions_x + i][1] = i * 2 + 1 + j * 2 * nodes_x;
      partitions[j * partitions_x + i][2] = i * 2 + (j * 2 + 1) * nodes_x;
      partitions[j * partitions_x + i][3] = i * 2 + 1 + (j * 2 + 1) * nodes_x;
      n2p[i * 2 + j * 2 * nodes_x] = j * partitions_x + i;
      n2p[i * 2 + 1 + j * 2 * nodes_x] = j * partitions_x + i;
      n2p[i * 2 + (j * 2 + 1) * nodes_x] = j * partitions_x + i;
      n2p[i * 2 + 1 + (j * 2 + 1) * nodes_x] = j * partitions_x + i;
    }
  }
  //std::cout << partitions << std::endl;
  //std::cout << n2p << std::endl;
  std::vector<std::vector<vertex_t>> clusters(num_clusters, std::vector<vertex_t>(points_per_cluster, 0));
  std::vector<int> po2c(num_points, 0);
  for(int i = 0; i < clusters_x; i++){
    for(int j = 0; j < clusters_y; j++){
        for(int k = 0; k < points_per_cluster_x; k++){
          for(int l = 0; l < points_per_cluster_y; l++){
            clusters[j * clusters_x + i][l * points_per_cluster_x + k] = k + l * width + i * points_per_cluster_x + j * points_per_cluster_y * width;
            po2c[k + l * width + i * points_per_cluster_x + j * points_per_cluster_y * width] = j * clusters_x + i;
          }
        }
    }
  }
  //std::cout << clusters << std::endl;
  //std::cout << po2c << std::endl;

  std::vector<int> p2c(num_partitions, 0);
  for(int i = 0; i < num_partitions; i++){
    p2c[i] = i;
  }
  std::vector<int> c2p(num_clusters, 0);
  for(int i = 0; i < num_clusters; i++){
    c2p[i] = i;
  }
  int max_iterations = 500;
  SATPlacement sat = SATPlacement(instance,partitions, clusters, n2p,po2c, p2c, c2p, max_iterations);
  */

  /*
  std::unique_ptr<PlacementVisualizer> visualizer;
  if (!visualization_file_prefix.empty())
  {
    visualizer = std::make_unique<PlacementVisualizer>(instance, visualization_file_prefix);
    /*visualizer->setHierarchy(hierarchy);
    visualizer->setClustering(points);
    visualizer->drawClustering();*/

  /*
  Verifier verifier{instance, assignment};
  size_t num_crossings = 0;
  bool valid = verifier.verify(num_crossings);
  std::cout << "Verification result: " << (valid? "Valid" : "Invalid") << std::endl;
  if (valid) std::cout << "Number of crossings " << num_crossings << std::endl;
  */
  return 0;
}
