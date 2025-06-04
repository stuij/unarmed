.ifndef PLAYER_INC
PLAYER_INC = 1

.code

;; -----------
;; player data


player_table:
.addr .loword(p1)
.addr .loword(p2)
.addr .loword(p3)
.addr .loword(p4)


; jump table of player movement states
move_table:
.addr idle
.addr run
.addr jump
.addr cling
.addr climb


player_start_coords:
.word $200 ; p1 x
.word $200 ; p1 y
.word $500 ; p2 x
.word $200 ; p2 y
.word $b00 ; p3 x
.word $200 ; p3 y
.word $900 ; p4 x
.word $700 ; p4 y


player_sprite_vtable:
.addr .loword(player_coll_end_callback)
.addr .loword(player_point_no_coll_callback)


player_bbox_default:
;; top left
.word $2  ;; y   $0
.word $0  ;; x   $2
;; bottom left
.word $d  ;; y   $4
.word $0  ;; x   $6
;; top right
.word $2  ;; y   $8
.word $6  ;; x   $a
;; bottom right
.word $d  ;; y   $c
.word $6  ;; x   $e
;; middle left
.word $8  ;; y   $10
.word $0  ;; x   $12
;; top middle
.word $2  ;; y   $14
.word $3  ;; x   $16
;; bottom middle
.word $d  ;; y   $18
.word $3  ;; x   $1a
;; middle right
.word $8  ;; y   $1c
.word $6  ;; x   $1e

player_bbox_default_fine:
;; top left
.word $20  ;; y  $0
.word $00  ;; x  $2
;; bottom left
.word $d0  ;; y   $4
.word $00  ;; x   $6
;; top right
.word $20  ;; y   $8
.word $60  ;; x   $a
;; bottom right
.word $d0  ;; y   $c
.word $60  ;; x   $e
;; middle left
.word $80  ;; y   $10
.word $00  ;; x   $12
;; top middle
.word $20  ;; y   $14
.word $30  ;; x   $16
;; bottom middle
.word $d0  ;; y   $18
.word $30  ;; x   $1a
;; middle right
.word $80  ;; y   $1c
.word $60  ;; x   $1e


;; for this point, do we need to test if it's on
;; a ledge, yes or no?
player_bbox_default_ledge_lookup:
.word 0 ; top left
.word 1 ; bottom left
.word 0 ; top right
.word 1 ; bottom right
.word 0 ; middle left
.word 0 ; top middle
.word 1 ; bottom middle
.word 0 ; middle right


;; X - current player offset times 2
;; D - current player mem loc
.a16
.i16
.proc set_player_bullet_queue
    ;; calc first bullet offset from bullet table
    ;; and then copy over the addresses of all the bullets
    ;; that have been assigned to the player
    ;; Then save the player address in all the bullet
    ;; instances assigned to the player.
    txa
    A8
    sta M7A
    stz M7A
    lda #BULLETS_PER_PLAYER
    lsr
    sta M7B
    A16
    lda MPYL
    clc
    adc #.loword(bullet_table)
    ;; so now we have the address of the bullet table entry of
    ;; first bullet for this player in A
    tax
    tdc
    clc
    adc #player::bullet_queue
    tay
    lda #BULLETS_PER_PLAYER
    dec
    mvn $80, $00
    lda #BULLETS_PER_PLAYER
    sta player::bullets_left
    ;; load player in bullets
    sta .loword(W0)
    ldx #0
    phx ;; we just need a stack slot here
loop:
    ;; save current bullet address on stack
    ;; so we can use indirect access on one of its members
    lda player::bullet_queue, x
    sta 1, s
    ;; save current player in bullet
    ldy #bullet::player
    tdc
    sta (1, s), y
    ;; save bullet queue  offset in bullet
    ldy #bullet::queue_offset
    txa
    sta (1, s), y
    inx
    inx
    cpx .loword(W0)
    bne loop
    pla
    rts
.endproc


.a16
.i16
.proc init_players
    ;; p1
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $3
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $7

    ;; p2
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $b
    lda #$20                ; tile offset of third sprite
    sta OAM_mirror + $e

    ;; p3
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $13
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $17

    ;; p4
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $1b
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $1e


    ; set top bit of x pos for all 4 sprites to 0 so we show them on
    ; screen, and set 8x8 tile.
    ; so a nibble, representing two sprites becomes b0000, aka 0.
    lda #$00
    sta OAM_mirror + $200


    ldx #$0
loop:
    lda .loword(player_table), x ; player struct under x offset
    tcd ; set dp to it

    txa
    lsr
    sta player::nr

    lda #$0
    sta sprite::h_velo
    sta sprite::v_velo
    sta player::joy
    sta sprite::anim_tick

    phx
    jsr set_player_bullet_queue
    plx

    ; p1 start position
    ; $80 pixel offset and $0 subpixels
    txa
    asl
    tax
    ldy player_start_coords, x
    sty sprite::x_pos
    sty sprite::x_new
    inx
    inx
    ldy player_start_coords, x
    sty sprite::y_pos
    sty sprite::y_new
    lsr
    tax

    ldy #move_state::idle
    sty sprite::move_state

    ldy #face_dir::right
    sty sprite::face_dir

    ldy #.loword(player_bbox_default)
    sty sprite::bbox

    ldy #.loword(player_bbox_default_fine)
    sty sprite::bbox_fine

    ldy #.sizeof(player_bbox)
    sty sprite::bbox_size

    ldy #.loword(player_bbox_default_ledge_lookup)
    sty player::bbox_ledge_lookup

    ldy #.loword(player_sprite_vtable)
    sty sprite::vptr

    txa
    inc
    inc
    tax
    cpx #PLAYER_TABLE_I
    bne loop

    lda #0
    tcd
    rts
.endproc


.a16
.i16
.proc jump
    ;; first do horizontal movement logic shared with running
    jsr h_move
    ;; decrease velocity
    lda sprite::v_velo
    ;; 65816 doesn't suppport signed compare without extra steps, so..
    ;; if velo is negative, we for sure need to increase velo
    bmi add_velo
    ;; now we can do unsigned compare
    cmp #V_VELO_DOWN_MAX
    bcs after_velo_add ;; we're at max down velocity, so skip velo increase
add_velo:
    clc ; carry set means we're not borrowing
    adc sprite::v_velo_dec
    sta sprite::v_velo
after_velo_add:
    ;; change player pos based on velocity
    lda sprite::y_pos
    clc
    adc sprite::v_velo ; at some point this will go negative,
                       ; which is excellent as that means we're going down now
    sta sprite::y_new
    rts
.endproc


.proc init_jump
    lda #V_VELO_INIT
    sta sprite::v_velo
    lda #V_VELO_DEC
    sta sprite::v_velo_dec
    lda #$0
    sta sprite::anim_tick
    lda #move_state::jump
    sta sprite::move_state
    asl ;; times 2 to get proper fn offset
    tax
    jmp (.loword(move_table), x)
.endproc


.proc h_move
    lda player::h_tribool
    beq no_push ;; we've got momentum, but we are not actually pushing a button
    bmi push_left ; we're pushing left on direction pad

    ;; we're pushing right
    lda #stance_dir::right
    sta player::stance_dir
    lda sprite::h_velo
    bmi push_right_move_left
    ;; we push right and we move right
    cmp #H_VELO_MAX
    bcs handle_velo ;; no velo change. velo is bigger, so we hit our max
    clc
    adc #H_VELO_INC
    sta sprite::h_velo
    bra handle_velo

push_right_move_left:
    clc
    adc #H_VELO_INC_OPPOSITE
    sta sprite::h_velo
    bra handle_velo

    ;; cpu doesn't do signed compare, so we need to do some work
    ;; - if velo is positive, we can just decrease. we're not hitting
    ;;   we can't be hitting max velo
    ;; - if velo is negative, we invert, and compare to H_MAX_VELO
push_left:
    lda #stance_dir::left
    sta player::stance_dir
    lda sprite::h_velo
    beq push_left_move_left
    bpl push_left_move_right
push_left_move_left:
    eor #$FFFF
    clc
    adc #1
    cmp #H_VELO_MAX
    bcs handle_velo ;; no velo change. velo is bigger, so we hit our max
    ;; otherwise increase velo
    lda sprite::h_velo
    sec
    sbc #H_VELO_INC
    sta sprite::h_velo
    bra handle_velo
push_left_move_right:
    ;; when pushing opposite dir of h_move, we want to decrease velo extra
    sec
    sbc #H_VELO_INC_OPPOSITE
    sta sprite::h_velo
    bra handle_velo
no_push:
    lda sprite::h_velo
    bmi left_no_push
    ;; so we're h_movening right
    sec
    sbc #H_VELO_INC_RELAX
    ;; but if we overshoot, we should snap to 0
    bmi snap_to_zero
    sta sprite::h_velo
    bra handle_velo

left_no_push:
    clc
    adc #H_VELO_INC_RELAX
    ;; but if we overshoot, we should snap to 0
    bpl snap_to_zero
    sta sprite::h_velo
    bra handle_velo

snap_to_zero:
    lda #0
    sta sprite::h_velo

handle_velo:
    ;; if velo is 0, we should state-change to idle, otherwise add velo to x_pos
    ldx sprite::h_velo
    beq set_to_idle
    lda sprite::x_pos
    clc
    adc sprite::h_velo
    sta sprite::x_new
    bra end

set_to_idle:
    ;; when we reach zero we want to set ourself to idle
    ;; state, but only when we are running. not when we're in the air
    lda sprite::move_state
    cmp #move_state::run
    bne end
    lda #move_state::idle
    sta sprite::move_state
    lda #$0
    sta sprite::anim_tick
end:
    rts
.endproc


.proc init_run
    lda #$0
    sta sprite::h_velo
    sta sprite::anim_tick
    lda #move_state::run
    sta sprite::move_state
    asl ;; times 2 to get proper fn offset
    tax
    jmp (.loword(move_table), x)
.endproc


.proc run
    ;; are we instructed to jump?
    lda player::joy_trigger_held
    and #JOY_B
    beq :+
    jmp init_jump
  : jmp h_move
.endproc



.proc idle
    ;; are we instructed to jump?
    lda player::joy_trigger_held
    and #JOY_B
    beq test_run
    jmp init_jump
test_run:
    ;; are we moving left or right
    lda player::h_tribool
    bne init_run ; we pressed left or right, so we want to run
    jmp still_idle ;; zero set, no match
    ;; state change to jump
still_idle:
    ;; new x/y is old x/y
    lda sprite::x_pos
    sta sprite::x_new
    lda sprite::y_pos
    sta sprite::y_new
    rts
.endproc


.proc cling
    rts
.endproc


.proc climb
    rts
.endproc


.a16
.i16
.proc handle_single_player_movement
    jsr set_player_direction
    lda sprite::move_state
    asl
    tax
    jsr (.loword(move_table), x)

    jsr check_collisions
    rts
.endproc


.proc handle_player_movement
    ;; loop over player movement
    ldx #$0
    phx
loop:
    lda .loword(player_table), x
    tcd ;; remapping dp to player x
    jsr handle_single_player_movement
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    bne loop
    plx ; clear the stack
    lda #$0
    tcd
    rts
.endproc


.proc player_point_no_coll_callback
    ;; we're ledge checking
    lda sprite::move_state ;; only check when we're idle or running
    cmp #move_state::idle
    beq ledge_check
    cmp #move_state::run
    bne end
ledge_check:
    tyx
    tya
    lsr ; shift point offset to find offset into ledge table
    lsr
    asl
    tay
    lda (player::bbox_ledge_lookup), y
    txy ; restore Y
    cmp #1
    bne end
    ; ok, we are interested in a ledge check
    lda COLL_STACK_POINT_TILE_OFF + 2, s
    clc
    adc #$20 ;; set to tile under point
    tax
    A8
    lda current_coll_map, x
    A16
    beq end ; not a collision, so we move on
    ;; it's a collision, so we register it
    ;; we only need one collision to know that we're standing with at least
    ;; one point on the ledge, so we can just write 1 whenever we find
    ;; something
    lda #1
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
end:
    rts
.endproc


.proc player_coll_end_callback
    lda COLL_STACK_TILE_PROPS + 2, s
    and #$8 ;; is equal to kill (should add vars for these)
    bne player_handle_kill ;; if so handle kill
    lda sprite::move_state ;; only execute
    cmp #move_state::idle
    beq ledge_fall
    cmp #move_state::run
    bne store_y_new
ledge_fall:
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
    bne store_y_new ; we didn't fall off the ledge
    ; we  did fall off the ledge. we're now fallling
    lda #0
    sta sprite::v_velo
    sta sprite::anim_tick
    lda #move_state::jump
    sta sprite::move_state
    lda #V_VELO_DEC
    sta sprite::v_velo_dec
store_y_new:
    lda COLL_STACK_Y_NEW_TMP + 2, s
    bmi store_x_new
    ;; y collision occured.
    ;; we record new y, and set y velo to 0
    sta sprite::y_new
    lda #0
    sta sprite::v_velo
    lda COLL_STACK_POINT_HIT_GROUND + 2, s
    beq store_x_new
    lda #move_state::idle
    sta sprite::move_state
    lda #$0
    sta sprite::anim_tick
store_x_new:
    lda COLL_STACK_X_NEW_TMP + 2, s
    bmi end
    ;; x collision occured.
    ;; we record new x, and set x velo to 0
    sta sprite::x_new
    lda #0
    sta sprite::h_velo
end:
    rts
.endproc


;; This is another exit out of the fn. My first one in this project.
;; The compiler engineer in me feels dirty.
.proc player_handle_kill
    lda player::nr
    asl
    asl
    tax
    lda player_start_coords, x
    sta sprite::x_new
    inx
    inx
    lda player_start_coords, x
    sta sprite::y_new

    rts
.endproc


;; ----------------
;; sprite animation

;; To not have to deal with an explosion of animations,
;; we're using two sprites to animate different types of things.
;; The upper sprite animates the shooting direction.
;; The lower sprite animates movement.


.a16
.i16
;; Y holds OAM shadow mem position of third player byte,
;; so the actual sprite address
.proc animate_player_shoot_dir
    lda sprite::face_dir
    lsr
    clc
    adc #PLAYER_OAM_SHOOT_DIR_BASE
    A8
    sta OAM_mirror, y
    A16
    rts
.endproc


.a16
.i16
;; Y holds OAM shadow mem position of third player byte,
;; so the actual sprite address
;;
;; First we need to know what move state we're in.
;; Currently we only have animation for running,
;; For the other states we just use the first
;; frame of the running state.
.proc animate_player_movement
    lda sprite::move_state
    cmp #move_state::run
    bne other
    lda sprite::anim_tick
    rshift 3
    and #$3
    ldx player::stance_dir
    bne set_sprite
    clc
    adc #$4
    bra set_sprite
other:
    lda #$1
    ldx player::stance_dir
    bne set_sprite
    clc
    adc #$4
set_sprite:
    clc
    adc #PLAYER_OAM_RUN_ANIM_BASE
    A8
    sta OAM_mirror, y
    A16
    rts
.endproc


.a16
.i16
.proc finalize_players
    ldx #$0
    ldy #PLAYER_OAM_OFFSET
    phx
loop:
    lda .loword(player_table), x
    tcd
    ;; we now know x and y won't change anymore, so lock them into x/y_pos
    ;; also write them to the OAM mirror
    ;; and we figure out what graphic the sprites should get
    ;;
    ;; first the top sprite - nuzzle direction
    lda sprite::x_new
    sta sprite::x_pos
    rshift 4
    sta .loword(W0)
    A8
    sta OAM_mirror, y
    A16
    iny
    lda sprite::y_new
    sta sprite::y_pos
    rshift 4
    sta .loword(W1)
    A8
    sta OAM_mirror, y
    A16
    ;; sprite animation logic here
    ;; which will deal with the two remaining OAM bytes
    iny
    jsr animate_player_shoot_dir
    iny
    iny
    ;; now bottom sprite - movement
    lda .loword(W0)
    A8
    sta OAM_mirror, y
    A16
    iny
    lda .loword(W1)
    clc
    adc #$8
    A8
    sta OAM_mirror, y
    A16
    iny
    jsr animate_player_movement
    iny
    iny
    ;; increment the anim tick, after all uses
    inc sprite::anim_tick
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    bne loop
    plx
    lda #$0
    tcd
    rts
.endproc


.a16
.i16
.proc set_player_direction
    lda player::joy
    tax
    and #(JOY_UP + JOY_LEFT)
    cmp #(JOY_UP + JOY_LEFT)
    bne check_down_left
    lda #face_dir::top_left
    sta sprite::face_dir
    bra done
check_down_left:
    txa
    and #(JOY_DOWN + JOY_LEFT)
    cmp #(JOY_DOWN + JOY_LEFT)
    bne check_up_right
    lda #face_dir::bottom_left
    sta sprite::face_dir
    bra done
check_up_right:
    txa
    and #(JOY_UP + JOY_RIGHT)
    cmp #(JOY_UP + JOY_RIGHT)
    bne check_down_right
    lda #face_dir::top_right
    sta sprite::face_dir
    bra done
check_down_right:
    txa
    and #(JOY_DOWN + JOY_RIGHT)
    cmp #(JOY_DOWN + JOY_RIGHT)
    bne check_left
    lda #face_dir::bottom_right
    sta sprite::face_dir
    bra done
check_left:
    txa
    and #JOY_LEFT
    beq check_up
    lda #face_dir::left
    sta sprite::face_dir
    bra done
check_up:
    txa
    and #JOY_UP
    beq check_down
    lda #face_dir::top
    sta sprite::face_dir
    bra done
check_down:
    txa
    and #JOY_DOWN
    beq check_right
    lda #face_dir::bottom
    sta sprite::face_dir
    bra done
check_right:
    txa
    and #JOY_RIGHT
    beq done
    lda #face_dir::right
    sta sprite::face_dir
done:
    ;; no more possible directions pressed, so we're keeping the
    ;; previous direction
    rts
.endproc


;; -------
;; actions

;; in:
;; W3 - start address of queue
;; X - pivot
;; queue items are 16-bit
;;
;; out:
;; X - new pivot
;; A - item pulled from the queue
.a16
.i16
.proc queue_get_active_item
    cpx #$0
    bne active_item
    lda #$0
    bra end
active_item:
    dex
    dex
    lda player::bullet_queue, x
end:
    rts
.endproc


.a16
.i16
.proc handle_player_actions
;; prologue
    ldx #$0
    phx
player_loop:
    lda .loword(player_table), x
    tcd
;; meat
    ;; are we instructed to shoot?
    lda player::joy_trigger
    and #JOY_A
    beq end
    ;; we are shooting something!
    ;; save left x and y
    ;; x
    lda sprite::face_dir
    asl
    tax
    clc
    adc #point::x_off
    tay
    lda (sprite::bbox_fine), y
    clc
    adc sprite::x_new
    sta .loword(W0)
    ;; y
    txy
    lda (sprite::bbox_fine), y
    clc
    adc sprite::y_new
    sta .loword(W1)

    ;; this one is already in x, but we'll rewrite this
    ;; and I'll have forgotten about this then
    ldx sprite::face_dir
    stx .loword(W2)

    ;; switch dp to bullet
    ldx player::bullets_left
    jsr queue_get_active_item
    beq end
    stx player::bullets_left
    tcd

    lda .loword(W0)
    ldy #point::x_off
    sec
    sbc (sprite::bbox_fine), y
    ;; adc #$10 ;; do we need to add 1 pixel?
    ;; on one hand this way it can't intersect w us when shooting
    ;; on the other hand, it might mess up wall collisions
    sta .loword(W0)

    lda .loword(W1)
    sec
    sbc (sprite::bbox_fine)
    sta .loword(W1)

    ldx .loword(W2)
    jsr (.loword(bullet_fire_direction_table), x)

    jsr bullet_set_for_flight
end:
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    bne player_loop
;; epilogue
    plx ; clear the stack
    lda #$0
    tcd
    rts
.endproc

.endif ; PLAYER_INC