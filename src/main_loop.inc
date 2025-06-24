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
.proc handle_main_loop
    jsr handle_win
    jsr handle_movement
    jsr handle_sprite_bumps
    jsr handle_player_actions
    jsr finalize_main_loop
    rts
.endproc

.endif ; MAIN_LOOP_INC
