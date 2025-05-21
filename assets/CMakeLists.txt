cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

target_compile_options( ${PROJECT_NAME}
    PRIVATE "SHELL:-I ${CMAKE_CURRENT_SOURCE_DIR}" )

# list of backgrounds
set(BGS town)

# add the custom commands for each background tileset
foreach(BG ${BGS})
  add_custom_command(
    OUTPUT ${BG}.map ${BG}.palette ${BG}.tiles
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${BG}.png ${BG} 4
    )

  # build a list of all the results
  list(APPEND BG_BINS ${BG}.map ${BG}.palette ${BG}.tiles)
endforeach()

# add the custom commands for each background map
set(MAPS town-map)

foreach(MAP ${MAPS})
  add_custom_command(
    OUTPUT ${MAP}.map
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-tmj.py ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${MAP}.tmj ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/tile-schema.json
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

set(TAD_PROD audio)

set(TAD_OUT_FILES ${TAD_PROD}.asm ${TAD_PROD}.bin ${TAD_PROD}.inc)

add_custom_command(
    OUTPUT ${TAD_OUT_FILES}
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-sound.sh ../../terrific-audio-driver/examples/example-project.terrificaudio ${TAD_OUT_FILES}
    )

    set(FONT_OUTPUT font.map font.palette font.tiles)

add_custom_command(
  OUTPUT ${FONT_OUTPUT}
  COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/fonts/boxy_bold.png font 2 --no-discard
  )

add_custom_target(ASSETS ALL DEPENDS ${BG_BINS} ${SPRITE_BINS} ${MAP_BINS} ${TAD_OUT_FILES} ${FONT_OUTPUT})
