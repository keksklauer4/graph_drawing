
#include "common/assignment.hpp"
#include "common/instance.hpp"
#include "common/misc.hpp"
#include "gd_types.hpp"
#include <CLI/CLI.hpp>

#include <fstream>
#include <placement/sampling_solver.hpp>
#include <io/json_handling.hpp>
#include <sstream>
#include <stdexcept>
#include <verification/verifier.hpp>
#include <filesystem>
#include <AABBCC/AABBCC.h>

using namespace gd;

namespace fs = std::filesystem;

namespace
{
  void create_path(const std::string& out_file)
  {
    fs::path path = out_file;
    auto parent = path.parent_path();
    fs::create_directories(parent);
  }

  void dump_stats(const instance_t& instance, const std::string out_file)
  {
    std::stringstream filename;
    create_path(out_file);
    filename << out_file << "stats.json";

    std::string name = filename.str();
    std::ofstream out(name);
    if (!out.is_open())
    {
      std::cout << "ERROR: Could not open file \'" << name << "\' for writing the statistics :(" << std::endl;
      throw std::runtime_error("Could not open output file for statistics :(");
    }
    gd::dump_statistics(out, instance.m_stats);
    out.close();
  }

  void dump_res(const instance_t& instance, const VertexAssignment& assignment,
                const std::string& out_file, size_t time_limit_ms)
  {
    create_path(out_file);
    size_t num_crossings = UINT_UNDEF;
    Verifier verifier{instance, assignment};
    bool valid = verifier.verify(num_crossings);
    std::stringstream filename;
    filename << out_file << "_" << time_limit_ms
             << "_" << (valid ? "valid" : "invalid") << "_";
    if (isDefined(num_crossings)) filename << num_crossings;
    else filename << "-1";
    filename << ".json";
    std::string name = filename.str();
    std::ofstream out(name);
    if (!out.is_open())
    {
      std::cout << "ERROR: Could not open file \'" << name << "\' :(" << std::endl;
      throw std::runtime_error("Could not open output file :(");
    }
    gd::dump_assignment(out, instance, assignment);
    out.close();

    STATS(dump_stats(instance, out_file);)
  }

}

int main(int argc, const char** argv)
{
  CLI::App app("Graph drawing algorithms");
  // add version output
  app.set_version_flag("--version", "3.14159265");
  std::string file;
  std::string out_file;
  std::string vis_path = "";
  double fraction_initial_placement = 0.2;
  size_t time_limit_ms;
  app.add_option("-f,--file", file, "File name")->required();
  app.add_option("-o,--out", out_file, "Output file prefix")->required();
  app.add_option("-t,--time", time_limit_ms, "Time limit in milliseconds")->required();
  app.add_option("-v,--visualize", vis_path, "Visualization path");
  app.add_option("-i,--initial_placement", fraction_initial_placement,
    "The fraction (in [0,1]) of the time limit to put into finding initial placements (optional).");

  CLI11_PARSE(app, argc, argv);
  if (fraction_initial_placement < 0 || fraction_initial_placement > 1)
  {
    std::cout << "ERROR: Bad initial placement fraction! Set to " << fraction_initial_placement << std::endl;
    return -1;
  }
  std::cout << "File " << file << std::endl;
  instance_t instance = parseInstanceFromFile(file);
  instance.m_timer.set_time_limit(time_limit_ms);
  SamplingSolver solver{instance};
  VertexAssignment assignment = solver.solve(vis_path, fraction_initial_placement);

  dump_res(instance, assignment, out_file, time_limit_ms);
  return 0;
}
