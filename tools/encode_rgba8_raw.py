"""Encode a tightly packed RGBA raster as a GameCube tiled RGBA8 BTI."""

from argparse import ArgumentParser
from pathlib import Path
import struct


def encode(source: bytes, width: int, height: int) -> bytes:
    if width % 4 or height % 4:
        raise ValueError("RGBA8 dimensions must be multiples of four")
    if len(source) != width * height * 4:
        raise ValueError("raw input size does not match width and height")

    header = bytearray(32)
    header[0:2] = bytes((6, 1))
    struct.pack_into(">HH", header, 2, width, height)
    header[20:22] = bytes((1, 1))
    header[24] = 1
    struct.pack_into(">I", header, 28, 32)

    output = bytearray(header)
    for tile_y in range(0, height, 4):
        for tile_x in range(0, width, 4):
            tile = []
            for y in range(4):
                for x in range(4):
                    offset = ((tile_y + y) * width + tile_x + x) * 4
                    tile.append(source[offset:offset + 4])
            for red, green, _blue, alpha in tile:
                output.extend((alpha, red))
            for _red, green, blue, _alpha in tile:
                output.extend((green, blue))
    return bytes(output)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("width", type=int)
    parser.add_argument("height", type=int)
    args = parser.parse_args()
    args.output.write_bytes(encode(
        args.source.read_bytes(), args.width, args.height))
