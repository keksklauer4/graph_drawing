
add_subdirectory(common)

target_sources(graph_drawing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/verifier.cpp
)

