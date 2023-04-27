#include "verifier.hpp"

using namespace gd;

bool gd::test_func(int v)
{
  return v > 0 && (v % 3) == 0;
}
