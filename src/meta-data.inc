.ifndef META_INC
META_INC = 1

; Sets up processor basics and ROM headers/vectors

.segment "HEADER"     ; +$7FE0 in file
    .byte ROM_NAME

.segment "ROMINFO"    ; +$7FD5 in file
    .byte $30         ; LoROM, fast-capable
    .byte 0           ; no battery RAM
    .byte $07         ; 128K ROM
    .byte 0,0,0,0
    .word $AAAA,$5555 ; dummy checksum and complement

.endif