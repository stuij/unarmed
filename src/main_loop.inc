.ifndef MAIN_LOOP_INC
MAIN_LOOP_INC = 1

.a16
.i16
finalize_main_loop:
    jsr finalize_players
    jsr finalize_bullets
    rts


.a16
.i16
handle_movement:
    jsr handle_player_movement
    jsr handle_bullet_movement
    rts


.a16
.i16
handle_main_loop:
    jsr handle_movement
    jsr handle_sprite_bumps
    jsr handle_player_actions
    jsr finalize_main_loop
    rts

.endif ; MAIN_LOOP_INC
