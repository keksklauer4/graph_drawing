#include "hierarchical_sat.hpp"
#include "EvalMaxSAT.h"
#include "common/instance.hpp"
#include "gd_types.hpp"
#include "glucose/core/Solver.h"
#include "verification/verification_utils.hpp"
#include <algorithm>
#include <utility>
#include <verification/line_crossings.hpp>
#include <iostream>
#include <vector>
#include <verification/trivial_collinear.cpp>

using namespace gd;

Hier_SATPlacement::Hier_SATPlacement(const Instance& instance):m_instance(instance){

}

