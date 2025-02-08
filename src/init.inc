.ifndef INIT_INC
INIT_INC = 1
; -----------------------------------------------------------------------------
;   File: InitSNES.inc
;   Description: Reset the SNES to a known state
; -----------------------------------------------------------------------------

;----- Import ------------------------------------------------------------------
.import     clear_registers      ; Clear all PPU and CPU registers of the SNES
.import     clear_VRAM           ; Clear the complete VRAM to $00
.import     clear_CGRAM          ; Clear CG-RAM to $00 (black)
.import     clear_OAMRAM         ; Clear OAM-RAM to $ff (all sprites off screen)
;-------------------------------------------------------------------------------

.endif
