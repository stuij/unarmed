.ifndef ASSETS_INC
ASSETS_INC = 1

;; we should auto-generate this one at some point
.segment "RODATA"

;; backgrounds
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

;; font
font_tiles:
.incbin "assets/font.tiles"
font_tiles_end:

font_map:
.incbin "assets/font_prio.map"
font_map_end:


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

.endif ; ASSETS_INC