set -xe

IN_IMAGE=${1}
NAME=${2}
BPP=${3}
PALETTE_BASE_OFFSET=${4}
EXTRA=${5}

superfamiconv -v \
  --in-image ${IN_IMAGE} \
  --out-palette ${NAME}.palette \
  --out-tiles ${NAME}.tiles \
  --out-map ${NAME}.map \
  --out-tiles-image ${NAME}-tiles.png \
  --palette-base-offset ${PALETTE_BASE_OFFSET} \
  --no-remap \
  ${EXTRA} \
  tiles [ --bpp ${BPP} ]
