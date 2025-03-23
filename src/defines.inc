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
    joy         .word ; 0 - last frame buttons
    joy_trigger .word ; 2 - only buttons that weren't pressed last frame
    joy_held    .word ; 4 - buttons pressed this frame and last frame
    joy_trigger_held .word ; 6 - both new and held buttons
    move_state  .word ; 8
    face_dir    .word ; a
    h_tribool   .word ; c
    v_tribool   .word ; e
    ;; v velo and h velo will also automatically give us moving
    ;; direction, and so will tell us what way to correct to
    ;; when moving through barrier tiles
    h_velo      .word ; 10
    x_pos       .word ; 12
    x_new       .word ; 14
    v_velo      .word ; 16
    y_pos       .word ; 18
    y_new       .word ; 1a
    v_velo_dec  .word ; 1c
    bbox        .addr ; 1e - pointer to actual bbox
    tmp         .word ; 20 - scratch pad
.endstruct

.endif ; DEFINES_INC