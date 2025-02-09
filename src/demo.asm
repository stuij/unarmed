.p816   ; 65816 processor
.smart

.define ROM_NAME "demo"
.include "meta-data.inc"
.include "macros.inc"
.include "io.inc"
.include "init.inc"

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


.segment "ZEROPAGE"
W0:
B0H:
.res 1
B0L:
.res 1
in_nmi:
; data read from joypad 13
joy1: .res 2
; trigger read from joypad 1
joy1_trigger: .res 2
; held buttons read from joypad 1
joy1_held: .res 2
; background 1 horizontal offset
bg2_x: .res 2


.code

.i16    ; X/Y are 16 bits
.a8     ; A is 8 bits
reset_handler:
    clc
    xce
	; FYI: coming out of emulation mode, the M and X bits of the
	; status registers are set to one. So resp. A and X/Y
	; are set to 8 bit.
	setXY16
    setA8

    ;; clear all
    jsr clear_registers
    jsr clear_VRAM
    jsr clear_CGRAM
    jsr clear_OAMRAM

    jmp main

VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BASE = $8800 ;; $800 alignment == 256x256 (32x32x2) map

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
.i16
main:
    ;; set up bg registers
    lda #1 ; mode 1
    sta BGMODE
    lda #((VRAM_MAP_BASE >> 10) << 2)
    sta BG2SC ; set bg 2 tile map
    ; set bg 2 char vram base addr (implicitly setting bg1 to 0, but we don't
    ; care)
    lda #((VRAM_CHR_BASE >> 12) << 4)
    sta BG12NBA

    ; let's copy over some binary data
    ;; first the palette
    stz CGADD ; set to begin of palette mem
    ldx #(town_palette_end - town_palette) ; # of palette entries
    ldy #0
@palette_loop:
    lda town_palette, y
    sta CGDATA
    iny
    dex
    bne @palette_loop

    lda #$80
    sta VMAIN

    ; load map
    ldx #VRAM_MAP_BASE
    stx VMADDL
    ldx #0
@map_loop:
     lda town_map,x
     sta VMDATAL
     inx
     lda town_map,x
     sta VMDATAH
     inx
     cpx #(town_map_end - town_map)
     bne @map_loop

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
    lda #$80
    sta CGADD ; set to begin of palette mem
    ldx #(demo_sprite_palette_end - demo_sprite_palette) ; # of palette entries
    ldy #0
@sprite_palette_loop:
    lda demo_sprite_palette, y
    sta CGDATA
    iny
    dex
    bne @sprite_palette_loop

    lda #$80
    sta VMAIN

    ; load map
    ldx #VRAM_MAP_BASE
    stx VMADDL
    ldx #0



    ldx #VRAM_SPRITE_BASE
    stx VMADDL
    ldx #0
@sprite_char_loop:
     lda demo_sprite_tiles,x
     sta VMDATAL
     inx
     lda demo_sprite_tiles,x
     sta VMDATAH
     inx
     cpx #(demo_sprite_tiles_end - demo_sprite_tiles)
     bne @sprite_char_loop

    ; set up sprite OAM data
    stz OAMADDL             ; set the OAM address to ...
    stz OAMADDH             ; ...at $0000
    ; OAM data for first sprite
    lda # (256/2 - 8)       ; horizontal position of first sprite
    sta OAMDATA
    lda #200                ; vertical position of first sprite
    sta OAMDATA
    lda #$00                ; name (place) of first sprite
    sta OAMDATA
    lda #$20                ; no flip, prio 2, palette 0
    sta OAMDATA

    stz OAMADDL             ; set the OAM
    lda #1                  ;   address to
    sta OAMADDH             ;   $0200
    lda #$fe                ; set top bit of x to 0, set 16x16 tile. keep rest of tiles at 1
    sta OAMDATA


    ; turn on BG2
    lda #$12
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP

    ; enable NMI, turn on automatic joypad polling
    lda #$81
    sta NMITIMEN

    ; set program variables
    lda #00000
    sta bg2_x
    

    jmp game_loop

game_loop:
    jsr wait_nmi ; wait for NMI / V-Blank

    ; we're in vblank. first we should do video memory update things
    ; ...

    setA8
wait_for_joypad:
    lda HVBJOY            ; get joypad status
    lsr a                 ; check whether joypad done reading...
    bcs wait_for_joypad   ; ...if not, wait a bit more

    setA16
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

check_right_button:
    lda #$0000                       ; set A to zero
    ora joy1_trigger                 ; check whether the right button was pressed this frame...
    ora joy1_held                    ; ...or held from last frame
    and #JOY_RIGHT
    beq check_right_button_done         ; if neither has occured, move on
    ; else, move bg
    setA8
    lda bg2_x
    inc
    sta bg2_x
    sta BG2HOFS
    stz BG2HOFS
check_right_button_done:

check_left_button:
    setA16
    lda #$0000                       ; set A to zero
    ora joy1_trigger                 ; check whether the up button was pressed this frame...
    ora joy1_held                    ; ...or held from last frame
    and #JOY_LEFT
    beq check_left_button_done         ; if neither has occured, move on
    ; else, move bg
    setA8
    lda bg2_x
    dec
    sta bg2_x
    sta BG2HOFS
    stz BG2HOFS
check_left_button_done:

    jmp game_loop


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

    lda #$18 ; $2118, VMDATAL
    sta BBAD0 ; so set DMA destination to VMDATAL
    lda #1
    sta DMAP0 ; transfer mode, 2 registers 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts



nmi_handler:
    bit RDNMI ; it is required to read this register
              ; in the NMI handler
    inc in_nmi
    rti

irq_handler:
	bit TIMEUP	; it is required to read this register
				; in the IRQ handler
@loop:
    jmp @loop

; This shouldn't get called, so if it does we'd like to know more
; in a controlled manner, instead of say branching to $00000 and
; crashing out violently.
spinloop_handler:
    jmp spinloop_handler


.segment "RODATA"

;; backgrounds
town_tiles:
.incbin"assets/town.tiles"
town_tiles_end:

town_palette:
.incbin"assets/town.palette"
town_palette_end:

town_map:
.incbin"assets/town.map"
town_map_end:

;; sprites
demo_sprite_tiles:
.incbin"assets/demo-sprites.tiles"
demo_sprite_tiles_end:

demo_sprite_palette:
.incbin"assets/demo-sprites.palette"
demo_sprite_palette_end:
