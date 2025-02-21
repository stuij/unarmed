.ifndef HEADER_INC
HEADER_INC = 1

.segment "HEADER"     ; +$7FE0 in file
.byte ROM_NAME    ; max 21 chars

.segment "ROMINFO"    ; +$7FD5 in file
.byte $30    ; Rom type: LoROM, FastROM
.byte $00    ; extra hardware in cartridge, 00: ROM only; 02: RAM with battery; etc..
.byte $08    ; ROM size, 1 << n KB. so $08 is 256KB, or 8 LoROM banks.
.byte $00    ; RAM size, 1 << n KB
.byte $01    ; region: US
.byte $33    ; publisher id
.byte $00    ; ROM revision number
.word $BEEF  ; ROM
.word $DEAD  ; checksum

.endif ; HEADER_INC
