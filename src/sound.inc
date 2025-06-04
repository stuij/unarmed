.ifndef SOUND_INC
SOUND_INC = 1

.code

.a8
.i16
.proc load_song
    pha
    ; Reset TAD State
    ldx     #256
    jsr     Tad_SetTransferSize
    jsr     Tad_SongsStartImmediately
    pla
    jsr     Tad_LoadSong

loop:
    jsr     Tad_IsSongLoaded
    bcs     return
    jsl     Tad_Process
    bra     loop
return:
    rts
.endproc

.endif ; SOUND_INC
