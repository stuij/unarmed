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
    sta current_coll_map, x
    A16
    tya
    asl
    tax
    lda canvas_menu::chosen_tile
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
    adc #VRAM_MAP_BG_GAME_BASE
    sta VMADD
    lda canvas_menu::chosen_tile
    sta VMDATA
    rts
.endproc


.a16
.i16
.proc draw_tile_menu_callback
    jsr canvas_set_tile
    A8
    jsr load_tilemap_to_vram
    A16
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

    lda #.loword(draw_between_menu)
    sta menu::draw_fn

    lda #$0
    tcd
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
    jsr level_screen_to_fore
    inc tile_canvas + menu::just_moved
    rts
.endproc


;; -----------
;; select menu

.proc select_menu_callback
    ;; if we're pressing A, we've selected a tile,
    ;; and we need to move on to the canvas menu/mode
    ldy #player::joy_trigger
    lda (menu::player), y
    and #JOY_A
    beq end
    lda #.loword(tile_canvas)
    sta game_data + game_data::curr_menu
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

    lda #$0
    tcd
    rts
.endproc


.a8
.i16
.proc clear_menu_map
    ldy #VRAM_MAP_MENU_BASE
    ldx #($20 * $18 * 2)
    stz a:W0
    jsr dma_fixed_to_vram
    rts
.endproc


.a8
.i16
.proc clear_menu_l3
    ldy #(VRAM_MAP_FONT_BASE + ($20 * 3 * 2))
    ldx #($20 * $18 * 2)
    stz a:W0
    jsr dma_fixed_to_vram
    rts
.endproc


.a8
.i16
.proc draw_select_screen
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
    lda #($7 * $20 + $5)
    clc
    adc #VRAM_MAP_BG_GAME_BASE
    sta a:W1
    tay
    clc
    adc #($6 * $20)
    sta a:W2
clear_bg_behind_tiles_loop:
    ldx #$30
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
    A8

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
    rts
.endproc


;; ----------------------
;; init between game menu

.proc between_menu_callback
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
    ;; print text
    print_pad 2

    lda #.loword(between_wins)
    phy
    jsr print_string
    ply

    print_pad 2

    lda wins, x
    print_nr

    lda a:W5
    clc
    adc #$20
    sta VMADD
    sta a:W5

    inx
    inx
    cpx #PLAYER_TABLE_I
    beq :+
    jmp loop
    :
    rts
.endproc


.a8
.i16
.proc draw_between_menu
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
    A8

    rts
.endproc


;; ------------
;; general menu

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
.proc init_caret
    lda #SELECT_CARET_SPRITE_OFFSET
    sta OAM_MENU_CARET_OFFSET + oam_entry::tile_pos
    lda #$30                ; no flip, prio 3, palette 0
    sta OAM_MENU_CARET_OFFSET + oam_entry::attrs

    ;; set sprite to 16x16px
    lda #$2
    A8
    sta OAM_MENU_CARET_XTRA_PROPS
    A16
    rts
.endproc


.a8
.i16
.proc hide_menu_caret
    lda #$F0
    sta OAM_MENU_CARET_OFFSET + oam_entry::y_pos
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

    jsr menu_cursor_just_moved
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
    lda game_data + game_data::curr_menu
    jsr handle_menu
    rts
.endproc

;; A - current player
;; X - address of menu struct
;; Y - address of parent menu struct
.proc switch_to_menu
    sta a:menu::player, x
    tya
    sta a:menu::parent_menu, x
    stx game_data + game_data::curr_menu

    A8
        lda #((VRAM_MAP_MENU_BASE >> 10) << 2)
        sta BG1SC ; set bg 1 tile map

        lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
        sta BG2SC ; set bg 2 tile map

        jsr (menu::draw_fn, x)
    A16
    rts
.endproc


.proc level_screen_to_fore
    ;; set map x/y coords back

    A8
    jsr hide_menu_caret
    jsr clear_menu_l3

    stz BG1HOFS
    stz BG1HOFS
    lda #$FF
    sta BG1VOFS
    stz BG1VOFS
    A16

    lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
    sta BG1SC ; set bg 1 tile map

    ;; TODO: set BG2SC to the background of the level

    A8
    jsr load_tilemap_to_vram
    A16
    rts
.endproc


.endif ; MENU_INC
