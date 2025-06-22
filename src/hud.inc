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
    lda #HUD_SPRITE_X_BASE
    sta a:W1
    ;; x = current player
    ;; y = sprite OAM offset
    ldx #$0
    ldy #HUD_GFX_OAM_OFFSET
loop:
    ;; this loop mixes placing tiles with graphics

    ;; tiles:
    ;; `W` for how many wins a player had
    print_char 'W'
    print_pad ::HUD_WIN_SKIP

    ;; sprites:
    A8
        ; set x
        lda a:W1
        sta OAM_mirror, y
        clc
        adc #HUD_SPRITE_X_SKIP
        sta a:W1
        iny

        ; set y
        lda #HUD_SPRITE_Y
        sta OAM_mirror, y
        iny

        ; set corresponding tile
        lda #PLAYER_OAM_MUG_SHOT
        sta OAM_mirror, y
        iny

        ; set misc attributes
        ;; set palette bits
        txa
        asl
        ora #$20  ; or with priority bits
        sta OAM_mirror, y
        iny

    A16

    inx
    cpx a:W0
    bne loop

    ;; set all 4 head sprites to 8x8 and on-screen
    stz OAM_mirror + $202

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
