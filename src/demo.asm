.p816   ; 65816 processor
.smart

.i16    ; X/Y are 16 bits
.a8     ; A is 8 bits

.define ROM_NAME "demo"
.include "header.inc"
.include "macros.inc"

.segment "VECTORS"
    .word 0, 0, 0, 0, 0, 0, 0, 0
    .word 0, 0, 0, 0, 0, 0, reset, 0

.code

reset:
    init_cpu

    ; Clear PPU registers
    ldx #$33
@loop:  stz $2100,x
    stz $4200,x
    dex
    bpl @loop

    ; Set background color to $03E0
    lda #$E0
    sta $2122
    lda #$03
    sta $2122

    ; Maximum screen brightness
    lda #$0F
    sta $2100

forever:
    jmp forever
