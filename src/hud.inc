.ifndef HUD_INC
HUD_INC = 1

.include "defines.inc"

.i16
.a16
init_score_graphics:
    lda #VRAM_MAP_FONT_BASE
    sta VMADD
    print_pad 5
    ldx #$1
    lda #PLAYER_TABLE_I
    lsr
    sta .loword(W0)
init_score_graphs_loop:
    print_char 'p'
    txa
    clc
    adc #ASCII_NR_BASE_OFFSET
    ora #$2000 ; set priority bit on tile
	sta VMDATA
    print_char ':'
    print_pad 3
    cpx .loword(W0)
    inx
    bcc init_score_graphs_loop
    rts


update_score_graphics:
    ;; end condition
    lda #PLAYER_TABLE_I
    sta .loword(W0)
    ;; vram start position
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #SCORE_NR_BASE
    sta VMADD
    tay
    ldx #$0
update_score_graphics_loop:
    lda hp, x
    clc
    adc #ASCII_NR_BASE_OFFSET
    ora #$2000 ; set priority bit on tile
	sta VMDATA
    tya
    clc
    adc #SCORE_NR_SKIP
    sta VMADD
    tay
    inx
    inx
    cpx .loword(W0)
    bcc update_score_graphics_loop
    rts


.endif ; HUD_INC