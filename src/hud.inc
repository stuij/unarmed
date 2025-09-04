.ifndef HUD_INC
HUD_INC = 1

.include "defines.inc"

.i16
.a16
.proc init_score_graphics
    lda #VRAM_MAP_FONT_BASE
    sta VMADD
    print_pad ::HUD_WIN_BASE

    lda #BG3_HEART_TILE_OFFSET
    ora #$2000 ; set priority bit on tile
    sta VMDATA

    print_char ' '
    ldx #$0
    lda a:game_data + game_data::no_players
    lsr
    sta a:W0
    ;; x = current player
    ldx #$0
  : txa
    jsr print_head
    print_pad ::HUD_WIN_SKIP

    inx
    cpx a:W0
    bne :-

    print_pad $10 ;; to overwrite potential artifacts from
                  ;; when last player count was higher

    lda #(VRAM_MAP_FONT_BASE + HUD_SET_BASE)
    sta VMADD


    lda #1
    lshift 10 ;; set palette
    ora #BG3_TROPHY_TILE_OFFSET
    ora #$2000 ; set priority bit on tile
    sta VMDATA

    print_char ' '

    ldx #$0
    lda a:game_data + game_data::no_players
    lsr
    sta a:W0
    ;; x = current player
    ldx #$0
  : txa
    jsr print_head
    print_pad ::HUD_WIN_SKIP

    inx
    cpx a:W0
    bne :-

    print_pad $10 ;; to overwrite potential artifacts from
                  ;; when last player count was higher


    rts
.endproc


.i16
.a16
.proc update_score_graphics
    ;; update hitpoints
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #HUD_SCORE_HP_BASE
    sta VMADD
    tay ;; unfortunately you can't read VMADD, so we need to keep track
        ;; of the offset into VRAM manually
    ldx #$0
  : lda hp, x
    print_nr
    tya
    clc
    adc #HUD_PLAYER_SKIP
    sta VMADD
    tay
    inx
    inx
    cpx a:game_data + game_data::no_players
    bcc :-

    ;; update sets
    lda #VRAM_MAP_FONT_BASE
    clc
    adc #HUD_SCORE_SET_BASE
    sta VMADD
    tay ;; unfortunately you can't read VMADD, so we need to keep track
        ;; of the offset into VRAM manually
    ldx #$0
  : lda wins, x
    print_nr
    tya
    clc
    adc #HUD_PLAYER_SKIP
    sta VMADD
    tay
    inx
    inx
    cpx a:game_data + game_data::no_players
    bcc :-
    rts



.endproc

.endif ; HUD_INC
