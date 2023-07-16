
set(CXX true)
include(FindGUROBI.cmake)
find_library(GUROBI REQUIRED)
message("Gurobi lib is here " ${GUROBI_INCLUDE_DIRS})

