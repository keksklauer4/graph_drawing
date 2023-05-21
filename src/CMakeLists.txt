
add_subdirectory(common)
add_subdirectory(io)
add_subdirectory(placement)
add_subdirectory(verification)

target_sources(graph_drawing PRIVATE
    #${CMAKE_CURRENT_SOURCE_DIR}/.cpp
)

