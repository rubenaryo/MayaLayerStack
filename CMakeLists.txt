cmake_minimum_required(VERSION 3.18)

project(LayerStack)

set(MAYA_LOCATION "$ENV{MAYA_LOCATION}")
set(MAYA_PLUGIN_PATH "$ENV{DEVKIT_LOCATION}/plug-ins/plug-ins")
set(LAYERSTACK_PLUGIN_PATH "${MAYA_PLUGIN_PATH}/LayerStack")
set(MAYA_MOD_PATH "$ENV{MAYA_LOCATION}/modules")

if (NOT EXISTS "${MAYA_LOCATION}")
  message(FATAL_ERROR "Maya not found at ${MAYA_LOCATION}, please ensure your MAYA_LOCATION environment variable is set.")
endif()

if (NOT EXISTS "${MAYA_PLUGIN_PATH}")
  message(FATAL_ERROR "Maya plugin path not found at ${MAYA_PLUGIN_PATH}, please ensure your DEVKIT_LOCATION environment variable is set.")
endif()


### MODULE INSTALLATION START ###
if (NOT EXISTS "${MAYA_MOD_PATH}/LayerStack.mod")
    set(MOD_FILE "${CMAKE_BINARY_DIR}/LayerStack.mod")
    file(WRITE ${MOD_FILE} 
    "+ LayerStack 1.0 ${LAYERSTACK_PLUGIN_PATH}\n"
    )
    
    if(WIN32)
	message(STATUS "Installing mod file at ${MAYA_MOD_PATH}")
        set(PS_COMMAND 
    		"Start-Process powershell -ArgumentList \"-NoProfile\", \"-ExecutionPolicy\", \"Bypass\", \"-Command\", \"Copy-Item '${MOD_FILE}' '${MAYA_MOD_PATH}'\" -Verb RunAs"
	)
	message(STATUS "${MAYA_MOD_PATH}/LayerStack.mod")
	execute_process(
    		COMMAND powershell -Command "${PS_COMMAND}"
    		RESULT_VARIABLE result
    		OUTPUT_VARIABLE output
    		ERROR_VARIABLE error_output
	)
    endif()
endif()
### MODULE INSTALLATION END ###

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(VS_SOLUTION_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.sln" CACHE INTERNAL "Path to top-level solution file")
set_property(GLOBAL PROPERTY VS_SOLUTION_FILE ${VS_SOLUTION_PATH})

add_subdirectory(LayerStackPlugin)


