cmake_minimum_required(VERSION 3.18)

project(LayerStackPlugin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add source files to the project
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

# Add header files to the project
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "src/*.h"
)

# JSON Library
include_directories("src/external/nlohmann")

file(GLOB_RECURSE SRC_PY
    "src/Python/*.py"
)

set(UI_ENTRY_SCRIPT_NAME layer_stack_ui.py)
set(UI_ENTRY_SCRIPT_PATH "${PROJECT_SOURCE_DIR}/src/Python")

find_package(Maya REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${SRC_PY})

source_group("UI" FILES ${SRC_PY})

target_link_libraries(${PROJECT_NAME} PRIVATE Maya::Maya)
target_include_directories(${PROJECT_NAME} 
    PRIVATE Maya::Maya
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
)
MAYA_PLUGIN(${PROJECT_NAME})

# Arnold linker inputs and includes
target_link_libraries(${PROJECT_NAME}
	PRIVATE $ENV{DEVKIT_LOCATION}/ArnoldSDK/lib/ai.lib
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE $ENV{DEVKIT_LOCATION}/ArnoldSDK/include
)

install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins/${MAYA_VERSION})

add_custom_command(TARGET LayerStackPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "$<TARGET_FILE:LayerStackPlugin>"
    "${MAYA_PLUGIN_PATH}/$<TARGET_FILE_NAME:LayerStackPlugin>"
    COMMENT "Copying MLL to Maya Devkit folder"
)

add_custom_command(TARGET LayerStackPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${UI_ENTRY_SCRIPT_PATH}/${UI_ENTRY_SCRIPT_NAME}"
    "${MAYA_SCRIPT_PATH}/${UI_ENTRY_SCRIPT_NAME}"
    COMMENT "Copying Python UI Script to Maya Devkit folder"
)