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
    pla
    jsr     Tad_LoadSong

loop:
    jsr     Tad_IsSongLoaded
    bcs     end
    jsl     Tad_Process
    bra     loop
end:
    rts
.endproc


; IN: A = sfx id
; IN: X = pan
.a8
; I unknown
; DB access lowram
; KEEP: X, Y
.proc Tad_QueuePannedSoundEffect_D
    cmp a:Tad_sfxQueue_sfx
    bcs @EndIf
        sta a:Tad_sfxQueue_sfx

        txa
        sta a:Tad_sfxQueue_pan

@EndIf:
    rts
.endproc


; IN: A = sfx_id
.a8
; I unknown
; DB access lowram
; KEEP: X, Y
.proc Tad_QueueSoundEffect_D
    cmp a:Tad_sfxQueue_sfx
    bcs @EndIf
        sta a:Tad_sfxQueue_sfx

        lda #TAD_CENTER_PAN
        sta a:Tad_sfxQueue_pan
@EndIf:
    rts
.endproc


.endif ; SOUND_INC
