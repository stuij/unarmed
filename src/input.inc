.ifndef INPUT_INC
INPUT_INC = 1

;; -----
;; input

;; kernel of this fn taken from serial NES read joy routine from
;; nesdev wiki: https://www.nesdev.org/wiki/Controller_reading_code
.a16
.i16
.proc read_serial
    lda #$1
    sta a:W2
    A8
    lda #$7F
    sta WRIO
    ;; Still not sure if prodding $4016 is needed after automatic
    ;; reading of p1-3, but snes9x, which on MacOS doesn't stay in
    ;; multitap mode when starting a game will show jumping behavior
    ;; so first bit is 1, if I don't tell controller to re-latch.
    ;; Mesen is working fine for 4 players when set in multitap mode
    ;; either way. Not having erratic behaviour on non-multitap mode
    ;; alone justifies just doing the prodding.
    lda #$1
    sta JOYOUT
    lsr
    sta JOYOUT
read_serial_loop:
    A8
    lda JOYSER1
    A16
    lsr              ; bit 0 -> Carry
    rol a:W2         ; Carry -> bit 0; bit 15 -> Carry
    bcc read_serial_loop
    A8
    lda #$FF
    sta WRIO
    A16
    lda a:W2
    rts
.endproc


.a8
.i16
.proc read_input
wait_for_joypad:
    lda HVBJOY            ; get joypad status
    lsr a                 ; check whether joypad done reading...
    bcs wait_for_joypad   ; ...if not, wait a bit more

    ; read joypad
    A16
    ldx #0 ;; we use this for both the offset into the player table
           ;; as well as the offset into the joypad info hw regs, as both are
           ;; spaced one word apart.
joy_loop:
    stx a:W1
    lda a:player_table, x
    tcd ;; remapping dp to player x
    cpx #$6
    beq joy_read_serial
    cpx #$4 ; in multitap config, the third physical joypad is read
            ; from JOY4
    bne :+
    inx
    inx
  : lda JOY1L, x                     ; get new input from this frame
    bra joy_set_player_vals
joy_read_serial:
    jsr read_serial
joy_set_player_vals:
    jsr set_player_joy
    ;; and increment x as loop var again
    ldx a:W1
    inx
    inx
    cpx #PLAYER_TABLE_I
    bne joy_loop
    lda #$0
    tcd
    rts
.endproc


.proc set_player_joy
    ldy player::joy                  ; get input from last frame
    sta player::joy                  ; store new input from this frame
    tya                              ; check for newly pressed buttons...
    eor player::joy                  ; filter buttons that were not pressed last frame
    and player::joy                  ; filter held buttons from last frame
    sta player::joy_trigger          ; ...and store them
    ; second, check for buttons held from last frame
    tya                              ; get input from last frame
    and player::joy                  ; filter held buttons from last frame...
    sta player::joy_held             ; ...store them
    ; for convenience store the or of trigger and held
    ora player::joy_trigger
    sta player::joy_trigger_held

    ; set some conveniet things
    bit_tribool ::JOY_RIGHT_SH, ::JOY_LEFT_SH
    sta player::h_tribool
    lda player::joy_trigger_held
    bit_tribool ::JOY_DOWN_SH, ::JOY_UP_SH
    sta player::v_tribool
    rts
.endproc

.endif ; input.inc
