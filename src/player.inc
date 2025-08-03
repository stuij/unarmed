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
.addr hit
.addr recoil
.addr dead

player_start_coords:
.word $200 ; p1 x
.word $220 ; p1 y
.word $d00 ; p2 x
.word $ba0 ; p2 y
.word $e80 ; p3 x
.word $1a0 ; p3 y
.word $300 ; p4 x
.word $b20 ; p4 y


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


shield_bbox_fine:
;; top left
.word $20  ;; y  $0
.word $00  ;; x  $2
;; bottom left
.word $d0  ;; y   $4
.word $00  ;; x   $6
;; top right
.word $20  ;; y   $8
.word $40  ;; x   $a
;; bottom right
.word $d0  ;; y   $c
.word $40  ;; x   $e
;; middle left
.word $80  ;; y   $10
.word $00  ;; x   $12
;; top middle
.word $20  ;; y   $14
.word $20  ;; x   $16
;; bottom middle
.word $d0  ;; y   $18
.word $20  ;; x   $1a
;; middle right
.word $80  ;; y   $1c
.word $40  ;; x   $1e



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


;; -----------------
;; player management

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
    sta a:W0
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
    cpx a:W0
    bne loop
    pla
    rts
.endproc


.a16
.i16
.proc init_players
    A8
    ;; p1
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $3
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_mirror + $13

    ;; p2
    lda #$22                ; no flip, prio 2, palette 1
    sta OAM_mirror + $7
    lda #$22                ; no flip, prio 2, palette 1
    sta OAM_mirror + $17

    ;; p3
    lda #$24                ; no flip, prio 2, palette 2
    sta OAM_mirror + $b
    lda #$24                ; no flip, prio 2, palette 2
    sta OAM_mirror + $1b

    ;; p4
    lda #$26                ; no flip, prio 2, palette 3
    sta OAM_mirror + $f
    lda #$26                ; no flip, prio 2, palette 3
    sta OAM_mirror + $1f


    ; set top bit of x pos for all 4 sprites to 0 so we show them on
    ; screen, and set 8x8 tile.
    ; so a nibble, representing two sprites becomes b0000, aka 0.
    stz OAM_mirror + $200
    stz OAM_mirror + $201
    A16

    ldx #$0
loop:
    lda a:player_table, x ; player struct under x offset
    tcd ; set dp to it

    txa
    lsr
    sta player::nr

    stz sprite::h_velo
    stz sprite::v_velo
    stz player::joy
    stz sprite::anim_tick
    stz player::shot_burst_counter
    stz player::shot_cooldown_timer
    stz player::hit_timer
    stz player::parry_timer
    stz player::shot_delay

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
    cpx a:game_data + game_data::no_players
    bne loop

    lda #0
    tcd
    rts
.endproc


.a16
.i16
.proc set_player_no
    lda game_data + game_data::no_players_new
    sta game_data + game_data::no_players
    tay

    ;; calc and save total bullets
    ldx #0
    lda #BULLETS_PER_PLAYER
    lsr
    jsr mul_ppu
    sta game_data + ::game_data::bullets_pp

    rts
.endproc


.proc hide_players
    lda #$F0
    sta OAM_mirror + ::oam_entry::y_pos
    sta OAM_mirror + ::oam_entry::y_pos + $4
    sta OAM_mirror + ::oam_entry::y_pos + $8
    sta OAM_mirror + ::oam_entry::y_pos + $C
    sta OAM_mirror + ::oam_entry::y_pos + $10
    sta OAM_mirror + ::oam_entry::y_pos + $14
    sta OAM_mirror + ::oam_entry::y_pos + $18
    sta OAM_mirror + ::oam_entry::y_pos + $1C

    rts
.endproc


;; a - player number
.proc set_OAM_player_top_x
    ;; upper part of the 4 players are in OAM $200
    ;; lower part of the 4 players are in OAM $201
    lda #$F000
    sta sprite::y_new

    cmp #0
        bne :+
        lda #%00000001
        bra set_bits
  : cmp #1
        bne :+
        lda #%00000100
        bra set_bits
  : cmp #2
        bne :+
        lda #%00010000
        bra set_bits
  ;; so it must be p4
  :     lda #%01000000

set_bits:
    A8
    ora OAM_mirror + $200
    sta OAM_mirror + $200
    ora OAM_mirror + $201
    sta OAM_mirror + $201
    A16
    rts 
.endproc


;; --------
;; movement

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
    lda player::joy_trigger
    and in_game_keys + igk::jump
    beq :+
    jmp init_jump
  : jmp h_move
.endproc


.proc idle
    ;; are we instructed to jump?
    lda player::joy_trigger
    and in_game_keys + igk::jump
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


.proc hit
    lda player::hit_h_velo
    sta sprite::h_velo
    lda player::hit_v_velo
    sta sprite::v_velo
    lda #move_state::recoil
    sta sprite::move_state
    jmp recoil
.endproc


.proc recoil
    lda sprite::x_pos
    clc
    adc sprite::h_velo
    sta sprite::x_new

    lda sprite::y_pos
    clc
    adc sprite::v_velo
    sta sprite::y_new
    rts
.endproc


.proc dead
    ;; we shouldn't get here
    jsr spinloop_handler
    rts
.endproc

;; We're already running a few timers here and there.
;; And we probably want to unify them here for sanity,
;; although it might also be nice to just have them ticking along
;; close to related code. Let's see..
;;
;; we hold ourselves to the contract that these timers should
;; decrement before any code is run that uses them
.proc handle_misc_player_timers
    lda player::hit_timer
    beq :+
    dec player::hit_timer

  : lda player::parry_timer
    beq end
    dec player::parry_timer
    ;; if we're in cooldown zone, hide shield
    lda player::parry_timer
    cmp #PARRY_COOLDOWN_ZONE
    bcs end
    lda player::nr
    jsr hide_shield
end:
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
    lda a:player_table, x
    tcd ;; remapping dp to player x
    lda sprite::move_state
    cmp #move_state::dead
    beq :+
    jsr handle_misc_player_timers
    jsr handle_single_player_movement
  : plx
    inx
    inx
    phx
    cpx a:game_data + game_data::no_players
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
    lda #0
    A8
    lda current_coll_map, x
    ;; we only need one collision with any of the collision tile properties
    ;; to feel their effects, so we can just OR them for all the relevant points
    A16
    ora COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 2, s
end:
    rts
.endproc


.proc player_coll_end_callback
    lda COLL_STACK_TILE_PROPS + 2, s
    and #tile_props::kill
    beq after_check_kill
    jsr player_handle_kill ;; if so handle kill
after_check_kill:
    lda COLL_STACK_TILE_PROPS + 2, s
    and #tile_props::out_of_bounds
    ;; we're calling out of the fn with this one, and are never coming back
    bne player_handle_out_of_bounds
    lda sprite::move_state ;; only execute
    cmp #move_state::idle
    beq do_ledge_fall
    cmp move_state::run
    beq do_ledge_fall
    bra store_y_new
do_ledge_fall:
    jsr handle_ledge_fall
store_y_new:
    lda COLL_STACK_Y_NEW_TMP + 2, s
    bmi store_x_new
    ;; y collision occured.
    ;; we record new y, and set y velo to 0
    sta sprite::y_new
    lda #0
    sta sprite::v_velo
    lda sprite::move_state
    ;; so when recoil we set move state to jump.
    ;; but we still check if we didn't hit the ground, in which
    ;; case we should change to idle
    cmp #move_state::recoil
    bne :+
    lda #move_state::jump
    sta sprite::move_state
  : lda COLL_STACK_POINT_HIT_GROUND + 2, s
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
    lda sprite::move_state
    cmp #move_state::recoil
    bne end
    lda #move_state::jump
    sta sprite::move_state
end:
    rts
.endproc


.a16
.i16
;; This is another exit out of the fn. My first one in this project.
;; The compiler engineer in me feels dirty.
.proc player_handle_out_of_bounds
    ;; by the power of pseudo rng, choose one of the 4 start coords at random
    jsr prng
    and #$3
    asl
    asl
    tax
    lda player_start_coords, x
    sta sprite::x_new
    inx
    inx
    lda player_start_coords, x
    sta sprite::y_new
    stz sprite::h_velo
    stz sprite::v_velo
    lda #move_state::jump
    sta ::sprite::move_state
    rts
.endproc


.proc handle_ledge_fall
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 4, s
    bit #tile_props::wall
    bne ledge_fall_check_kill_prop ; we didn't fall off the ledge
    ; we  did fall off the ledge. we're now fallling
    lda #0
    sta sprite::v_velo
    sta sprite::anim_tick
    lda #move_state::jump
    sta sprite::move_state
    lda #V_VELO_DEC
    sta sprite::v_velo_dec
ledge_fall_check_kill_prop:
    lda COLL_STACK_POINT_SPRITE_CALLBACK_TMP + 4, s
    bit #tile_props::kill
    beq end
    jsr player_handle_kill
end:
    rts
.endproc


.proc player_handle_kill
    lda player::hit_timer
    bne end
    A8
    lda #SFX::robot_fires_laser
    jsr Tad_QueueSoundEffect_D
    A16
    lda player::nr
    asl
    tax
    jsr player_dec_hp
    lda #HIT_TIMER_LEN
    sta player::hit_timer
end:
    rts
.endproc




;; in - X: player offset
.proc player_dead
    phd
    lda a:player_table, x
    tcd

    lda #move_state::dead
    sta sprite::move_state
    txa
    lsr
    lda #$E00
    sta sprite::y_new

    pld
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
.proc anim_sprite
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


;; Y holds OAM shadow mem position of 4th player byte,
;; so the additional properties byte. Palette is encoded in
;; bits 1, 2 and 3
.proc hit_palette_swap
    ;; first zero out the palette bits
    lda #$F1
    A8
    and OAM_mirror, y
    sta OAM_mirror, y
    A16
    lda ::player::hit_timer
    rshift 4
    and #$1
    bne hit_palette
    ;; we set the player palette
    lda player::nr
    bra set_palette
hit_palette:
    lda #4
set_palette:
    asl
    A8
    ora OAM_mirror, y
    sta OAM_mirror, y
    A16
    rts
.endproc


;; Y holds OAM shadow mem position of third player byte,
;; so the actual sprite address
.proc animate_player_movement
    jsr anim_sprite

    lda player::hit_timer
    beq end
    ;; do for legs
    iny
    jsr hit_palette_swap
    ;; and head
    tya
    sec
    sbc #(4 * 4)
    tay
    jsr hit_palette_swap
    ;; and move back y to original position
    tya
    clc
    adc #((4 * 4) - 1)
    tay
end:
    rts
.endproc


.a16
.i16
;; out: A - x offset
.proc calc_shield_x_offset
    lda player::stance_dir
    beq stance_left
    lda sprite::x_new
    clc
    adc #SHIELD_RIGHT_OFFSET
    bra end
stance_left:
    lda sprite::x_new
    sec
    sbc #SHIELD_LEFT_OFFSET
end:
    rts
.endproc


.a16
.i16
.proc handle_parry_anim
    lda player::nr
    lshift 2
    clc
    adc #OAM_SHIELD_OFFSET
    tay
    ;; we're at our shield entry
    ;; first calculate x
    jsr calc_shield_x_offset
    rshift 4
    A8
    sta a:mem_start, y
    A16
    iny
    lda sprite::y_new
    rshift 4
    A8
    sta a:mem_start, y
    A16

    ; handle shield flip
    iny
    iny ;; we're now at sprite attributes
    A8
    lda a:mem_start, y
    A16
    ldx player::stance_dir
    beq stance_left
    and #$bf ;; mask everything but flip horizontal
    bra save_dir
stance_left:
    ora #$40 ;; or with horizontal flip
save_dir:
    A8
    sta a:mem_start, y
    A16
    rts
.endproc


.a16
.i16
.proc finalize_players
    ldx #$0
    phx
loop:
    lda a:player_table, x
    tcd
    ;; we now know x and y won't change anymore, so lock them into x/y_pos
    ;; also write them to the OAM mirror
    ;; and we figure out what graphic the sprites should get
    ;;
    ;; first the top sprite - nuzzle direction
    txa
    asl
    clc
    adc #PLAYER_OAM_OFFSET
    tay

    lda sprite::x_new
    sta sprite::x_pos
    rshift 4
    sta a:W0
    A8
    sta OAM_mirror, y
    A16
    iny
    lda sprite::y_new
    sta sprite::y_pos
    rshift 4
    sta a:W1
    A8
    sta OAM_mirror, y
    A16
    ;; sprite animation logic here
    ;; which will deal with the two remaining OAM bytes
    iny
    jsr animate_player_shoot_dir

    ;; now bottom sprite - movement
    tya
    dec
    dec
    clc
    adc #$10 ;; 4 players hardwired
    tay
    lda a:W0
    A8
    sta OAM_mirror, y
    A16
    iny
    lda a:W1
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

    lda player::parry_timer
    cmp #PARRY_COOLDOWN_ZONE
    bcc :+
    ;; we're reusing Y here, but it will be refreshed at top of the loop
    jsr handle_parry_anim
  : plx
    inx
    inx
    phx
    cpx a:game_data + game_data::no_players
    bne loop
    plx
    lda #$0
    tcd
    rts
.endproc

;;   x
;; y   a
;;   b
.a16
.i16
.proc set_player_direction
    lda player::joy
    tax
    and in_game_keys + ::igk::face_top_left
    cmp in_game_keys + ::igk::face_top_left
    bne check_down_left
    lda #face_dir::top_left
    sta sprite::face_dir
    bra done
check_down_left:
    txa
    and in_game_keys + ::igk::face_bottom_left
    cmp in_game_keys + ::igk::face_bottom_left
    bne check_up_right
    lda #face_dir::bottom_left
    sta sprite::face_dir
    bra done
check_up_right:
    txa
    and in_game_keys + ::igk::face_top_right
    cmp in_game_keys + ::igk::face_top_right
    bne check_down_right
    lda #face_dir::top_right
    sta sprite::face_dir
    bra done
check_down_right:
    txa
    and in_game_keys + ::igk::face_bottom_right
    cmp in_game_keys + ::igk::face_bottom_right
    bne check_left
    lda #face_dir::bottom_right
    sta sprite::face_dir
    bra done
check_left:
    txa
    and in_game_keys + ::igk::face_left
    beq check_up
    lda #face_dir::left
    sta sprite::face_dir
    bra done
check_up:
    txa
    and in_game_keys + ::igk::face_top
    beq check_down
    lda #face_dir::top
    sta sprite::face_dir
    bra done
check_down:
    txa
    and in_game_keys + ::igk::face_bottom
    beq check_right
    lda #face_dir::bottom
    sta sprite::face_dir
    bra done
check_right:
    txa
    and in_game_keys + ::igk::face_right
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


.proc handle_press_shield
    lda player::parry_timer
    bne end
    lda player::joy_trigger
    and in_game_keys + igk::parry
    beq end
    lda #PARRY_COUNT
    sta player::parry_timer
end:
    rts
.endproc


.a16
.i16
;; out: A: 0 = no shoot, 1 = yes shoot
.proc handle_shot_decision
    ;; first handle burst. if we're bursting, we're also still
    ;; in the cooldown timer
    ;;
    ;; note, atm we're not really doing bursts (multiple bullets in consequetive frames)
    lda player::shot_burst_counter
    bne handle_burst

    ;; shot cooldown timer is counting down. 0 is shoot
    lda player::shot_cooldown_timer
    bne handle_cooldown

    lda player::shot_delay
    beq check_trigger_shot_delay

    ;; so we're prepping to shoot
    dec player::shot_delay
    bne no_shoot
    ;; if shot_delay is now 0, it was 1 before, and we should shoot
    ;; but only if shoot buttons are still firing
    lda player::joy_held
    and in_game_keys + igk::shoot
    beq no_shoot ;; nothing to do

    lda #SHOT_BURST_COUNT
    sta player::shot_burst_counter
    lda #SHOT_COOLDOWN_FRAMES
    sta player::shot_cooldown_timer
    bra yes_shoot

check_trigger_shot_delay:
    lda player::joy_trigger
    and in_game_keys + igk::shoot
    beq no_shoot ;; nothing to do
    lda #SHOT_DELAY_AMOUT
    sta player::shot_delay
    bra no_shoot

handle_burst:
    dec player::shot_burst_counter
    dec player::shot_cooldown_timer
yes_shoot:
    lda #1
    bra end
handle_cooldown:
    dec player::shot_cooldown_timer
    bra no_shoot
no_shoot:
    lda #0
end:
    rts
.endproc


.proc handle_player_shoot
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
    sta a:W0
    ;; y
    txy
    lda (sprite::bbox_fine), y
    clc
    adc sprite::y_new
    sta a:W1

    ;; this one is already in x, but we'll rewrite this
    ;; and I'll have forgotten about this then
    ldx sprite::face_dir
    stx a:W2

    ;; switch dp to bullet
    ldx player::bullets_left
    jsr queue_get_active_item
    beq end
    stx player::bullets_left
    tcd

    lda a:W0
    ldy #point::x_off
    sec
    sbc (sprite::bbox_fine), y
    ;; adc #$10 ;; do we need to add 1 pixel?
    ;; on one hand this way it can't intersect w us when shooting
    ;; on the other hand, it might mess up wall collisions
    sta a:W0

    lda a:W1
    sec
    sbc (sprite::bbox_fine)
    sta a:W1

    ldx a:W2
    jsr (.loword(bullet_fire_direction_table), x)

    jsr bullet_set_for_flight
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
    lda a:player_table, x
    tcd
;; meat
    ;; are we instructed to shoot?
    jsr handle_shot_decision
    beq :+
    jsr handle_player_shoot
  : jsr handle_press_shield
    plx
    inx
    inx
    phx
    cpx a:game_data + game_data::no_players
    bne player_loop
;; epilogue
    plx ; clear the stack
    lda #$0
    tcd
    rts
.endproc


.a16
.i16
.proc set_aim_shooting_separate
    lda #(JOY_L)
    sta in_game_keys + igk::shoot

    lda #(JOY_R)
    sta in_game_keys + igk::jump

    lda #(JOY_L)
    sta in_game_keys + ::igk::face_top_left

    lda #(JOY_B | JOY_Y)
    sta in_game_keys + ::igk::face_bottom_left

    lda #(JOY_X | JOY_A)
    sta in_game_keys + ::igk::face_top_right

    lda #(JOY_B | JOY_A)
    sta in_game_keys + ::igk::face_bottom_right

    lda #(JOY_Y)
    sta in_game_keys + ::igk::face_left

    lda #(JOY_X)
    sta in_game_keys + ::igk::face_top

    lda #(JOY_B)
    sta in_game_keys + ::igk::face_bottom

    lda #(JOY_A)
    sta in_game_keys + ::igk::face_right

    lda #(JOY_L)
    sta in_game_keys + ::igk::parry

    rts
.endproc


.a16
.i16
.proc set_best_keys
    lda #(JOY_A | JOY_B | JOY_X | JOY_Y)
    sta in_game_keys + igk::shoot

    lda #(JOY_R)
    sta in_game_keys + igk::jump

    lda #(JOY_X | JOY_Y)
    sta in_game_keys + ::igk::face_top_left

    lda #(JOY_B | JOY_Y)
    sta in_game_keys + ::igk::face_bottom_left

    lda #(JOY_X | JOY_A)
    sta in_game_keys + ::igk::face_top_right

    lda #(JOY_B | JOY_A)
    sta in_game_keys + ::igk::face_bottom_right

    lda #(JOY_Y)
    sta in_game_keys + ::igk::face_left

    lda #(JOY_X)
    sta in_game_keys + ::igk::face_top

    lda #(JOY_B)
    sta in_game_keys + ::igk::face_bottom

    lda #(JOY_A)
    sta in_game_keys + ::igk::face_right

    lda #(JOY_L)
    sta in_game_keys + ::igk::parry

    rts
.endproc


;; --------
;; parrying

.a8
.i16
.proc init_shields
    A16
    lda #OAM_SHIELD_OFFSET
    tcd
    A8
    ldx #0
loop:
        lda #$20                ; no flip, prio 3, palette 0
        sta oam_entry::attrs

        ;; current value carets aren't going to change tile id for now
        lda #($a + ($4 * $10))
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
    cpx #4 ; 4 shields
    bne loop
    A16
    lda #0
    tcd
    A8

    ;; set sprites to 16x16px
    lda #$aa
    sta OAM_SHIELD_XTRA_PROPS

    rts
.endproc


.a16
.i16
.proc hide_shield
    lshift 2
    clc
    adc #(OAM_SHIELD_OFFSET + 1)
    tay
    lda #$F0
    A8
    sta a:mem_start, y
    A16
    rts
.endproc

.endif ; PLAYER_INC
