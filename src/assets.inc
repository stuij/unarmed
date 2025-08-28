.ifndef ASSETS_INC
ASSETS_INC = 1

;; we should auto-generate this one at some point
.segment "RODATA"


;; font
font_tiles:
.incbin "assets/font.tiles"
font_tiles_end:

font_map:
.incbin "assets/font_prio.map"
font_map_end:

font_palette:
.incbin "assets/font.palette"
font_palette_end:


;; sprites
demo_sprite_tiles:
.incbin "assets/demo-sprites.tiles"
demo_sprite_tiles_end:

demo_sprite_palette:
.incbin "assets/demo-sprites.palette"
demo_sprite_palette_end:


;;; menus

;; after game
between_menu_map:
.incbin "assets/between_menu.map"
between_menu_map_end:



;; tile select
select_tile_map:
.incbin "assets/select_tiles.map"
select_tile_map_end:

select_row_count:
.incbin "assets/select_row_count.bin"
select_row_count_end:

select_row_table:
.incbin "assets/select_row_table.bin"
select_row_table_end:

select_row_table_cumul:
.incbin "assets/select_row_table_cumulative.bin"
select_row_table_cumul_end:


select_row_name:
.incbin "assets/select_row_name.bin"
select_row_name_end:

select_row_tile_types:
.incbin "assets/select_row_tile_types.bin"
select_row_tile_types_end:

select_tiles_max:
.incbin "assets/select_tiles_max.bin"
select_tiles_max_end:


.segment "BANK3"

;; backgrounds

;; these are the level tiles
;; map is coming from the
town_tiles:
.incbin "assets/town.tiles"
town_tiles_end:

town_palette:
.incbin "assets/town.palette"
town_palette_end:

town_map:
.incbin "assets/town-map.map"
town_map_end:

town_coll:
.incbin "assets/town-map.coll"
town_coll_end:


.segment "BANK4"

;; city
city_tiles:
.incbin "assets/city.tiles"
city_tiles_end:

city_map:
.incbin "assets/city.map"
city_map_end:

city_palette:
.incbin "assets/city.palette"
city_palette_end:


;; title screen
title_screen_tiles:
.incbin "assets/title-screen.tiles"
title_screen_tiles_end:

title_screen_map:
.incbin "assets/title-screen.map"
title_screen_map_end:

title_screen_palette:
.incbin "assets/title-screen.palette"
title_screen_palette_end:


.endif ; ASSETS_INC
