COMPILER=../../terrific-audio-driver/target/release/tad-compiler
PROJECT=$1 # ../terrific-audio-driver/examples/example-project.terrificaudio
ASM_OUT=$2 # audio.asm
BIN_OUT=$3 # audio.bin
INC_OUT=$4 # audio.inc

$COMPILER ca65-export \
  $PROJECT \
  --output-asm $ASM_OUT \
  --output-bin $BIN_OUT \
  --segment BANK1 \
  --lorom

$COMPILER ca65-enums \
   --output $INC_OUT \
   $PROJECT
