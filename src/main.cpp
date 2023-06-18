#include "gd_types.hpp"
#include <iostream>
#include <CLI/CLI.hpp>

#include <common/instance.hpp>
#include <io/json_handling.hpp>

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

  CLI11_PARSE(app, argc, argv);

  std::cout << "Parsing file " << file << std::endl;

  instance_t instance = parseInstanceFromFile(file);
  std::cout << "Done parsing file." << std::endl;

  std::vector<std::vector<vertex_t>> partitions = {{0, 1, 3, 4}, {2, 5}, {6, 7}, {8}};
  std::vector<std::vector<point_id_t>> clusters = {{0, 1, 2, 3, 7, 8, 9, 10, 14, 15, 16, 17}, {21, 22, 23, 24, 28, 29, 30, 31}, {4, 5, 6, 11, 12, 13, 18,19, 20}, {25, 26, 27, 32, 33, 34}};
  std::vector<int> n2p{0, 0, 1, 0, 0, 1, 2, 2, 3};
  std::vector<int> po2c{0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 1, 1, 1, 1, 3, 3, 3, 1, 1, 1, 1, 3, 3, 3};
  std::vector<int> p2c{0, 1, 2, 3};
  std::vector<int> c2p{0, 1, 2, 3};
  int max_iterations = 500;
  SATPlacement sat = SATPlacement(instance,partitions, clusters, n2p,po2c, p2c, c2p, max_iterations);


  return 0;
}