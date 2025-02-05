.p816   ; 65816 processor
.smart

.define ROM_NAME "demo"
.include "meta-data.inc"
.include "macros.inc"
.include "io.inc"

.segment "VECTORS"
.word 0, 0                          ;; Native mode handlers
.word .loword(spinloop_handler)     ; COP
.word .loword(spinloop_handler)     ; BRK
.word .loword(spinloop_handler)     ; ABORT
.word .loword(nmi_handler)          ; NMI
.word .loword(spinloop_handler)     ; RST
.word .loword(irq_handler)          ; IRQ

.word 0, 0                          ;; Emulation mode
.word .loword(spinloop_handler)     ; COP
.word 0
.word .loword(spinloop_handler)     ; ABORT
.word .loword(spinloop_handler)     ; NMI
.word .loword(reset_handler)        ; RESET
.word .loword(spinloop_handler)     ; IRQBRK


.segment "ZEROPAGE"
in_nmi:
; data read from joypad 13
joy1: .res 2
; trigger read from joypad 1
joy1_trigger: .res 2
; held buttons read from joypad 1
joy1_held: .res 2
; background 1 horizontal offset
bg2_x: .res 2


.code

.i16    ; X/Y are 16 bits
.a8     ; A is 8 bits
reset_handler:
    clc
    xce
	; FYI: coming out of emulation mode, the M and X bits of the
	; status registers are set to one. So resp. A and X/Y
	; are set to 8 bit.
	setXY16
    setA8

    ; Clear PPU registers
    ldx #$33
@loop:
    stz INIDISP,x
    stz NMITIMEN,x
    dex
    bpl @loop

    lda #$80
    sta INIDISP ; Turn off screen (forced blank)

    jmp main


VRAM_MAP_BASE = $0000
VRAM_CHR_BASE = $1000

.a8
.i16
wait_nmi:
    ;should work fine regardless of size of A
    lda in_nmi ;load A register with previous in_nmi
@check_again:
	wai ;wait for an interrupt
    cmp in_nmi  ;compare A to current in_nmi
                ;wait for it to change
                ;make sure it was an nmi interrupt
    beq @check_again
    rts

.i16
.a8
main:
    ;; set up bg registers
    lda #1 ; mode 1
    sta BGMODE
    lda #(VRAM_MAP_BASE >> 10) ; still 0
    sta BG1SC ; set bg 2 tile map
    ; set bg 2 char vram base addr (implicitly setting bg1 to 0, but we don't
    ; care)
    lda #((VRAM_CHR_BASE >> 12) << 4)
    sta BG12NBA

    ; let's copy over some binary data
    ;; first the palette
    stz CGADD ; set to begin of palette mem
    ldx #(town_palette_end - town_palette) ; # of palette entries
    ldy #0
@palette_loop:
    lda town_palette, y
    sta CGDATA
    iny
    dex
    bne @palette_loop

    lda #$80
    sta VMAIN

    ; load map
    ldx #VRAM_MAP_BASE
    stx VMADDL
    ldx #0
@map_loop:
     lda town_map,x
     sta VMDATAL
     inx
     lda town_map,x
     sta VMDATAH
     inx
     cpx #(town_map_end - town_map)
     bne @map_loop

    ; load tiles
    ldx #VRAM_CHR_BASE
    stx VMADDL
    ldx #0
@char_loop:
     lda town_tiles,x
     sta VMDATAL
     inx
     lda town_tiles,x
     sta VMDATAH
     inx
     cpx #(town_tiles_end - town_tiles)
     bne @char_loop

    ; turn on BG2
    lda #2
    sta TM

    ; Maximum screen brightness
    lda #$0F
    sta INIDISP

    ; enable NMI, turn on automatic joypad polling
    lda #$81
    sta NMITIMEN

    ; set program variables
    lda #00000
    sta bg2_x
    

    jmp game_loop

game_loop:
    jsr wait_nmi ; wait for NMI / V-Blank

    ; we're in vblank. first we should do video memory update things
    ; ...

    setA8
wait_for_joypad:
    lda HVBJOY            ; get joypad status
    lsr a                 ; check whether joypad done reading...
    bcs wait_for_joypad   ; ...if not, wait a bit more

    setA16
    ; read joypad
    rep #$20                         ; set A to 16-bit
    lda JOY1L                        ; get new input from this frame
    ldy joy1                         ; get input from last frame
    sta joy1                         ; store new input from this frame
    tya                              ; check for newly pressed buttons...
    eor joy1                         ; filter buttons that were not pressed last frame
    and joy1                         ; filter held buttons from last frame
    sta joy1_trigger                 ; ...and store them
    ; second, check for buttons held from last frame
    tya                              ; get input from last frame
    and joy1                         ; filter held buttons from last frame...
    sta joy1_held                    ; ...store them

check_right_button:
    lda #$0000                       ; set A to zero
    ora joy1_trigger                 ; check whether the right button was pressed this frame...
    ora joy1_held                    ; ...or held from last frame
    and #JOY_RIGHT
    beq check_right_button_done         ; if neither has occured, move on
    ; else, move bg
    setA8
    lda bg2_x
    inc
    sta bg2_x
    sta BG2HOFS
    stz BG2HOFS
check_right_button_done:

check_left_button:
    setA16
    lda #$0000                       ; set A to zero
    ora joy1_trigger                 ; check whether the up button was pressed this frame...
    ora joy1_held                    ; ...or held from last frame
    and #JOY_LEFT
    beq check_left_button_done         ; if neither has occured, move on
    ; else, move bg
    setA8
    lda bg2_x
    dec
    sta bg2_x
    sta BG2HOFS
    stz BG2HOFS
check_left_button_done:

    jmp game_loop




nmi_handler:
    bit RDNMI ; it is required to read this register
              ; in the NMI handler
    inc in_nmi
    rti

irq_handler:
	bit TIMEUP	; it is required to read this register
				; in the IRQ handler
@loop:
    jmp @loop

; This shouldn't get called, so if it does we'd like to know more
; in a controlled manner, instead of say branching to $00000 and
; crashing out violently.
spinloop_handler:
    jmp spinloop_handler


.segment "RODATA"

;; backgrounds
town_tiles:
.incbin"assets/town.tiles"
town_tiles_end:

town_palette:
.incbin"assets/town.palette"
town_palette_end:

town_map:
.incbin"assets/town.map"
town_map_end:
