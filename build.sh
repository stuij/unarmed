ca65 --cpu 65816 -g -o demo.o src/demo.asm
ld65 -C lorom128.cfg demo.o -o demo.smc --dbgfile demo.dbg
