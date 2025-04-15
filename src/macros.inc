.ifndef MACROS_INC
MACROS_INC = 1

.macro A8
	sep #$20
.endmacro

.macro A16
	rep #$20
.endmacro

.macro AI8
	sep #$30
.endmacro

.macro AI16
	rep #$30
.endmacro

.macro I8
	sep #$10
.endmacro

.macro I16
	rep #$10
.endmacro


.macro rshift n
	.if (n > 0)
		lsr
		rshift (n - 1)
	.endif
.endmacro

;; arithmetic rshift
.macro a_rshift n
	.if (n > 0)
		cmp #$8000
		ror
		a_rshift (n - 1)
	.endif
.endmacro

;; A in = value to tribool on
;; A out = 1 if only plus set, -1 if only min set, 0 if values are equal
;; not exactly code efficient, but we only use this twice and this way code is
;; nice and readable.
.a16
.i16
.macro bit_tribool plus, minus
	tax
	rshift minus
	and #1
	sta .loword(W0)  ; y = minus: temp W0
	txa              ; load joy1_held and repeat
	rshift plus
	and #1
	sec
	sbc .loword(W0)  ; subtract minus from plus
.endmacro

.endif ;; MACROS_INC
