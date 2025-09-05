.ifndef MAIN_LOOP_INC
MAIN_LOOP_INC = 1

.a16
.i16
.proc finalize_main_loop
    jsr finalize_players
    jsr finalize_bullets
    rts
.endproc


.a16
.i16
.proc handle_movement
    jsr handle_player_movement
    jsr handle_bullet_movement
    rts
.endproc


.a16
.i16
.proc handle_win
    ;; we don't want to switch modes if the game is already over
    ;; even though we still want to continue the game logic, as it's fun to see
    ;; things continuing to move in the bg.
    lda a:game_data + game_data::in_game
    beq end
    jsr check_win
    cmp #0
    bne end
    cmp #$ffff
    beq set_first_player ;; skip player win increment. no-one wins. no celebrations.
    ;; It was a win. Winner is in Y
    bra end_game
set_first_player:
    ;; everyone dead, so we set menu control player to 1st
    ldy #0
end_game:
    jsr handle_end_game
end:
    rts
.endproc


;; Y - player nr (x2)
.proc handle_end_game
    tyx
    inc wins, x ;; increment wins for player
    ;; go to win/game over menu state
    jsr init_game_epilogue
    rts
.endproc


;; out: Y - winning player (x2 bytes)
;;          (#$ffff means everyone is dead,
;;           the last two players could have died at same time)
;;      A - game over: 0, not game over: 1
.a16
.i16
.proc check_win
    stz W0     ; dead-count
    ldx #$0    ; iterator
    ldy #$ffff ; we assume everyone is dead
loop:
    lda hp, x
    bne player_alive
    ;; yup, this player is dead
    inc W0
    bra loop_check
player_alive:
    txy
loop_check:
    inx
    inx
    cpx a:game_data + game_data::no_players
    bne loop

    lda a:game_data + game_data::no_players
    lsr
    dec
    dec
    cmp W0
    bcc game_over   ; game is over. Either one player alive or
                    ; everyone dead. See OUT description above.
    lda #1
    bra end
game_over:
    lda #0
end:
    rts
.endproc


;; -----------
;; spawn boxes
;;
;; both used as player startting portals, and as respawn points

.a8
.i16
.proc init_spawn_boxes
    A16
    lda #OAM_SPAWN_BOX_OFFSET
    tcd
    A8
    ldx #0
loop:
        lda #$20                ; no flip, prio 2, palette 0
        sta oam_entry::attrs

        lda #($6 + ($4 * $10))
        sta oam_entry::tile_pos

        A16
        lda player_start_coords, x
        rshift 4
        A8
        sta oam_entry::x_pos

    ;; increment spawn box OAM entry
    A16
    tdc
    clc
    adc #4
    tcd
    A8
    ;; increment player start pos to next x
    inx
    inx
    inx
    inx
    cpx #(4 * 2 * 2) ; 4 players * 2 coords (x and y) * 2 bytes
    bne loop
    A16
    lda #0
    tcd
    A8

    ;; set sprites to 16x16px
    lda #$aa
    sta OAM_SPAWN_BOX_XTRA_PROPS

    jsr show_spawn_boxes
    rts
.endproc


.a8
.i16
.proc hide_spawn_boxes
    lda #$F0
    sta OAM_SPAWN_BOX_OFFSET + oam_entry::y_pos
    sta OAM_SPAWN_BOX_OFFSET + $4 + oam_entry::y_pos
    sta OAM_SPAWN_BOX_OFFSET + $8 + oam_entry::y_pos
    sta OAM_SPAWN_BOX_OFFSET + $c + oam_entry::y_pos
    rts
.endproc


.a8
.i16
.proc show_spawn_boxes
    A16
    lda #OAM_SPAWN_BOX_OFFSET
    tcd
    A8
    ;; start at y pos
    ldx #2
loop:
        A16
        lda player_start_coords, x
        rshift 4
        A8
        sta oam_entry::y_pos
    ;; increment spawn box OAM entry
    A16
    tdc
    clc
    adc #4
    tcd
    A8
    ;; increment player start pos to next x
    inx
    inx
    inx
    inx
    ; 4 players * 2 coords (x and y) * 2 bytes + 1 for y pos
    cpx #((4 * 2 * 2) + 2)
    bne loop
    A16
    lda #0
    tcd
    A8
    rts
.endproc


;; -------
;; overlay

.a16
.i16
; a - asciiz pointer
.proc set_overlay_message
    tax
    A8
    force_vblank
    A16
    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START)
    sta VMADD
    txa
    jsr print_string
    A8
    unforce_vblank
    A16
    rts
.endproc

; a - text
; x - duration
; y - input mask
.a16
.i16
.proc set_overlay
    stx game_data + game_data::overlay_counter
    sty game_data + game_data::input_mask
    rts
.endproc


.proc exit_overlay
    lda #$FFFF
    sta game_data + game_data::input_mask

    ldy #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START)
    ldx #40
    stz a:W0
    A8
    jsr dma_fixed_to_vram
    A16
    rts
.endproc


.proc init_game_prologue
    lda #.loword(start_game_overlay_text)
    jsr set_overlay_message

    ldx #OVERLAY_START_DURATION
    ldy #OVERLAY_INPUT_MASK
    jsr set_overlay

    lda #.loword(handle_game_prologue)
    sta a:game_data + game_data::game_handler
    rts
.endproc


.proc handle_game_prologue
    lda a:game_data + game_data::overlay_counter
    beq game_start
    dec a:game_data + game_data::overlay_counter
    jsr handle_main_loop
    bra end
game_start:
    jsr exit_overlay
    lda #.loword(handle_main_loop)
    sta a:game_data + game_data::game_handler
    lda #1
    sta a:game_data + game_data::in_game
end:
    rts
.endproc


.proc no_winner
    A8
    force_vblank
    A16

    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START + $3)
    sta VMADD
    lda #.loword(end_game_no_winner_text)
    jsr print_string

    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START + $43)
    sta VMADD
    lda #.loword(end_game_no_winner_cont_text)
    jsr print_string

    A8
    unforce_vblank
    A16

    ;; some general overlay things
    ldx #OVERLAY_END_DURATION_GAME
    ldy #OVERLAY_INPUT_MASK
    jsr set_overlay

    lda #.loword(handle_game_epilogue)
    sta a:game_data + game_data::game_handler
    rts
.endproc


;; Y - player nr (x2)
.proc init_game_epilogue
    stz a:game_data + game_data::in_game

    tya
    ;; if Y is negative (#FFFF), it means that there was no winner
    ;; so we need to make that clear
    cmp #$FFFF
    beq no_winner

    lda a:player_table, y
    sta a:game_data + game_data::curr_winner

    lda wins, y
    cmp game_data + game_data::wins
    bcc still_in_match
    ;; otherwise we transition to game over
    ;; but first we increment the matches won counter
    tyx
    lda matches, x
    cmp #99
    bcs reset_match_count
    inc matches, x
    bra after_inc_matches
reset_match_count:
    stz matches, x
after_inc_matches:
    ;; set overlay text
    A8
    force_vblank
    A16
    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START)
    sta VMADD
    tya
    lsr
    jsr print_head
    lda #.loword(end_match_overlay_text)
    jsr print_string

    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START + $3e)
    sta VMADD
    lda #.loword(end_match_fetch_arms_text)
    jsr print_string

    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START + $5e)
    sta VMADD
    lda #.loword(end_match_fetch_arms_cont_text)
    jsr print_string

    A8
    unforce_vblank
    A16

    ;; some general overlay things
    ldx #OVERLAY_END_DURATION_MATCH
    ldy #OVERLAY_INPUT_MASK
    jsr set_overlay

    lda #.loword(handle_match_epilogue)
    sta a:game_data + game_data::game_handler
    rts

still_in_match:
    ;; set overlay text
    A8
    force_vblank
    A16
    lda #(VRAM_MAP_FONT_BASE + OVERLAY_TEXT_START + 2)
    sta VMADD
    tya
    lsr
    jsr print_head
    lda #.loword(end_game_overlay_text)
    jsr print_string
    A8
    unforce_vblank
    A16

    ;; some general overlay things
    ldx #OVERLAY_END_DURATION_GAME
    ldy #OVERLAY_INPUT_MASK
    jsr set_overlay

    lda #.loword(handle_game_epilogue)
    sta a:game_data + game_data::game_handler
    rts
.endproc


.proc handle_game_epilogue
    lda a:game_data + game_data::overlay_counter
    beq end_epilogue
    dec a:game_data + game_data::overlay_counter
    jsr handle_main_loop
    bra end
end_epilogue:
    jsr exit_overlay

    jsr start_new_game
end:
    rts
.endproc


.proc handle_match_epilogue
    lda a:game_data + game_data::overlay_counter
    beq end_epilogue
    dec a:game_data + game_data::overlay_counter
    jsr handle_main_loop
    bra end
end_epilogue:
    jsr exit_overlay

    A8
    lda #Song::menu
    jsr load_song
    A16

    ;; I had it so the winner would get control over the
    ;; menu, but if you're playing by yourself as player 1,
    ;; now you're stuck. Which doesn't really make sense for a 2 player
    ;; game, but as this is a game jam game, this migh happen
    ;; more often than not.
    ;; lda a:game_data + game_data::curr_winner
    lda a:player_table
    ldx #.loword(between_games_menu)
    ldy #0
    jsr switch_to_menu
end:
    rts
.endproc


.a16
.i16
.proc handle_main_loop
    jsr handle_win
    jsr handle_movement
    jsr handle_sprite_bumps
    jsr handle_player_actions
    jsr finalize_main_loop
    rts
.endproc

.endif ; MAIN_LOOP_INC
