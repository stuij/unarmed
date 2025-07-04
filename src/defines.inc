.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram things
VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $5000
VRAM_FONT_CHR_BASE = $6000

VRAM_MAP_FONT_BASE = $8800 ;; $800 alignment == 256x256 pixels,
VRAM_MAP_MENU_BASE = $9000 ;; aka (32x32x2 bytes) map
VRAM_MAP_BG_GAME_BASE = $9800

GAME_MAP_SIZE = $700
COLL_MAP_SIZE = $380
COLL_MAP_BOTTOM_SIZE = $60

BG_TILE_PALETTE_ENTRY = $0800 ;; palette 2

; OAM offsets

.struct oam_entry
    x_pos       .byte
    y_pos       .byte
    tile_pos    .byte
    attrs       .byte
.endstruct


OAM_PLAYER_SIZE     = (4 * 2 * 4)
OAM_BULLET_SIZE     = (4 * ((BULLETS_PER_PLAYER / 2) * PLAYER_MAX))
OAM_MENU_CARET_SIZE = (4 * 3)

OAM_MENU_CARET_OFFSET = (OAM_mirror + OAM_PLAYER_SIZE + OAM_BULLET_SIZE)
OAM_MENU_CARET_SELECT_OFFSET = OAM_MENU_CARET_OFFSET + 4

;;------------
;; sprite base struct

;; when doing collision checking, if we move past these consts,
;; we are out of bounds of the playing field, and we incur a penalty
OUT_OF_BOUNDS_X = $fff
OUT_OF_BOUNDS_Y = $f7f

.struct sprite_vtable
    collision_end_callback       .addr
    coll_point_no_coll_callback  .addr
.endstruct

;; locked to player_bbox order
.enum face_dir
    top_left        = $0
    bottom_left     = $2
    top_right       = $4
    bottom_right    = $6
    left            = $8
    top             = $a
    bottom          = $c
    right           = $e
.endenum


.struct point
    y_off .word
    x_off .word
.endstruct

; x/y nrs are in sub-pixels,
; aka .1 precision fixed point behind the pixel-comma
.struct sprite
    ;; v velo and h velo will also automatically give us moving
    ;; direction, and so will tell us what way to correct to
    ;; when moving through barrier tiles
    h_velo              .word ;  0
    x_pos               .word ;  2
    x_new               .word ;  4
    h_velo_inc          .word ;  6
    v_velo              .word ;  8
    y_pos               .word ;  a
    y_new               .word ;  c
    v_velo_dec          .word ;  e
    bbox                .addr ; 10 - pointer to actual bbox
    bbox_fine           .addr ; 12
    bbox_size           .word ; 14 - how many points in the bbox
    move_state          .word ; 16
    face_dir            .word ; 18
    anim_tick           .word ; 1a - keep track of where we should
                              ;      be in our sprite animation
    vptr                .word ; 1c - pointer to the sprite vtable
.endstruct


;; -------------
;; player things


;; for iterating over the player table
;; double the amount of actual players, as we're iterating over words
;; pointing to the player structs
PLAYER_MAX = 4
PLAYER_OAM_OFFSET = $0
PLAYER_OAM_MUG_SHOT = $7 ;; for HUD
PLAYER_OAM_SHOOT_DIR_BASE = ($0 + ($0 * $0))
PLAYER_OAM_RUN_ANIM_BASE = ($0 + ($1 * $10))

PLAYER_HP_START = 5

;; move state of player, potential other things
H_VELO_INC = $1 ; 1 subpixel
H_VELO_MAX = $20
H_VELO_INC_OPPOSITE = $5
H_VELO_INC_RELAX = $2

V_VELO_INIT = $ffd7 ;; -29
V_VELO_DEC = $1 ;; we decrease the minus, until we start plussing
V_VELO_DOWN_MAX = $40

BULLETS_PER_PLAYER = $10 ;; times 2, because it's easier to code with
SHOT_BURST_COUNT = 0 ;; times 1, this will shoot 4 bullets
SHOT_COOLDOWN_FRAMES = 14 ;; half a second

HIT_TIMER_LEN = (3 * 60) ;; after hit, player gets X seconds invulnerability
HIT_H_VELO = 10
HIT_V_VELO = 10

HIT_PALETTE_ID = 4

.enum move_state
    idle
    run
    jump
    cling
    climb
    hit
    recoil
    dead
.endenum

; ------------
; bounding box

.struct square_bbox
    top_left      .tag point ; 0
    bottom_left   .tag point ; 8
    top_right     .tag point ; 14
    bottom_right  .tag point ; 1c
.endstruct


;; points are offset from top-left corner:
;;   one byte for x, and one for y
.struct player_bbox
    square        .tag square_bbox
    middle_left   .tag point ; 4
    top_middle    .tag point ; c
    bottom_middle .tag point ; 10
    middle_right  .tag point ; 18
.endstruct


.struct stance_dir
    left  .word
    right .word
.endstruct


.struct player
    sprite              .tag sprite
    bbox_ledge_lookup   .word   ; 1e
    joy                 .word   ; 20 - last frame buttons
    joy_trigger         .word   ; 22 - only buttons that weren't pressed last frame
    joy_held            .word   ; 24 - buttons pressed this frame and last frame
    joy_trigger_held    .word   ; 26 - both new and held buttons
    h_tribool           .word   ; 28
    v_tribool           .word   ; 2a
    stance_dir          .word   ; 2c
    bullets_left        .word   ; 2e
    ;; fix me: this should really read BULLETS_PER_PLAYER
    ;; but the ca65 assembler has a bug where in these
    ;; circumstances you need to add a global scope specifier: `::`
    ;; and the vscode ca65 plugin I'm using can't parse this.
    bullet_queue        .res $10 ; 30
    nr                  .word   ; 40 - player nr
    shot_burst_counter  .word   ; 42
    shot_cooldown_timer .word   ; 44
    hit_timer           .word   ; 48
    ; when hit, we do hit movement in next frame, to make it easier on
    ; collision detection. So for this we need to know the horizontal
    ; velocity of the thing that hit us. This will give us direction
    ; and force. Although we probably won't do much with the force
    ; aspect to begin with.
    hit_h_velo          .word   ; 4a
    hit_v_velo          .word   ; 4c
    ; just a bit of padding to round the struct for easy eye-balling
    pad1                .word   ; 4e
    pad2                .word   ; 4e ;; pad1's offset isn't right, but that means
                                ; we need to figure out what's wrong, and I'm lazy
.endstruct


;; in game keybindings
.struct igk
    face_top_left       .word
    face_bottom_left    .word
    face_top_right      .word
    face_bottom_right   .word
    face_left           .word
    face_top            .word
    face_bottom         .word
    face_right          .word
    shoot               .word
    jump                .word
.endstruct


;; -------------
;; bullet things

BULLET_OAM_DEFAULT = $00
BULLET_X_POS_INIT = $150
BULLET_Y_POS_INIT = $F00

BULLET_SHOOT_DOWN_Y_ADJUST = $60
BULLET_SHOOT_SIDE_Y_ADJUST = $40

BULLET_H_VELO = $47
BULLET_V_VELO = $47

BULLET_OAM_OFFSET = $20

; sprite tile offset of bullet. first slot on x axis,
; and on 7th row of y axis (rows are $10 wide)
BULLET_SPRITE_OFFSET = ($0 + ($6 * $10))

BULLET_MAX_FLIGHT = 2 * 60 ;; 1 sec


.enum bullet_state
    idle
    fly
.endenum


.struct bullet_bbox
    top_left      .tag point ; 0
.endstruct


.struct bullet
    sprite .tag sprite
    player          .word
    queue_offset    .word
    flight_timer    .word
.endstruct


;; -----
;; menus

MENU_MOVE_TIMEOUT = 8

.enum menus
    select_tile = $0
    stamp_tile = $2
.endenum


.struct menu
    cursor_x_pos_origin .word ; 0
    cursor_y_pos_origin .word ; 2
    cursor_x_pos        .word ; 4
    cursor_y_pos        .word ; 6
    no_rows             .word ; 8
    row_table           .addr ; a
    curr_row            .word ; c
    curr_column         .word ; e
    player              .addr ; 10
    callback            .addr ; 12
    timeout_tick        .word ; 14
    dirty_p             .word ; 16
    just_moved          .word ; 18
    parent_menu         .word ; 1a
    draw_fn             .word ; 1c
    caret_sprite        .word ; 1e
.endstruct


;; tile select menu

.struct select_tile_menu
    menu  .tag menu
    tile_type_names .addr
.endstruct


OAM_MENU_CARET_XTRA_PROPS = OAM_mirror + $20A

SELECT_SCREEN_X_COORDS = $E0
SELECT_SCREEN_X_TABLE_OFFSET = $27
SELECT_SCREEN_Y_COORDS = $ce
SELECT_SCREEN_Y_TABLE_OFFSET = $38


;; canvas draw menu

.struct canvas_menu
    menu  .tag menu
    chosen_tile         .word ; 1a
    tile_type           .word ; 1c
.endstruct

DRAW_CARET_X_ORIGIN = $FFFF
DRAW_CARET_Y_ORIGIN = $FFFF

DRAW_CARET_INIT_ROW = 13
DRAW_CARET_INIT_COLUMN = 16
DRAW_CARET_X_INIT_OFFSET = (DRAW_CARET_INIT_COLUMN * 8) - 1
DRAW_CARET_Y_INIT_OFFSET = (DRAW_CARET_INIT_ROW * 8) - 1


;; between games screen

; - wins section; not parts of games screen proper
; - start game
; - customize level
; - player count select
; - level select

; TODO:
; - amount of hitpoints
; - wins per match
; - level select type: manual or random

;; for setting BG
BETWEEN_MENU_X_COORDS = $e0
BETWEEN_MENU_Y_COORDS = $ce

BETWEEN_MENU_X_TABLE_OFFSET = $73
BETWEEN_MENU_Y_TABLE_OFFSET = ($d * $8) + 3
BETWEEN_PLAYER_SKIP = $18

BETWEEN_MENU_ROW_COUNT = 4

BETWEEN_MENU_PLAYER_FONT_MAP_OFFSET = $e6
BETWEEN_MENU_ITEMS_FONT_MAP_OFFSET = $1a5

BG3_HEAD_CHAR_OFFSET = $60

.struct between_games_menu
    menu .tag menu
.endstruct


.enum between_rows
    start
    edit
    players
    level
.endenum ; Enm between


;; ----------------
;; global game data

.struct game_data
    no_players      .word
    no_players_new  .word
    bullets_pp      .word
    curr_map        .word
    fight_p         .word
    game_handler    .word
    curr_menu       .word
    menu_dirty_draw .word
    in_game         .word
    ; AND filter for any of the input buttons
    ; masking in the input polling code
    input_mask      .word
.endstruct

.struct hp
    p1_hp           .word
    p2_hp           .word
    p3_hp           .word
    p4_hp           .word
.endstruct


.struct wins
    p1_wins         .word
    p2_wins         .word
    p3_wins         .word
    p4_wins         .word
.endstruct


; ---
; HUD

ASCII_NR_BASE_OFFSET = 48 - 32  ; 48 is ascii position of 0,
                                ; and our tiles start from ascii nr 32

HUD_WIN_BASE = 4
HUD_WIN_SKIP = 2

HUD_SCORE_NR_BASE = 5
HUD_PLAYER_SKIP = 3

HUD_SPRITE_X_BASE = $1a
HUD_SPRITE_X_SKIP = $38
HUD_SPRITE_Y = 3

.endif ; DEFINES_INC
