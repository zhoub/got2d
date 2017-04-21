cmake_minimum_required(VERSION 3.6)

#
project(got2d)

#
include(CTest)
include(ExternalProject)

# gml
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

# res
#
set(RES_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/stage")

ExternalProject_Add(res
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/res
	DOWNLOAD_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/extern
	DOWNLOAD_COMMAND git submodule init
	UPDATE_COMMAND git submodule update
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/res
	CMAKE_GENERATOR ${CMAKE_GENERATOR}
	CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${RES_STAGE_DIR}"
)

if(MSVC)
	SET(RES_LIBRARY ${RES_STAGE_DIR}/lib/res.lib)
else()
	SET(RES_LIBRARY ${RES_STAGE_DIR}/lib/libres.lib)
endif()

#
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(got2d)

if(MSVC)
	if(BUILD_TESTING)
		add_subdirectory(testbed)
	endif()
endif()
