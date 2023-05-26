cmake_minimum_required(VERSION 3.7)

project(graph_drawing)

option(GD_BUILD_TESTS "Build graph_drawing test executable" ON)
enable_language(C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(GD_SAT_SOLVER_INCLUDES external/EvalMaxSAT/lib/EvalMaxSAT/src external/EvalMaxSAT/lib/MaLib/src external/EvalMaxSAT/lib/glucose/src)
set(GD_INCLUDE_LIBS build/external/lemon/ external/lemon/ external/ src/ external/rapidjson/include/ ${GD_SAT_SOLVER_INCLUDES})
set(GD_LINK_LIBS lemon CGAL::CGAL EvalMaxSAT)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LEMON_ENABLE_GLPK CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_COIN CACHE BOOL OFF)
set(LEMON_ENABLE_SOPLEX CACHE BOOL OFF)

add_compile_definitions(CGAL_DISABLE_ROUNDING_MATH_CHECK)
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


target_include_directories(graph_drawing PRIVATE ${GD_INCLUDE_LIBS})

add_subdirectory(src)
target_link_libraries(graph_drawing ${GD_LINK_LIBS})
add_executable(gd src/main.cpp)
target_include_directories(gd PRIVATE src ${GD_INCLUDE_LIBS} external/cli11/include/)
target_link_libraries(gd graph_drawing)


if ( ${GD_BUILD_TESTS} )
    add_executable(gd_test)
    add_subdirectory(test)
    target_include_directories(gd_test PRIVATE src ${GD_INCLUDE_LIBS} test)
    target_link_libraries(gd_test graph_drawing gtest_main)

    file(COPY test/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY test/gdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()