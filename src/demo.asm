.p816   ; 65816 processor
.smart

.define ROM_NAME "demo"
.include "meta-data.inc"
.include "macros.inc"
.include "io.inc"

.segment "VECTORS"
    .word 0, 0, 0, 0, 0, 0, 0, 0
    .word 0, 0, 0, 0, 0, 0, reset, 0

.code

.i16    ; X/Y are 16 bits
.a8     ; A is 8 bits
reset:
    clc
    xce
	; FYI: coming out of emulation mode, the M and X bits of the
	; status registers are set to one. So resp. A and X/Y
	; are set to 8 bit.
	setXY16
    setA8

    ; Clear PPU registers
    ldx #$33
@loop:  stz INIDISP,x
    stz NMITIMEN,x
    dex
    bpl @loop

    lda #$80
    sta INIDISP ; Turn off screen (forced blank)

    jmp main


VRAM_MAP_BASE = $0000
VRAM_CHR_BASE = $1000

.i16
.a8
main:
    ;; set up bg registers
    lda #1 ; mode 1
    sta BGMODE
    lda #(VRAM_MAP_BASE >> 10) ; still 0
    sta BG1SC ; set bg 2 tile map
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
    ldx #VRAM_CHR_BASE
    stx VMADDL
    ldx #0
@char_loop:
     lda town_tiles,x
     sta VMDATAL
     inx
     lda town_tiles,x
     sta VMDATAH
     inx
     cpx #(town_tiles_end - town_tiles)
     bne @char_loop

    ; turn on BG2
    lda #2
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP

forever:
    jmp forever

.segment "RODATA"

town_tiles:
.incbin"assets/town.tiles"
town_tiles_end:

town_palette:
.incbin"assets/town.palette"
town_palette_end:

town_map:
.incbin"assets/town.map"
town_map_end:
