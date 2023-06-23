
add_subdirectory(divide)

target_sources(graph_drawing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/placement_metrics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/greedy_placement.cpp
)