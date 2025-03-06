cmake_minimum_required(VERSION 3.18)

project(LayerStack)

set(MAYA_LOCATION "C:/Program Files/Autodesk/Maya2024")

if (NOT EXISTS "${MAYA_LOCATION}")
  message(FATAL_ERROR "Maya not found at ${MAYA_LOCATION}, please set MAYA_LOCATION to the correct path!!!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

add_subdirectory(LayerStackPlugin)