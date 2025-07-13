.ifndef COLL_SPRITE_INC
COLL_SPRITE_INC = 1


;; ------------
;; sprite-bumps


.proc check_bullet_points
    ldy #square_bbox::top_left + point::x_off
    lda (3, s), y
    clc
    adc a:W6
    sta a:W0

    ldy #square_bbox::top_left + point::y_off
    lda (3, s), y
    clc
    adc a:W7
    sta a:W1

    ldy #square_bbox::bottom_right + point::x_off
    lda (3, s), y
    clc
    adc a:W6
    sta a:W2

    ldy #square_bbox::bottom_right + point::y_off
    lda (3, s), y
    clc
    adc a:W7
    sta a:W3
    rts
.endproc


.proc check_bullet_hits
   ldx #$0
bullet_loop:
    lda a:bullet_table, x
    tcd

    ;; don't process if bullet isn't active
    lda sprite::move_state
    bne :+
    jmp bullet_end
    :
    ;; you can't get hit by your own bullets, unless rogue is set
    lda bullet::rogue
    bne :+
    lda bullet::player
    cmp a:W5
    bne :+
    jmp bullet_end

    ;; take Y coord of our new bullet pos
  : lda (sprite::bbox_fine) ;; y
    clc
    adc sprite::y_new

    ;; is it higer than top y?
    cmp a:W1
    bmi check_old_pos ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp a:W3
    bpl check_old_pos

    ;; check x
    ldy #point::x_off
    lda (sprite::bbox_fine), y ;; x
    clc
    adc sprite::x_new
    ;; is it lefter than left x?
    cmp a:W0
    bmi check_old_pos ;; yes, so no collision
    ;; is it righter than right x?
    cmp a:W2
    bmi bullet_hit

check_old_pos:
    ;; take Y coord of our old bullet
    lda (sprite::bbox_fine) ;; y
    clc
    adc sprite::y_pos

    ;; is it higer than top y?
    cmp a:W1
    bmi check_middle_pos ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp a:W3
    bpl check_middle_pos

    ;; check x
    ldy #point::x_off
    lda (sprite::bbox_fine), y ;; x
    clc
    adc sprite::x_pos
    ;; is it lefter than left x?
    cmp a:W0
    bmi check_middle_pos ;; yes, so no collision
    ;; is it righter than right x?
    cmp a:W2
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
    cmp a:W1
    bmi bullet_end ;; yes, so no collision
    ;; is it lower than bottom y?
    cmp a:W3
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
    cmp a:W0
    bmi bullet_end ;; yes, so no collision
    ;; is it righter than right x?
    cmp a:W2
    bpl bullet_end


bullet_hit:
    ;; it's a hit!!
    phx
    ldx #0
    jsr (W6, x)
    plx

bullet_end:
    inx
    inx
    cpx a:game_data + game_data::bullets_pp
    beq :+
    jmp bullet_loop
  : rts
.endproc


.proc handle_shield_hit
    ;; W4 == player offset
    ldy a:W4
    ldx a:player_table, y
    stz a:player::parry_timer, x

    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::h_velo

    inc bullet::rogue

    lda a:player::nr, x
    jsr hide_shield
    rts
.endproc


.proc handle_bullet_hit
    ;; W4 == player offset
    ldx a:W4
    ldy a:player_table, x
    lda a:player::hit_timer, y
    bne end
    ;; we're actually hit hit!!
    ;; play player hit sound
    A8
    lda #SFX::robot_fires_laser
    jsr Tad_QueueSoundEffect_D
    A16
    ; set various hit-related bookkeeping things
    lda #HIT_TIMER_LEN
    sta a:player::hit_timer, y
    lda #move_state::hit
    sta a:sprite::move_state, y
    lda sprite::h_velo
    sta a:player::hit_h_velo, y
    lda sprite::v_velo
    sta a:player::hit_v_velo, y
    jsr player_dec_hp
end:
    jsr bullet_retire
    rts
.endproc


.a16
.i16
.proc handle_sprite_bumps
    ldx #$0
    phx
player_loop:
    lda a:player_table, x
    tcd
    sta a:W5
    stx a:W4

    lda sprite::move_state
    cmp #move_state::dead
    bne :+
    jmp next_player

  : ;; check shield collision
    lda player::parry_timer
    cmp #PARRY_COOLDOWN_ZONE
    bcc check_player_collision

    lda #.loword(shield_bbox_fine)
    pha
        jsr calc_shield_x_offset
        sta a:W6
        lda sprite::y_new
        sta a:W7
        jsr check_bullet_points
    pla    

    lda #.loword(handle_shield_hit)
    sta a:W6
    jsr check_bullet_hits
    lda a:W5
    tcd

check_player_collision:
    lda sprite::bbox_fine
    pha
        lda sprite::x_new
        sta a:W6
        lda sprite::y_new
        sta a:W7
        jsr check_bullet_points
    pla

    lda #.loword(handle_bullet_hit)
    sta a:W6
    jsr check_bullet_hits
    ;; tcd is now set to bullet, but next player iteration sets to next player
 
    ;; check for player loop things
next_player:
    plx
    inx
    inx
    phx
    cpx a:game_data + game_data::no_players
    beq :+
    jmp player_loop
  : plx ; clear the stack
    lda #$0
    tcd
    rts
.endproc


.endif ; COLL_SPRITE_INC
