file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY gdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_sources(gd_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/json_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/verifier_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_graph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colinearity_test.cpp
)