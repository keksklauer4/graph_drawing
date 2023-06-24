
target_sources(graph_drawing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/local_sat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/local_gurobi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/local_functors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/local_reopt.cpp
)
