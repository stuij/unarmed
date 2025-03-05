.ifndef DATA_INC
DATA_INC = 1

.segment "ZEROPAGE"
W0:
B0L:
.res 1
B0H:
.res 1
W1:
B1L:
.res 1
B1H:
.res 1
W2:
B2L:
.res 1
B2H:
.res 1
W3:
B3L:
.res 1
B3H:
.res 1
in_nmi:

;; current player reference
P: .res 2

player_table:
.word .loword(p1)
.word .loword(p2)
.word .loword(p3)
.word .loword(p4)


v_velo_init:
.word V_VELO_INIT
h_velo_max:
.word H_VELO_MAX
v_velo_down_max:
.word V_VELO_DOWN_MAX

;; map data
map_x: .res 2
map_y: .res 2


.segment "BSS"

;; player data
p1: .tag player
p2: .tag player
p3: .tag player
p4: .tag player

OAM_MIRROR: .res 512

.endif ; DATA_INC