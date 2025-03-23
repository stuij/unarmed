.p816   ; 65816 processor
.smart

.define ROM_NAME "demo" ; max 21 chars

.include "defines.inc"
.include "header.inc"
.include "macros.inc"
.include "io.inc"

.include "init.inc"
.include "../build/assets/audio.inc"
.include "../terrific-audio-driver/audio-driver/ca65-api/tad-audio.inc"
.include "assets.inc"
.include "data.inc"

.segment "VECTORS"
.word 0, 0                          ;; Native mode handlers
.word .loword(spinloop_handler)     ; COP
.word .loword(spinloop_handler)     ; BRK
.word .loword(spinloop_handler)     ; ABORT
.word .loword(nmi_handler)          ; NMI
.word .loword(spinloop_handler)     ; RST
.word .loword(irq_handler)          ; IRQ

.word 0, 0                          ;; Emulation mode
.word .loword(spinloop_handler)     ; COP
.word 0
.word .loword(spinloop_handler)     ; ABORT
.word .loword(spinloop_handler)     ; NMI
.word .loword(reset_handler)        ; RESET
.word .loword(spinloop_handler)     ; IRQBRK


.code
;; handlers

.a8     ; A is 8 bits
.i16    ; X/Y are 16 bits
reset_handler:
    jml :+
:   sei
    clc
    xce
	; FYI: coming out of emulation mode, the M and X bits of the
	; status registers are set to one. So resp. A and X/Y
	; are set to 8 bit.
	I16
    A8
    cld                     ; clear decimal flag
    lda #$80                ; force v-blanking
    sta INIDISP
    stz NMITIMEN            ; disable NMI
    ; set the stack pointer to $1fff
    ldx #$1fff              ; load X with $1fff
    txs                     ; copy X to stack pointer

    lda     #$01            ; Enable FastROM. Should not be necessary
    sta     MEMSEL          ; when FastROM is enabled in header?

    phk
    plb                     ; set b to current bank

    ;; clear all
    jsr clear_registers
    jsr clear_VRAM
    jsr clear_CGRAM
    jsr clear_OAM_mirror
    A16
    I8
    jsr dma_OAM
    A8
	I16
    jmp main


    nmi_handler:
    jml :+
:   bit RDNMI ; it is required to read this register
              ; in the NMI handler
    inc .loword(in_nmi)
    rti

irq_handler:
    jml :+
:   bit TIMEUP	; it is required to read this register
				; in the IRQ handler
@loop:
    jmp @loop


; This shouldn't get called, so if it does we'd like to know more
; in a controlled manner, instead of say branching to $00000 and
; crashing out violently.
spinloop_handler:
    jml :+
:   jmp spinloop_handler


;; setup lib code

; dma_to_vram
; arguments:
;   - a: src bank
;   - x: src loword
;   - y: dst VRAM base address
;   - stack: size
.a8
.i16
dma_to_vram:
    sta A1B0 ; set bank of source address
    stx A1T0L ; set loword of source address
    sty VMADDL ; set VRAM base address
    ldy <W0 ; get size from zero page word reg 1
    sty DAS0L ; set DMA byte counter

    lda #<VMDATAL
    sta BBAD0 ; so set DMA destination to VMDATAL
    lda #1
    sta DMAP0 ; transfer mode, 2 registers 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts

; dma_to_palette
; arguments:
;   - a: src bank
;   - x: src loword
;   - y: dst VRAM base address
;   - stack: size
.a8
.i16
dma_to_palette:
    sta A1B0 ; set bank of source address
    stx A1T0L ; set loword of source address
    tya ; CGADD is a byte
    sta CGADD ; set VRAM base address
    ldy <W0 ; get size from zero page word reg 1
    sty DAS0L ; set DMA byte counter

    lda #<CGDATA
    sta BBAD0 ; so set DMA destination to VMDATAL
    lda #00
    sta DMAP0 ; transfer mode, 1 register, 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts


SpriteUpperEmpty:
DMAZero:
.word $0000

SpriteEmptyVal:
.byte $e0 ; 224


.a8
.i16
clear_OAM_mirror:
;fills the buffer with 224 for low table
;and $00 for high table
	php
	ldx #.loword(OAM_MIRROR)
	stx WMADDL ;WRAM_ADDR_L
	stz WMADDH ;WRAM_ADDR_H

	ldx #$8008 ;fixed transfer to WRAM data 2180
	stx DMAP0
	ldx	#.loword(SpriteEmptyVal)
	stx A1T0L ; and 4303
	lda #^SpriteEmptyVal ;bank #
	sta A1B0
	ldx #$200 ;size 512 bytes
	stx DAS0L ;and 4306
	lda #1
	sta MDMAEN ; DMA_ENABLE start dma, channel 0

	ldx	#.loword(SpriteUpperEmpty)
	stx A1T0L ; and 4303
	lda #^SpriteUpperEmpty ;bank #
	sta A1B0
	ldx #$0020 ;size 32 bytes
	stx DAS0L ;and 4306
	lda #1
	sta MDMAEN ; DMA_ENABLE start dma, channel 0
	plp
	rts


.a16
.i8
dma_OAM:
;copy from OAM_MIRROR to the OAM RAM
	php
	stz OAMADDL ;OAM address

	lda #$0400 ;1 reg 1 write, 2104 oam data
	sta DMAP0
	lda #.loword(OAM_MIRROR)
	sta A1T0L ; source
	ldx #^OAM_MIRROR
	stx A1B0 ; bank
	lda #$220
	sta DAS0L ; length
	ldx #1
	stx MDMAEN ; DMA_ENABLE start dma, channel 0
	plp
	rts


;; game specific

.a8
.i16
;; set up data
init_game_data:
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


    ; set init program variables
    ldx #0
    stx map_x
    stx map_y

    A16
    ;; ldx is still 0
player_init_loop:
    lda .loword(player_table), x ; player struct under x offset
    tcd ; set dp to it

    ldy #0
    sty player::h_velo
    sty player::v_velo
    sty player::joy

    ; p1 start position
    ; $80 pixel offset and $0 subpixels
    ldy #$7b0
    sty player::x_pos
    sty player::y_pos

    ldy #move_state::idle
    sty player::move_state

    ldy #face_dir::right
    sty player::face_dir

    ldy #.loword(player_bbox_default)
    sty player::bbox

    txa
    inc
    inc
    tax
    cpx #8
    bne player_init_loop

    lda #0
    tcd

    A8
    ;; set oam for sprite 1 directly.
    ;; this and the above sprite init data should all be done with some
    ;; kind of generalized player sprite init routine
    lda #$00                ; name (place) of first sprite
    sta OAM_MIRROR + 2
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + 3

    lda #$fe                ; set top bit of x to 0, set 16x16 tile.
                            ; keep rest of tiles at 1
    sta OAMDATA + 200

    rts


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

    lda JOY1L, x                     ; get new input from this frame
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
    cpx #8
    bne joy_loop
    lda $0
    tcd
    rts


jump:
    ;; decrease velocity
    lda player::v_velo
    ;; 65816 doesn't suppport signed compare without extra steps, so..
    ;; if velo is negative, we for sure need to increase velo
    bmi jump_add_velo
    ;; now we can do unsigned compare
    cmp #V_VELO_DOWN_MAX
    bcs jump_after_velo_add ;; we're at max down velocity, so skip velo increase

jump_add_velo:
    clc ; carry set means we're not borrowing
    adc player::v_velo_dec
    sta player::v_velo

jump_after_velo_add:
    ;; change player pos based on velocity
    lda player::y_pos
    clc
    adc player::v_velo ; at some point this will go negative,
                       ; which is excellent as that means we're going down now
    sta player::y_new

    rts



init_jump:
    lda #V_VELO_INIT
    sta player::v_velo
    lda #V_VELO_DEC
    sta player::v_velo_dec
    lda #move_state::jump_up
    sta player::move_state
    asl ;; times 2 to get proper fn offset
    tax
    jmp (.loword(move_table), x)


idle:
    ;; are we instructed to jump?
    lda player::joy_trigger_held
    and #JOY_B
    beq still_idle ;; zero set, no match
    ;; state change to jump
    jmp init_jump

still_idle:
    ;; new x/y is old x/y
    lda player::x_pos
    sta player::x_new
    lda player::y_pos
    sta player::y_new
    rts


run:
    rts

cling:
    rts

climb:
    rts

; jump table of player movement states
move_table:
.addr idle
.addr run
.addr jump ;; both jump_up and jump_down states
.addr jump ;; lead us to the jump callback for now.
.addr cling
.addr climb


player_table:
.addr .loword(p1)
.addr .loword(p2)
.addr .loword(p3)
.addr .loword(p4)

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
.a16
.i16
handle_player_movement:
    lda player::move_state
    asl
    tax
    jsr (.loword(move_table), x)

    jsr check_collisions
    rts


.a16
.i16
handle_movement:
    ;; loop over player movement
    ldx #0
    phx
player_movement_loop:
    lda .loword(player_table), x
    tcd ;; remapping dp to player x
    jsr handle_player_movement
    pla
    inc
    inc
    tax
    phx
    cpx #8
    bne player_movement_loop
    plx ; clear the stack
    lda $0
    tcd
    rts

COLL_STACK_ROOM = $17
COLL_STACK_POINT_Y_COORD = 13
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

    lda player::y_new  ; load y of first sprite
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
    lda player::x_new   ; load x of first sprite
    rshift 4            ; remove sub-pixels
    tax
    and #7
    sta COLL_STACK_X_OFF_NEW, s  ; position of x within tile
    txa
    rshift 3            ; divide by 8 to get x tile offset
    clc
    adc RDMPYL          ; add y to x, tile offset is in A
    sta COLL_STACK_TILE_OFF, s  ; save tile offset to stack for later
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
    adc (player::bbox), y
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
    adc (player::bbox), y ; again, do x in the meantime
    A16
    sta COLL_STACK_POINT_X_OFF_NEW, s
    rshift 3
    clc
    adc COLL_STACK_POINT_TILE_OFF, s ;; add to point-local collision map offset
    sta COLL_STACK_POINT_TILE_OFF, s
    tax
    A8
    lda town_coll, x    ; check collision map for point
    A16
    bne collision       ; if not zero, collision
    jmp collision_end   ; otherwise we can return, doing nothing
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
    lda (player::bbox), y
    asl
    asl
    asl
    asl
    adc player::y_new
    sta COLL_STACK_POINT_Y_COORD
    and #$7f
    sta COLL_STACK_POINT_Y_OFF_NEW_SUB, s

    iny ;; and increment again to get to x
    iny
    lda (player::bbox), y
    asl
    asl
    asl
    asl
    adc player::x_new
    sta COLL_STACK_POINT_X_COORD
    and #$7f
    sta COLL_STACK_POINT_X_OFF_NEW_SUB, s


    lda player::h_velo
    eor #$FFFF
    adc #1
    clc
    adc COLL_STACK_POINT_X_OFF_NEW_SUB, s ; (~velocity) + point offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    beq coll_snap_y ; not moved out of x, so after snapping y, we're done
    ;; if n flag set, we moved from left block, so towards right
    bmi coll_x_right


    ;; x = moving left
    lda player::v_velo
    eor #$FFFF
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
    lda player::v_velo
    eor #$FFFF
    adc #1
    clc
    adc COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; add velocity and block offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    ;; if n flag set, we moved from upper block, so downwards
    beq coll_snap_to_left
    bmi coll_right_down
    bra coll_right_up

coll_snap_y:
    lda player::v_velo
    tax
    eor #$FFFF
    adc #1
    sta player::tmp
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; point offset + (~velocity)
    adc player::tmp
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    beq collision_end ;; assuming we got here from x also not out of bounds
    ;; if n flag set, we moved from upper block, so downwards
    bmi coll_snap_to_top
    bra coll_snap_to_bottom

;; now we resolved straight up/down, left/right
;; but if we moved diagonal into a new block, which way should we snap?
coll_left_up:
    lda COLL_STACK_POINT_X_OFF_NEW_SUB, s
    cmp COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; for both, higher nr means shallower
    ;; carry clear means x is higher
    bcc coll_snap_to_right
    bra coll_snap_to_bottom


coll_left_down:
    lda #7 ;; reverse x to be able to compare sensibly
    sec
    sbc COLL_STACK_POINT_X_OFF_NEW_SUB, s
    cmp COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; carry clear means x is higher (while inversed direction)
    bcc coll_snap_to_top
    bra coll_snap_to_right
    
coll_right_up:
    lda #7 ;; reverse x to be able to compare sensibly
    sec
    sbc COLL_STACK_POINT_X_OFF_NEW_SUB, s
    cmp COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; carry clear means x is higher (while inversed direction)
    bcc coll_snap_to_left
    bra coll_snap_to_bottom    

coll_right_down:
    lda COLL_STACK_POINT_X_OFF_NEW_SUB, s
    cmp COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; carry clear means x is higher
    bcc coll_snap_to_top
    bra coll_snap_to_left


;; snapping to what?
coll_snap_to_top:
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; so the bit that sticks out upwards is now in A
    ;; we AND with 7f, so we know how much of it sticks up,
    and #$7f
    sta player::tmp
    lda player::y_new ; so we're effectively
    sec
    sbc player::tmp
    sbc #$20
    sta player::y_new ; doing y_new - nr

    ;; this means we just hit the bottom
    ;; let's start by just putting us in idle mode
    lda #move_state::idle
    sta player::move_state

    bra collision_end

coll_snap_to_bottom:
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB, s
    ;; so the bit that sticks out upwards is now in A
    and #$7f
    eor #$FFFF
    adc #$1
    clc
    adc #$80 ; effectively y_new + (8 - nr)
    adc player::y_new ; and we want to add that to the new y
    sta player::y_new
    bra collision_end


coll_snap_to_left:

coll_snap_to_right:


collision_end:
    iny
    iny
    cpy player_bbox_end_offs
    beq collision_cleanup
    jmp coll_point_loop ; not equal so we do another round
collision_cleanup:
    ; end of point loop so we're done
    lda COLL_STACK_ROOM - 1, s ; restore stack
    tcs                        ; pointer
    rts


player_bbox_end_offs:
.word .sizeof(player_bbox)


player_bbox_default:
;; top left
.word 1  ;; y
.word 3  ;; x
;; middle left
.word 8  ;; y
.word 3  ;; x
;; bottom left
.word $f ;; y
.word 3  ;; x
;; top middle
.word 1  ;; y
.word 8  ;; x
;; bottom middle
.word $f ;; y
.word 8  ;; x
;; top right
.word 1  ;; y
.word $d ;; x
;; middle right
.word 8  ;; y
.word $d ;; x
;; bottom right
.word $f ;; y
.word $d ;; x


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
    ;; for shame, make this more fancy

    ;; we now know x and y won't change anymore, so lock them into x/y_pos
    ;; also write them to the OAM mirror
    lda p1 + player::x_new
    sta p1 + player::x_pos
    rshift 4
    A8
    sta OAM_MIRROR
    A16
    lda p1 + player::y_new
    sta p1 + player::y_pos
    rshift 4
    A8
    sta OAM_MIRROR + 1
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
    jsr finalise
    A8
    jmp game_loop


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
