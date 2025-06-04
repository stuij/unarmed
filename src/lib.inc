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
    inc .loword(in_nmi)
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
    ldy .loword(W0) ; get size from zero page word
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
    ldy .loword(W0) ; get size from zero page word
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
.proc dma_to_wram
    sta A1B0 ; set bank of source address
    stx A1T0L ; set loword of source address
    lda .loword(B1L)
    sta WMADDH ; set bank of wram dest addr
    sty WMADDL ; set loword of wram dest addr
    ldy .loword(W0) ; get size from zero page word reg 1
    sty DAS0L ; set DMA byte counter

    lda #<WMDATA
    sta BBAD0 ; so set DMA destination to WMDATA
    lda #00
    sta DMAP0 ; transfer mode, 1 register, 1 write
    lda #1
    sta MDMAEN ; start dma, channel 0
    rts
.endproc


.a8
.i16
.proc load_tilemap_to_vram
    ; load tile map from wram to vram
    ldy #GAME_MAP_SIZE
    sty .loword(W0)
    lda #^current_tile_map
    ldx #.loword(current_tile_map)
    ldy #VRAM_MAP_BG_GAME_BASE
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
;fills the buffer with 224 for low table
;and $00 for high table
	php
	ldx #.loword(current_coll_map_bottom)
	stx WMADDL ;WRAM_ADDR_L
	stz WMADDH ;WRAM_ADDR_H

	ldx #$8008 ;fixed transfer to WRAM data 2180
	stx DMAP0
	ldx	#.loword(DMAZero)
	stx A1T0L ; and 4303
	lda #^SpriteEmptyVal ;bank #
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
    ldx .loword(W0)
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