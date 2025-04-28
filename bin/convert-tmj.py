#!/usr/bin/env python

import json
import os
from struct import *
import sys

# for debugging
tmj = None

class IndexEntry:
    def __init__(self, choose_row, choose_row_idx, choose_idx,
                 tile_idx, props, ty_name, valid):
        self.choose_row = choose_row
        self.choose_row_idx = choose_row_idx
        self.choose_idx = choose_idx
        self.tile_idx = tile_idx
        self.props = props
        self.ty_name = ty_name
        self.valid = valid


def read_file(path):
    with open(path) as f:
        return json.load(f)

def encode_word(top_byte, bottom_byte):
    return pack("BB", bottom_byte & 0xFF, top_byte)

def encode_tile(top_byte, bottom_byte):
    return encode_word(top_byte, bottom_byte)

def encode_byte(byte):
    return pack("B", byte)

def encode_props(byte):
    return encode_byte(byte)

def choose_nr_to_props(nr, choose_table):
    entry = choose_table[nr]
    if not entry.valid:
        print("entry is invalid: {}, {}, {}".format(entry.choose_row,
                                                    entry.choose_row_idx,
                                                    nr))
        return 0
    else:
        return entry.props


def tiled_tile_to_props(tile_idx, choose_table):
    nr = tile_idx & 0x0FFFFFFF
    byte = choose_nr_to_props(nr if nr == 0 else nr - 1, choose_table)
    return encode_props(byte)


def choose_nr_to_tile_nr(nr, choose_table):
    entry = choose_table[nr]
    if not entry.valid:
        print("entry is invalid: {}, {}, {}".format(entry.choose_row,
                                                    entry.choose_row_idx,
                                                    nr))
        return 0
    else:
        return entry.tile_idx


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
def tiled_tile_to_tile(tile_idx, choose_table):
    h_flip = (tile_idx & 0x80000000) >> 25
    v_flip = (tile_idx & 0x40000000) >> 23
    top_byte = (h_flip | v_flip)
    nr = tile_idx & 0x0FFFFFFF
    bottom_byte = choose_nr_to_tile_nr(nr if nr == 0 else nr - 1, choose_table)
    return encode_tile(top_byte, bottom_byte)


def encode_map(layer, choose_table):
    if layer['width'] != 32 or layer['height'] != 28:
        error("map isn't 32x28")

    with open(layer['name'] + ".map", "wb") as tile_map:
        with open(layer['name'] + ".coll", "wb") as coll:
            count = 0
            # print("{}".format(count))
            for tile in layer['data']:
                row, col = divmod(count, 32)
                # print("row: {}, column: {}".format(row, col))
                tile_map.write(tiled_tile_to_tile(tile, choose_table))
                coll.write(tiled_tile_to_props(tile, choose_table))
                count += 1


def tile_spec_from_file (file_name):
    return read_file(file_name)['schema']


def prop_list_to_nr(prop_list):
    nr = 0
    for prop in prop_list:
        match prop:
            case "w": # wall
                nr |= 1
            case "b": # bounce
                nr |= 1 << 1
            case "c": # climb
                nr |= 1 << 2
            case _:
                raise ValueError("unknown propery: {}".format(prop))
    return nr


def make_invalid_IndexEntry(choose_row, choose_row_idx, choose_idx):
    return IndexEntry(choose_row, choose_row_idx, choose_idx, -1, -1, "invalid", False)


def tile_spec_to_index_lookup(schema):
    choose_row = 0
    choose_row_idx = 0
    choose_idx = 0
    tile_idx = 0
    ty_name = ""
    index_table = []
    choose_table = []

    for row in schema:
        ty_name = row[0]
        tiles_in_row = row[1]
        props = prop_list_to_nr(row[2])

        for _ in range(tiles_in_row):
            entry = IndexEntry(choose_row, choose_row_idx, choose_idx,
                               tile_idx, props, ty_name, True)
            index_table.append(entry)
            choose_table.append(entry)
            # create choose_bg table binary items inline
            choose_idx += 1
            choose_row_idx += 1
            tile_idx += 1

        for _ in range(32 - tiles_in_row):
            choose_table.append(make_invalid_IndexEntry(choose_row, choose_row_idx, choose_idx))
            choose_idx += 1
            choose_row_idx += 1

        choose_row += 1
        choose_row_idx = 0

    return index_table, choose_table


def schema_sanity_check(schema):
    index, choose = tile_spec_to_index_lookup(schema)
    for i in choose:
        print("{}, {}, {}, {}, {}, {}, {}".format(i.choose_idx, i.choose_row, i.choose_row_idx, i.tile_idx, i.props, i.ty_name, i.valid))

    for i in index:
        print("{}, {}, {}, {}".format(i.choose_idx, i.tile_idx, i.props,  i.ty_name))


def encode_string(string, max_len, file):
    str_len = len(string)
    if str_len > max_len:
        raise ValueError("string length {} higher than max {}"
                           .format(str_len, length))
    for char in string:
        file.write(encode_byte(ord(char)))

    for pad in range(max_len - str_len):
        file.write(encode_byte(0))


def encode_tile_select_bits(choose_table, schema):
    with open("select_tile.map", "wb") as tile_map:
        for item in choose_table:
            tile_map.write(encode_tile(0, item.tile_idx if item.valid else 0))

    with open("select_row_count.bin", "wb") as row_count:
        row_count.write(encode_word(0, len(schema)))

    with open("select_column_count.bin", "wb") as column_count:
        for row in schema:
            column_count.write(encode_word(0, row[1]))

    with open("select_row_name.bin", "wb") as row_name:
        for row in schema:
            encode_string(row[0], 16, row_name)

def main(tmj_in, spec_in):
    tmj = read_file(tmj_in)
    schema = tile_spec_from_file(spec_in)

    index_table, choose_table = tile_spec_to_index_lookup(schema)

    for layer in tmj['layers']:
        encode_map(layer, choose_table)

    encode_tile_select_bits(choose_table, schema)


if __name__ == "__main__":
    tmj_in = sys.argv[1]
    spec_in = sys.argv[2]
    main(tmj_in, spec_in)
