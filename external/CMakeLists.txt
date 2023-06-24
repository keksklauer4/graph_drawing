
file(COPY EvalMaxSAT DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if ( ${GD_BUILD_TESTS} )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google-test)
endif()

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lemon)
add_subdirectory(HIP)
add_subdirectory(Eigen)
add_subdirectory(hclust)
add_subdirectory(gurobi)

# -----------------------
# EvalMaxSAT
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EvalMaxSAT)
# -----------------------

find_package(CGAL)

# copy rapidjson to build folder since we did not integrate it into cmake
# file(COPY rapidjson DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external)
