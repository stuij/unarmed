.p816   ; 65816 processor
.smart

.define ROM_NAME "demo" ; max 21 chars

.include "defines.inc"
.include "header.inc"
.include "macros.inc"
.include "io.inc"

.include "init.inc"
.include "lib.inc"
.include "vectors.inc"
.include "../build/assets/audio.inc"
.include "../terrific-audio-driver/audio-driver/ca65-api/tad-audio.inc"
.include "assets.inc"
.include "data.inc"

.code

;; ---------------
;; entity specific


;; player data
;; ------

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
.word $1  ;; y   $0
.word $3  ;; x   $2
;; bottom left
.word $f  ;; y   $4
.word $3  ;; x   $6
;; top right
.word $1  ;; y   $8
.word $d  ;; x   $a
;; bottom right
.word $f  ;; y   $1c
.word $d  ;; x   $1e
;; middle left
.word $8  ;; y   $20
.word $3  ;; x   $22
;; top middle
.word $1  ;; y   $24
.word $8  ;; x   $26
;; bottom middle
.word $f  ;; y   $28
.word $8  ;; x   $2a
;; middle right
.word $8  ;; y   $2c
.word $d  ;; x   $2e

player_bbox_default_fine:
;; top left
.word $10  ;; y  $0
.word $30  ;; x  $2
;; bottom left
.word $f0  ;; y   $4
.word $30  ;; x   $6
;; top right
.word $10  ;; y   $8
.word $d0  ;; x   $a
;; bottom right
.word $f0  ;; y   $c
.word $d0  ;; x   $e



;; for this point, do we need to test if it's on
;; a ledge, yes or no?
player_bbox_default_ledge_lookup:
.word 0 ; top left
.word 0 ; middle left
.word 1 ; bottom left
.word 0 ; top middle
.word 1 ; bottom middle
.word 0 ; top right
.word 0 ; middle right
.word 1 ; bottom right


.a16
.i16
init_players:
    ;; set oam for sprite 1 directly.
    ;; this and the above sprite init data should all be done with some
    ;; kind of generalized player sprite init routine
    lda #$00                ; tile offset of first sprite
    sta OAM_MIRROR + 2
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + 3

    lda #$00                ; tile offset of second sprite
    sta OAM_MIRROR + 6
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + 7

    lda #$00                ; tile offset of third sprite
    sta OAM_MIRROR + $a
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + $b

    ; lda #$00                ; tile offset of fourth sprite
    ; sta OAM_MIRROR + $e
    ; lda #$20                ; no flip, prio 2, palette 0
    ; sta OAM_MIRROR + $f


    ; set top bit of x pos for all 4 sprites to 0 so we show them on
    ; screen, and set 16x16 tile
    ; so a nibble, representing two sprites becomes 1010, aka a.
    lda #$aa
    sta OAM_MIRROR + $200


    ldx #$0
    ;; ldx is still 0
player_init_loop:
    lda .loword(player_table), x ; player struct under x offset
    tcd ; set dp to it

    ldy #$0
    sty sprite::h_velo
    sty sprite::v_velo
    sty player::joy

    ; p1 start position
    ; $80 pixel offset and $0 subpixels

    txa
    asl
    tax
    ldy player_start_coords, x
    sty sprite::x_pos
    inx
    inx
    ldy player_start_coords, x
    sty sprite::y_pos
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
h_move_end:
    rts


init_run:
    lda #0
    sta sprite::h_velo
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
    lda town_coll, x
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


.a16
.i16
finalize_players:
    ;; for shame, make this more fancy

    ;; we now know x and y won't change anymore, so lock them into x/y_pos
    ;; also write them to the OAM mirror
    lda p1 + sprite::x_new
    sta p1 + sprite::x_pos
    rshift 4
    A8
    sta OAM_MIRROR
    A16
    lda p1 + sprite::y_new
    sta p1 + sprite::y_pos
    rshift 4
    A8
    sta OAM_MIRROR + 1

    A16
    lda p2 + sprite::x_new
    sta p2 + sprite::x_pos
    rshift 4
    A8
    sta OAM_MIRROR + 4
    A16
    lda p2 + sprite::y_new
    sta p2 + sprite::y_pos
    rshift 4
    A8
    sta OAM_MIRROR + 5

    A16
    lda p3 + sprite::x_new
    sta p3 + sprite::x_pos
    rshift 4
    A8
    sta OAM_MIRROR + 8
    A16
    lda p3 + sprite::y_new
    sta p3 + sprite::y_pos
    rshift 4
    A8
    sta OAM_MIRROR + 9
    A16
    ; lda p4 + sprite::x_new
    ; sta p4 + sprite::x_pos
    ; rshift 4
    ; A8
    ; sta OAM_MIRROR + $c
    ; A16
    ; lda p4 + sprite::y_new
    ; sta p4 + sprite::y_pos
    ; rshift 4
    ; A8
    ; sta OAM_MIRROR + $d
    rts

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
.addr .loword(bullet_point_no_coll_callback)


bullet_bbox_default:
.word $2  ;; y
.word $2  ;; x

bullet_bbox_default_fine:
.word $20 ;; y
.word $20 ;; x

.a16
.i16
;; game specific
init_bullets:
    ldx #$150
    stx .loword(W0)
    ldx #$0
    ldy #$12
init_bullets_loop:
    lda .loword(bullet_table), x
    tcd

    lda #$04                ; tile offset of second sprite
    sta OAM_MIRROR, y
    iny
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR, y
    iny
    iny
    iny

    lda #BULLET_H_VELO
    sta sprite::h_velo

    txa
    clc
    adc #10
    lsr
    sta sprite::v_velo

    ; p1 start position
    ; $80 pixel offset and $0 subpixels

    lda .loword(W0)
    sta sprite::x_pos

    lda #$100
    sta sprite::y_pos

    lda #bullet_state::fly
    sta sprite::move_state

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

    lda .loword(W0)
    clc
    adc #$60
    sta .loword(W0)
    inx
    inx
    cpx #BULLET_TABLE_I
    bne init_bullets_loop

    lda #0
    tcd

    ;; handle this blasted table separately
    lda #$00
    sta OAM_MIRROR + $201
    lda #$00
    sta OAM_MIRROR + $202
    lda #$00
    sta OAM_MIRROR + $203
    lda #$00
    sta OAM_MIRROR + $204
    lda #$00
    sta OAM_MIRROR + $205
    lda #$00
    sta OAM_MIRROR + $206
    lda #$00
    sta OAM_MIRROR + $207
    lda #$00
    sta OAM_MIRROR + $208

    rts


.a16
.i16
handle_bullet_movement:
    ldx #$0
    phx
handle_bullet_loop:
    lda .loword(bullet_table), x
    tcd
    lda sprite::x_pos
    clc
    adc sprite::h_velo
    sta sprite::x_new

    lda sprite::y_pos
    clc
    adc sprite::v_velo
    sta sprite::y_new

    jsr check_collisions
    plx
    inx
    inx
    phx
    cpx #BULLET_TABLE_I
    bne handle_bullet_loop
    plx
    lda #$0
    tcd
    rts


bullet_point_no_coll_callback:
    rts


bullet_coll_end_callback:
bullet_coll_end_flip_h:
    lda COLL_STACK_Y_NEW_TMP + 2, s
    bmi bullet_coll_end_flip_v
    ;; x collision occured, save new x.
    sta sprite::y_new
    ;; we flip x speed
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::v_velo
    bra bullet_coll_end_callback_end
bullet_coll_end_flip_v:
    lda COLL_STACK_X_NEW_TMP + 2, s
    bmi bullet_coll_end_callback_end
    ;; x collision occured, save new y
    sta sprite::x_new
    ;; we flip y speed
    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #$1
    sta sprite::h_velo
bullet_coll_end_callback_end:
    rts


finalize_bullets:
    ;; bullets
    ldx #$0
    ldy #$10;; here's where we track where in OAM we need to put vals
bullets_to_oam_loop:
    lda .loword(bullet_table), x
    tcd
    lda sprite::x_new
    sta sprite::x_pos
    rshift 4
    A8
    sta OAM_MIRROR, y
    iny
    A16
    lda sprite::y_new
    sta sprite::y_pos
    rshift 4
    A8
    sta OAM_MIRROR, y
    A16
    iny
    iny
    iny

    inx
    inx
    cpx #BULLET_TABLE_I
    bne bullets_to_oam_loop
    lda #$0
    tcd
    rts


;; ----
;; init

.a8
.i16
init_binary_data:
    ; let's copy over some binary data
    ;; first the palette
    ldy #(town_palette_end - town_palette) ; # of palette entries
    sty <W0
    lda #^town_palette
    ldx #.loword(town_palette)
    ldy #00000
    jsr dma_to_palette

    ; load map
    ldy #(town_map_end - town_map)
    sty <W0
    lda #^town_map
    ldx #.loword(town_map)
    ldy #VRAM_MAP_BASE
    jsr dma_to_vram

    ; load tiles
    ldy #(town_tiles_end - town_tiles) ; size of transfer
    sty <W0
    lda #^town_tiles
    ldx #.loword(town_tiles)
    ldy #VRAM_CHR_BASE
    jsr dma_to_vram

    ;; sprites
    ; set sprite base
    lda #(VRAM_SPRITE_BASE >> 13)
    sta OBJSEL

    ; load sprites
    ldy #(demo_sprite_palette_end - demo_sprite_palette) ; # of palette entries
    sty <W0
    lda #^demo_sprite_palette
    ldx #.loword(demo_sprite_palette)
    ldy #$80 ; sprite palette offset
    jsr dma_to_palette

    ; load sprite tiles
    ldy #(demo_sprite_tiles_end - demo_sprite_tiles)
    sty <W0
    lda #^demo_sprite_tiles
    ldx #.loword(demo_sprite_tiles)
    ldy #VRAM_SPRITE_BASE
    jsr dma_to_vram
    rts


.a8
.i16
init_sound:
    A8
    I16
    ; init sound
    lda     #$7f
    pha
    plb
    ; DB = $7f
    jsl     Tad_Init

    phk
    plb
    ; DB = $80
    rts


.a8
.i16
;; set up data
init_game_data:
    jsr init_binary_data
    jsr init_sound

    A16
    jsr init_players
    jsr init_bullets

    A8
    ;; set up bg registers
    lda #1 ; mode 1
    sta BGMODE
    lda #((VRAM_MAP_BASE >> 10) << 2)
    sta BG2SC ; set bg 2 tile map
    ; set bg 2 char vram base addr (implicitly setting bg1 to 0, but we don't
    ; care)
    lda #((VRAM_CHR_BASE >> 12) << 4)
    sta BG12NBA

    lda #$80
    sta VMAIN

    ; set init program variables
    ldx #$FF   ;; first vertical line is blank, and there's an extra line at the
    stx map_y  ;; bottom, so we shift vertical offset by 1
    ldx #$0
    stx map_x

    rts


;; -----
;; input

.a8
.i16
read_input:
wait_for_joypad:
    lda HVBJOY            ; get joypad status
    lsr a                 ; check whether joypad done reading...
    bcs wait_for_joypad   ; ...if not, wait a bit more

    A16
    ; read joypad
    rep #$20                         ; set A to 16-bit

    ldx #0 ;; we use this for both the offset into the player table
           ;; as well as the offset into the joypad info hw regs, as both are
           ;; spaced one word apart.
joy_loop:
    stx .loword(W1)
    lda .loword(player_table), x
    tcd ;; remapping dp to player x

    cpx #$4 ; in multitap config, the third joypad needs to be read raw
            ; the 4th joypad can be read through the convenience joy
            ; registers
    bne :+
    inx
    inx
  : lda JOY1L, x                     ; get new input from this frame
    ldy player::joy                 ; get input from last frame
    sta player::joy                 ; store new input from this frame
    tya                              ; check for newly pressed buttons...
    eor player::joy                  ; filter buttons that were not pressed last frame
    and player::joy                  ; filter held buttons from last frame
    sta player::joy_trigger          ; ...and store them
    ; second, check for buttons held from last frame
    tya                              ; get input from last frame
    and player::joy                  ; filter held buttons from last frame...
    sta player::joy_held             ; ...store them
    ; for convenience store the or of trigger and held
    ora player::joy_trigger
    sta player::joy_trigger_held

    ; set some conveniet things
    bit_tribool JOY_RIGHT_SH, JOY_LEFT_SH
    sta player::h_tribool
    lda player::joy_trigger_held
    bit_tribool JOY_DOWN_SH, JOY_UP_SH
    sta player::v_tribool

    ;; and increment x and redo for
    lda .loword(W1)
    inc
    inc
    tax
    cpx #PLAYER_TABLE_I

    bne joy_loop
    lda #$0
    tcd
    rts


;; --------
;; movement

;; my current thinking is:
;; - first handle all movement to see what new coordinate
;;   our sprite would end up
;; - then do collision tests, for now only with terrain
;; - if collision, this will mean a new state, and we need to process the
;; aftermath
;;
;; - sprites have a bounding box that we need to check for collisions.
;;   depending on which direction we move in, we can check different points
;;   of this box. So say we go left, we check left up and left down,
;;   we fall left, we check left up, left down and right down
;;   we might want to check 6 points so we don't allow impaling ourselves horizontally on
;;   8x8 blocks

handle_movement:
    jsr handle_player_movement
    jsr handle_bullet_movement
    rts

COLL_STACK_ROOM = $21
COLL_STACK_TMP = $1d
COLL_STACK_POINT_HIT_GROUND = $1b
COLL_STACK_POINT_SPRITE_CALLBACK_TMP = $19
COLL_STACK_Y_NEW_TMP = $17
COLL_STACK_X_NEW_TMP = $15
COLL_STACK_POINT_Y_COORD = $13
COLL_STACK_POINT_X_COORD = $11
COLL_STACK_POINT_Y_OFF_NEW_SUB = $f
COLL_STACK_POINT_X_OFF_NEW_SUB = $d
COLL_STACK_Y_OFF_NEW = $b
COLL_STACK_X_OFF_NEW = $9
COLL_STACK_TILE_OFF = $7
COLL_STACK_POINT_TILE_OFF = $5
COLL_STACK_POINT_Y_OFF_NEW = $3
COLL_STACK_POINT_X_OFF_NEW = $1

.a16
.i16
check_collisions:
    ;; set up stack pointer
    tsc ;; pull current stack pointer to A
    pha ;; push to stack, so we can easily reset later

    sec
    sbc #COLL_STACK_ROOM ;; make room for bunch of stack arguments
                         ;; (we just pushed the sp so we got one 16bit arg less)
    tcs

    ;; we set default impossible minus value, so we know if it has been set before
    lda #$FFFF
    sta COLL_STACK_Y_NEW_TMP, s
    sta COLL_STACK_X_NEW_TMP, s

    lda sprite::y_new  ; load y of first sprite
    rshift 4           ; remove sub-pixels
    tay
    and #7
    sta COLL_STACK_Y_OFF_NEW, s ; position of y within tile
    tya
    rshift 3                ; divide by 8, truncating to get y tile offset
    A8
    ; multiply Y by 32
    sta WRMPYA ; set first nr to muliply: y offset
    lda #$20
    sta WRMPYB ; set second nr to multiply row size
    ;; now calculate X tile offset while we wait (more than) 8 cycles for
    ;; multiplication to complete
    A16
    lda sprite::x_new   ; load x of first sprite
    rshift 4            ; remove sub-pixels
    tax
    and #7
    sta COLL_STACK_X_OFF_NEW, s  ; position of x within tile
    txa
    rshift 3            ; divide by 8 to get x tile offset
    clc
    adc RDMPYL          ; add y to x, tile offset is in A
    sta COLL_STACK_TILE_OFF, s  ; save tile offset to stack for later

    lda #0
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP, s ; set to 0
    sta COLL_STACK_POINT_HIT_GROUND, s
    ;; So now we have our base
    ;; Our stack, growing down looks like:
    ;; - top-left corner y offset into 8x8 tile
    ;; - top-left corner x offset into 8x8 tile
    ;; - top-left corner offset into collision map <- SP
    ;;
    ;; below the stack is augmented with:
    ;; - running offset in collision map for individual point
    ;; - y offset for point into sprite + top-left corner y offset into 8x8 tile
    ;; - x offset for point into sprite + top-left corner x offset into 8x8 tile
    ;;
    ;; From here on it becomes a game of iterating over the points of our
    ;; bounding box and figuring out if they cause a collision. If so, by how
    ;; much should we move backwards from our point of travel to snap to the
    ;; grid.
    ;;
    ;; We should also check if one pixel beyond our bounding box is a surface we
    ;; want to walk on/cling to. Once we get there I guess we will chop this fn
    ;; up for some code reusability.
    ;;
    ;; First up, see if we need to move the offset into the collision map for y
    ;;
    ;; We iterate over our points, so first, set them up
    ;; for duration of this loop, bbox offset stays in Y
    ldy #0 ;; bbox point offset
coll_point_loop:
    lda COLL_STACK_Y_OFF_NEW, s
    clc
    adc (sprite::bbox), y
    sta COLL_STACK_POINT_Y_OFF_NEW, s ;; save offset for if we need to do micro pushback
    rshift 3 ;; truncate to see if we're spilling over into another tile
    beq y_no_spill  ; we're not spilling over
    A8
    sta WRMPYA      ; unfortunately we have to do more muliplications
    lda #$20
    sta WRMPYB ; set second nr to multiply row size
    A16
    lda COLL_STACK_TILE_OFF, s  ; collision map tile from stack 4 cycles
    clc        ; 2 cycles
    adc RDMPYL ; 4 cycles = enough cycles before reading to make up
               ; multiplication budget
    sta COLL_STACK_POINT_TILE_OFF, s   ; push collision map tile for this point (we need other tile later)
    bra point_x_calc
y_no_spill: ;; y didn't spill. to keep symmetry with above basic block, push unmodified
            ;; collision map tile to stack
    lda COLL_STACK_TILE_OFF, s
    sta COLL_STACK_POINT_TILE_OFF, s
point_x_calc:
    ;; calc possible x collision tile, and check collision before we start
    ;; thinking about calculating possible sprite pushback to grid coords
    lda COLL_STACK_X_OFF_NEW, s
    iny ;; move y to x offset in point
    iny
    clc
    adc (sprite::bbox), y ; again, do x in the meantime
    sta COLL_STACK_POINT_X_OFF_NEW, s
    rshift 3
    clc
    adc COLL_STACK_POINT_TILE_OFF, s ;; add to point-local collision map offset
    sta COLL_STACK_POINT_TILE_OFF, s
    tax
    A8
    lda town_coll, x           ; check collision map for point
    A16
    bne collision              ; if not zero, collision
    jmp coll_no_coll_for_point ; otherwise, do no collision things
collision:
    ;; Once we know x and y direction,
    ;; we can make sensible decision on snapping.
    ;; We will first test which axis is the shallowest, as it's hopefully
    ;; a sensible and inexpensive proxy for what makes the most sense to snap
    ;; to:
    ;;
    ;; Most often the shallower side will be the side that was actually
    ;; penetrated (should mostly just be a pixel or two in depth), and if not,
    ;; snapping to the shallow side will be less invasive, as it will be less
    ;; noticed.
    ;;
    ;; The whole setup becomes a bit convoluted unfortunately.
    ;; To make things a bit more manageable, we will have greater than tests
    ;; for all 4 diagonal directions, which will go to either x or y snap.
    ;; If x or y is not moving we go straight to the opposite direction snap
    ;; in the snap sections. We again test for direction to not go crazy with
    ;; the logic (it's pretty cheap), and decide there on snap to the right
    ;; side. Then we do another (simpler) collision test, and if we do have
    ;; a collision, we go straight to snapping the other axis.
    ;; after this, no extra snapping should be necessary.
    ;;
    ;; If we do end up in an endless loop, we know our logic is wrong,
    ;; and it will be easy to spot it was the collision handling :)
    ;;
    ;; first we check if we straddled a block boundry in x or y direction
    ;; x direction
    ;;
    ;; reconstruct exact y position of point
    ;; Ideally I'd like to keep all calculations in subpixel format
    ;; but that makes the multiplication calculations above expensive.
    ;; As these calculations when we know collision happens will happen
    ;; much less frequent, this seems the better way.
    dey ;; move back to y within current bbox
    dey
    lda (sprite::bbox), y
    asl
    asl
    asl
    asl
    clc
    adc sprite::y_new
    sta COLL_STACK_POINT_Y_COORD, s
    and #$7f
    sta COLL_STACK_POINT_Y_OFF_NEW_SUB, s

    iny ;; and increment again to get to x
    iny
    lda (sprite::bbox), y
    asl
    asl
    asl
    asl
    clc
    adc sprite::x_new
    sta COLL_STACK_POINT_X_COORD, s
    and #$7f
    sta COLL_STACK_POINT_X_OFF_NEW_SUB, s


    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_X_OFF_NEW_SUB, s ; (~velocity) + point offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    beq coll_snap_y ; not moved out of x, so after snapping y, we're done
    ;; if n flag set, we moved from left block, so towards right
    bmi coll_x_right


    ;; x = moving left
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; (~velocity) + point offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    bne :+
    jmp coll_snap_to_right
    ;; if n flag set, we moved from upper block, so downwards
  : bmi coll_left_down
    bra coll_left_up

coll_x_right:
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; add velocity and block offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    ;; if n flag set, we moved from upper block, so downwards
    bne coll_x_right_cont
    jmp coll_snap_to_left
coll_x_right_cont:
    bpl coll_x_right_end
    jmp coll_right_down
coll_x_right_end:
    bra coll_right_up

coll_snap_y:
    lda sprite::v_velo
    tax
    eor #$FFFF
    clc
    adc #1
    sta COLL_STACK_TMP, s
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; point offset + (~velocity)
    clc
    adc COLL_STACK_TMP, s
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    bne coll_snap_y_cont
    jmp collision_player_end ;; assuming we got here from x also not out of bounds
coll_snap_y_cont:
    ;; if n flag set, we moved from upper block, so downwards
    bpl coll_snap_y_end
    jmp coll_snap_to_top
coll_snap_y_end:
    jmp coll_snap_to_bottom

;; now we resolved straight up/down, left/right
;; but if we moved diagonal into a new block, which way should we snap?
coll_left_up:
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$1
    tax
    A8
    lda town_coll, x
    A16
    bne :+
    jmp coll_snap_to_right
  : lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$20
    tax
    A8
    lda town_coll, x
    A16
    bne :+
    jmp coll_snap_to_bottom
  : jsr snap_to_right
    jsr snap_to_bottom
    jmp collision_player_end

coll_left_down:
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$1
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_right
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$20
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_top
    jsr snap_to_right
    jsr snap_to_top
    bra collision_player_end

coll_right_up:
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$1
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_left
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$20
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_bottom
    jsr snap_to_left
    jsr snap_to_bottom
    bra collision_player_end


; we're moving in right/down direction
coll_right_down:
    ;; first we check if the squares that we want to snap into
    ;; aren't taken up by tiles.
    ;; We first check for left. If the tile there isn't obstructed,
    ;; we snap to it. Then we do same for right.
    ;; If both squares are obstructed we snap to both, aka, the corner
    ;; of the tile we left.
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$1
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_left
    ;; check top
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$20
    tax
    A8
    lda town_coll, x
    A16
    beq coll_snap_to_top
    ; no space at left or up
    ; we need to snap to tile we came from,
    ; aka both left and top
    ; of current tile
    jsr snap_to_left
    jsr snap_to_top
    bra collision_player_end


;; snapping to what?
coll_snap_to_top:
    jsr snap_to_top
    jmp collision_player_end

coll_snap_to_bottom:
    jsr snap_to_bottom
    bra collision_player_end


coll_snap_to_left:
    jsr snap_to_left
    bra collision_player_end

coll_snap_to_right:
    jsr snap_to_right
    bra collision_player_end

;; put code here, if you want to do something specific if no
;; collision has happened
coll_no_coll_for_point:
    lda sprite::vptr
    clc
    adc #sprite_vtable::coll_point_no_coll_callback
    tax
    jsr (0,x)

collision_player_end:
    iny
    iny
    cpy sprite::bbox_size
    beq collision_cleanup
    jmp coll_point_loop ; not equal so we do another round
collision_cleanup:
    lda sprite::vptr
    clc
    adc #sprite_vtable::collision_end_callback
    tax
    jsr (0, x)

collision_unwind_stack:
    ; end of point loop so we're done
    lda COLL_STACK_ROOM - 1, s ; restore stack
    tcs                        ; pointer
    rts

;; -------- end of mega collision fn

snap_to_top:
   lda COLL_STACK_POINT_Y_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out upwards is now in A
    ;; we AND with 7f, so we know how much of it sticks up,
    and #$7f
    sta COLL_STACK_TMP + 2, s
    lda sprite::y_new ; so we're effectively
    sec
    sbc COLL_STACK_TMP + 2, s
    sbc #$1
    tax
    lda COLL_STACK_Y_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi snap_to_top_save_new_y  ; if not we can directly save this one
    txa
    cmp COLL_STACK_Y_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcs snap_to_top_cont; if current y is higher, we don't save
snap_to_top_save_new_y:
    txa
    sta COLL_STACK_Y_NEW_TMP + 2, s
snap_to_top_cont:
    ;; this means we just hit the bottom
    lda #1
    ;; this one we should probably make a bit more generic.
    ;; for example by registering in all of these snap fns
    ;; what we hit in one var.
    sta COLL_STACK_POINT_HIT_GROUND + 2, s
    rts


snap_to_bottom:
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out downwards is now in A
    and #$7f
    eor #$FFFF
    clc
    adc #$1
    clc
    adc #$80 ; effectively y_new + (8 - nr)
    adc sprite::y_new ; and we want to add that to the new y
    tax
    lda COLL_STACK_Y_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi snap_to_bottom_save_new_y  ; if not we can directly save this one
    txa
    cmp COLL_STACK_Y_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    rts ; if current y is lower, we don't save
snap_to_bottom_save_new_y:
    txa
    sta COLL_STACK_Y_NEW_TMP + 2, s
    rts


snap_to_left:
   lda COLL_STACK_POINT_X_OFF_NEW_SUB +2, s
    ;; so the bit that sticks out right-wards is now in A
    ;; we AND with 7f, so we know how much of it sticks up,
    and #$7f
    sta COLL_STACK_TMP +2, s
    lda sprite::x_new ; so we're effectively
    sec
    sbc COLL_STACK_TMP +2, s
    sbc #$1
    tax
    lda COLL_STACK_X_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi coll_snap_to_left_save_new_x  ; if not we can directly save this one
    txa
    cmp COLL_STACK_X_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcs coll_snap_to_left_cont ; if current y is higher, we don't save
coll_snap_to_left_save_new_x:
    txa
    sta COLL_STACK_X_NEW_TMP + 2, s
coll_snap_to_left_cont:
    rts


snap_to_right:
    lda COLL_STACK_POINT_X_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out left-wards is now in A
    and #$7f
    eor #$FFFF
    clc
    adc #$1
    clc
    adc #$80 ; effectively x_new + (8 - nr)
    adc sprite::x_new ; and we want to add that to the new x
    tax
    lda COLL_STACK_X_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi snap_to_right_save_new_x  ; if not we can directly save this one
    txa
    cmp COLL_STACK_X_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcc snap_to_right_cont ; if current y is lower, we don't save
snap_to_right_save_new_x:
    txa
    sta COLL_STACK_X_NEW_TMP + 2, s
snap_to_right_cont:
    rts


;; --------- 
;; sound

.a8
.i16
load_song:
    pha
    ; Reset TAD State
    ldx     #256
    jsr     Tad_SetTransferSize
    jsr     Tad_SongsStartImmediately
    pla
    jsr     Tad_LoadSong

@loop:
    jsr     Tad_IsSongLoaded
    bcs     @return
    jsl     Tad_Process
    bra     @loop
@return:
    rts


;; ------
;; sprite-bumps

BBOX_SQUARE_SIZE = $10 ;; 4 * x/y coords * 2 (word) = 16

BBOX_X_LEFT = $0
BBOX_Y_TOP = $2
BBOX_X_RIGHT = $4
BBOX_Y_BOTTOM = $6

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
    bne sprite_bumps_player_loop
    plx ; clear the stack

    lda #$0
    tcd
    rts

;; ---------
;; game loop


.a8
.i16
wait_nmi:
    ;should work fine regardless of size of A
    lda .loword(in_nmi) ;load A register with previous in_nmi
@check_again:
	wai ;wait for an interrupt
    cmp .loword(in_nmi)  ;compare A to current in_nmi
                ;wait for it to change
                ;make sure it was an nmi interrupt
    beq @check_again
    rts

.a8
update_bgs:
    lda map_x
    ;; this is one of those latching regs
    sta BG2HOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG2HOFS

    lda map_y
    ;; this is one of those latching regs
    sta BG2VOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG2VOFS
    rts

.a8
.i16
update_vram:
    jsr update_bgs
    A16
    I8
    jsr dma_OAM
    A8
    I16
    rts

.a16
.i16
finalise:
    jsr finalize_players
    jsr finalize_bullets
    rts

.a8
.i16
game_loop:
    jsr wait_nmi ; wait for NMI / V-Blank
    ; we're in vblank, so first upddate video memory things
    jsr update_vram
    jsr read_input
    A16
    jsr handle_movement
    jsr handle_sprite_bumps
    jsr finalise
    A8
    jmp game_loop


;; ----
;; main

.a8
.i16
main:
    jsr init_game_data

    ;; play some music
    lda #1
    jsr load_song


    ; turn on BG2
    lda #$12
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP


    ; enable NMI, turn on automatic joypad polling
    lda #$81
    sta NMITIMEN

    jmp game_loop
