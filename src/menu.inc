.ifndef MENU_INC
MENU_INC = 1

.code

;; ---------
;; draw menu

.a16
.i16
canvas_calc_tile_under_cursor:
    lda menu::curr_row
    ;; row x 32
    asl
    asl
    asl
    asl
    asl
    adc menu::curr_column
    rts


canvas_set_tile:
    ldy #player::joy_trigger_held
    lda (menu::player), y
    and #JOY_A
    beq canvas_set_tile_end
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

canvas_set_tile_end:
    rts


;; patch tile map in vram where cursor currently is hovering
canvas_patch_chosen_tile:
    jsr canvas_calc_tile_under_cursor
    clc
    adc #VRAM_MAP_BG_GAME_BASE
    sta VMADD
    lda canvas_menu::chosen_tile
    sta VMDATA
    rts


.a16
.i16
draw_tile_menu_callback:
    jsr canvas_set_tile
    A8
    jsr load_tilemap_to_vram
    A16
    jsr canvas_patch_chosen_tile
    rts


init_draw_menu:
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

    stz menu::timeout_tick
    stz menu::dirty_p
    stz menu::just_moved

    lda #.loword(draw_tile_menu_callback)
    sta menu::callback

    lda #$0
    tcd
    rts


;; initialize chosen tile map entry with idx selected in select menu
; A - tile idx of chosen tile
draw_init_chosen_tile:
    ;; as it happens the idx is essentially already the selected
    ;; tile map entry, as currently we're using bank 1, horizontal
    ;; and vertical flip aren't set yet, and it's an 8px tile.
    ;; but adjust for tile map entries being 16-bit
    sta tile_canvas + canvas_menu::chosen_tile
    rts


; A - chosen tile ID
; X - current player
; Y - chosen tile type
activate_draw_menu:
    ;; We might just want to store the actual tile here.
    ;; At 2 bytes it's big enough.
    stx tile_canvas + menu::player
    sty tile_canvas + canvas_menu::tile_type
    jsr draw_init_chosen_tile
    jsr level_screen_to_fore
    inc tile_canvas + menu::just_moved
    rts


;; -----------
;; select menu

select_menu_callback:
    ;; if we're pressing A, we've selected a tile,
    ;; and we need to move on to the canvas menu/mode
    ldy #player::joy_trigger
    lda (menu::player), y
    and #JOY_A
    beq select_menu_callback_end
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
select_menu_callback_end:
    rts


init_select_menu:
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

    lda #$0
    sta menu::curr_row
    sta menu::curr_column
    sta menu::timeout_tick
    sta menu::dirty_p
    sta menu::just_moved

    lda #.loword(select_menu_callback)
    sta menu::callback

    lda #.loword(select_row_name)
    sta select_tile_menu::tile_type_names

    lda #$0
    tcd
    rts


.a8
.i16
draw_select_screen:
    ldy #(select_tile_map_end - select_tile_map)
    sty .loword(W0)
    lda #^select_tile_map
    ldx #.loword(select_tile_map)
    ldy #VRAM_MAP_BG_TILE_SELECT_BASE
    jsr dma_to_vram

    ;; set background to tiles to transparent, as
    ;; otherwise the transparent parts of the tiles would show
    ;; whatever is occupying the background
    A16
    stz .loword(W0) ;; first tile, aka transparent
    lda #($7 * $20 + $5)
    clc
    adc #VRAM_MAP_BG_GAME_BASE
    sta .loword(W1)
    tay
    clc
    adc #($6 * $20)
    sta .loword(W2)
draw_select_clear_bg_behind_tiles_loop:
    ldx #$30
    A8
    jsr dma_fixed_to_vram
    A16
    lda .loword(W1)
    clc
    adc #$20
    tay
    sta .loword(W1)
    cmp .loword(W2)
    bmi draw_select_clear_bg_behind_tiles_loop
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


;; ------------
;; general menu
.a16
.i16
init_caret:
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


.a8
.i16
hide_menu_caret:
    lda #$F0
    sta OAM_MENU_CARET_OFFSET + oam_entry::y_pos
    rts


.a16
.i16
set_menu_caret_position:
    lda menu::cursor_x_pos
    A8
    sta OAM_MENU_CARET_OFFSET + oam_entry::x_pos
    bmi caret_pos_set_x_neg ;; x pos is negative, so set x pos top bit
    ;; otherwise we clear the x pos top bit
    lda #$fe
    and OAM_MENU_CARET_XTRA_PROPS
    sta OAM_MENU_CARET_XTRA_PROPS
    bra caret_pos_set_y
caret_pos_set_x_neg:
    lda #$1
    ora OAM_MENU_CARET_XTRA_PROPS
    sta OAM_MENU_CARET_XTRA_PROPS
caret_pos_set_y:
    lda menu::cursor_y_pos
    sta OAM_MENU_CARET_OFFSET + oam_entry::y_pos
    A16
    rts


;; both check if we can move cursor, and if not, do internal
;; housekeeping on cursor movement
menu_cursor_just_moved:
    lda menu::just_moved
    beq menu_check_move_cursor_ret

    lda menu::timeout_tick
    cmp #MENU_MOVE_TIMEOUT
    bcc menu_move_cursor_still_in_timeout ; tick lower than timeout
    stz menu::timeout_tick
    stz menu::just_moved
    rep #$2
    bra menu_check_move_cursor_ret
menu_move_cursor_still_in_timeout:
    inc menu::timeout_tick
    rep #$2
menu_check_move_cursor_ret:
    rts


menu_handle_row:
    ldy #player::h_tribool
    lda (menu::player), y
    beq menu_handle_row_done ;; no left/right button pressed
    bmi menu_handle_row_left
    ;; handling right
    lda menu::curr_row
    asl
    tay
    lda menu::curr_column
    inc a ;; we're comparing against total columns in this row
          ;; which is counted from 1
    cmp (menu::row_table), y
    bpl menu_handle_row_done
    ;; we can move the caret right, so we do
    inc menu::curr_column
    lda menu::cursor_x_pos
    clc
    adc #$8
    sta menu::cursor_x_pos
    inc menu::dirty_p
    stz menu::timeout_tick
    bra menu_handle_row_done
menu_handle_row_left:
    ;; handling left
    lda menu::curr_column
    beq menu_handle_row_done ;; if zero, we can't move further right
    ;; but else, we move left
    dec menu::curr_column
    lda menu::cursor_x_pos
    sec
    sbc #$8
    sta menu::cursor_x_pos
    inc menu::dirty_p
    stz menu::timeout_tick
menu_handle_row_done:
    rts


menu_handle_column:
    ldy #player::v_tribool
    lda (menu::player), y
    beq menu_handle_column_done ;; no left/right button pressed
    bmi menu_handle_column_up
    ;; handle column down
    lda menu::curr_row
    inc a ;; we're comparing against total no of rows
    cmp menu::no_rows
    bpl menu_handle_column_done
    inc menu::curr_row
    lda menu::cursor_y_pos
    clc
    adc #$8
    sta menu::cursor_y_pos
    inc menu::dirty_p
    stz menu::timeout_tick
    bra menu_handle_column_clamp_column
menu_handle_column_up:
    ;; handling up
    lda menu::curr_row
    beq menu_handle_row_done ;; if zero, we can't move further up
    ;; but else, we move up
    dec menu::curr_row
    lda menu::cursor_y_pos
    sec
    sbc #$8
    sta menu::cursor_y_pos
    inc menu::dirty_p
    stz menu::timeout_tick
menu_handle_column_clamp_column:
    ;; for the new row we're now on, we need to check if we are past
    ;; the bounds of the array, and if so, clamp to the lesser of the 2
    lda menu::curr_row
    asl
    tay
    lda (menu::row_table), y
    dec ;; compare against 0-based array, so minus 1
    cmp menu::curr_column
    bpl menu_handle_column_done
    sta menu::curr_column
    ;; column times 8 + position of first row = new caret x position:
    asl
    asl
    asl
    clc
    adc menu::cursor_x_pos_origin
    sta menu::cursor_x_pos

menu_handle_column_done:
    rts


;; A - map base
handle_menu:
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


handle_current_menu:
    lda game_data + game_data::curr_menu
    jsr handle_menu
    rts


switch_to_select_tile_menu:
    A8
    lda #((VRAM_MAP_BG_TILE_SELECT_BASE >> 10) << 2)
    sta BG1SC ; set bg 2 tile map

    lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
    sta BG2SC ; set bg 1 tile map

    jsr draw_select_screen
    A16

    rts


switch_to_canvas_menu:
    rts


level_screen_to_fore:
    ;; set map x/y coords back
    A8
    jsr hide_menu_caret

    lda #$0
    sta BG1HOFS
    stz BG1HOFS
    lda #$FF
    sta BG1VOFS
    stz BG1VOFS
    A16

    lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
    sta BG1SC ; set bg 1 tile map

    A8
    jsr load_tilemap_to_vram
    A16
    ;; lda #((VRAM_MAP_BG_TILE_SELECT_BASE >> 10) << 2)
    ;; sta BG2SC ; set bg 2 tile map
    rts

.endif ; MENU_INC