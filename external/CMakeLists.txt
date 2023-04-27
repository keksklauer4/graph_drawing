

if ( ${GD_BUILD_TESTS} )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google-test)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lemon)
