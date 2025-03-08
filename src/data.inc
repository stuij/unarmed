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
.res 2

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