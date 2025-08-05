.ifndef LIB_INC
LIB_INC = 1

.code
;; handlers

.a8     ; A is 8 bits
.i16    ; X/Y are 16 bits
.proc reset_handler
    jml :+
:   sei
    clc
    xce
	; FYI: coming out of emulation mode, the M and X bits of the
	; status registers are set to one. So resp. A and X/Y
	; are set to 8 bit.
	I16
    A8
    cld                     ; clear decimal flag
    lda #$80                ; force v-blanking
    sta INIDISP
    stz NMITIMEN            ; disable NMI
    ; set the stack pointer to $1fff
    ldx #$1fff              ; load X with $1fff
    txs                     ; copy X to stack pointer

    lda     #$01            ; Enable FastROM. Should not be necessary
    sta     MEMSEL          ; when FastROM is enabled in header?

    phk
    plb                     ; set b to current bank

    ;; clear all
    jsr clear_registers
    jsr clear_VRAM
    jsr clear_CGRAM
    jsr clear_OAM_mirror
    A16
    I8
    jsr dma_OAM
    A8
	I16
    jmp main
.endproc


.proc nmi_handler
    jml :+
:   bit RDNMI ; it is required to read this register
              ; in the NMI handler
    inc a:in_nmi
    rti
.endproc


.proc irq_handler
    jml :+
:   bit TIMEUP	; it is required to read this register
				; in the IRQ handler
@loop:
    jmp @loop
.endproc


; This shouldn't get called, so if it does we'd like to know more
; in a controlled manner, instead of say branching to $00000 and
; crashing out violently.
.proc spinloop_handler
    jml :+
:   jmp spinloop_handler
.endproc


;; strings

.a16
.i16
;; in:
;; A - map offset
;; X - string to print
.proc print_string_to_offset
    clc
    adc #VRAM_MAP_FONT_BASE
    sta VMADD

    txa
    jsr print_string
    rts
.endproc


.a16
.i16
;; A - string address
.proc print_string
    pha
    ldy #0
loop:
    lda #0 ;; clear upper C byte
    A8
    lda (1, S), y
    A16
    beq end
    print_a
    pla
    inc
    pha
    bra loop
end:
    pla
    rts
.endproc


.i16
.a16
;; A - player idx
.proc print_head
    inc
    lshift 10 ;; set palette
    ora #BG3_HEAD_CHAR_OFFSET
    ora #$2000 ; set priority bit on tile
    sta VMDATA
    rts
.endproc

.a16
.i16
;; in:
;; A - a nr bottom byte
;; X - a nr top byte
;; Y - b nr byte
;;
;; out:
;; A - result
.proc mul_ppu
    A8
    I8
    sta M7A
    stx M7A
    sty M7B
    A16
    I16
    lda MPYL
    rts
.endproc


;; RNG
;; taken from https://www.nesdev.org/wiki/Random_number_generator

; Returns a random 8-bit number in A (0-255), clobbers Y (0).
;
; Requires a 2-byte value on the zero page called "seed".
; Initialize seed to any value except 0 before the first call to prng.
; (A seed value of 0 will cause prng to always return 0.)
;
; This is a 16-bit Galois linear feedback shift register with polynomial $0039.
; The sequence of numbers it generates will repeat after 65535 calls.
;
; Execution time is an average of 125 cycles (excluding jsr and rts)

.proc prng
	ldy #8     ; iteration count (generates 8 bits)
	lda a:seed+0
:
	asl        ; shift the register
	rol a:seed+1
	bcc :+
	eor #$39   ; apply XOR feedback whenever a 1 bit is shifted out
:
	dey
	bne :--
	sta a:seed+0
	rts
.endproc


;; setup lib code

; dma_to_vram
; arguments:
;   - a: src bank
;   - x: src loword
;   - y: dst VRAM base address
;   - W0: size
.a8
.i16
.proc dma_to_vram
    sta A1B0 ; set bank of source address
    stx A1T0L ; set loword of source address
    sty VMADDL ; set VRAM base address
    ldy a:W0 ; get size from zero page word
    sty DAS0L ; set DMA byte counter

    lda #<VMDATAL
    sta BBAD0 ; so set DMA destination to VMDATAL
    lda #1
    sta DMAP0 ; transfer mode, 2 registers 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts
.endproc

; dma_to_palette
; arguments:
;   - a: src bank
;   - x: src loword
;   - y: dst palette base address
;   - W0: size
.a8
.i16
.proc dma_to_palette
    sta A1B0 ; set bank of source address
    stx A1T0L ; set loword of source address
    tya ; CGADD is a byte
    sta CGADD ; set palette base address
    ldy a:W0 ; get size from zero page word
    sty DAS0L ; set DMA byte counter

    lda #<CGDATA
    sta BBAD0 ; so set DMA destination to CGDATA
    lda #00
    sta DMAP0 ; transfer mode, 1 register, 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts
.endproc


; dma_to_wram
; arguments:
;   - a: src bank
;   - x: src loword
;   - B1L: dst WRAM bank address
;   - y: dst WRAM loword address
;   - W0: size
.a8
.i16
.proc dma_to_wram
    sta A1B0    ; set bank of source address
    stx A1T0L   ; set loword of source address
    lda a:B1L
    sta WMADDH  ; set bank of wram dest addr
    sty WMADDL  ; set loword of wram dest addr
    ldy a:B0L   ; get size from zero page word reg 1
    sty DAS0L   ; set DMA byte counter

    lda #<WMDATA
    sta BBAD0   ; so set DMA destination to WMDATA
    lda #00
    sta DMAP0   ; transfer mode, 1 register, 1 write
    lda #1
    sta MDMAEN  ; start dma, channel 0
    rts
.endproc


.a8
.i16
;; A - map index
.proc load_coll_map_to_staging_wram
    A16
        tay
        lda #(COLL_MAP_SIZE & $FF)
        ldx #(COLL_MAP_SIZE >> 8)
        jsr mul_ppu
        clc
        adc #.loword(town_coll)
        tax
    A8
    lda #^current_coll_map_staging
    sta a:B1L
    ldy #COLL_MAP_SIZE
    sty a:W0
    lda #^town_coll
    ldy #.loword(current_coll_map_staging)
    jsr dma_to_wram
    rts
.endproc


.a16
.i16
;; no DMA for WRAM to WRAM
.proc copy_coll_map_staging
    lda #(COLL_MAP_SIZE - 1)
    ldx #.loword(current_coll_map_staging)
    ldy #.loword(current_coll_map)
    mvn #^current_coll_map_staging, #^current_coll_map
    rts
.endproc


.a8
.i16
;; A - map index
.proc load_tilemap_to_wram
    A16
        tay
        lda #(GAME_MAP_SIZE & $FF)
        ldx #(GAME_MAP_SIZE >> 8)
        jsr mul_ppu
        clc
        adc #.loword(town_map)
        tax
    A8
    lda #^current_tile_map
    sta a:B1L
    ldy #GAME_MAP_SIZE
    sty a:W0
    lda #^town_map
    ldy #.loword(current_tile_map)
    jsr dma_to_wram
    rts
.endproc


.a8
.i16
.proc load_tilemap_to_vram
    ; load tile map from wram to vram
    ldy #GAME_MAP_SIZE
    sty a:W0
    lda #^current_tile_map
    ldx #.loword(current_tile_map)
    ldy #VRAM_MAP_BLOCKS_GAME_BASE
    jsr dma_to_vram
    rts
.endproc


SpriteUpperEmpty:
DMAZero:
.word $0000

SpriteEmptyVal:
.byte $e0 ; 224


.a8
.i16
.proc clear_coll_map_bottom
	php
	ldx #.loword(current_coll_map_bottom)
	stx WMADDL ;WRAM_ADDR_L
	stz WMADDH ;WRAM_ADDR_H

	ldx #$8008 ;fixed transfer to WRAM data 2180
	stx DMAP0
	ldx	#.loword(DMAZero)
	stx A1T0L ; and 4303
	lda #^DMAZero ;bank #
	sta A1B0
	ldx #COLL_MAP_BOTTOM_SIZE
	stx DAS0L ;and 4306
	lda #1
	sta MDMAEN ; DMA_ENABLE start dma, channel 0
    plp
    rts
.endproc


.a8
.i16
; Copy a fixed val in W0 to VRAM. So repeat same val in consequetive
; memory locations.
;
; arguments:
;   - y: dst VRAM address
;   - x: size
;   - W0: value
.proc dma_fixed_to_vram
    stx DAS0L ; set DMA byte counter
    stz A1B0 ; set bank of W0, aka 0
    ldx a:W0
    stx A1T0L ; set loword of source address
    sty VMADDL ; set vram base addr

    lda #<VMDATA
    sta BBAD0 ; so set DMA destination to WMDATA
    lda #09
    sta DMAP0 ; transfer mode, 2 register, 1 write, fixed
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts
.endproc


.a8
.i16
.proc clear_OAM_mirror
;fills the buffer with 224 for low table
;and $00 for high table
	php
	ldx #.loword(OAM_mirror)
	stx WMADDL ;WRAM_ADDR_L
	stz WMADDH ;WRAM_ADDR_H

	ldx #$8008 ;fixed transfer to WRAM data 2180
	stx DMAP0
	ldx	#.loword(SpriteEmptyVal)
	stx A1T0L ; and 4303
	lda #^SpriteEmptyVal ;bank #
	sta A1B0
	ldx #$200 ;size 512 bytes
	stx DAS0L ;and 4306
	lda #1
	sta MDMAEN ; DMA_ENABLE start dma, channel 0

	ldx	#.loword(SpriteUpperEmpty)
	stx A1T0L ; and 4303
	lda #^SpriteUpperEmpty ;bank #
	sta A1B0
	ldx #$0020 ;size 32 bytes
	stx DAS0L ;and 4306
	lda #1
	sta MDMAEN ; DMA_ENABLE start dma, channel 0
	plp
	rts
.endproc


.a16
.i8
.proc dma_OAM
;copy from OAM_MIRROR to the OAM RAM
	php
	stz OAMADDL ;OAM address

	lda #$0400 ;1 reg 1 write, 2104 oam data
	sta DMAP0
	lda #.loword(OAM_mirror)
	sta A1T0L ; source
	ldx #^OAM_mirror
	stx A1B0 ; bank
	lda #$220
	sta DAS0L ; length
	ldx #1
	stx MDMAEN ; DMA_ENABLE start dma, channel 0
	plp
	rts
.endproc

.endif ; LIB_INC