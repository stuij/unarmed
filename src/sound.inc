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
    bcs     set_volume
    jsl     Tad_Process
    bra     loop
set_volume:
    ;; I think the volume is incrementally turned down
    ;; so that's why it seems like we're not immediately
    ;; reacting to turning the music down.
    ;; not totally sure about this though.
    A8
    jsr Tad_GlobalVolumesPersist

    lda #TadCommand::SET_GLOBAL_VOLUMES
    ldx #64
    ldy #255
    jsr Tad_QueueCommand
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
