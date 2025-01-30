.macro init_cpu
    clc
    xce
    rep #$10        ; X/Y 16-bit
    sep #$20        ; A 8-bit
.endmacro

.macro setA8
	sep #$20
.endmacro

.macro setA16
	rep #$20
.endmacro

.macro setAXY8
	sep #$30
.endmacro

.macro setAXY16
	rep #$30
.endmacro

.macro setXY8
	sep #$10
.endmacro

.macro setXY16
	rep #$10
.endmacro
