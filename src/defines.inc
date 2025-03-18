.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram offsets
VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BASE = $8800 ;; $800 alignment == 256x256 (32x32x2) map

;; move state of player, potential other things
H_VELO_INC = $1 ; 1 subpixel
H_VELO_MAX = $60
V_VELO_INIT = $FFD0 ;; -30
V_VELO_DEC = $1 ;; we decrease the minus, until we start plussing
V_VELO_DOWN_MAX = $42

.enum move_state
    idle
    run
    jump_up
    jump_down
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
    top_left      .tag point
    middle_left   .tag point
    bottom_left   .tag point
    top_middle    .tag point
    bottom_middle .tag point
    top_right     .tag point
    middle_right  .tag point
    bottom_right  .tag point
.endstruct

; x/y nrs are in sub-pixels,
; aka .1 precision fixed point behind the pixel-comma
.struct player
    joy         .word ; last frame buttons
    joy_trigger .word ; only buttons that weren't pressed last frame
    joy_held    .word ; buttons pressed this frame and last frame
    joy_trigger_held .word ; both new and held buttons
    move_state  .word
    face_dir    .word
    h_tribool   .word
    v_tribool   .word
    x_pos       .word
    y_pos       .word
    x_new       .word
    y_new       .word
    ;; v velo and h velo will also automatically give us moving
    ;; direction, and so will tell us what way to correct to
    ;; when moving through barrier tiles
    h_velo      .word
    v_velo      .word
    v_velo_dec  .word
    bbox        .addr ;; pointer to actual bbox
.endstruct

.endif ; DEFINES_INC