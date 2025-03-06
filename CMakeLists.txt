cmake_minimum_required(VERSION 3.18)

project(LayerStack)

set(MAYA_LOCATION "C:/Program Files/Autodesk/Maya2024")
set(MAYA_DEVKIT_PATH "C:/Users/Ruben/devkitBase")

if (NOT EXISTS "${MAYA_LOCATION}")
  message(FATAL_ERROR "Maya not found at ${MAYA_LOCATION}, please set MAYA_LOCATION to the correct path!!!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(VS_SOLUTION_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sln" CACHE INTERNAL "Path to top-level solution file")
set_property(GLOBAL PROPERTY VS_SOLUTION_FILE ${VS_SOLUTION_PATH})

add_subdirectory(LayerStackPlugin)


