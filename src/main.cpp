
#include <CLI/CLI.hpp>

#include <placement/sampling_solver.hpp>
#include <io/json_handling.hpp>

using namespace gd;

int main(int argc, const char** argv)
{
  CLI::App app("Graph drawing algorithms");
  // add version output
  app.set_version_flag("--version", "3.14159265");
  std::string file;
  std::string vis_path = "";
  size_t time_limit_ms;
  app.add_option("-f,--file", file, "File name")->required();
  app.add_option("-t,--time", time_limit_ms, "Time limit in milliseconds")->required();
  app.add_option("-v,--visualize", vis_path, "Visualization path");

  CLI11_PARSE(app, argc, argv);
  std::cout << "File " << file << std::endl;
  instance_t instance = parseInstanceFromFile(file);
  instance.m_timer.set_time_limit(time_limit_ms);
  SamplingSolver solver{instance};
  solver.solve(vis_path);
  return 0;
}
