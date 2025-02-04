cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

target_compile_options( ${PROJECT_NAME}
    PRIVATE "SHELL:-I ${CMAKE_CURRENT_SOURCE_DIR}" )

# list of backgrounds
set(BGS town)

# add the custom commands for each background
foreach(BG ${BGS})
  add_custom_command(
    OUTPUT ${BG}.map ${BG}.palette ${BG}.tiles
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${BG}.png ${BG}
    )

  # build a list of all the results
  list(APPEND BG_BINS ${BG}.map ${BG}.palette ${BG}.tiles)
endforeach()

add_custom_target(ASSETS ALL DEPENDS ${BG_BINS})