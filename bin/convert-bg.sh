IN_IMAGE=${1}
NAME=${2}
BPP=${3}
EXTRA=${4}

superfamiconv -v \
  --in-image ${IN_IMAGE} \
  --out-palette ${NAME}.palette \
  --out-tiles ${NAME}.tiles \
  --out-map ${NAME}.map \
  --out-tiles-image ${NAME}-tiles.png \
  --no-remap \
  ${EXTRA} \
  tiles [ --bpp ${BPP} ]
