cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

target_compile_options( ${PROJECT_NAME}
    PRIVATE "SHELL:-I ${CMAKE_CURRENT_SOURCE_DIR}" )

# list of backgrounds
set(BGS town)

# add the custom commands for each background tileset
foreach(BG ${BGS})
  add_custom_command(
    OUTPUT ${BG}.map ${BG}.palette ${BG}.tiles
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${BG}.png ${BG}
    )

  # build a list of all the results
  list(APPEND BG_BINS ${BG}.map ${BG}.palette ${BG}.tiles)
endforeach()

# add the custom commands for each background map
set(MAPS town-map)

foreach(MAP ${MAPS})
  add_custom_command(
    OUTPUT ${MAP}.map
    COMMAND ${PROJECT_SOURCE_DIR}/bin/tmj-to-map-bin.py ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${MAP}.tmj ${MAP}.map
    )

  # build a list of all the results
  list(APPEND MAP_BINS ${MAP}.map)
endforeach()


# list of sprite sheets
set(SHEETS demo-sprites)

# add the custom commands for each sprite sheet
foreach(SH ${SHEETS})
  add_custom_command(
    OUTPUT ${SH}.palette ${SH}.tiles
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-sprite-sheet.sh ${CMAKE_CURRENT_SOURCE_DIR}/sprites/${SH}.png ${SH}
    )

  # build a list of all the results
  list(APPEND SPRITE_BINS ${SH}.palette ${SH}.tiles)
endforeach()

add_custom_target(ASSETS ALL DEPENDS ${BG_BINS} ${SPRITE_BINS} ${MAP_BINS})