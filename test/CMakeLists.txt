
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_sources(gd_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/first_test.cpp
)