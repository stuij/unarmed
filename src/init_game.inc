.ifndef INIT_GAME_INC
INIT_GAME_INC = 1

.code

.a8
.i16
.proc init_title_binary
    ;; palette
    ldy #(title_screen_palette_end - title_screen_palette) ; # of palette entries
    sty <W0
    lda #^title_screen_palette
    ldx #.loword(title_screen_palette)
    ldy #$30
    jsr dma_to_palette

    ldy #(font_palette_end - font_palette) ; # of palette entries
    sty <W0
    lda #^font_palette
    ldx #.loword(font_palette)
    ldy #$00
    jsr dma_to_palette


    ;; tiles
    ldy #(title_screen_tiles_end - title_screen_tiles) ; size of transfer
    sty <W0
    lda #^title_screen_tiles
    ldx #.loword(title_screen_tiles)
    ldy #VRAM_BG_CHR_BASE
    jsr dma_to_vram

    ldy #(font_tiles_end - font_tiles) ; size of transfer
    sty <W0
    lda #^font_tiles
    ldx #.loword(font_tiles)
    ldy #VRAM_FONT_CHR_BASE
    jsr dma_to_vram


    ;; map
    ldy #(title_screen_map_end - title_screen_map) ; size of transfer
    sty a:W0
    lda #^title_screen_map
    ldx #.loword(title_screen_map)
    ldy #VRAM_MAP_BG_GAME_BASE
    jsr dma_to_vram

    rts
.endproc


.a8
.i16
.proc set_lore_background
    ; palette
    ldy #(lore_screen_palette_end - lore_screen_palette) ; # of palette entries
    sty <W0
    lda #^lore_screen_palette
    ldx #.loword(lore_screen_palette)
    ldy #$30
    jsr dma_to_palette

    ; tiles
    ldy #(lore_screen_tiles_end - lore_screen_tiles) ; size of transfer
    sty <W0
    lda #^lore_screen_tiles
    ldx #.loword(lore_screen_tiles)
    ldy #VRAM_BG_CHR_BASE
    jsr dma_to_vram

    ;; map
    ldy #(lore_screen_map_end - lore_screen_map) ; size of transfer
    sty a:W0
    lda #^lore_screen_map
    ldx #.loword(lore_screen_map)
    ldy #VRAM_MAP_BG_GAME_BASE
    jsr dma_to_vram
    rts
.endproc


.a8
.i16
.proc init_binary_data
    ; let's copy over some binary data
    ;; first the palette
    ldy #(city_palette_end - city_palette) ; # of palette entries
    sty <W0
    lda #^city_palette
    ldx #.loword(city_palette)
    ldy #$30
    jsr dma_to_palette

    ldy #(town_palette_end - town_palette) ; # of palette entries
    sty <W0
    lda #^town_palette
    ldx #.loword(town_palette)
    ldy #$20
    jsr dma_to_palette


    ;; load tiles
    ldy #(city_tiles_end - city_tiles) ; size of transfer
    sty <W0
    lda #^city_tiles
    ldx #.loword(city_tiles)
    ldy #VRAM_BG_CHR_BASE
    jsr dma_to_vram

    ldy #(town_tiles_end - town_tiles) ; size of transfer
    sty <W0
    lda #^town_tiles
    ldx #.loword(town_tiles)
    ldy #VRAM_BLOCKS_CHR_BASE
    jsr dma_to_vram

    ;; bg map
    ldy #(city_map_end - city_map) ; size of transfer
    sty a:W0
    lda #^city_map
    ldx #.loword(city_map)
    ldy #VRAM_MAP_BG_GAME_BASE
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
.proc set_game_map
    lda a:game_data + game_data::curr_map
    jsr load_coll_map_to_staging_wram
    A16
    jsr copy_coll_map_staging
    A8
    jsr clear_coll_map_bottom

    lda a:game_data + game_data::curr_map
    jsr load_tilemap_to_staging_wram
    A16
    jsr copy_tile_map_staging
    A8

    rts
.endproc


.a16
.i16
.proc copy_game_map
    jsr copy_coll_map_staging
    jsr copy_tile_map_staging
    A8
    jsr load_tilemap_to_vram
    A16
    rts
.endproc


.a16
.i16
.proc write_title_text
    lda #(VRAM_MAP_FONT_BASE + PRESS_START_FONT_MAP_OFFSET)
    sta VMADD
    lda #.loword(press_start_str)
    jsr print_string
    rts
.endproc


.a8
.i16
;; X - lore text
.proc write_lore
    force_vblank
    A16
    lda #(VRAM_MAP_FONT_BASE + LORE_FONT_MAP_OFFSET)
    sta VMADD
    txa
    jsr print_string
    A8
    unforce_vblank
    rts
.endproc


.a8
.i16
.proc title_screen_init_music

    jsr Tad_SongsStartImmediately

    lda #Song::menu
    jsr load_song

    jsr Tad_GlobalVolumesPersist
    lda #TadCommand::SET_GLOBAL_MUSIC_VOLUME
    ldx #64
    ldy #$ff
    jsr Tad_QueueCommand

    ;; it takes apparently 16 more cycles to actually get Tad_Process
    ;; to do the volume lowering
    A16
    ldx #$10
    stx W4
    A8
        :
        jsr wait_nmi
        jsl Tad_Process
        A16
        dec W4
        lda W4
        A8
        bne :-
    rts
.endproc


.a8
.i16
.proc title_screen_init
    jsr init_title_binary

    ; set bg 1 and bg 2 char vram base addr.
    ; level and menu share character data
    lda #((VRAM_BG_CHR_BASE >> 12) << 4 )
    sta BG12NBA

    lda #((VRAM_MAP_BG_GAME_BASE >> 10) << 2)
    sta BG2SC ; set bg 2 tile map

    stz BG2HOFS
    stz BG2HOFS

    ;; put vertical offset one below, because of how SNES works
    lda #BG_V_TOP_ALIGN
    sta BG2VOFS
    stz BG2VOFS

    lda #BG3_H_OFFS_DEFAULT
    sta BG3HOFS
    stz BG3HOFS

    lda #BG3_V_OFFS_DEFAULT
    sta BG3VOFS
    stz BG3VOFS

    ; set bg 3 and bg4 vram base addr. both the same for now.
    lda #(VRAM_FONT_CHR_BASE >> 12)
    sta BG34NBA

    lda #((VRAM_MAP_FONT_BASE >> 10) << 2)
    sta BG3SC ; set bg 1 tile map

    ;; get stuff in place to be able to run input checking
    A16
    stz p1 + player::joy_trigger

    lda #$ffff
    sta a:game_data + game_data::input_mask

    lda #$4
    sta a:game_data + game_data::no_players
    A8

    ;; set up bg registers
    lda #$9 ; mode 1, BG3 prio high
    sta BGMODE

    A16
    jsr write_title_text
    A8

    lda #$80
    sta VMAIN

    ; turn on BG1, BG2, BG3 and sprites
    lda #($2 | $4)
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP


    ; enable NMI, turn on automatic joypad polling
    lda #$81
    sta NMITIMEN

    rts
.endproc


.proc setup_game_proper
    jsr init_game_data

    ; turn on BG1, BG2, BG3 and sprites
    lda #$17
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP
    rts
.endproc

.a8
.i16
;; set up data
.proc init_game_data
    A16
    lda #4
    sta a:game_data + game_data::no_players_new
    stz a:game_data + game_data::curr_map
    lda #$4
    sta a:game_data + game_data::hitpoints
    lda #$2
    sta a:game_data + game_data::wins

    A8
    jsr init_binary_data
    jsr set_game_map
    jsr init_spawn_boxes
    jsr init_shields
    A16
    jsr reset_game
    jsr init_select_menu
    jsr init_draw_menu
    jsr init_between_games_menu
    jsr init_carets
    jsr set_best_keys
    jsr reset_wins
    jsr reset_matches

    jsr init_score_graphics

    ;; we start in menu mode
    lda a:player_table
    ldx #.loword(between_games_menu)
    ldy #0
    jsr switch_to_menu

    stz a:game_data + game_data::in_game

    lda #$ffff
    sta a:game_data + game_data::input_mask

    ;; set our seed for the prng
    lda #$a723
    sta a:seed


    A8
    ; set bg 3 and bg4 vram base addr. both the same for now.
    lda #(VRAM_FONT_CHR_BASE >> 12)
    sta BG34NBA

    lda #((VRAM_MAP_FONT_BASE >> 10) << 2)
    sta BG3SC ; set bg 1 tile map

    ; set init program variables
    ldx #$FF   ;; first vertical line is blank, and there's an extra line at the
    stx map_y  ;; bottom, so we shift vertical offset by 1
    ldx #$0
    stx map_x

    ;; this is one of those latching regs
    stz BG2HOFS
    stz BG2HOFS

    lda #BG3_H_OFFS_DEFAULT
    sta BG3HOFS
    stz BG3HOFS

    ;; put vertical offset one below, because of how SNES works
    lda #BG_V_TOP_ALIGN
    sta BG2VOFS
    stz BG2VOFS

    lda #BG3_V_OFFS_DEFAULT
    sta BG3VOFS
    stz BG3VOFS

    rts
.endproc


.a16
.i16
.proc reset_game
    jsr reset_hp
    jsr set_player_no
    jsr init_players
    jsr init_bullets
    A8
    jsr hide_players
    jsr hide_menu_carets
    force_vblank
    A16
    jsr copy_game_map
    jsr init_score_graphics
    A8
    unforce_vblank
    A16
    rts
.endproc


.a16
.i16
.proc start_new_game
    A8
    lda #Song::level
    jsr load_song
    A16

    jsr level_screen_to_fore
    jsr reset_game
    jsr init_game_prologue
    rts
.endproc

.endif ; INIT_GAME_INC
