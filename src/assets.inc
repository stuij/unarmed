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

;; sprites
demo_sprite_tiles:
.incbin "assets/demo-sprites.tiles"
demo_sprite_tiles_end:

demo_sprite_palette:
.incbin "assets/demo-sprites.palette"
demo_sprite_palette_end:

;; tile select
select_tile_map:
.incbin "assets/select_tiles.map"
select_tile_map_end:

select_row_count:
.incbin "assets/select_row_count.bin"
select_row_count_end:

select_column_count:
.incbin "assets/select_column_count.bin"
select_column_count_end:

select_row_name:
.incbin "assets/select_row_name.bin"
select_row_name_end:

.endif ; ASSETS_INC