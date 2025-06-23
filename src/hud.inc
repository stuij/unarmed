.ifndef HUD_INC
HUD_INC = 1

.include "defines.inc"

.i16
.a16
.proc init_score_graphics
    lda #VRAM_MAP_FONT_BASE
    sta VMADD
    print_pad ::HUD_WIN_BASE
    ldx #$0
    lda #PLAYER_TABLE_I
    lsr
    sta a:W0
    ;; x = current player
    ldx #$0
loop:
    txa
    jsr print_head
    print_pad ::HUD_WIN_SKIP

    inx
    cpx a:W0
    bne loop

    rts
.endproc


.proc update_score_graphics
    ;; vram start position
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #HUD_SCORE_NR_BASE
    sta VMADD
    tay ;; unfortunately you can't read VMADD, so we need to keep track
        ;; of the offset into VRAM manually
    ldx #$0
loop:
    lda hp, x
    print_nr
    tya
    clc
    adc #HUD_PLAYER_SKIP
    sta VMADD
    tay
    inx
    inx
    cpx #PLAYER_TABLE_I
    bcc loop
    rts
.endproc

.endif ; HUD_INC
