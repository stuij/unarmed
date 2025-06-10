.ifndef COLL_SPRITE_INC
COLL_SPRITE_INC = 1


;; ------------
;; sprite-bumps

.a16
.i16
.proc handle_sprite_bumps
    ldx #$0
    phx
player_loop:
    lda .loword(player_table), x
    tcd
    sta .loword(W5)
    stx .loword(W4)

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
bullet_loop:
    lda .loword(bullet_table), x
    tcd

    ;; don't process if bullet isn't active
    lda sprite::move_state
    bne :+
    jmp bullet_end
    :
    ;; you can't get hit by your own bullets
    lda bullet::player
    cmp .loword(W5)
    bne :+
    jmp bullet_end
    :
    ;; take Y coord of our new bullet pos
    lda (sprite::bbox_fine) ;; y
    clc
    adc sprite::y_new

    ;; is it higer than top y?
    cmp .loword(W1)
    bmi check_old_pos ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp .loword(W3)
    bpl check_old_pos

    ;; check x
    ldy #point::x_off
    lda (sprite::bbox_fine), y ;; x
    clc
    adc sprite::x_new
    ;; is it lefter than left x?
    cmp .loword(W0)
    bmi check_old_pos ;; yes, so no collision
    ;; is it righter than right x?
    cmp .loword(W2)
    bmi bullet_hit

check_old_pos:
    ;; take Y coord of our old bullet
    lda (sprite::bbox_fine) ;; y
    clc
    adc sprite::y_pos

    ;; is it higer than top y?
    cmp .loword(W1)
    bmi check_middle_pos ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp .loword(W3)
    bpl check_middle_pos

    ;; check x
    ldy #point::x_off
    lda (sprite::bbox_fine), y ;; x
    clc
    adc sprite::x_pos
    ;; is it lefter than left x?
    cmp .loword(W0)
    bmi check_middle_pos ;; yes, so no collision
    ;; is it righter than right x?
    cmp .loword(W2)
    bmi bullet_hit


check_middle_pos:
    ;; check half way point between starting pos and end pos
    ;; so first we need to find the halfway point of y
    lda sprite::v_velo
    cmp #0
    bmi y_neg_half
    lsr
    bra y_middle_cont
y_neg_half:
    sec
    ror
y_middle_cont:
    clc
    adc (sprite::bbox_fine)
    adc sprite::y_pos

    ;; is it higer than top y?
    cmp .loword(W1)
    bmi bullet_end ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp .loword(W3)
    bpl bullet_end

    ;; x pos
   lda sprite::h_velo
    cmp #0
    bmi x_neg_half
    lsr
    bra x_middle_cont
x_neg_half:
    sec
    ror
x_middle_cont:
    ldy #point::x_off
    clc
    adc (sprite::bbox_fine), y ;; x
    adc sprite::x_pos
    ;; is it lefter than left x?
    cmp .loword(W0)
    bmi bullet_end ;; yes, so no collision
    ;; is it righter than right x?
    cmp .loword(W2)
    bpl bullet_end


bullet_hit:
    ;; it's a hit!!
    jsr bullet_retire
    ;; play player hit sound
    phx
    A8
    lda #SFX::robot_fires_laser
    jsr Tad_QueueSoundEffect_D
    A16
    plx
    phx
    ldx .loword(W4)
    jsr player_dec_hp
    plx

bullet_end:
    inx
    inx
    cpx #BULLET_TABLE_I
    beq :+
    jmp bullet_loop
    :
    ;; check for player loop things
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    beq :+
    jmp player_loop
  : plx ; clear the stack
    lda #$0
    tcd
    rts
.endproc


.endif ; COLL_SPRITE_INC
