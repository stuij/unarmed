.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram things
VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BG1_BASE = $9000 ;; $800 alignment == 256x256 pixels,
VRAM_MAP_BG2_BASE = $9800 ;; aka (32x32x2 bytes) map
GAME_MAP_SIZE = $700
COLL_MAP_SIZE = $380



; OAM offsets

.struct oam_entry
    x_pos       .byte
    y_pos       .byte
    tile_pos    .byte
    attrs       .byte
.endstruct


OAM_PLAYER_SIZE     = (4 * 4)
OAM_BULLET_SIZE     = (4 * 32)
OAM_MENU_CARET_SIZE = (1 * 32)

OAM_MENU_CARET_OFFSET = (OAM_MIRROR + OAM_PLAYER_SIZE + OAM_BULLET_SIZE)

;;------------
;; sprite base struct

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
    anim_tick           .word ; 20 - keep track of where we should
                              ;      be in our sprite animation
    vptr                .word ; 22 - pointer to the sprite vtable
.endstruct


;; -------------
;; player things

;; for iterating over the player table
;; double the amount of actual players, as we're iterating over words
;; pointing to the player structs
PLAYER_TABLE_I = $8
PLAYER_OAM_OFFSET = $0
PLAYER_OAM_SHOOT_DIR_BASE = ($0 + ($0 * $0))
PLAYER_OAM_RUN_ANIM_BASE = ($0 + ($1 * $10))

;; move state of player, potential other things
H_VELO_INC = $1 ; 1 subpixel
H_VELO_MAX = $20
H_VELO_INC_OPPOSITE = $5
H_VELO_INC_RELAX = $2

V_VELO_INIT = $ffd7 ;; -29
V_VELO_DEC = $1 ;; we decrease the minus, until we start plussing
V_VELO_DOWN_MAX = $40

.enum move_state
    idle
    run
    jump
    cling
    climb
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
    bbox_ledge_lookup   .word ; 24
    joy                 .word ; 26 - last frame buttons
    joy_trigger         .word ; 28 - only buttons that weren't pressed last frame
    joy_held            .word ; 2a - buttons pressed this frame and last frame
    joy_trigger_held    .word ; 2c - both new and held buttons
    h_tribool           .word ; 2e
    v_tribool           .word ; 30
    bullets             .word ; 32
    stance_dir          .word ; 34
;   crouch              .word ; 36
.endstruct


;; -------------
;; bullet things

BULLET_OAM_DEFAULT = $00
BULLET_X_POS_INIT = $150
BULLET_Y_POS_INIT = $10

BULLET_SHOOT_DOWN_Y_ADJUST = $60
BULLET_SHOOT_SIDE_Y_ADJUST = $40

BULLET_H_VELO = $77
BULLET_V_VELO = $77

BULLET_TABLE_I = $8

BULLET_OAM_OFFSET = $20

; sprite tile offset of bullet. first slot on x axis,
; and on 7th row of y axis (rows are $10 wide)
BULLET_SPRITE_OFFSET = ($0 + ($6 * $10))


.enum bullet_state
    fly
    rest
.endenum


.struct bullet_bbox
    top_left      .tag point ; 0
.endstruct


.struct bullet
    sprite .tag sprite
.endstruct


;; -----
;; menus
MENU_MOVE_TIMEOUT = 8

.enum menus
    select_tile = $0
    stamp_tile = $2
.endenum

.struct menu
    cursor_x_pos_origin .word
    cursor_y_pos_origin .word
    cursor_x_pos        .word
    cursor_y_pos        .word
    no_rows             .word
    row_table           .addr
    curr_row            .word
    curr_column         .word
    player              .addr
    callback            .addr
    timeout_tick        .word
    just_moved          .word
.endstruct


;; tile menu
.struct select_tile_menu
    menu  .tag menu
    tile_type_names .addr
.endstruct


SELECT_SCREEN_X_COORDS = $E0
SELECT_SCREEN_X_TABLE_OFFSET = $27
SELECT_SCREEN_Y_COORDS = $ce
SELECT_SCREEN_Y_TABLE_OFFSET = $38

;; ----------------
;; global game data

SELECT_CARET_SPRITE_OFFSET = ($0 + ($4 * $10))

.struct game_data
    no_players      .word
    curr_map        .word
    curr_map_dirty  .word
    game_handler    .word
    curr_menu       .word
.endstruct

.endif ; DEFINES_INC
