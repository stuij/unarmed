.ifndef COLL_BG_INC
COLL_BG_INC = 1

;; -------------------
;; collision detection

;; todo: sprites have a bounding box that we need to check for collisions.
;; Depending on which direction we move in, we can constrain checking the
;; different points of this box to only the ones that we care about. So say
;; we go left, we check left up and left down, we fall left, we check left up,
;; left down and right down, plus the intermediate points so we don't allow
;; impaling ourselves horizontally on 8x8 blocks

COLL_STACK_ROOM = $23
COLL_STACK_TMP = $1f
COLL_STACK_TILE_PROPS = $1d
COLL_STACK_POINT_HIT_GROUND = $1b
COLL_STACK_POINT_SPRITE_CALLBACK_TMP = $19
COLL_STACK_Y_NEW_TMP = $17
COLL_STACK_X_NEW_TMP = $15
COLL_STACK_POINT_Y_COORD = $13
COLL_STACK_POINT_X_COORD = $11
COLL_STACK_POINT_Y_OFF_NEW_SUB = $f
COLL_STACK_POINT_X_OFF_NEW_SUB = $d
COLL_STACK_Y_OFF_NEW = $b
COLL_STACK_X_OFF_NEW = $9
COLL_STACK_TILE_OFF = $7
COLL_STACK_POINT_TILE_OFF = $5
COLL_STACK_POINT_Y_OFF_NEW = $3
COLL_STACK_POINT_X_OFF_NEW = $1

.a16
.i16
.proc check_collisions
    ;; set up stack pointer
    tsc ;; pull current stack pointer to A
    pha ;; push to stack, so we can easily reset later

    sec
    sbc #COLL_STACK_ROOM ;; make room for bunch of stack arguments
                         ;; (we just pushed the sp so we got one 16bit arg less)
    tcs

    ;; we set default impossible minus value, so we know if it has been set before
    lda #$FFFF
    sta COLL_STACK_Y_NEW_TMP, s
    sta COLL_STACK_X_NEW_TMP, s

    lda sprite::y_new           ; load y of first sprite
    a_rshift 4                  ; remove sub-pixels
    tay
    and #7
    sta COLL_STACK_Y_OFF_NEW, s ; position of y within tile
    tya
    a_rshift 3                  ; divide by 8, truncating to get y tile offset
    lshift 5                    ; multiply by $20
    sta COLL_STACK_TMP, s
    lda sprite::x_new           ; load x of first sprite
    a_rshift 4                  ; remove sub-pixels
    tax
    and #7
    sta COLL_STACK_X_OFF_NEW, s ; position of x within tile
    txa
    a_rshift 3                  ; divide by 8 to get x tile offset
    clc
    adc COLL_STACK_TMP, s       ; add y to x, tile offset is in A
    sta COLL_STACK_TILE_OFF, s  ; save tile offset to stack for later

    lda #0
    sta COLL_STACK_POINT_SPRITE_CALLBACK_TMP, s ; set to 0
    sta COLL_STACK_POINT_HIT_GROUND, s
    sta COLL_STACK_TILE_PROPS, s
    ;; So now we have our base info calculated.
    ;;
    ;; From here on it becomes a game of iterating over the points of our
    ;; bounding box and figuring out if they cause a collision. If so, by how
    ;; much should we move backwards from our point of travel to snap to the
    ;; grid.
    ;;
    ;; We also check if one pixel beyond our bounding box is a surface we
    ;; want to walk on/cling to.
    ;;
    ;; First up, see if we need to move the offset into the collision map for y
    ;;
    ;; We iterate over our points, so first, set them up
    ;; for duration of this loop, bbox offset stays in Y
    ldy #0 ;; bbox point offset
coll_point_loop:
    ;; first we check if we're out of bounds for this point. If so, we can
    ;; skip everything and declare player/anything else dead.
    lda sprite::y_new
    clc
    adc (sprite::bbox_fine), y
    bmi out_of_bounds
    cmp #OUT_OF_BOUNDS_Y
    bpl out_of_bounds
    ;; otherwise we now do proper bounds checking on this point
    lda COLL_STACK_Y_OFF_NEW, s
    clc
    adc (sprite::bbox), y
    sta COLL_STACK_POINT_Y_OFF_NEW, s ;; save offset for if we need to do micro pushback
    a_rshift 3 ;; truncate to see if we're spilling over into another tile
    beq y_no_spill                      ; we're not spilling over
    lshift 5                            ; muliply by $20
    clc
    adc COLL_STACK_TILE_OFF, s
    sta COLL_STACK_POINT_TILE_OFF, s   ; push collision map tile for this point (we need other tile later)
    bra point_x_calc
y_no_spill: ;; y didn't spill. to keep symmetry with above basic block, push unmodified
            ;; collision map tile to stack
    lda COLL_STACK_TILE_OFF, s
    sta COLL_STACK_POINT_TILE_OFF, s
point_x_calc:
    iny ;; move y to x offset in point
    iny
    ;; also for X we first check if we're out of bounds
    lda sprite::x_new
    clc
    adc (sprite::bbox_fine), y
    bmi out_of_bounds
    cmp #OUT_OF_BOUNDS_X
    bpl out_of_bounds
    ;; calc possible x collision tile, and check collision before we start
    ;; thinking about calculating possible sprite pushback to grid coords
    lda COLL_STACK_X_OFF_NEW, s
    clc
    adc (sprite::bbox), y
    sta COLL_STACK_POINT_X_OFF_NEW, s
    a_rshift 3
    clc
    adc COLL_STACK_POINT_TILE_OFF, s ;; add to point-local collision map offset
    sta COLL_STACK_POINT_TILE_OFF, s
    tax
    A8
    lda current_coll_map, x           ; check collision map for point
    ;; save the tile props only if the tile props aren't nil
    ;; otherwise they'll get overwritten by the props of points that
    ;; don't actually touch anything
    A16
    beq after_prop_save
    sta COLL_STACK_TILE_PROPS, s
after_prop_save:
    bne collision              ; if not zero, collision
    jmp no_coll_for_point ; otherwise, do no collision things
out_of_bounds:
    lda #tile_props::out_of_bounds
    sta COLL_STACK_TILE_PROPS, s
    jmp cleanup
collision:
    ;; Once we know x and y direction,
    ;; we can make sensible decision on snapping.
    ;; We will first test which axis is the shallowest, as it's hopefully
    ;; a sensible and inexpensive proxy for what makes the most sense to snap
    ;; to:
    ;;
    ;; Most often the shallower side will be the side that was actually
    ;; penetrated (should mostly just be a pixel or two in depth), and if not,
    ;; snapping to the shallow side will be less invasive, as it will be less
    ;; noticed.
    ;;
    ;; The whole setup becomes a bit convoluted unfortunately.
    ;; To make things a bit more manageable, we will have greater than tests
    ;; for all 4 diagonal directions, which will go to either x or y snap.
    ;; If x or y is not moving we go straight to the opposite direction snap
    ;; in the snap sections. We again test for direction to not go crazy with
    ;; the logic (it's pretty cheap), and decide there on snap to the right
    ;; side. Then we do another (simpler) collision test, and if we do have
    ;; a collision, we go straight to snapping the other axis.
    ;; after this, no extra snapping should be necessary.
    ;;
    ;; If we do end up in an endless loop, we know our logic is wrong,
    ;; and it will be easy to spot it was the collision handling :)
    ;;
    ;; first we check if we straddled a block boundry in x or y direction
    ;; x direction
    ;;
    ;; reconstruct exact y position of point
    ;; Ideally I'd like to keep all calculations in subpixel format
    ;; but that makes the multiplication calculations above expensive.
    ;; As these calculations when we know collision happens will happen
    ;; much less frequent, this seems the better way.
    dey ;; move back to y within current bbox
    dey
    lda (sprite::bbox), y
    lshift 4
    clc
    adc sprite::y_new
    sta COLL_STACK_POINT_Y_COORD, s
    and #$7f
    sta COLL_STACK_POINT_Y_OFF_NEW_SUB, s

    iny ;; and increment again to get to x
    iny
    lda (sprite::bbox), y
    lshift 4
    clc
    adc sprite::x_new
    sta COLL_STACK_POINT_X_COORD, s
    and #$7f
    sta COLL_STACK_POINT_X_OFF_NEW_SUB, s


    lda sprite::h_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_X_OFF_NEW_SUB, s ; (~velocity) + point offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    beq coll_snap_y ; not moved out of x, so after snapping y, we're done
    ;; if n flag set, we moved from left block, so towards right
    bmi coll_x_right


    ;; x = moving left
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; (~velocity) + point offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    bne :+
    jmp coll_snap_to_right
    ;; if n flag set, we moved from upper block, so downwards
  : bmi coll_left_down
    bra coll_left_up

coll_x_right:
    lda sprite::v_velo
    eor #$FFFF
    clc
    adc #1
    clc
    adc COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; add velocity and block offset
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    ;; if n flag set, we moved from upper block, so downwards
    bne coll_x_right_cont
    jmp coll_snap_to_left
coll_x_right_cont:
    bpl coll_x_right_end
    jmp coll_right_down
coll_x_right_end:
    bra coll_right_up

coll_snap_y:
    lda sprite::v_velo
    tax
    eor #$FFFF
    clc
    adc #1
    sta COLL_STACK_TMP, s
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB, s ; point offset + (~velocity)
    clc
    adc COLL_STACK_TMP, s
    and #$FF80 ; not z flag set, so bigger than 8 + subpixels
               ; tells us we moved out of the block
    bne coll_snap_y_cont
    jmp point_end ;; assuming we got here from x also not out of bounds
coll_snap_y_cont:
    ;; if n flag set, we moved from upper block, so downwards
    bpl coll_snap_y_end
    jmp coll_snap_to_top
coll_snap_y_end:
    jmp coll_snap_to_bottom

;; now we resolved straight up/down, left/right
;; but if we moved diagonal into a new block, which way should we snap?
coll_left_up:
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$1
    tax
    A8
    lda current_coll_map, x
    A16
    bne :+
    jmp coll_snap_to_right
  : lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$20
    tax
    A8
    lda current_coll_map, x
    A16
    bne :+
    jmp coll_snap_to_bottom
  : jsr snap_to_right
    jsr snap_to_bottom
    jmp point_end

coll_left_down:
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$1
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_right
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$20
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_top
    jsr snap_to_right
    jsr snap_to_top
    bra point_end

coll_right_up:
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$1
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_left
    lda COLL_STACK_POINT_TILE_OFF, s
    clc
    adc #$20
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_bottom
    jsr snap_to_left
    jsr snap_to_bottom
    bra point_end


; we're moving in right/down direction
coll_right_down:
    ;; first we check if the squares that we want to snap into
    ;; aren't taken up by tiles.
    ;; We first check for left. If the tile there isn't obstructed,
    ;; we snap to it. Then we do same for right.
    ;; If both squares are obstructed we snap to both, aka, the corner
    ;; of the tile we left.
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$1
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_left
    ;; check top
    lda COLL_STACK_POINT_TILE_OFF, s
    sec
    sbc #$20
    tax
    A8
    lda current_coll_map, x
    A16
    beq coll_snap_to_top
    ; no space at left or up
    ; we need to snap to tile we came from,
    ; aka both left and top
    ; of current tile
    jsr snap_to_left
    jsr snap_to_top
    bra point_end


;; snapping to what?
coll_snap_to_top:
    jsr snap_to_top
    jmp point_end

coll_snap_to_bottom:
    jsr snap_to_bottom
    bra point_end


coll_snap_to_left:
    jsr snap_to_left
    bra point_end

coll_snap_to_right:
    jsr snap_to_right
    bra point_end

;; put code here, if you want to do something specific if no
;; collision has happened
no_coll_for_point:
    lda sprite::vptr
    clc
    adc #sprite_vtable::coll_point_no_coll_callback
    tax
    jsr (0,x)

point_end:
    iny
    iny
    cpy sprite::bbox_size
    beq cleanup
    jmp coll_point_loop ; not equal so we do another round
cleanup:
    lda sprite::vptr
    clc
    adc #sprite_vtable::collision_end_callback
    tax
    jsr (0, x)
    ; end of point loop so we're done
    lda COLL_STACK_ROOM - 1, s ; restore stack
    tcs                        ; pointer
    rts
.endproc

;; -------- end of mega collision fn


.proc snap_to_top
   lda COLL_STACK_POINT_Y_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out upwards is now in A
    ;; we AND with 7f, so we know how much of it sticks up,
    and #$7f
    sta COLL_STACK_TMP + 2, s
    lda sprite::y_new ; so we're effectively
    sec
    sbc COLL_STACK_TMP + 2, s
    sec
    sbc #$1
    tax
    lda COLL_STACK_Y_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi save_new_y  ; if not we can directly save this one
    txa
    cmp COLL_STACK_Y_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcs cont; if current y is higher, we don't save
save_new_y:
    txa
    sta COLL_STACK_Y_NEW_TMP + 2, s
cont:
    ;; this means we just hit the bottom
    lda #1
    ;; this one we should probably make a bit more generic.
    ;; for example by registering in all of these snap fns
    ;; what we hit in one var.
    sta COLL_STACK_POINT_HIT_GROUND + 2, s
    rts
.endproc


.proc snap_to_bottom
    lda COLL_STACK_POINT_Y_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out downwards is now in A
    and #$7f
    eor #$FFFF
    clc
    adc #$1
    clc
    adc #$80 ; effectively y_new + (8 - nr)
    clc
    adc sprite::y_new ; and we want to add that to the new y
    tax
    lda COLL_STACK_Y_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi save_new_y  ; if not we can directly save this one
    txa
    cmp COLL_STACK_Y_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    rts ; if current y is lower, we don't save
save_new_y:
    txa
    sta COLL_STACK_Y_NEW_TMP + 2, s
    rts
.endproc


.proc snap_to_left
   lda COLL_STACK_POINT_X_OFF_NEW_SUB +2, s
    ;; so the bit that sticks out right-wards is now in A
    ;; we AND with 7f, so we know how much of it sticks up,
    and #$7f
    sta COLL_STACK_TMP +2, s
    lda sprite::x_new ; so we're effectively
    sec
    sbc COLL_STACK_TMP +2, s
    sec
    sbc #$1
    tax
    lda COLL_STACK_X_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi save_new_x  ; if not we can directly save this one
    txa
    cmp COLL_STACK_X_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcs cont ; if current y is higher, we don't save
save_new_x:
    txa
    sta COLL_STACK_X_NEW_TMP + 2, s
cont:
    rts
.endproc


.proc snap_to_right
    lda COLL_STACK_POINT_X_OFF_NEW_SUB + 2, s
    ;; so the bit that sticks out left-wards is now in A
    and #$7f
    eor #$FFFF
    clc
    adc #$1
    clc
    adc #$80 ; effectively x_new + (8 - nr)
    clc
    adc sprite::x_new ; and we want to add that to the new x
    tax
    lda COLL_STACK_X_NEW_TMP + 2, s      ; did we already save a new temp y?
    bmi save_new_x  ; if not we can directly save this one
    txa
    cmp COLL_STACK_X_NEW_TMP + 2, s      ; otherwise we need to see which one is lower
    bcc cont ; if current y is lower, we don't save
save_new_x:
    txa
    sta COLL_STACK_X_NEW_TMP + 2, s
cont:
    rts
.endproc

.endif ; COLL_BG_INC