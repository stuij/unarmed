.ifndef BULLET_INC
BULLET_INC = 1

.code

;; bullet
;; ------
bullet_table:
.addr .loword(b0)
.addr .loword(b1)
.addr .loword(b2)
.addr .loword(b3)
.addr .loword(b4)
.addr .loword(b5)
.addr .loword(b6)
.addr .loword(b7)
.addr .loword(b8)
.addr .loword(b9)
.addr .loword(b10)
.addr .loword(b11)
.addr .loword(b12)
.addr .loword(b13)
.addr .loword(b14)
.addr .loword(b15)
.addr .loword(b16)
.addr .loword(b17)
.addr .loword(b18)
.addr .loword(b19)
.addr .loword(b20)
.addr .loword(b21)
.addr .loword(b22)
.addr .loword(b23)
.addr .loword(b24)
.addr .loword(b25)
.addr .loword(b26)
.addr .loword(b27)
.addr .loword(b28)
.addr .loword(b29)
.addr .loword(b30)
.addr .loword(b31)


bullet_sprite_vtable:
.addr .loword(bullet_coll_end_callback)
.addr .loword(coll_callback_dummy)
.addr .loword(bullet_coll_point_callback)
.addr .loword(bullet_is_collision_callback)


.a16
.i16
.proc bullet_is_collision_callback
    lda COLL_STACK_POINT_TILE_PROPS + 2, s
    bit #(tile_props::bounce | tile_props::explode)
    rts
.endproc


bullet_bbox_default:
.word $7  ;; y
.word $5  ;; x

bullet_bbox_default_fine:
.word $70 ;; y
.word $50 ;; x


.a8
.i16
.proc init_explosions
    A16
    lda #OAM_EXPLOSION_OFFSET
    tcd
    A8
    ldx #0
loop:
        lda #$2a                ; no flip, prio 3, palette 5
        sta oam_entry::attrs

        ;; current value carets aren't going to change tile id for now
        lda #EXPLOSION_SPRITE_OFFSET
        sta oam_entry::tile_pos

        ;; set them offscreen
        lda #$f0
        sta oam_entry::y_pos
        stz oam_entry::x_pos
    ;; increment OAM entry
    A16
    tdc
    clc
    adc #4
    tcd
    A8
    inx
    cpx #((BULLETS_PER_PLAYER / 2) * 4) ; amount of bullets
    bne loop
    A16
    lda #0
    tcd
    A8

    ;; set  to 16x16px
    lda #$aa
    sta OAM_EXPLOSION_XTRA_PROPS
    sta OAM_EXPLOSION_XTRA_PROPS + $1
    sta OAM_EXPLOSION_XTRA_PROPS + $2
    sta OAM_EXPLOSION_XTRA_PROPS + $3

    rts
.endproc


.a16
.i16
;; game specific
.proc init_bullets
    ldx #$0
    ldy #(OAM_BULLET_OFFSET + $2)
loop:
    lda a:bullet_table, x
    tcd

    txa
    lsr
    sta bullet::nr

    A8
    lda #BULLET_SPRITE_OFFSET
    sta 0, y
    iny
    txa
    ;; giving bullets colors. this is a bit of a hack depending on
    ;; exactly giving 4 bullets per player. too lazy today
    ;; to do this properly
    ;;
    ;; shift to right to get to player nr. then move one to left
    ;; to get into proper palette position
    rshift 3
    asl
    ora #$20                ; no flip, prio 2
    sta 0, y
    iny
    iny
    iny
    A16

    jsr bullet_reset

    lda #BULLET_X_POS_INIT
    sta sprite::x_pos
    sta sprite::x_new

    lda #face_dir::right
    sta sprite::face_dir

    lda #.loword(bullet_bbox_default)
    sta sprite::bbox

    lda #.loword(bullet_bbox_default_fine)
    sta sprite::bbox_fine

    lda #.sizeof(bullet_bbox)
    sta sprite::bbox_size

    lda #.loword(bullet_sprite_vtable)
    sta sprite::vptr

    lda #$0
    sta sprite::anim_tick

    inx
    inx
    cpx a:game_data + game_data::bullets_pp
    bne loop

    lda #0
    tcd

    A8
    ;; handle this blasted table separately
    lda #BULLET_OAM_DEFAULT
    sta OAM_BULLET_XTRA_PROPS
    lda #BULLET_OAM_DEFAULT
    sta OAM_BULLET_XTRA_PROPS + $1
    lda #BULLET_OAM_DEFAULT
    sta OAM_BULLET_XTRA_PROPS + $2
    lda #BULLET_OAM_DEFAULT
    sta OAM_BULLET_XTRA_PROPS + $3
    A16

    rts
.endproc


.a16
.i16
.proc handle_bullet_movement
    ldx #$0
    phx
loop:
    lda a:bullet_table, x
    tcd
    ;; if bullet is inactive, ignore
    lda sprite::move_state
    cmp #bullet_state::fly
    bne loop_end

    ;; check if we've overstayed our welcome
    lda bullet::flight_timer
    cmp #BULLET_MAX_FLIGHT
    bcs retire

    inc bullet::flight_timer

    lda sprite::x_pos
    clc
    adc sprite::h_velo
    sta sprite::x_new

    lda sprite::y_pos
    clc
    adc sprite::v_velo
    sta sprite::y_new

    jsr check_collisions
    bra loop_end
retire:
    jsr bullet_retire
loop_end:
    ;; loop epilogue
    plx
    inx
    inx
    phx
    cpx a:game_data + game_data::bullets_pp
    bne loop
    plx
    lda #$0
    tcd
    rts
.endproc

.proc bullet_coll_point_callback
    ;; currently we're only interested in exploding
    ;; destructible tiles
    ;; If we have already registered a tile,
    ;; we don't need another tile. We're done.
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
    bne end
    ;; check the current point map for destructo
    lda COLL_STACK_POINT_TILE_PROPS + 2, s
    bit #::tile_props::destructable
    beq end

    ;; we've got a live one. let's save
    ;; we currently don't really need tile props info
    ;; but let's see..
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
    lda COLL_STACK_POINT_TILE_OFF + 2, s
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP_2 + 2, s
end:
    rts
.endproc


.a16
.i16
.proc do_bullet_impact
    A8
    lda #SFX::explode
    jsr Tad_QueueSoundEffect_D
    A16

    jsr bullet_set_explosion
    rts
.endproc


.a16
.i16
.proc bullet_coll_end_callback
    lda COLL_STACK_TILE_PROPS + 2, s
    ;; no collision with anything meaningful
    beq end
    ;; either contact with no-bounce wall or out of bounds
    ;; means the bullet disappears
    bit #tile_props::out_of_bounds
    bne bullet_retire
    bit #tile_props::bounce
    ;; no bounce means retire bullet
    bne bounce
    bit #tile_props::explode
    beq end
    jsr do_bullet_impact
    bra check_destructo ;; bullet retired, but might still have done damage
bounce:
    A8
    lda #SFX::bullet_bounce
    jsr Tad_QueueSoundEffect_D
    A16

    lda COLL_STACK_Y_NEW_TMP + 2, s
    bmi flip_v
    ;; x collision occured, save new x.
    sta sprite::y_new
    ;; we flip x speed
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::v_velo
flip_v:
    lda COLL_STACK_X_NEW_TMP + 2, s
    bmi check_destructo
    ;; x collision occured, save new y
    sta sprite::x_new
    ;; we flip y speed
    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::h_velo
check_destructo:
    ;; if this isn't 0, this is the offset that we want
    ;; to modify
    jsr check_destructo_bullet
end:
    rts
.endproc


.proc bullet_set_explosion
    lda #bullet_state::explosion
    sta sprite::move_state

    stz sprite::anim_tick

    stz sprite::h_velo
    stz sprite::v_velo
    rts
.endproc

.proc bullet_retire
    jsr bullet_reset
    ;; set bullet as inactive in queue
    ldx bullet::queue_offset
    ldy bullet::player
    jsr set_player_queue_item_inactive
    rts
.endproc


.proc check_destructo_bullet
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP_2 + 4, s
    beq end
    ;; modify the current collison map
    tax
    lda #0
    A8
    sta current_coll_map, x
    A16
    ;; modify the tile map
    txa
    asl
    tax
    make_forbidden_tile_entry BROKEN_WALL_TILE_OFFSET
    sta current_tile_map, x
end:
    rts
.endproc


.proc bullet_set_for_flight
    lda #bullet_state::fly
    sta sprite::move_state
    rts
.endproc


.proc bullet_reset
    ;; set speed to 0
    stz sprite::h_velo
    stz sprite::v_velo
    stz sprite::anim_tick
    stz bullet::flight_timer
    stz bullet::rogue

    ;; set move state to idle
    lda #bullet_state::idle
    sta sprite::move_state

    lda #BULLET_Y_POS_INIT
    sta sprite::y_pos
    sta sprite::y_new

    rts
.endproc


.proc bounce_bullet
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

    rts
.endproc


;       2
;      piv  bull
;       |    |
; |  |  |  |  |
;  1  2   3  4
;; in:
;; X - queue offset of bullet
;; Y - player bullet belongs to
.proc set_player_queue_item_inactive
    tdc
    pha
    tya
    tcd ;; set DP to player
    ;; we check if the bullet is already in place,
    ;; and if so, we're done
    cpx player::bullets_left
    bcc done ;; this item already is inactive
    beq in_position
    ;; otherwise, we will need to put the pointer to the bullet currently
    ;; residing at the new pivot position at the current position of our
    ;; target bullet, and update it with its new position. And of course
    ;; we put a pointer to our target bullet at the pivot position,
    ;; and update its position as well. As we already have the latter in
    ;; A, we do that one first.
    lda player::bullets_left
    ldy #bullet::queue_offset
    sta ($1, s), y ; write target bullet new queue_offset
    txy
    tax
    lda player::bullet_queue, x
    tyx
    sta player::bullet_queue, x ; write pivot bullet to target bullet spot
    pha
    txa
    ldy #bullet::queue_offset
    sta ($1, s), y ; write it's new queue_offset
    pla
    lda $1, s
    ldx player::bullets_left
    sta player::bullet_queue, x ;; write target bullet to pivot pos
in_position:
    ;; regardless of where our bullet currently is,
    ;; we always move the bullets_left pivot one to the right
    lda player::bullets_left
    inc
    inc
    sta player::bullets_left
done:
    pla
    tcd
    rts
.endproc


.a16
.i16
;; X - bullet table offset
;; Y - OAM bullet offset - set at tile address
.proc anim_bullet
    lda sprite::move_state
    cmp #bullet_state::explosion
    bne end

    ;; we're initializing the explosion
    ;; (setting OAM entry to right sprite)
    lda sprite::anim_tick
    bne after_init

    iny
    lda #$2a
    A8
    sta 0,y
    A16
    dey
after_init:
    lda sprite::anim_tick
    rshift 2
    cmp #EXPLOSION_ANIM_FRAMES
    bcs explosion_done
    clc
    asl
    adc #EXPLOSION_SPRITE_OFFSET
    A8
    sta 0, y
    A16
    inc sprite::anim_tick
    bra end

explosion_done:
    lda #BULLET_SPRITE_OFFSET
    A8
    sta 0, y
    A16
    iny
    tyx
    ldy bullet::player
    lda player::nr, y ;; y will be absolute addressing
    lshift 1
    ora #$20 ; no flip, prio 2
    txy
    A8
    sta 0,y
    A16
    jsr bullet_retire
end:
    rts
.endproc


.a16
.i16
.proc finalize_bullets
    ;; bullets
    ldx #$0
    ldy #OAM_BULLET_OFFSET
bullets_to_oam_loop:
    lda a:bullet_table, x
    tcd

    lda sprite::x_new
    sta sprite::x_pos
    rshift 4
    A8
    sta 0, y
    iny
    A16
    lda sprite::y_new
    sta sprite::y_pos
    rshift 4
    A8
    sta 0, y
    A16
    iny

    phy
    phx
    jsr anim_bullet
    plx
    ply
    iny
    iny

    inx
    inx
    cpx a:game_data + game_data::bullets_pp
    bne bullets_to_oam_loop
    lda #$0
    tcd
    rts
.endproc


bullet_fire_direction_table:
.addr bullet_fire_top_left
.addr bullet_fire_bottom_left
.addr bullet_fire_top_right
.addr bullet_fire_bottom_right
.addr bullet_fire_left
.addr bullet_fire_top
.addr bullet_fire_bottom
.addr bullet_fire_right


.a16
.i16
.proc bullet_fire_top_left
    lda #(BULLET_V_VELO)
    eor #$ffff
    inc
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    eor #$ffff
    inc
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_bottom_left
    lda #(BULLET_V_VELO)
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    eor #$ffff
    inc
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sec
    sbc #BULLET_SHOOT_DOWN_Y_ADJUST
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_top_right
    lda #(BULLET_V_VELO)
    eor #$ffff
    inc
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_bottom_right
    lda #(BULLET_V_VELO)
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sec
    sbc #BULLET_SHOOT_DOWN_Y_ADJUST
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_left
    lda #$0
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    eor #$ffff
    inc
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sec
    sbc #BULLET_SHOOT_SIDE_Y_ADJUST
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_top
    lda #(BULLET_V_VELO)
    eor #$ffff
    inc
    sta sprite::v_velo
    lda #$0
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_bottom
    lda #(BULLET_V_VELO)
    sta sprite::v_velo
    lda #$0
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sta sprite::y_new
    rts
.endproc


.proc bullet_fire_right
    lda #$0
    sta sprite::v_velo
    lda #(BULLET_H_VELO)
    sta sprite::h_velo
    ;; bullet position adjustments
    lda a:W0
    sta sprite::x_new
    lda a:W1
    sec
    sbc #BULLET_SHOOT_SIDE_Y_ADJUST
    sta sprite::y_new
    rts
.endproc

.endif ; BULLET_INC
