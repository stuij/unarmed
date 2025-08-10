.ifndef DATA_INC
DATA_INC = 1

.segment "ZEROPAGE"
mem_start:
W0:
B0L:
.res 1
B0H:
.res 1
W1:
B1L:
.res 1
B1H:
.res 1
W2:
B2L:
.res 1
B2H:
.res 1
W3:
B3L:
.res 1
B3H:
.res 1
W4:
B4L:
.res 1
B4H:
.res 1
W5:
B5L:
.res 1
B5H:
.res 1
W6:
B6L:
.res 1
B6H:
.res 1
W7:
B7L:
.res 1
B7H:
.res 1

in_nmi:
.res 2

;; used for geneneral purpose pseudo rng needs
seed:
.res 2

;; map data
map_x: .res 2
map_y: .res 2


.segment "BSS"

;; player data
p1: .tag player
p2: .tag player
p3: .tag player
p4: .tag player


.align 16

wins: .tag wins
matches: .tag matches
hp: .tag hp
game_data: .tag game_data
tile_canvas: .tag canvas_menu
select_tile_menu: .tag select_tile_menu
between_games_menu: .tag between_games_menu
in_game_keys: .tag igk

;; bullet data
;; start at 0 because we're not insane. player start kinda forced by
;; snes naming

.align 16

b0: .tag bullet
b1: .tag bullet
b2: .tag bullet
b3: .tag bullet
b4: .tag bullet
b5: .tag bullet
b6: .tag bullet
b7: .tag bullet
b8: .tag bullet
b9: .tag bullet
b10: .tag bullet
b11: .tag bullet
b12: .tag bullet
b13: .tag bullet
b14: .tag bullet
b15: .tag bullet
b16: .tag bullet
b17: .tag bullet
b18: .tag bullet
b19: .tag bullet
b20: .tag bullet
b21: .tag bullet
b22: .tag bullet
b23: .tag bullet
b24: .tag bullet
b25: .tag bullet
b26: .tag bullet
b27: .tag bullet
b28: .tag bullet
b29: .tag bullet
b30: .tag bullet
b31: .tag bullet


OAM_mirror: .res $220

current_coll_map: .res COLL_MAP_SIZE
current_coll_map_bottom: .res COLL_MAP_BOTTOM_SIZE

current_tile_map: .res GAME_MAP_SIZE


.segment "BSS7E"

current_coll_map_staging: .res COLL_MAP_SIZE
current_tile_map_staging: .res GAME_MAP_SIZE


.segment "RODATA"

;; menus
select_screen_title: .asciiz "Choose tile to draw"
select_screen_tile_type_desc: .asciiz "type:"

menu_legend_between_screen_root: .asciiz "a:choose"
menu_legend_between_screen_pause: .asciiz "a:choose start:unpause"
menu_legend_select_screen: .asciiz "a:choose b:back"
menu_legend_select_screen_pause_1: .asciiz "a:choose b:back"
menu_legend_select_screen_pause_2: .asciiz "start:unpause"

menu_legend_draw_screen_1: .asciiz "a:choose b:back"
menu_legend_draw_screen_2: .asciiz "x:flip horz y:flip vert"
menu_legend_draw_screen_pause_1: .asciiz "a:choose b:back start:unpause"

between_wins: .asciiz "wins"

between_start_str:     .asciiz  "start match >"
between_restart_str:   .asciiz  "  new match >"
between_edit_str:      .asciiz  "       edit >"
between_players_str:   .asciiz  "    players 234"
between_hitpoints_str: .asciiz  "  hitpoints 123456789"
between_wins_str:      .asciiz  "       sets 123456789"
between_level_str:     .asciiz  "      level 123456789"

between_menu_row_table:
.word 1                 ; start/restart
.word 1                 ; edit
.word (PLAYER_MAX - 1)  ; players
.word $9                ; hitpoints
.word $9                ; wins
.word $9                ; level

draw_tile_row_count: .word $1c

start_game_overlay_text:
.asciiz "    Get set to KILL!"

end_game_overlay_text:
.asciiz " wins game!!!"

end_match_overlay_text:
.asciiz " wins match!!!"


;; So this seems like it is taking the soul a bit out of
;; the programmers mind, but this is the price I'm willing
;; to pay for a uniform menu system.

;; we're pasting three extra rows to the collision map, so
;; players have the feeling they fall down, instead of just going out
;; of bounds on the ground
draw_tile_row_table:
.repeat 28
.word $20
.endrep

.endif ; DATA_INC
