cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

target_compile_options( ${PROJECT_NAME}
    PRIVATE "SHELL:-I ${CMAKE_CURRENT_SOURCE_DIR}" )

# list of backgrounds
list(APPEND BGS town city title-screen splash-screen-non-text)
# list of background palette offsets
# we don't care about the 'town' bg palette because the maps for that
# are generated with a python script. However for documentation, currently
# we use palette entry 2 for that
list(APPEND BG_PALLS 2 3 3 3)
list(APPEND BG_EXTRA "--no-remap" "--no-remap" "--no-remap" "")

# add the custom commands for each background tileset
foreach(BG IN ZIP_LISTS BGS BG_PALLS BG_EXTRA)
  add_custom_command(
    OUTPUT ${BG_0}.map ${BG_0}.palette ${BG_0}.tiles
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/${BG_0}.png ${BG_0} 4 ${BG_1} ${BG_2}
    )

  # build a list of all the results
  list(APPEND BG_BINS ${BG_0}.map ${BG_0}.palette ${BG_0}.tiles)
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
    COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-sound.sh ${CMAKE_CURRENT_SOURCE_DIR}/sound/sounds_n_stuff.terrificaudio ${TAD_OUT_FILES}
    )

    set(FONT_OUTPUT font.map font.palette font.tiles)

add_custom_command(
  OUTPUT ${FONT_OUTPUT}
  COMMAND ${PROJECT_SOURCE_DIR}/bin/convert-bg.sh ${CMAKE_CURRENT_SOURCE_DIR}/fonts/boxy_bold.png font 2 0 "--no-discard --no-remap"
  )

add_custom_target(ASSETS ALL DEPENDS ${BG_BINS} ${SPRITE_BINS} ${MAP_BINS} ${TAD_OUT_FILES} ${FONT_OUTPUT})
