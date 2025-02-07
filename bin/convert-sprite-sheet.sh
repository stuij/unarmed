IN_IMAGE=${1}
NAME=${2}

superfamiconv -v \
  --sprite-mode \
  --in-image ${IN_IMAGE} \
  --out-palette ${NAME}.palette \
  --out-tiles ${NAME}.tiles \
  --out-tiles-image ${NAME}-tiles.png
