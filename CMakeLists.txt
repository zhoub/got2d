cmake_minimum_required(VERSION 3.6)

#
include(CTest)
include(ExternalProject)

#
set(GML_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/gml/stage")

ExternalProject_Add(gml
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gml
	DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern
	DOWNLOAD_COMMAND git submodule init
	UPDATE_COMMAND git submodule update
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/gml
	CMAKE_GENERATOR ${CMAKE_GENERATOR}
	CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${GML_STAGE_DIR}"
)

#
project(got2d)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(got2d)

if(MSVC)
	if(BUILD_TESTING)
		add_subdirectory(testbed)
	endif()
endif()
