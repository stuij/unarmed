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
    ;; TODO: check for everyone dead
    tyx
    inc wins, x ;; increment wins for player
    ;; go to win/game over menu state
    bra end_game
set_first_player:
    ;; everyone dead, so we set menu control player to 1st
    ldy #0
end_game:
    lda a:player_table, y
    ldx #.loword(between_games_menu)
    ldy #0
    jsr switch_to_menu
    stz a:game_data + game_data::in_game
end:
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
    jsr set_overlay_message
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


.proc init_game_preamble
    lda #.loword(start_game_overlay_text)
    ldx #OVERLAY_START_DURATION
    ldy #OVERLAY_INPUT_MASK
    jsr set_overlay

    lda #.loword(handle_game_preamble)
    sta a:game_data + game_data::game_handler
    rts
.endproc


.proc handle_game_preamble
    lda a:game_data + ::game_data::overlay_counter
    beq game_start
    dec a:game_data + ::game_data::overlay_counter
    bra end
game_start:
    jsr exit_overlay
    lda #.loword(handle_main_loop)
    sta a:game_data + game_data::game_handler
    ;; at some point we need to do this
    lda #1
    sta a:game_data + game_data::in_game
end:
    jsr handle_main_loop
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
