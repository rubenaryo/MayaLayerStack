cmake_minimum_required(VERSION 3.18)

project(LayerStack)

set(MAYA_LOCATION "$ENV{MAYA_LOCATION}")
set(MAYA_PLUGIN_PATH "$ENV{DEVKIT_LOCATION}/plug-ins/plug-ins")
set(MAYA_SCRIPT_PATH "$ENV{DEVKIT_LOCATION}/plug-ins/scripts")


string(REPLACE "/" "\\" ${MAYA_PLUGIN_PATH} ${MAYA_PLUGIN_PATH})
string(REPLACE "/" "\\" ${MAYA_SCRIPT_PATH} ${MAYA_SCRIPT_PATH})


#set(LAYERSTACK_PLUGIN_PATH "${MAYA_PLUGIN_PATH}/LayerStack")

if (NOT EXISTS "${MAYA_LOCATION}")
  message(FATAL_ERROR "Maya not found at ${MAYA_LOCATION}, please ensure your MAYA_LOCATION environment variable is set.")
endif()

if (NOT EXISTS "${MAYA_PLUGIN_PATH}")
  message(FATAL_ERROR "Maya plugin path not found at ${MAYA_PLUGIN_PATH}, please ensure your DEVKIT_LOCATION environment variable is set.")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(VS_SOLUTION_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sln" CACHE INTERNAL "Path to top-level solution file")
set_property(GLOBAL PROPERTY VS_SOLUTION_FILE ${VS_SOLUTION_PATH})

add_subdirectory(LayerStackPlugin)
add_subdirectory(ArnoldPlugin)


