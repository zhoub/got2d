cmake_minimum_required(VERSION 3.6)

project(got2d)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(extern/gml)
add_subdirectory(extern/res)
add_subdirectory(got2d)

if(WITH_GOT2D_TESTS)

if(MSVC)
add_subdirectory(testbed)
endif(MSVC)

endif()