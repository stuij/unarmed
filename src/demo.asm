.p816   ; 65816 processor
.smart

.define ROM_NAME "demo" ; max 21 chars

.include "header.inc"
.include "macros.inc"
.include "io.inc"

.include "init.inc"
.include "../build/assets/audio.inc"
.include "../terrific-audio-driver/audio-driver/ca65-api/tad-audio.inc"

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


.segment "RODATA"

;; backgrounds
town_tiles:
.incbin"assets/town.tiles"
town_tiles_end:

town_palette:
.incbin"assets/town.palette"
town_palette_end:

town_map:
.incbin"assets/town-map.map"
town_map_end:

town_coll:
.incbin"assets/town-map.coll"
town_coll_end:

;; sprites
demo_sprite_tiles:
.incbin"assets/demo-sprites.tiles"
demo_sprite_tiles_end:

demo_sprite_palette:
.incbin"assets/demo-sprites.palette"
demo_sprite_palette_end:


.segment "ZEROPAGE"
W0:
B0L:
.res 1
B0H:
.res 1
W1:
B1L:
.res 1
B1H:
.res 1
W2:
B2L:
.res 1
B2H:
.res 1
W3:
B3L:
.res 1
B3H:
.res 1
in_nmi:
; data read from joypad 13
joy1: .res 2
; trigger read from joypad 1
joy1_trigger: .res 2
; held buttons read from joypad 1
joy1_held: .res 2
; background 1 horizontal offset
map_x: .res 2
map_y: .res 2

.segment "BSS"
OAM_MIRROR: .res 512

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
    inc in_nmi
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
.word $FFFF

SpriteEmptyVal:
.byte $FF ; 224


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

VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BASE = $8800 ;; $800 alignment == 256x256 (32x32x2) map


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
    lda #00000
    sta map_x
    sta map_y

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
    lda JOY1L                        ; get new input from this frame
    ldy joy1                         ; get input from last frame
    sta joy1                         ; store new input from this frame
    tya                              ; check for newly pressed buttons...
    eor joy1                         ; filter buttons that were not pressed last frame
    and joy1                         ; filter held buttons from last frame
    sta joy1_trigger                 ; ...and store them
    ; second, check for buttons held from last frame
    tya                              ; get input from last frame
    and joy1                         ; filter held buttons from last frame...
    sta joy1_held                    ; ...store them

    rts

;; collison checking
; town_coll



;; horizontal joypad callbacks
.a8
move_bg_horz:
    clc
    adc map_x
    sta map_x
    rts

.a8
move_sprite_horz:

    clc
    adc OAM_MIRROR
    sta OAM_MIRROR
    rts

;; vertical joypad callbacks
.a8
move_bg_vert:
    clc
    adc map_y
    sta map_y
    rts

.a8
move_sprite_vert:
    clc
    adc OAM_MIRROR + 1
    sta OAM_MIRROR + 1
    rts


handle_player_movement:
.addr handle_movement

.a8
.i16
;; B0L - horizontal tribool
;; B0H - vertical tribool
handle_movement:
    ;; first find out what is our next position
    lda OAM_MIRROR + 1  ; load y of first sprite
    clc
    adc B3H             ; add/subtract tibool to/from y
    sta W1              ; save y for later
    rshift 3            ; divide by 8, truncating to get y tile offset
    ; multiply Y by 32
    sta WRMPYA ; set first nr to muliply: y offset
    lda #$20
    sta WRMPYB ; set second nr to multiply row size
    ;; now calculate X tile offset while we wait (more than) 8 cycles for
    ;; multiplication to complete
    lda #$00
    xba                 ; make sure that B is 0x00
    lda OAM_MIRROR      ; load x of first sprite
    clc
    adc B3L             ; add/subtract tribool to/from x
    sta W2              ; save x for later
    rshift 3            ; divide by 8 to get x tile offset
    A16
    clc
    adc RDMPYL          ; add y to x, tile offset is in A
    tax
    A8
    lda town_coll, x    ; coll map entry
    bne :+              ; if not zero, collision

    lda W2              ; reload new x
    sta OAM_MIRROR
    lda W1              ; reload and save new y
    sta OAM_MIRROR + 1
:   rts


.a16
.i16
handle_input:
    ; what are we pressing
    lda #0000
    ora joy1_trigger
    ora joy1_held
    sta W1
    ; handle left and right
    bit_tribool JOY_RIGHT_SH, JOY_LEFT_SH
    A8
    sta B3L
    A16
    lda W1
    bit_tribool JOY_DOWN_SH, JOY_UP_SH
    A8
    sta B3H
    ldx #0
    jsr (.loword(handle_player_movement), x)
    rts


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
    lda in_nmi ;load A register with previous in_nmi
@check_again:
	wai ;wait for an interrupt
    cmp in_nmi  ;compare A to current in_nmi
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

.a8
.i16
game_loop:
    jsr wait_nmi ; wait for NMI / V-Blank
    ; we're in vblank, so first upddate video memory things
    jsr update_vram

    jsr read_input
    ;; we already are and stay in A16
    jsr handle_input
    ;; handle_input sets A8

    jmp game_loop


.a8
.i16
main:
    jsr init_game_data

    ; set up sprite OAM data
    lda #(256/2 - 8)       ; horizontal position of first sprite
    sta OAM_MIRROR
    lda #100                ; vertical position of first sprite
    sta OAM_MIRROR + 1
    lda #$00                ; name (place) of first sprite
    sta OAM_MIRROR + 2
    lda #$20                ; no flip, prio 2, palette 0
    sta OAM_MIRROR + 3

    lda #$fe                ; set top bit of x to 0, set 16x16 tile. keep rest of tiles at 1
    sta OAMDATA + 200


    ;; play some music
    lda     #1
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
