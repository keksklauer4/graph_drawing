#include "gd_types.hpp"
#include "placement/greedy_placement.hpp"
#include "verification/verifier.hpp"
#include <iostream>
#include <CLI/CLI.hpp>

#include <common/instance.hpp>
#include <io/json_handling.hpp>
#include <memory>

#include <io/placement_visualizer.hpp>

#include <EvalMaxSAT.h>
#include <iterator>
#include <ostream>
#include <vector>
#include <placement/sat_placement.cpp>

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

  std::cout << "Done parsing file." << std::endl;

  std::vector<std::vector<vertex_t>> partitions = {{0, 1, 3, 4}, {2, 5}, {6, 7}, {8}};
  std::vector<std::vector<point_id_t>> clusters = {{0, 1, 2, 3, 7, 8, 9, 10, 14, 15, 16, 17}, {21, 22, 23, 24, 28, 29, 30, 31}, {4, 5, 6, 11, 12, 13, 18,19, 20}, {25, 26, 27, 32, 33, 34}};
  std::vector<int> n2p{0, 0, 1, 0, 0, 1, 2, 2, 3};
  std::vector<int> po2c{0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 1, 1, 1, 1, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3};
  std::vector<int> p2c{0, 1, 2, 3};
  std::vector<int> c2p{0, 1, 2, 3};
  int max_iterations = 5000;
  SATPlacement sat = SATPlacement(instance,partitions, clusters, n2p,po2c, p2c, c2p, max_iterations);


  /*
  std::unique_ptr<PlacementVisualizer> visualizer;
  if (!visualization_file_prefix.empty()) visualizer = std::make_unique<PlacementVisualizer>(instance, visualization_file_prefix);
  GreedyPlacement placement{instance, visualizer.get()};
  const auto& assignment = placement.findPlacement();
  placement.improve(improvement_iters);

  Verifier verifier{instance, assignment};
  size_t num_crossings = 0;
  bool valid = verifier.verify(num_crossings);
  std::cout << "Verification result: " << (valid? "Valid" : "Invalid") << std::endl;
  if (valid) std::cout << "Number of crossings " << num_crossings << std::endl;
  */
  return 0;
}