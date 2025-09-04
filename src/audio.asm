LOROM = 1

.define TAD_CODE_SEGMENT    "CODE"
.define TAD_PROCESS_SEGMENT "CODE"

; so we remember this is possible
; TAD_CUSTOM_DEFAULTS = 1
; .define TAD_DEFAULT_AUDIO_MODE          TadAudioMode::STEREO

.include "../terrific-audio-driver/audio-driver/ca65-api/tad-audio.s"
