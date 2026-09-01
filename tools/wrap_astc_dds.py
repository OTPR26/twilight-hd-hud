#!/usr/bin/env python3
"""Wrap an ASTC 4x4 payload in a DX10 DDS header for Dusklight."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


DDS_MAGIC = b"DDS "
ASTC_MAGIC = b"\x13\xab\xa1\x5c"
DDS_DX10_HEADER_SIZE = 148
DXGI_FORMAT_ASTC_4X4_UNORM = 134


def astc_dimension(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 3], "little")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("base_dds", type=Path)
    parser.add_argument("astc", type=Path)
    parser.add_argument("output_dds", type=Path)
    args = parser.parse_args()

    dds = bytearray(args.base_dds.read_bytes())
    astc = args.astc.read_bytes()

    if len(dds) < DDS_DX10_HEADER_SIZE or dds[:4] != DDS_MAGIC:
        raise SystemExit(f"not a DX10 DDS file: {args.base_dds}")
    if dds[84:88] != b"DX10":
        raise SystemExit(f"DDS file has no DX10 header: {args.base_dds}")
    if len(astc) < 16 or astc[:4] != ASTC_MAGIC:
        raise SystemExit(f"not an ASTC file: {args.astc}")
    if tuple(astc[4:7]) != (4, 4, 1):
        raise SystemExit(f"ASTC block size is not 4x4x1: {args.astc}")

    dds_height = struct.unpack_from("<I", dds, 12)[0]
    dds_width = struct.unpack_from("<I", dds, 16)[0]
    astc_width = astc_dimension(astc, 7)
    astc_height = astc_dimension(astc, 10)
    astc_depth = astc_dimension(astc, 13)
    if (dds_width, dds_height) != (astc_width, astc_height) or astc_depth != 1:
        raise SystemExit(
            "image dimensions differ: "
            f"DDS={dds_width}x{dds_height}, ASTC={astc_width}x{astc_height}x{astc_depth}"
        )

    struct.pack_into("<I", dds, 128, DXGI_FORMAT_ASTC_4X4_UNORM)
    output = bytes(dds[:DDS_DX10_HEADER_SIZE]) + astc[16:]
    args.output_dds.parent.mkdir(parents=True, exist_ok=True)
    args.output_dds.write_bytes(output)


if __name__ == "__main__":
    main()
