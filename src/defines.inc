.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram things


;; we're deviding everything by two because
;; VRAM sees everything as 32K words,
;; instead of 64K bytes

;; sprite base is 2x8KB blocks aka 2x$2000 KB blocks for the two sprite sheets
;; for now, we'll see if 1 $2000 KB block is enough
VRAM_SPRITE_BASE = $0000

;; BG CHAR alignment: $2000 B
VRAM_BG_CHR_BASE = VRAM_SPRITE_BASE + ($2000 / 2)
VRAM_BLOCKS_CHR_BASE = VRAM_BG_CHR_BASE + ($4000 / 2)
VRAM_FONT_CHR_BASE = VRAM_BLOCKS_CHR_BASE + ($2000 / 2)

;; maps
;; (if need be, we can put these within the gaps between the char data blocks)
;; $800 alignment == 256x256 pixels,
;; aka (32x32x2 bytes) map
VRAM_MAP_FONT_BASE = VRAM_FONT_CHR_BASE + ($2000 / 2)
VRAM_MAP_MENU_BASE = VRAM_MAP_FONT_BASE + ($800 / 2)
VRAM_MAP_BLOCKS_GAME_BASE = VRAM_MAP_MENU_BASE + ($800 / 2)
VRAM_MAP_BG_GAME_BASE = VRAM_MAP_BLOCKS_GAME_BASE + ($800 / 2)

;; ---------------------------- +
;; $B800 bytes used, $4800 free


GAME_MAP_SIZE = $700
COLL_MAP_SIZE = $380
COLL_MAP_BOTTOM_SIZE = $60

BG_TILE_PALETTE_ENTRY = $0800 ;; palette 2


;; ----------
;; BG offsets

BG_H_TOP_ALIGN = 0
;; one scanline down for aligning with top, because of how the SNES hardware works
BG_V_TOP_ALIGN = $FF

BG3_H_OFFS_DEFAULT = $FC
BG3_V_OFFS_DEFAULT = $F8

;; OAM offsets

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

OAM_SPAWN_BOX_OFFSET = OAM_MENU_CARET_SELECT_OFFSET + (7 * 4)
OAM_SHIELD_OFFSET = OAM_SPAWN_BOX_OFFSET + (4 * 4)

;; xtra props
OAM_MENU_CARET_XTRA_PROPS = OAM_mirror + $20A
OAM_SPAWN_BOX_XTRA_PROPS = OAM_MENU_CARET_XTRA_PROPS + 2
OAM_SHIELD_XTRA_PROPS = OAM_SPAWN_BOX_XTRA_PROPS + 1


;; --------
;; BG tiles

.enum tile_props
    wall            = $1
    bounce          = $2
    destructable    = $4
    kill            = $8
    out_of_bounds   = $80
.endenum


;;------------
;; sprite base struct

;; when doing collision checking, if we move past these consts,
;; we are out of bounds of the playing field, and we incur consequences
OUT_OF_BOUNDS_X = $fff
OUT_OF_BOUNDS_Y = $f7f

.struct sprite_vtable
    collision_end_callback      .addr
    coll_point_no_coll_callback .addr
    coll_point_coll_callback    .addr
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
V_VELO_DOWN_MAX = $20

BULLETS_PER_PLAYER = $10 ;; times 2, because it's easier to code with
SHOT_BURST_COUNT = 0 ;; times 1, this will shoot 4 bullets
SHOT_COOLDOWN_FRAMES = $30

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


.enum stance_dir
    left
    right
.endenum


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
    ; when hit, we do hit movement in next frame, to make it easier on
    ; collision detection. So for this we need to know the horizontal
    ; velocity of the thing that hit us. This will give us direction
    ; and force. Although we probably won't do much with the force
    ; aspect to begin with.
    hit_h_velo          .word   ; 42
    hit_v_velo          .word   ; 44
    ; just a bit of padding to round the struct for easy eye-balling
    parry_timer         .word   ; 46
    shot_burst_counter  .word   ; 48
    shot_cooldown_timer .word   ; 4a
    hit_timer           .word   ; 4c
    ;; shot delay: we're trading one or two frames of delay
    ;; for more chance of diagonals being detected.
    ;; meaning of possible different values:
    ;; 0 - we're not in shot delay
    ;; 1 - we shoot in this frame
    ;; > 1 - we're in shot delay, and counting down
    shot_delay          .word   ; 4e
.endstruct


SHOT_DELAY_AMOUT = $2

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
    parry               .word
.endstruct

;; ------
;; parry

; relative to player sprite x origin
SHIELD_LEFT_OFFSET = $110
SHIELD_RIGHT_OFFSET = $80

PARRY_COUNT = $14
PARRY_COOLDOWN_ZONE = $7

;; -------------
;; bullet things

BULLET_OAM_DEFAULT = $00
BULLET_X_POS_INIT = $150
BULLET_Y_POS_INIT = $F00

BULLET_SHOOT_DOWN_Y_ADJUST = $60
BULLET_SHOOT_SIDE_Y_ADJUST = $40

BULLET_H_VELO = $27
BULLET_V_VELO = $27

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
    rogue       .word ;; if set, can hit owner
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

SELECT_SCREEN_X_COORDS = $e4
SELECT_SCREEN_Y_COORDS = $cc

SELECT_SCREEN_X_TABLE_OFFSET = $23
SELECT_SCREEN_Y_TABLE_OFFSET = $52

SELECT_SCREEN_DRAW_BEHIND_OFFSET = ($a * $20 + $4)
SELECT_SCREEN_DRAW_BEHIND_AMOUNT = ($8 * $20)

SELECT_SCREEN_HEADER_OFFSET = $5 + ($7 * $20)

SELECT_SCREEN_TILE_TYPE_DESC_HEADER_OFFSET = $5 + ($12 * $20)
SELECT_SCREEN_TILE_TYPE_DESC_OFFSET = $7 + ($13 * $20)
SELECT_SCREEN_TILE_TYPE_ERASE_LENGTH = $1e ;; 15 chars x 2 cause tiles are a word long

SELECT_SCREEN_LEGEND_MAP_OFFSET = $5 + ($15 * $20)

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

DRAW_SCREEN_LEGEND_MAP_OFFSET = $1 + ($18 * $20)


;; --------------------
;; between games screen

;; for setting BG
BETWEEN_MENU_X_COORDS = SELECT_SCREEN_X_COORDS
BETWEEN_MENU_Y_COORDS = SELECT_SCREEN_Y_COORDS

BETWEEN_MENU_LEGEND_MAP_OFFSET = $5 + ($15 * $20)

BETWEEN_MENU_X_TABLE_OFFSET = $8b
BETWEEN_MENU_Y_TABLE_OFFSET = ($d * $8) + 6
BETWEEN_PLAYER_SKIP = $18

BETWEEN_MENU_ROW_COUNT = $6

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
    hitpoints
    wins
    level
.endenum ; Enm between


;; -------------
;; overlay menus

OVERLAY_TEXT_START      = (($a * $20) + $5)
OVERLAY_START_DURATION  = (60 * 1) + 30
OVERLAY_END_DURATION    = (60 * 2)
OVERLAY_INPUT_MASK      = 0


;; ----------------
;; global game data

.struct game_data
    no_players      .word
    no_players_new  .word
    wins            .word
    wins_new        .word
    hitpoints       .word
    bullets_pp      .word
    game_handler    .word
    menu_dirty_draw .word
    curr_map        .word
    curr_menu       .word
    curr_winner     .word
    in_game         .word
    ; AND filter for any of the input buttons
    ; masking in the input polling code
    input_mask      .word
    overlay_counter .word
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

.struct matches
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
