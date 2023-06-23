#include "gd_types.hpp"
#include "placement/greedy_placement.hpp"
#include "verification/verifier.hpp"
#include <iostream>
#include <CLI/CLI.hpp>

#include <common/instance.hpp>
#include <io/json_handling.hpp>
#include <memory>

#include <io/placement_visualizer.hpp>

#include <placement/divide/graph_partitioning.hpp>
#include <placement/divide/point_clustering.hpp>

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
  std::cout << hierarchy << std::endl;
  PointClustering points{instance};
  points.cluster();
  std::cout << points << std::endl;

  std::unique_ptr<PlacementVisualizer> visualizer;
  if (!visualization_file_prefix.empty())
  {
    visualizer = std::make_unique<PlacementVisualizer>(instance, visualization_file_prefix);
    /*visualizer->setHierarchy(hierarchy);
    visualizer->setClustering(points);
    visualizer->drawClustering();*/
  }
  GreedyPlacement placement{instance, visualizer.get()};
  const auto& assignment = placement.findPlacement();
  placement.improve(improvement_iters);

  Verifier verifier{instance, assignment};
  size_t num_crossings = 0;
  bool valid = verifier.verify(num_crossings);
  std::cout << "Verification result: " << (valid? "Valid" : "Invalid") << std::endl;
  if (valid) std::cout << "Number of crossings " << num_crossings << std::endl;
  return 0;
}