
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

using namespace gd;

namespace
{
  void dump_res(const instance_t& instance, const VertexAssignment& assignment,
                const std::string& out_file, size_t time_limit_ms)
  {
    size_t num_crossings = UINT_UNDEF;
    Verifier verifier{instance, assignment};
    bool valid = verifier.verify(num_crossings);
    std::stringstream filename;
    filename << out_file << "_" << time_limit_ms
             << "_" << (valid ? "valid" : "invalid") << "_";
    if (isDefined(num_crossings)) filename << num_crossings;
    else filename << "-1";
    filename << ".json";

    std::ofstream out{filename.str()};
    if (!out.is_open()) throw std::runtime_error("Could not open output file :(");
    gd::dump_assignment(out, instance, assignment);
    out.close();
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
  size_t time_limit_ms;
  app.add_option("-f,--file", file, "File name")->required();
  app.add_option("-o,--out", out_file, "Output file prefix")->required();
  app.add_option("-t,--time", time_limit_ms, "Time limit in milliseconds")->required();
  app.add_option("-v,--visualize", vis_path, "Visualization path");

  CLI11_PARSE(app, argc, argv);
  std::cout << "File " << file << std::endl;
  instance_t instance = parseInstanceFromFile(file);
  instance.m_timer.set_time_limit(time_limit_ms);
  SamplingSolver solver{instance};
  VertexAssignment assignment = solver.solve(vis_path);


  

  return 0;
}
