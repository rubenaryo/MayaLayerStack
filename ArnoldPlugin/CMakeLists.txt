cmake_minimum_required(VERSION 3.11)

project("ArnoldLayerStack")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/plugin)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(ARNOLD_DIR "$ENV{ARNOLD_PATH}")

include_directories("${ARNOLD_DIR}/include/arnold")

file(GLOB_RECURSE core_headers
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.hpp")

file(GLOB_RECURSE core_sources
	"${PROJECT_SOURCE_DIR}/src/*.c"
	"${PROJECT_SOURCE_DIR}/src/*.cpp")

if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	GroupSources(src)
endif()

add_library(${CMAKE_PROJECT_NAME} SHARED ${core_headers} ${core_sources})

target_link_libraries(${CMAKE_PROJECT_NAME} "${ARNOLD_DIR}/lib/ai.lib")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
