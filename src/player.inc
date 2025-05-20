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


.a16
.i16
init_players:
    ;; p1
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $3
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $7

    ;; p2
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $b
    lda #$10                ; tile offset of third sprite
    sta OAM_MIRROR + $e

    ;; p3
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $13
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $17

    ;; p4
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $1b
    lda #$10                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $1e


    ; set top bit of x pos for all 4 sprites to 0 so we show them on
    ; screen, and set 8x8 tile.
    ; so a nibble, representing two sprites becomes b0000, aka 0.
    lda #$00
    sta OAM_MIRROR + $200


    ldx #$0
player_init_loop:
    lda .loword(player_table), x ; player struct under x offset
    tcd ; set dp to it

    lda #$0
    sta sprite::h_velo
    sta sprite::v_velo
    sta player::joy
    sta sprite::anim_tick

    ;; nice and hacky right now
    ;; at some point this will hold some sensible structure
    ;; say a list of bullets
    lda .loword(bullet_table), x
    sta player::bullets

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
    bne player_init_loop

    lda #0
    tcd
    rts


.a16
.i16
jump:
    ;; first do horizontal movement logic shared with running
    jsr h_move
    ;; decrease velocity
    lda sprite::v_velo
    ;; 65816 doesn't suppport signed compare without extra steps, so..
    ;; if velo is negative, we for sure need to increase velo
    bmi jump_add_velo
    ;; now we can do unsigned compare
    cmp #V_VELO_DOWN_MAX
    bcs jump_after_velo_add ;; we're at max down velocity, so skip velo increase
jump_add_velo:
    clc ; carry set means we're not borrowing
    adc sprite::v_velo_dec
    sta sprite::v_velo
jump_after_velo_add:
    ;; change player pos based on velocity
    lda sprite::y_pos
    clc
    adc sprite::v_velo ; at some point this will go negative,
                       ; which is excellent as that means we're going down now
    sta sprite::y_new
    rts


init_jump:
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

h_move:
    lda player::h_tribool
    beq h_move_no_push ;; we've got momentum, but we are not actually pushing a button
    bmi h_move_push_left ; we're pushing left on direction pad

    ;; we're pushing right
    lda #stance_dir::right
    sta player::stance_dir
    lda sprite::h_velo
    bmi h_move_push_right_move_left
    ;; we push right and we move right
    cmp #H_VELO_MAX
    bcs h_move_handle_velo ;; no velo change. velo is bigger, so we hit our max
    clc
    adc #H_VELO_INC
    sta sprite::h_velo
    bra h_move_handle_velo

h_move_push_right_move_left:
    clc
    adc #H_VELO_INC_OPPOSITE
    sta sprite::h_velo
    bra h_move_handle_velo

    ;; cpu doesn't do signed compare, so we need to do some work
    ;; - if velo is positive, we can just decrease. we're not hitting
    ;;   we can't be hitting max velo
    ;; - if velo is negative, we invert, and compare to H_MAX_VELO
h_move_push_left:
    lda #stance_dir::left
    sta player::stance_dir
    lda sprite::h_velo
    beq h_move_push_left_move_left
    bpl h_move_push_left_move_right
h_move_push_left_move_left:
    eor #$FFFF
    clc
    adc #1
    cmp #H_VELO_MAX
    bcs h_move_handle_velo ;; no velo change. velo is bigger, so we hit our max
    ;; otherwise increase velo
    lda sprite::h_velo
    sec
    sbc #H_VELO_INC
    sta sprite::h_velo
    bra h_move_handle_velo
h_move_push_left_move_right:
    ;; when pushing opposite dir of h_move, we want to decrease velo extra
    sec
    sbc #H_VELO_INC_OPPOSITE
    sta sprite::h_velo
    bra h_move_handle_velo
h_move_no_push:
    lda sprite::h_velo
    bmi h_move_left_no_push
    ;; so we're h_movening right
    sec
    sbc #H_VELO_INC_RELAX
    ;; but if we overshoot, we should snap to 0
    bmi h_move_snap_to_zero
    sta sprite::h_velo
    bra h_move_handle_velo

h_move_left_no_push:
    clc
    adc #H_VELO_INC_RELAX
    ;; but if we overshoot, we should snap to 0
    bpl h_move_snap_to_zero
    sta sprite::h_velo
    bra h_move_handle_velo

h_move_snap_to_zero:
    lda #0
    sta sprite::h_velo

h_move_handle_velo:
    ;; if velo is 0, we should state-change to idle, otherwise add velo to x_pos
    ldx sprite::h_velo
    beq h_move_set_to_idle
    lda sprite::x_pos
    clc
    adc sprite::h_velo
    sta sprite::x_new
    bra h_move_end
h_move_set_to_idle:
    ;; when we reach zero we want to set ourself to idle
    ;; state, but only when we are running. not when we're in the air
    lda sprite::move_state
    cmp #move_state::run
    bne h_move_end
    lda #move_state::idle
    sta sprite::move_state
    lda #$0
    sta sprite::anim_tick
h_move_end:
    rts


init_run:
    lda #$0
    sta sprite::h_velo
    sta sprite::anim_tick
    lda #move_state::run
    sta sprite::move_state
    asl ;; times 2 to get proper fn offset
    tax
    jmp (.loword(move_table), x)


run:
    ;; are we instructed to jump?
    lda player::joy_trigger_held
    and #JOY_B
    beq run_eval_h_move
    jmp init_jump
run_eval_h_move:
    jmp h_move


idle:
    ;; are we instructed to jump?
    lda player::joy_trigger_held
    and #JOY_B
    beq idle_test_run
    jmp init_jump
idle_test_run:
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


cling:
    rts


climb:
    rts


.a16
.i16
handle_single_player_movement:
    jsr set_player_direction
    lda sprite::move_state
    asl
    tax
    jsr (.loword(move_table), x)

    jsr check_collisions
    rts


handle_player_movement:
    ;; loop over player movement
    ldx #$0
    phx
player_movement_loop:
    lda .loword(player_table), x
    tcd ;; remapping dp to player x
    jsr handle_single_player_movement
    plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    bne player_movement_loop
    plx ; clear the stack
    lda #$0
    tcd
    rts


player_point_no_coll_callback:
    ;; we're ledge checking
    lda sprite::move_state ;; only check when we're idle or running
    cmp #move_state::idle
    beq player_point_no_call_ledge_check
    cmp #move_state::run
    bne player_point_no_call_end
player_point_no_call_ledge_check:
    tyx
    tya
    lsr ; shift point offset to find offset into ledge table
    lsr
    asl
    tay
    lda (player::bbox_ledge_lookup), y
    txy ; restore Y
    cmp #1
    bne player_point_no_call_end
    ; ok, we are interested in a ledge check
    lda COLL_STACK_POINT_TILE_OFF + 2, s
    clc
    adc #$20 ;; set to tile under point
    tax
    A8
    lda CURRENT_COLL_MAP, x
    A16
    beq player_point_no_call_end ; not a collision, so we move on
    ;; it's a collision, so we register it
    ;; we only need one collision to know that we're standing with at least
    ;; one point on the ledge, so we can just write 1 whenever we find
    ;; something
    lda #1
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
player_point_no_call_end:
    rts


player_coll_end_callback:
    lda sprite::move_state ;; only execute
    cmp #move_state::idle
    beq player_coll_ledge_fall
    cmp #move_state::run
    bne player_coll_store_y_new
player_coll_ledge_fall:
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
    bne player_coll_store_y_new ; we didn't fall off the ledge
    ; we  did fall off the ledge. we're now fallling
    lda #0
    sta sprite::v_velo
    sta sprite::anim_tick
    lda #move_state::jump
    sta sprite::move_state
    lda #V_VELO_DEC
    sta sprite::v_velo_dec
player_coll_store_y_new:
    lda COLL_STACK_Y_NEW_TMP + 2, s
    bmi player_coll_store_x_new
    ;; y collision occured.
    ;; we record new y, and set y velo to 0
    sta sprite::y_new
    lda #0
    sta sprite::v_velo
    lda COLL_STACK_POINT_HIT_GROUND + 2, s
    beq player_coll_store_x_new
    lda #move_state::idle
    sta sprite::move_state
    lda #$0
    sta sprite::anim_tick

player_coll_store_x_new:
    lda COLL_STACK_X_NEW_TMP + 2, s
    bmi player_coll_end_callback_end
    ;; x collision occured.
    ;; we record new x, and set x velo to 0
    sta sprite::x_new
    lda #0
    sta sprite::h_velo
player_coll_end_callback_end:
    rts


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
animate_player_shoot_dir:
    lda sprite::face_dir
    lsr
    clc
    adc #PLAYER_OAM_SHOOT_DIR_BASE
    A8
    sta OAM_MIRROR, y
    A16
    rts


.a16
.i16
;; Y holds OAM shadow mem position of third player byte,
;; so the actual sprite address
;;
;; First we need to know what move state we're in.
;; Currently we only have animation for running,
;; For the other states we just use the first
;; frame of the running state.
animate_player_movement:
    lda sprite::move_state
    cmp #move_state::run
    bne animate_player_movement_other
    lda sprite::anim_tick
    rshift 3
    and #$3
    ldx player::stance_dir
    bne animate_player_movement_set_sprite
    clc
    adc #$4
    bra animate_player_movement_set_sprite
animate_player_movement_other:
    lda #$1
    ldx player::stance_dir
    bne animate_player_movement_set_sprite
    clc
    adc #$4
animate_player_movement_set_sprite:
    clc
    adc #PLAYER_OAM_RUN_ANIM_BASE
    A8
    sta OAM_MIRROR, y
    A16
    rts


.a16
.i16
finalize_players:
    ldx #$0
    ldy #PLAYER_OAM_OFFSET
    phx
finalize_player_loop:
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
    sta OAM_MIRROR, y
    A16
    iny
    lda sprite::y_new
    sta sprite::y_pos
    rshift 4
    sta .loword(W1)
    A8
    sta OAM_MIRROR, y
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
    sta OAM_MIRROR, y
    A16
    iny
    lda .loword(W1)
    clc
    adc #$8
    A8
    sta OAM_MIRROR, y
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
    bne finalize_player_loop
    plx
    lda #$0
    tcd
    rts


.a16
.i16
set_player_direction:
    lda player::joy
    tax
    and #(JOY_UP + JOY_LEFT)
    cmp #(JOY_UP + JOY_LEFT)
    bne set_player_direction_check_down_left
    lda #face_dir::top_left
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_down_left:
    txa
    and #(JOY_DOWN + JOY_LEFT)
    cmp #(JOY_DOWN + JOY_LEFT)
    bne set_player_direction_check_up_right
    lda #face_dir::bottom_left
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_up_right:
    txa
    and #(JOY_UP + JOY_RIGHT)
    cmp #(JOY_UP + JOY_RIGHT)
    bne set_player_direction_check_down_right
    lda #face_dir::top_right
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_down_right:
    txa
    and #(JOY_DOWN + JOY_RIGHT)
    cmp #(JOY_DOWN + JOY_RIGHT)
    bne set_player_direction_check_left
    lda #face_dir::bottom_right
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_left:
    txa
    and #JOY_LEFT
    beq set_player_direction_check_up
    lda #face_dir::left
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_up:
    txa
    and #JOY_UP
    beq set_player_direction_check_down
    lda #face_dir::top
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_down:
    txa
    and #JOY_DOWN
    beq set_player_direction_check_right
    lda #face_dir::bottom
    sta sprite::face_dir
    bra set_player_direction_done
set_player_direction_check_right:
    txa
    and #JOY_RIGHT
    beq set_player_direction_done
    lda #face_dir::right
    sta sprite::face_dir
set_player_direction_done:
    ;; no more possible directions pressed, so we're keeping the
    ;; previous direction
    rts


;; -------
;; actions

.a16
.i16
handle_player_actions:
;; prologue
    ldx #$0
    phx
actions_player_loop:
    lda .loword(player_table), x
    tcd
;; meat
    ;; are we instructed to shoot?
    lda player::joy_trigger
    and #JOY_A
    beq :+
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
    stx .loword(W3)

    ;; switch dp to bullet
    lda player::bullets
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

    ldx .loword(W3)
    jsr (.loword(bullet_fire_direction_table), x)

    lda #bullet_state::fly
    sta sprite::move_state
    lda #$0
    sta sprite::anim_tick

  : plx
    inx
    inx
    phx
    cpx #PLAYER_TABLE_I
    bne actions_player_loop
;; epilogue
    plx ; clear the stack
    lda #$0
    tcd
    rts

.endif ; PLAYER_INC