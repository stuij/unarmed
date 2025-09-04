cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

set( MEMORY_MAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/lorom256.cfg )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

project( unarmed
         LANGUAGES CA65816 )
include_directories (Project)

add_executable( ${PROJECT_NAME} "" )


add_subdirectory( assets )
add_subdirectory( src )
set_target_properties( ${PROJECT_NAME}
    PROPERTIES SUFFIX ".sfc" )

add_dependencies( ${PROJECT_NAME} ASSETS )

# Check if the path to the memory map file is set
if( NOT DEFINED MEMORY_MAP_FILE )
  message( FATAL_ERROR "Path to memory map file not set!" )
endif( NOT DEFINED MEMORY_MAP_FILE )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND superfamicheck ${PROJECT_NAME}.sfc -f -s
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Fixing header..."
)