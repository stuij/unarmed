.ifndef INIT_GAME_INC
INIT_GAME_INC = 1

.code

.a8
.i16
.proc init_binary_data
    ; let's copy over some binary data
    ;; first the palette
    ldy #(town_palette_end - town_palette) ; # of palette entries
    sty <W0
    lda #^town_palette
    ldx #.loword(town_palette)
    ldy #$20
    jsr dma_to_palette

    ldy #(font_palette_end - font_palette) ; # of palette entries
    sty <W0
    lda #^font_palette
    ldx #.loword(font_palette)
    ldy #$00
    jsr dma_to_palette


    ; load collision map to wram
    lda #^current_coll_map
    sta <B1L
    ldy #COLL_MAP_SIZE
    sty <W0
    lda #^town_coll
    ldx #.loword(town_coll)
    ldy #.loword(current_coll_map)
    jsr dma_to_wram

    jsr clear_coll_map_bottom

    ; load tiles
    ldy #(town_tiles_end - town_tiles) ; size of transfer
    sty <W0
    lda #^town_tiles
    ldx #.loword(town_tiles)
    ldy #VRAM_CHR_BASE
    jsr dma_to_vram

    ldy #(font_tiles_end - font_tiles) ; size of transfer
    sty <W0
    lda #^font_tiles
    ldx #.loword(font_tiles)
    ldy #VRAM_FONT_CHR_BASE
    jsr dma_to_vram


    ;; this should soon be handled somewhere else

    ; load tile maps to wram
    ; ldy #(fox_map_end - fox_map) ; size of transfer
    ; sty <W0
    ; lda #^fox_map
    ; ldx #.loword(fox_map)
    ; ldy #VRAM_MAP_FONT_BASE
    ; jsr dma_to_vram


    lda #^current_tile_map
    sta <B1L
    ldy #GAME_MAP_SIZE
    sty <W0
    lda #^town_map
    ldx #.loword(town_map)
    ldy #.loword(current_tile_map)
    jsr dma_to_wram

    jsr load_tilemap_to_vram

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

    rts
.endproc


.a8
.i16
.proc init_sound
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
    rts
.endproc


.a8
.i16
;; set up data
.proc init_game_data
    jsr init_binary_data
    jsr init_sound

    A16
    lda #8
    sta a:game_data + game_data::no_players
    ;; ((BULLETS_PER_PLAYER / 2) * PLAYERS)
    lda #$28
    sta a:game_data + game_data::bullets_pp

    jsr init_players
    jsr init_bullets
    jsr init_select_menu
    jsr init_draw_menu
    jsr init_between_games_menu
    jsr init_caret

    jsr reset_hp
    jsr reset_wins

    jsr init_score_graphics

    ;; we start in menu mode
    lda a:player_table
    ldx #.loword(between_games_menu)
    ldy #0
    jsr switch_to_menu

    stz a:game_data + game_data::in_game

    A8
    ;; set up bg registers
    lda #$9 ; mode 1, BG3 prio high
    sta BGMODE

    ; set bg 1 and bg 2 char vram base addr. both the same for now.
    lda #((VRAM_CHR_BASE >> 12) << 4 | (VRAM_CHR_BASE >> 12))
    sta BG12NBA

    ; set bg 3 and bg4 vram base addr. both the same for now.
    lda #(VRAM_FONT_CHR_BASE >> 12)
    sta BG34NBA

    lda #((VRAM_MAP_FONT_BASE >> 10) << 2)
    sta BG3SC ; set bg 1 tile map

    lda #$80
    sta VMAIN

    ; set init program variables
    ldx #$FF   ;; first vertical line is blank, and there's an extra line at the
    stx map_y  ;; bottom, so we shift vertical offset by 1
    ldx #$0
    stx map_x

    ;; this is one of those latching regs
    stz BG2HOFS
    stz BG2HOFS

    lda #$FC
    sta BG3HOFS
    stz BG3HOFS

    ;; put vertical offset one below, because of how SNES works
    lda #$FF
    sta BG2VOFS
    stz BG2VOFS

    lda #$FB
    sta BG3VOFS
    stz BG3VOFS

    rts
.endproc


.proc reset_game
    jsr reset_hp
    jsr init_players
    jsr init_bullets
    rts
.endproc


.a16
.i16
.proc start_new_game
    jsr reset_game
    jsr switch_to_fight
    lda #1
    sta a:game_data + game_data::in_game
    rts
.endproc

.endif ; INIT_GAME_INC
