.p816   ; 65816 processor
.smart

.define ROM_NAME "Unarmed" ; max 21 chars

.include "defines.inc"
.include "header.inc"
.include "macros.inc"
.include "io.inc"

.include "init.inc"
.include "lib.inc"
.include "vectors.inc"
.include "../build/assets/audio.inc"
.include "../terrific-audio-driver/audio-driver/ca65-api/tad-audio.inc"
.include "assets.inc"
.include "data.inc"
.include "player.inc"
.include "bullet.inc"
.include "menu.inc"
.include "sound.inc"
.include "init_game.inc"
.include "input.inc"
.include "coll_bg.inc"
.include "coll_sprite.inc"
.include "main_loop.inc"
.include "hud.inc"


.code

;; ---------
;; score-keeping

.i16
.a16
.proc reset_hp
    lda a:game_data + game_data::hitpoints
    sta hp
    sta hp + 2
    sta hp + 4
    sta hp + 6
    rts
.endproc 


.i16
.a16
.proc reset_wins
    stz wins
    stz wins + 2
    stz wins + 4
    stz wins + 6
    rts
.endproc


.i16
.a16
.proc reset_matches
    stz matches
    stz matches + 2
    stz matches + 4
    stz matches + 6
    rts
.endproc


;; X - player offset
.proc player_dec_hp
    lda hp, x
    cmp #1
    beq hp_zero

    A8
    lda #SFX::hurt
    jsr Tad_QueueSoundEffect_D
    A16

    dec hp, x
    rts
hp_zero:
    dec hp, x
    jsr player_dead
    rts
.endproc


;; ---------
;; game loop

.a8
.i16
.proc wait_nmi
    ; should work fine regardless of size of A
    lda a:in_nmi    ; load A register with previous in_nmi
check_again:
	wai             ; wait for an interrupt
    cmp a:in_nmi    ; compare A to current in_nmi
                    ; wait for it to change
                    ; make sure it was an nmi interrupt
    beq check_again
    rts
.endproc


.a16
.i16
;; A - current player
.proc switch_game_mode
    sta a:W0
    lda a:game_data + game_data::game_handler
    cmp #.loword(handle_main_loop)
    beq switch_to_menu_mode
    ;; we're switching to fight_mode
    jsr switch_to_fight
    bra return
switch_to_menu_mode:
    lda a:W0
    ldx #.loword(between_games_menu)
    ldy #0
    ;; we initialize the select menu
    jsr switch_to_menu
return:
    rts
.endproc


;; Check if a player pressed a button to take us out of
;; a main loop. So for example if we press `select` during gameplay
;;
;; We need to do this before our main game loop, so we can move to another
;; state before we've already processed some of the player's data
;; and we find ourselves in an inconsitent game state.
;;
;; I'd like to do this in `read_input` but it kinda breaks the abstraction.
.a16
.i16
.proc check_game_state_change
    ldx #0
loop:
    lda a:player_table, x
    tcd ;; remapping dp to player x
    lda player::joy_trigger
    and #JOY_START
    beq continue
    lda a:game_data + game_data::in_game
    beq continue
    ;; player pressed start in-game. we drop them in, for now,
    ;; the select screen. First we record the player in the menu,
    ;; as it's their presses we care about
    tdc
    jsr switch_game_mode
    bra end
continue:
    ;; bit dangerous. we assume the only way we get here is if
    ;; no-one presses a button and so no interesting thing happens
    ;; to disrupt X. Otherwise we should do some register saving
    ;; and restoring magic.
    inx
    inx
    cpx a:game_data + game_data::no_players
    bne loop
end:
    lda #$0
    tcd
    rts
.endproc


.a8
.i16
.proc update_bgs
    lda map_x
    ;; this is one of those latching regs
    sta BG1HOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG1HOFS

    lda map_y
    ;; this is one of those latching regs
    sta BG1VOFS
    ;; we're effectively lobbing off the top two bits of the offset..
    stz BG1VOFS
    rts
.endproc


.a8
.i16
.proc update_vram
    ;; jsr update_bgs
    jsr vblank_draw_menu
    A16
    I16
    jsr update_score_graphics
    A16
    I8
    jsr dma_OAM
    ;; switch back for sanity
    I16
    ;; basically we're guarding against overwriting vram with
    ;; in mem representation if we're in draw mode. but we only really
    ;; care updating every frame for the main loop, so this should be
    ;; good enough.
    lda a:game_data + game_data::game_handler
    cmp #.loword(handle_main_loop)
    A8
    bne end
    jsr load_tilemap_to_vram
end:
    rts
.endproc


.a8
.i16
.proc game_loop
    jsr wait_nmi ; wait for NMI / V-Blank

    ; we're in vblank, so first update video memory things
    jsr update_vram

    ; then do the rest
    jsr read_input

    A16
    jsr check_game_state_change

    ldx #$0
    jsr (game_data + game_data::game_handler, x)
    A8
    jsl Tad_Process
    jmp game_loop
.endproc


.a8
.i16
.proc wait_for_start_loop
    loop:
        jsr wait_nmi
        jsr read_input

        A16
        ;; if start, continue
        lda p1 + player::joy_trigger
        and #JOY_START
        A8

        beq loop

    rts
.endproc


.a8
.i16
.proc clear_lore_text
    ldy #(VRAM_MAP_FONT_BASE + LORE_FONT_MAP_OFFSET)
    ldx #($18 * $20 * 2)
    stz a:W0
    jsr dma_fixed_to_vram
    rts
.endproc


;; ----
;; main

.a8
.i16
.proc main
    jsr init_sound
    jsr title_screen_init
    jsr title_screen_init_music
    jsr wait_for_start_loop

    force_vblank
    jsr set_lore_background
    unforce_vblank
    ldx #.loword(lore_text)
    jsr write_lore
    jsr wait_for_start_loop

    ldx #.loword(controls_text)
    jsr write_lore
    jsr wait_for_start_loop

    lda #$80
    sta INIDISP

    jsr clear_lore_text
    jsr setup_game_proper
    jmp game_loop
.endproc
