cmake_minimum_required(VERSION 3.7)

project(graph_drawing)

option(GD_BUILD_TESTS "Build graph_drawing test executable" ON)
enable_language(C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(GD_INCLUDE_LIBS build/external/lemon/ external/lemon/ external/ src/ external/rapidjson/include/rapidjson/)
set(GD_LINK_LIBS lemon)

set(LEMON_ENABLE_GLPK CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_ILOG CACHE BOOL OFF)
set(LEMON_ENABLE_COIN CACHE BOOL OFF)
set(LEMON_ENABLE_SOPLEX CACHE BOOL OFF)


# build gtest and other libs
add_subdirectory(external)


if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -fdiagnostics-color=always")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4 /WX")
endif()

add_library(graph_drawing)


target_include_directories(graph_drawing PRIVATE ${GD_INCLUDE_LIBS})

add_subdirectory(src)
target_link_libraries(graph_drawing ${GD_LINK_LIBS})


add_executable(gd src/main.cpp)
target_include_directories(gd PRIVATE src ${GD_INCLUDE_LIBS})
target_link_libraries(gd graph_drawing)


if ( ${GD_BUILD_TESTS} )
    add_executable(gd_test)
    add_subdirectory(test)
    target_include_directories(gd_test PRIVATE src ${GD_INCLUDE_LIBS})
    target_link_libraries(gd_test graph_drawing gtest_main)
endif()