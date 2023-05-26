
file(COPY EvalMaxSAT DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if ( ${GD_BUILD_TESTS} )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google-test)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lemon)


# -----------------------
# EvalMaxSAT
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/EvalMaxSAT)

#get_target_property(target_options EvalMaxSAT COMPILE_OPTIONS)
#list(REMOVE_ITEM target_options "-Wpedantic -Werror")
#set_property(TARGET EvalMaxSAT PROPERTY COMPILE_OPTIONS ${target_options})
# -----------------------

find_package(CGAL)
