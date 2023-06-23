

if ( ${GD_BUILD_TESTS} )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google-test)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lemon)

find_package(CGAL)

# copy rapidjson to build folder since we did not integrate it into cmake
# file(COPY rapidjson DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/external)
