.ifndef DEFINES_INC
DEFINES_INC = 1


;; vram offsets
VRAM_SPRITE_BASE = $2000
VRAM_CHR_BASE = $6000
VRAM_MAP_BASE = $8800 ;; $800 alignment == 256x256 (32x32x2) map

;; move state of player, potential other things
H_VELO_MAX = $60
V_VELO_INIT = $ce
V_VELO_DOWN_MAX = $FF

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

; bounding box
.struct bbox
    top_left .byte
    middle_left .byte
    bottom_left .byte
    top_right .byte
    middle_right .byte
    bottom_right .byte
.endstruct

; x/y nrs are in sub-pixels,
; aka .2 precision fixed point behind the pixel-comma
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
    v_velo      .word
    h_velo      .word
    velo_init   .word
    bbox .tag bbox
.endstruct

.endif ; DEFINES_INC