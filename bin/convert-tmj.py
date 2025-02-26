#!/usr/bin/env python

import json
from struct import *
import sys

# for debugging
tmj = None

def read_file(path):
    with open(path) as f:
        return json.load(f)


# Tiled documentation (Global Tile IDs): Bit 32 is used for storing whether the
# tile is horizontally flipped, bit 31 is used for the vertically flipped tiles.
#
# (top 4 bytes of format are taken up by these kinds of modifiers)
#
# snes tilemap encoding:
#
# vhopppcc cccccccc
# v/h        = Vertical/Horizontal flip this tile.
# o          = Tile priority.
# ppp        = Tile palette. The number of entries in the palette depends on
#              the Mode and the BG.
# cccccccccc = Tile number.
#
# drat Tiled, keep your v/h in order!
def encode_tile(tile_idx):
    h_flip = (tile_idx & 0x80000000) >> 25
    v_flip = (tile_idx & 0x40000000) >> 23
    top_byte = (h_flip | v_flip)
    nr = tile_idx & 0x0FFFFFFF
    bottom_byte = nr if nr == 0 else nr - 1

    return pack("BB", bottom_byte & 0xFF, top_byte)
    
def encode_collision_map(tile_idx):
    nr = tile_idx & 0x0FFFFFFF
    return pack("B", nr if nr == 0 else 1)

def encode_map(layer, out):
    if layer['width'] != 32 or layer['height'] != 32:
        error("map isn't 32x32")

    with open(out + ".map", "wb") as f:
        for tile in layer['data']:
            f.write(encode_tile(tile))

    with open(out + ".coll", "wb") as f:
        for tile in layer['data']:
            f.write(encode_collision_map(tile))



def main(file_in, file_out):
    file_in = sys.argv[1]
    out = sys.argv[2]

    tmj = read_file(file_in)

    for layer in tmj['layers']:
        if layer['name'] == 'tilemap':
            encode_map(layer, file_out)


if __name__ == "__main__":
    file_in = sys.argv[1]
    file_out = sys.argv[2]
    main(file_in, file_out)
