
target_sources(graph_drawing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/line_crossings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trivial_collinear.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/verifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/incremental_crossing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/incremental_collinear.cpp
)
