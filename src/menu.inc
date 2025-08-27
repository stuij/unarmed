.ifndef MENU_INC
MENU_INC = 1

.code

;; ---------
;; draw menu

.a16
.i16
.proc canvas_calc_tile_under_cursor
    lda menu::curr_row
    ;; row x 32
    asl
    asl
    asl
    asl
    asl
    adc menu::curr_column
    rts
.endproc


.proc canvas_set_tile
    ldy #player::joy_trigger_held
    lda (menu::player), y
    and #JOY_A
    beq flip_h
    jsr canvas_calc_tile_under_cursor
    tay
    tax
    lda canvas_menu::tile_type
    A8
    sta f:current_coll_map_staging, x
    sta current_coll_map, x
    A16
    tya
    asl
    tax
    lda canvas_menu::chosen_tile
    sta f:current_tile_map_staging, x
    sta current_tile_map, x
flip_h:
    ldy #player::joy_trigger
    lda (menu::player), y
    sta a:W0
    and #JOY_X
    beq flip_v
    lda canvas_menu::chosen_tile
    eor #$4000
    sta canvas_menu::chosen_tile
flip_v:
    lda a:W0
    and #JOY_Y
    beq prio
    lda canvas_menu::chosen_tile
    eor #$8000
    sta canvas_menu::chosen_tile
prio:
    lda a:W0
    and #JOY_L
    beq end
    lda canvas_menu::chosen_tile
    eor #$2000
    sta canvas_menu::chosen_tile
end:
    rts
.endproc


;; patch tile map in vram where cursor currently is hovering
.proc canvas_patch_chosen_tile
    jsr canvas_calc_tile_under_cursor
    clc
    adc #VRAM_MAP_BLOCKS_GAME_BASE
    sta VMADD
    lda canvas_menu::chosen_tile
    sta VMDATA
    rts
.endproc


.a16
.i16
.proc draw_tile_menu_callback
    A8
    jsr load_tilemap_to_vram
    A16
    jsr canvas_set_tile
    jsr canvas_patch_chosen_tile
    rts
.endproc


.proc init_draw_menu
    lda #tile_canvas
    tcd
    lda #DRAW_CARET_X_INIT_OFFSET
    sta menu::cursor_x_pos
    lda #DRAW_CARET_X_ORIGIN
    sta menu::cursor_x_pos_origin

    lda #DRAW_CARET_Y_INIT_OFFSET
    sta menu::cursor_y_pos
    lda #DRAW_CARET_Y_ORIGIN
    sta menu::cursor_y_pos_origin

    lda draw_tile_row_count
    sta menu::no_rows

    lda #.loword(draw_tile_row_table)
    sta menu::row_table

    lda #DRAW_CARET_INIT_ROW
    sta menu::curr_row
    lda #DRAW_CARET_INIT_COLUMN
    sta menu::curr_column

    jsr init_menu_common

    lda #.loword(draw_tile_menu_callback)
    sta menu::callback

    lda #.loword(draw_draw_menu)
    sta menu::draw_fn

    lda #($4 + ($4 * $10))
    sta menu::caret_sprite

    lda #$0
    tcd
    rts
.endproc


;; not used, as draw menu is a bit different
.proc draw_draw_menu
    rts
.endproc


;; initialize chosen tile map entry with idx selected in select menu
; A - tile idx of chosen tile
.proc draw_init_chosen_tile
    eor #BG_TILE_PALETTE_ENTRY
    sta tile_canvas + canvas_menu::chosen_tile
    rts
.endproc


; A - chosen tile ID
; X - current player
; Y - chosen tile type
.proc activate_draw_menu
    stx tile_canvas + menu::player
    sty tile_canvas + canvas_menu::tile_type
    jsr draw_init_chosen_tile
    A8
    jsr clear_menu_l3
    A16
    jsr level_screen_to_fore
    lda #.loword(select_tile_menu)
    sta tile_canvas + menu::parent_menu
    inc tile_canvas + menu::just_moved

    ;; help text
    A8
    force_vblank
    A16
    ldx a:game_data + game_data::in_game
    bne :+
    lda #(DRAW_SCREEN_LEGEND_MAP_OFFSET)
    ldx #.loword(menu_legend_draw_screen_1)
    jsr print_string_to_offset
    lda #(DRAW_SCREEN_LEGEND_MAP_OFFSET + $20)
    ldx #.loword(menu_legend_draw_screen_2)
    jsr print_string_to_offset
    bra :++
  : lda #DRAW_SCREEN_LEGEND_MAP_OFFSET
    ldx #.loword(menu_legend_draw_screen_pause_1)
    jsr print_string_to_offset
    lda #(DRAW_SCREEN_LEGEND_MAP_OFFSET + $20)
    ldx #.loword(menu_legend_draw_screen_2)
    jsr print_string_to_offset
  : A8
    unforce_vblank
    A16
    rts
.endproc


;; -----------
;; select menu

.a8
.i16
.proc select_menu_clear_selected_tile_txt
    ldy #(VRAM_MAP_FONT_BASE + SELECT_SCREEN_TILE_TYPE_DESC_OFFSET)
    ldx #SELECT_SCREEN_TILE_TYPE_ERASE_LENGTH
    stz a:W0 ;; write zeros
    jsr dma_fixed_to_vram
    rts
.endproc


.a16
.i16
.proc select_menu_callback
    ;; if we're pressing A, we've selected a tile,
    ;; and we need to move on to the canvas menu/mode
    ldy #player::joy_trigger
    lda (menu::player), y
    and #JOY_A
    beq end
    lda #.loword(tile_canvas)
    sta a:game_data + game_data::curr_menu
    ;; we need to pass tile canvas the current player
    ;; plus the tile id and tile type of our selection
    lda menu::curr_row
    asl
    tax
    lda select_row_tile_types, x
    tay
    lda menu::curr_row
    asl
    tax
    lda select_row_table_cumul, x
    clc
    adc menu::curr_column
    ldx menu::player
    jsr activate_draw_menu
end:
    ;; update current tile description
    ;; first erase the last msg
    A8
    jsr select_menu_clear_selected_tile_txt
    A16
    lda menu::curr_row
    lshift 4 ;; tile descriptions currently are padded to $10
    clc
    adc select_tile_menu::tile_type_names
    tax
    lda #SELECT_SCREEN_TILE_TYPE_DESC_OFFSET
    jsr print_string_to_offset
    rts
.endproc


.proc init_select_menu
    lda #select_tile_menu
    tcd
    lda #SELECT_SCREEN_X_TABLE_OFFSET
    sta menu::cursor_x_pos
    sta menu::cursor_x_pos_origin
    lda #SELECT_SCREEN_Y_TABLE_OFFSET
    sta menu::cursor_y_pos
    sta menu::cursor_y_pos_origin

    lda select_row_count
    sta menu::no_rows

    lda #.loword(select_row_table)
    sta menu::row_table

    stz menu::curr_row
    stz menu::curr_column

    jsr init_menu_common

    lda #.loword(select_menu_callback)
    sta menu::callback

    lda #.loword(select_row_name)
    sta select_tile_menu::tile_type_names

    lda #.loword(draw_select_screen)
    sta menu::draw_fn

    lda #($4 + ($4 * $10))
    sta menu::caret_sprite

    lda #$0
    tcd
    rts
.endproc


.a8
.i16
.proc draw_select_screen
    force_vblank
    jsr clear_menu_map

    ldy #(select_tile_map_end - select_tile_map)
    sty a:W0
    lda #^select_tile_map
    ldx #.loword(select_tile_map)
    ldy #VRAM_MAP_MENU_BASE
    jsr dma_to_vram

    ;; set background to tiles to transparent, as
    ;; otherwise the transparent parts of the tiles would show
    ;; whatever is occupying the background
    A16
    lda #BG_TILE_PALETTE_ENTRY
    sta a:W0 ;; first tile, aka transparent
    lda #SELECT_SCREEN_DRAW_BEHIND_OFFSET
    clc
    adc #VRAM_MAP_BLOCKS_GAME_BASE
    sta a:W1
    tay
    clc
    adc #SELECT_SCREEN_DRAW_BEHIND_AMOUNT
    sta a:W2
clear_bg_behind_tiles_loop:
    ldx #$2c
    A8
    jsr dma_fixed_to_vram
    A16
    lda a:W1
    clc
    adc #$20
    tay
    sta a:W1
    cmp a:W2
    bmi clear_bg_behind_tiles_loop

    lda #SELECT_SCREEN_HEADER_OFFSET
    ldx #.loword(select_screen_title)
    jsr print_string_to_offset

    lda #SELECT_SCREEN_TILE_TYPE_DESC_HEADER_OFFSET
    ldx #.loword(select_screen_tile_type_desc)
    jsr print_string_to_offset


    ldx a:game_data + game_data::in_game
    bne :+
    lda #(SELECT_SCREEN_LEGEND_MAP_OFFSET + $20)
    ldx #.loword(menu_legend_select_screen)
    jsr print_string_to_offset
    bra :++
  : lda #SELECT_SCREEN_LEGEND_MAP_OFFSET
    ldx #.loword(menu_legend_select_screen_pause_1)
    jsr print_string_to_offset
    lda #(SELECT_SCREEN_LEGEND_MAP_OFFSET + $20)
    ldx #.loword(menu_legend_select_screen_pause_2)
    jsr print_string_to_offset
  : A8

    lda #SELECT_SCREEN_X_COORDS
    ;; this is one of those latching regs
    sta BG1HOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG1HOFS

    lda #SELECT_SCREEN_Y_COORDS
    ;; this is one of those latching regs
    sta BG1VOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG1VOFS
    unforce_vblank
    rts
.endproc


;; ----------------------
;; init between game menu

.a16
.i16
.proc handle_between_edit
    lda menu::player
    ldx #.loword(select_tile_menu)
    ldy #.loword(between_games_menu)
    jsr switch_to_menu
    A8
    jsr hide_menu_carets
    A16
    rts
.endproc


.a16
.i16
.proc handle_between_players
    lda menu::curr_column
    clc
    adc #2
    asl
    sta a:game_data + game_data::no_players_new
    rts
.endproc


.a16
.i16
.proc handle_between_hitpoints
    lda menu::curr_column
    inc
    sta a:game_data + game_data::hitpoints
    rts
.endproc


.a16
.i16
.proc handle_between_wins
    lda menu::curr_column
    inc
    sta a:game_data + game_data::wins
    rts
.endproc


.a16
.i16
.proc handle_between_level
    lda menu::curr_column
    sta a:game_data + game_data::curr_map
    A8
    jsr set_game_map
    force_vblank
    jsr load_tilemap_to_vram
    unforce_vblank
    A16
    rts
.endproc


.a16
.i16
.proc between_menu_handle_row_callback
    ldy #player::joy_trigger
    lda (menu::player), y
    and #JOY_A
    beq end
    ;; We pressed a button. We do different things depending on the row.
    lda ::menu::curr_row

    cmp #between_rows::start
    bne after_start
    jsr reset_wins
    jsr start_new_game
    bra end
after_start:

    cmp #between_rows::edit
    bne after_edit
    jsr handle_between_edit
    bra end
after_edit:

    cmp #::between_rows::players
    bne after_players
    jsr handle_between_players
    bra end
after_players:

    cmp #between_rows::hitpoints
    bne after_hitpoints
    jsr handle_between_hitpoints
    bra end
after_hitpoints:

    cmp #between_rows::wins
    bne after_wins
    jsr handle_between_wins
    bra end
after_wins:

    cmp #between_rows::level
    bne end
    jsr handle_between_level
end:
    rts
.endproc


.a16
.i16
;; I was thinking of generalizing this, but you run into the issue
;; that in our menu framework the amount of rows in a menu are dynamic
;; and most rows in most menus don't want/have selected things to
;; highlight. Plus if you want to make all of this dynamic, you also
;; need to have dynamic sprite allocation, etc.. It's all possible
;; but since we only need this for two rows in one menu right now,
;; we're just gonna roll things by hand, and see at what time
;; it pays off to do the hard work.
.proc between_menu_set_select_carets

    ;; player caret
    lda a:game_data + game_data::no_players_new
    sec
    sbc #4 ;; players menu starts at player 2, so dec by 2 to get to index
    lshift 2 ;; x8 (but players is already x2, so only 2 left shifts)
    clc
    adc #BETWEEN_MENU_X_TABLE_OFFSET
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + oam_entry::x_pos
    A16

    lda #(BETWEEN_MENU_Y_TABLE_OFFSET + (between_rows::players * 8))
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + oam_entry::y_pos
    A16


    ;; hitpoints caret
    lda a:game_data + game_data::hitpoints
    dec
    lshift 3
    clc
    adc #BETWEEN_MENU_X_TABLE_OFFSET
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $4 + oam_entry::x_pos
    A16

    lda #(BETWEEN_MENU_Y_TABLE_OFFSET + (between_rows::hitpoints * 8))
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $4 + oam_entry::y_pos
    A16


    ;; wins caret
    lda a:game_data + game_data::wins
    dec
    lshift 3
    clc
    adc #BETWEEN_MENU_X_TABLE_OFFSET
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $8 + oam_entry::x_pos
    A16

    lda #(BETWEEN_MENU_Y_TABLE_OFFSET + (between_rows::wins * 8))
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $8 + oam_entry::y_pos
    A16


    ;; level caret
    lda a:game_data + game_data::curr_map
    lshift 3
    clc
    adc #BETWEEN_MENU_X_TABLE_OFFSET
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $c + oam_entry::x_pos
    A16


    lda #(BETWEEN_MENU_Y_TABLE_OFFSET + (between_rows::level * 8))
    A8
    sta OAM_MENU_CARET_SELECT_OFFSET + $c + oam_entry::y_pos
    A16

    rts
.endproc


.a16
.i16
.proc between_menu_callback
    jsr between_menu_set_select_carets
    jsr between_menu_handle_row_callback
    rts
.endproc


.a16
.i16
.proc init_between_games_menu
    lda #between_games_menu
    tcd
    lda #BETWEEN_MENU_X_TABLE_OFFSET
    sta menu::cursor_x_pos
    sta menu::cursor_x_pos_origin
    lda #BETWEEN_MENU_Y_TABLE_OFFSET
    sta menu::cursor_y_pos
    sta menu::cursor_y_pos_origin

    lda #BETWEEN_MENU_ROW_COUNT
    sta menu::no_rows

    lda #.loword(between_menu_row_table)
    sta menu::row_table

    stz menu::curr_row
    stz menu::curr_column

    jsr init_menu_common

    lda #.loword(between_menu_callback)
    sta menu::callback

    lda #($0 + ($4 * $10))
    sta menu::caret_sprite

    lda #.loword(draw_between_menu)
    sta menu::draw_fn
    lda #$0
    tcd
    rts
.endproc


.i16
.a16
.proc draw_between_wins
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #BETWEEN_MENU_PLAYER_FONT_MAP_OFFSET
    sta VMADD
    sta a:W5
    ldx #$0

loop:
    ;; first set head
    txa
    lsr
    jsr print_head

    print_pad 1

    lda #.loword(between_wins)
    phy
    jsr print_string
    ply

    print_pad 2

    ;; calculate digits
    lda matches, x
    sta WRDIVL ; and 4205
    lda #$a
    A8
    sta WRDIVB
    A16
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    lda RDDIVL
    print_nr
    lda RDMPYL
    print_nr

    lda a:W5
    clc
    adc #$20
    sta VMADD
    sta a:W5

    inx
    inx
    cpx a:game_data + game_data::no_players
    bne loop
    rts
.endproc

.a16
.i16
.proc draw_between_menu_items
    ;; start
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #BETWEEN_MENU_ITEMS_FONT_MAP_OFFSET
    sta VMADD
    tax

    lda a:game_data + game_data::in_game
    bne restart
    lda #.loword(between_start_str)
    jsr print_string
    bra edit
restart:
    lda #.loword(between_restart_str)
    jsr print_string

edit:
    ;; edit
    txa
    clc
    adc #$20
    sta VMADD
    tax

    lda #.loword(between_edit_str)
    jsr print_string

    ;; players
    txa
    clc
    adc #$20
    sta VMADD
    tax

    lda #.loword(between_players_str)
    jsr print_string

    ;; hitpoints
    txa
    clc
    adc #$20
    sta VMADD
    tax

    lda #.loword(between_hitpoints_str)
    jsr print_string

    ;; wins
    txa
    clc
    adc #$20
    sta VMADD
    tax

    lda #.loword(between_wins_str)
    jsr print_string

    ;; level
    txa
    clc
    adc #$20
    sta VMADD
    tax

    lda #.loword(between_level_str)
    jsr print_string

    rts
.endproc


.a8
.i16
.proc draw_between_menu
    force_vblank
    jsr clear_menu_map
    ldy #(between_menu_map_end - between_menu_map)
    sty a:W0
    lda #^between_menu_map
    ldx #.loword(between_menu_map)
    ldy #VRAM_MAP_MENU_BASE
    jsr dma_to_vram

    lda #BETWEEN_MENU_X_COORDS
    sta BG1HOFS
    stz BG1HOFS

    lda #BETWEEN_MENU_Y_COORDS
    sta BG1VOFS
    stz BG1VOFS

    A16
    jsr draw_between_wins
    jsr draw_between_menu_items

    lda #BETWEEN_MENU_LEGEND_MAP_OFFSET
    ldx a:game_data + game_data::in_game
    beq :+
    ldx #.loword(menu_legend_between_screen_pause)
    bra :++
  : ldx #.loword(menu_legend_between_screen_root)
  : jsr print_string_to_offset
    A8
    unforce_vblank
    rts
.endproc


;; ------------
;; general menu

.a8
.i16
.proc clear_menu_l3
    ldy #(VRAM_MAP_FONT_BASE + ($3 * $20 * 2))
    ldx #($18 * $20 * 2)
    stz a:W0
    jsr dma_fixed_to_vram
    rts
.endproc


.a8
.i16
.proc clear_menu_map
    ldy #VRAM_MAP_MENU_BASE
    ldx #($18 * $20 * 2)
    stz a:W0
    jsr dma_fixed_to_vram
    rts
.endproc


.a16
.i16
.proc init_menu_common
    stz menu::timeout_tick
    stz menu::dirty_p
    stz menu::just_moved
    stz menu::parent_menu
    rts
.endproc


.a16
.i16
.proc init_carets
    A8
    ;; menu selection caret
    lda #$30                ; no flip, prio 3, palette 0
    sta OAM_MENU_CARET_OFFSET + oam_entry::attrs
    sta OAM_MENU_CARET_SELECT_OFFSET + oam_entry::attrs
    sta OAM_MENU_CARET_SELECT_OFFSET + $4 + oam_entry::attrs
    sta OAM_MENU_CARET_SELECT_OFFSET + $8 + oam_entry::attrs
    sta OAM_MENU_CARET_SELECT_OFFSET + $c + oam_entry::attrs


    ;; current value carets aren't going to change tile id for now
    lda #($2 + ($4 * $10))
    sta OAM_MENU_CARET_SELECT_OFFSET + oam_entry::tile_pos
    sta OAM_MENU_CARET_SELECT_OFFSET + $4 + oam_entry::tile_pos
    sta OAM_MENU_CARET_SELECT_OFFSET + $8 + oam_entry::tile_pos
    sta OAM_MENU_CARET_SELECT_OFFSET + $c + oam_entry::tile_pos

    ;; set sprites to 16x16px
    lda #$aa
    sta OAM_MENU_CARET_XTRA_PROPS
    sta OAM_MENU_CARET_XTRA_PROPS + 1
    A16
    rts
.endproc


.a8
.i16
.proc hide_menu_carets
    lda #$F0
    sta OAM_MENU_CARET_OFFSET + oam_entry::y_pos
    sta OAM_MENU_CARET_OFFSET + $4 + oam_entry::y_pos
    sta OAM_MENU_CARET_OFFSET + $8 + oam_entry::y_pos
    sta OAM_MENU_CARET_OFFSET + $c + oam_entry::y_pos
    sta OAM_MENU_CARET_OFFSET + $10 + oam_entry::y_pos
    rts
.endproc


.a16
.i16
.proc set_menu_caret_position
    lda menu::cursor_x_pos
    A8
    sta OAM_MENU_CARET_OFFSET + oam_entry::x_pos
    bmi set_x_neg ;; x pos is negative, so set x pos top bit
    ;; otherwise we clear the x pos top bit
    lda #$fe
    and OAM_MENU_CARET_XTRA_PROPS
    sta OAM_MENU_CARET_XTRA_PROPS
    bra set_y
set_x_neg:
    lda #$1
    ora OAM_MENU_CARET_XTRA_PROPS
    sta OAM_MENU_CARET_XTRA_PROPS
set_y:
    lda menu::cursor_y_pos
    sta OAM_MENU_CARET_OFFSET + oam_entry::y_pos
    A16
    rts
.endproc


;; both check if we can move cursor, and if not, do internal
;; housekeeping on cursor movement
.proc menu_cursor_just_moved
    lda menu::just_moved
    beq end

    lda menu::timeout_tick
    cmp #MENU_MOVE_TIMEOUT
    bcc still_in_timeout ; tick lower than timeout
    stz menu::timeout_tick
    stz menu::just_moved
    rep #$2
    bra end
still_in_timeout:
    inc menu::timeout_tick
    rep #$2
end:
    rts
.endproc


.proc menu_handle_row
    ldy #player::h_tribool
    lda (menu::player), y
    beq end ;; no left/right button pressed
    bmi left
    ;; handling right
    lda menu::curr_row
    asl
    tay
    lda menu::curr_column
    inc a ;; we're comparing against total columns in this row
          ;; which is counted from 1
    cmp (menu::row_table), y
    bpl end
    ;; we can move the caret right, so we do
    inc menu::curr_column
    lda menu::cursor_x_pos
    clc
    adc #$8
    sta menu::cursor_x_pos
    inc menu::dirty_p
    stz menu::timeout_tick
    bra end
left:
    ;; handling left
    lda menu::curr_column
    beq end ;; if zero, we can't move further right
    ;; but else, we move left
    dec menu::curr_column
    lda menu::cursor_x_pos
    sec
    sbc #$8
    sta menu::cursor_x_pos
    inc menu::dirty_p
    stz menu::timeout_tick
end:
    rts
.endproc


.proc menu_handle_column
    ldy #player::v_tribool
    lda (menu::player), y
    beq end ;; no left/right button pressed
    bmi up
    ;; handle column down
    lda menu::curr_row
    inc a ;; we're comparing against total no of rows
    cmp menu::no_rows
    bpl end
    inc menu::curr_row
    lda menu::cursor_y_pos
    clc
    adc #$8
    sta menu::cursor_y_pos
    inc menu::dirty_p
    stz menu::timeout_tick
    bra clamp_column
up:
    ;; handling up
    lda menu::curr_row
    beq end ;; if zero, we can't move further up
    ;; but else, we move up
    dec menu::curr_row
    lda menu::cursor_y_pos
    sec
    sbc #$8
    sta menu::cursor_y_pos
    inc menu::dirty_p
    stz menu::timeout_tick
clamp_column:
    ;; for the new row we're now on, we need to check if we are past
    ;; the bounds of the array, and if so, clamp to the lesser of the 2
    lda menu::curr_row
    asl
    tay
    lda (menu::row_table), y
    dec ;; compare against 0-based array, so minus 1
    cmp menu::curr_column
    bpl end
    sta menu::curr_column
    ;; column times 8 + position of first row = new caret x position:
    asl
    asl
    asl
    clc
    adc menu::cursor_x_pos_origin
    sta menu::cursor_x_pos
end:
    rts
.endproc


;; A - map base
.proc handle_menu
    tcd
    ;; Check for B(ack) button. If pressed we move to previous
    ;; menu, if there is one. And we can skip the menu callback.
    ldy #player::joy_trigger
    lda (menu::player), y
    and #JOY_B
    beq :+

    ldx menu::parent_menu
    beq :+
    lda menu::player
    ldy #$ffff
    jsr switch_to_menu
    bra handle_menu_done

  : jsr menu_cursor_just_moved
    bne handle_menu_done

    jsr menu_handle_row
    jsr menu_handle_column

    jsr set_menu_caret_position
    ;; no direct page indirect jumping, so we need to
    ;; create an absolute address by adding the D register, which
    ;; currently points to the current menu address, to the callback
    ;; offset
    tdc
    clc
    adc #menu::callback
    tax
    jsr (0, x)
after_callback:
    lda menu::dirty_p
    beq handle_menu_done
    inc menu::just_moved
    stz menu::dirty_p

handle_menu_done:
    lda #0
    tcd
    rts
.endproc


.proc handle_current_menu
    lda a:game_data + game_data::curr_menu
    jsr handle_menu
    rts
.endproc


.proc switch_to_fight
    jsr level_screen_to_fore
    lda #.loword(handle_main_loop)
    sta a:game_data + game_data::game_handler
    rts
.endproc


;; A - current player
;; X - address of menu struct
;; Y - address of parent menu struct.
;;     0    = no parent menu, but do write that in parent field
;;     ffff = we're moving back down the menu chain. Don't save
;;            to parent field, because that effectively means
;;            clobbering it
.proc switch_to_menu
    sta a:menu::player, x

    cpy #$ffff
    beq after_parent_save
    tya
    sta a:menu::parent_menu, x
after_parent_save:

    stx a:game_data + game_data::curr_menu

    lda #.loword(handle_current_menu)
    sta a:game_data + game_data::game_handler

    inc a:game_data + game_data::menu_dirty_draw

    rts
.endproc


.a8
.i16
.proc vblank_draw_menu
    lda a:game_data + game_data::menu_dirty_draw
    beq end

    ; set bg 1 and bg 2 char vram base addr.
    ; level and menu share character data
    lda #((VRAM_BLOCKS_CHR_BASE >> 12) << 4 | (VRAM_BLOCKS_CHR_BASE >> 12))
    sta BG12NBA

    lda #((VRAM_MAP_MENU_BASE >> 10) << 2)
    sta BG1SC ; set bg 1 tile map

    lda #((VRAM_MAP_BLOCKS_GAME_BASE >> 10) << 2)
    sta BG2SC ; set bg 2 tile map

    jsr clear_menu_l3
    ldx a:game_data + game_data::curr_menu

    ;; this one doesn't need to be here, but I figured
    ;; good to have everything together
    lda a:menu::caret_sprite, x
    sta OAM_MENU_CARET_OFFSET + oam_entry::tile_pos

    jsr (menu::draw_fn, x)
    stz a:game_data + game_data::menu_dirty_draw
end:
    rts
.endproc


.a16
.i16
.proc level_screen_to_fore
    ;; set map x/y coords back

    A8
    force_vblank
    jsr hide_menu_carets
    jsr clear_menu_l3
    jsr load_tilemap_to_vram

    stz BG1HOFS
    stz BG1HOFS
    lda #BG_V_TOP_ALIGN
    sta BG1VOFS
    stz BG1VOFS

    ; set bg 1 and bg 2 char vram base addr.
    ; level and menu share character data
    lda #((VRAM_BG_CHR_BASE >> 12) << 4 | (VRAM_BLOCKS_CHR_BASE >> 12))
    sta BG12NBA

    lda #((VRAM_MAP_BLOCKS_GAME_BASE >> 10) << 2)
    sta BG1SC ; set bg 1 tile map

    lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
    sta BG2SC ; set bg 2 tile map

    unforce_vblank
    A16

    ;; TODO: set BG2SC to the background of the level
    rts
.endproc


.endif ; MENU_INC
