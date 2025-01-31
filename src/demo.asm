.p816   ; 65816 processor
.smart

.i16    ; X/Y are 16 bits
.a8     ; A is 8 bits

.define ROM_NAME "demo"
.include "meta-data.inc"
.include "macros.inc"
.include "io.inc"

.segment "VECTORS"
    .word 0, 0, 0, 0, 0, 0, 0, 0
    .word 0, 0, 0, 0, 0, 0, reset, 0

.code

reset:
    init_cpu

    ; Clear PPU registers
    ldx #$33
@loop:  stz INIDISP,x
    stz NMITIMEN,x
    dex
    bpl @loop

    ; Set background color to $03E0
    lda #$E0
    sta CGDATA
    lda #$03
    sta CGDATA

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP

forever:
    jmp forever
