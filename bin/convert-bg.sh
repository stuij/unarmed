IN_IMAGE=${1}
NAME=${2}

superfamiconv -v \
  --in-image ${IN_IMAGE} \
  --out-palette ${NAME}.palette \
  --out-tiles ${NAME}.tiles \
  --out-map ${NAME}.map \
  --out-tiles-image ${NAME}-tiles.png \
  tiles [ -B 4 ]
