.ifndef COLL_SPRITE_INC
COLL_SPRITE_INC = 1


;; ------------
;; sprite-bumps

.a16
.i16
handle_sprite_bumps:
    ldx #$0
    phx
sprite_bumps_player_loop:
    lda .loword(player_table), x
    tcd

    ldy #square_bbox::top_left + point::x_off
    lda (sprite::bbox_fine), y
    clc
    adc sprite::x_new
    sta .loword(W0)

    ldy #square_bbox::top_left + point::y_off
    lda (sprite::bbox_fine), y
    clc
    adc sprite::y_new
    sta .loword(W1)

    ldy #square_bbox::bottom_right + point::x_off
    lda (sprite::bbox_fine), y
    clc
    adc sprite::x_new
    sta .loword(W2)

    ldy #square_bbox::bottom_right + point::y_off
    lda (sprite::bbox_fine), y
    clc
    adc sprite::y_new
    sta .loword(W3)

    ;; ------------------------------------------
    ;; now we can start comparing with our bullet

    ldx #$0
sprite_bumps_bullet_loop:
    lda .loword(bullet_table), x
    tcd

    ;; don't process if bullet isn't active
    lda sprite::move_state
    beq sprite_bumps_bullet_end
    ;; take Y coord of our bullet
    lda (sprite::bbox_fine) ;; y
    clc
    adc sprite::y_new

    ;; is it higer than top y?
    cmp .loword(W1)
    bmi sprite_bumps_bullet_end ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp .loword(W3)
    bpl sprite_bumps_bullet_end

    ;; check x
    ldy #point::x_off
    lda (sprite::bbox_fine), y ;; x
    clc
    adc sprite::x_new
    ;; is it lefter than left x?
    cmp .loword(W0)
    bmi sprite_bumps_bullet_end ;; yes, so no collision
    ;; is it righter than right x?
    cmp .loword(W2)
    bpl sprite_bumps_bullet_end

    ;; it's a hit!!
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::v_velo

    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::h_velo
sprite_bumps_bullet_end:
    inx
    inx
    cpx #BULLET_TABLE_I
    bne sprite_bumps_bullet_loop

    ;; check for player loop things
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    beq sprite_bumps_player_loop_cont
    jmp sprite_bumps_player_loop
sprite_bumps_player_loop_cont:
    plx ; clear the stack

    lda #$0
    tcd
    rts

.endif ; COLL_SPRITE_INC
