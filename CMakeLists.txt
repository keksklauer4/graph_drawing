cmake_minimum_required(VERSION 3.7)

project(graph_drawing)

option(GD_BUILD_TESTS "Build graph_drawing test executable" ON)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

#added line 11-18
set(GUROBI_INCLUDE_DIRS "/opt/gurobi1002/linux64/include/")
find_library(GUROBI_LIBRARY
    NAMES gurobi gurobi100
    HINTS "/opt/gurobi1002/linux64/"
    PATH_SUFFIXES lib)
#set(GUROBI_LIBRARY "/opt/gurobi1002/linux64/lib/libgurobi100.so" "/opt/gurobi1002/linux64/lib/libgurobi.so.10.0.2")
set(GUROBI_CXX_LIBRARY "/opt/gurobi1002/linux64/lib/libgurobi_c++.a")
set(GUROBI_CXX_DEBUG_LIBRARY ${GUROBI_CXX_LIBRARY})

set(GD_SAT_SOLVER_INCLUDES external/EvalMaxSAT/lib/EvalMaxSAT/src external/EvalMaxSAT/lib/MaLib/src external/EvalMaxSAT/lib/glucose/src)

set(CMAKE_BUILD_TYPE Release)
#added external/aabbcc/include/
set(GD_INCLUDE_LIBS external/ src/ # build/external/lemon/ external/lemon/
                    external/rapidjson/include/ external/HIP/interface/
                    external/aabbcc/src/ external/aabbcc/include/ ${AABBCC_INCLUDE_DIR}
                    ${EIGEN3_INCLUDE_DIR}/ external/hclust/
                    ${GD_SAT_SOLVER_INCLUDES}
                    ${GUROBI_INCLUDE_DIRS})

set(GD_LINK_LIBS CGAL::CGAL kahip
        Eigen3::Eigen hclust EvalMaxSAT
        aabbcc)

set(GD_GUROBI_LINK_LIBS optimized ${GUROBI_CXX_LIBRARY} ${GUROBI_LIBRARY} debug ${GUROBI_CXX_DEBUG_LIBRARY} ${GUROBI_LIBRARY})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HCLUST_BUILD_DEMO CACHE BOOL OFF)

set(LEMON_ENABLE_GLPK CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_COIN CACHE BOOL OFF)
set(LEMON_ENABLE_SOPLEX CACHE BOOL OFF)

add_compile_definitions(CGAL_DISABLE_ROUNDING_MATH_CHECK)
add_compile_definitions(CGAL_EIGEN3_ENABLED)
add_compile_definitions(EIGEN_MPL2_ONLY)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

# build gtest and other libs
add_subdirectory(external)

file(COPY external/rapidjson/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external)


#if ( CMAKE_COMPILER_IS_GNUCC )
#    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -frounding-math -Wextra -Wpedantic -Werror -fdiagnostics-color=always")
#endif()
#if ( MSVC )
#    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4 /WX")
#endif()

add_library(graph_drawing)

message("Gurobi lib " ${GUROBI_CXX_LIBRARY})
message("AABBCC include dir " ${AABBCC_INCLUDE_DIR})

target_include_directories(graph_drawing PRIVATE ${GD_INCLUDE_LIBS})

add_subdirectory(src)
target_link_libraries(graph_drawing ${GD_LINK_LIBS} ${GD_GUROBI_LINK_LIBS})
add_executable(gd src/main.cpp)
target_include_directories(gd PRIVATE src ${GD_INCLUDE_LIBS} external/cli11/include/)
target_link_libraries(gd graph_drawing ${GD_GUROBI_LINK_LIBS})


if ( ${GD_BUILD_TESTS} )
    add_executable(gd_test)
    add_subdirectory(test)
    target_include_directories(gd_test PRIVATE src ${GD_INCLUDE_LIBS} test)
    target_link_libraries(gd_test graph_drawing gtest_main ${GD_GUROBI_LINK_LIBS})

    file(COPY test/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY test/gdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

