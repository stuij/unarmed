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


;; out: Y - winning player (x2 bytes)
;;          (#$ffff means everyone is dead,
;;           the last two players could have died at same time)
;;      A - game over
;; side effects: increment wins, if someone one
.a16
.i16
.proc check_win
    ldy #$ffff
    stz W0  ; dead-count
    ldy #$0 ; last player we saw that is still alive
    ldx #$0 ; iterator
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
    cpx #PLAYER_TABLE_I
    bne loop

    lda #PLAYER_TABLE_I
    lsr
    dec
    dec
    cmp W0
    bcc game_over   ; game is over. Either one player alive or
                    ; everyone dead. See OUT description above.
    lda #0
    bra end
game_over:
    lda #1
    tyx
    inc wins, x
end:
    rts
.endproc


.a16
.i16
.proc handle_main_loop
    jsr check_win
    jsr handle_movement
    jsr handle_sprite_bumps
    jsr handle_player_actions
    jsr finalize_main_loop
    A8
    jsl Tad_Process
    A16
    rts
.endproc

.endif ; MAIN_LOOP_INC
