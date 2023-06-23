#include <iostream>
#include <CLI/CLI.hpp>

#include <common/verifier.hpp>
#include <io/json_handling.hpp>

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
  return 0;
}