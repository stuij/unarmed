.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram offsets
VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BASE = $8800 ;; $800 alignment == 256x256 (32x32x2) map

;; move state of player, potential other things
H_VELO_INC = $1 ; 1 subpixel
H_VELO_INC_OPPOSITE = $5
H_VELO_INC_RELAX = $2
H_VELO_MAX = $40
V_VELO_INIT = $ffd0 ;; -30
V_VELO_DEC = $1 ;; we decrease the minus, until we start plussing
V_VELO_DOWN_MAX = $42

P1_START_X = $7a0
P1_START_Y = $970

.enum move_state
    idle
    run
    jump
    cling
    climb
.endenum

.enum face_dir
    left
    right
.endenum

.struct point
    y_off .word
    x_off .word
.endstruct

; bounding box
;; points are offset from top-left corner:
;;   one byte for x, and one for y
.struct player_bbox
    top_left      .tag point ; 0
    middle_left   .tag point ; 4
    bottom_left   .tag point ; 8
    top_middle    .tag point ; c
    bottom_middle .tag point ; 10
    top_right     .tag point ; 14
    middle_right  .tag point ; 18
    bottom_right  .tag point ; 1c
.endstruct

; x/y nrs are in sub-pixels,
; aka .1 precision fixed point behind the pixel-comma
.struct player
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
    move_state          .word ; 12
    tmp                 .word ; 14 - scratch pad
    face_dir            .word ; 16
    ledge_p             .word ; 18
    bbox_ledge_lookup   .word ; 1a
    joy                 .word ; 1c - last frame buttons
    joy_trigger         .word ; 1e - only buttons that weren't pressed last frame
    joy_held            .word ; 20 - buttons pressed this frame and last frame
    joy_trigger_held    .word ; 22 - both new and held buttons
    h_tribool           .word ; 24
    v_tribool           .word ; 26
    ;; these are just padding to more easily see the
    ;; values of the individual players in the debugger
    p1                  .word ; 28
    p2                  .word ; 2a
    p3                  .word ; 2c
    p4                  .word ; 2e

.endstruct

.endif ; DEFINES_INC
