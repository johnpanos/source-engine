#!/usr/bin/env python3
"""Derive a legacy BSP's leaf ambient lighting from its PRBV probe volume.

    python3 tools/quality/leaf_ambient_from_prbv.py --bsp in.bsp --prbv map.prbv \\
        --out out.bsp [--receipt out.json]

RFC 0011 G1: the legacy exporter's `LUMP_LEAF_AMBIENT_*` samples come from
the probe volume, so a renderer or engine path that reads leaf ambient (the
D3D9/DXVK model path, an engine without PRBV support) lights models from the
same bake as the probe volume, not from vrad's fallback light.

Each non-solid leaf gets a sample at every active probe inside it, so leaf
ambient keeps the probe grid's spatial resolution (a pipeline map's collision
shell is often one leaf holding several rooms; its detail solids do not split
leaves). A leaf holding no probe gets eight samples at the centres of its
octants; every sample is clamped one normal bias inside the volume's first grid. Each sample holds the probe volume's
ambient cube there (total light, visibility on, +X -X +Y -Y +Z -Z), encoded as
ColorRGBExp32 (value = c * 2^exponent, the engine's ColorRGBExp32ToVector).
Solid leaves get none. The HDR and LDR lumps (55/51 and 56/52) receive the
same linear samples.

Only those four lumps change. Their new payloads are appended to the file and
their old byte ranges are zeroed; every other lump keeps its offset, so the
game lump's absolute offsets stay valid and the BSP2 converter carries the
file as usual.
"""

import argparse
import hashlib
import json
import math
import struct
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import probe_volume  # noqa: E402

HEADER_LUMPS = 64
HEADER_BYTES = 8 + HEADER_LUMPS * 16 + 4
LUMP_LEAFS = 10
LUMP_LEAF_AMBIENT_INDEX_HDR = 51
LUMP_LEAF_AMBIENT_INDEX = 52
LUMP_LEAF_AMBIENT_LIGHTING_HDR = 55
LUMP_LEAF_AMBIENT_LIGHTING = 56
LEAF_AMBIENT_LIGHTING_VERSION = 1
LEAF_BYTES = 32
CONTENTS_SOLID = 1
OCTANTS = 8
MAX_SAMPLES_PER_LEAF = 1024


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def encode_rgbexp32(rgb):
    """Linear RGB -> (r, g, b, exponent), value = c * 2^exponent."""
    peak = max(rgb)
    if not peak > 0 or not math.isfinite(peak):
        return (0, 0, 0, 0)
    exponent = math.floor(math.log2(peak)) - 7
    exponent = max(-128, min(127, exponent))
    scale = 2.0 ** -exponent
    return tuple(max(0, min(255, int(round(c * scale)))) for c in rgb) + (exponent,)


def read_lumps(data):
    ident, version = struct.unpack_from("<4si", data)
    if ident != b"VBSP":
        raise ValueError("not a legacy VBSP file")
    lumps = [list(struct.unpack_from("<iii4s", data, 8 + 16 * i)) for i in range(HEADER_LUMPS)]
    return version, lumps


def leaves(data, lumps):
    offset, length, version, _ = lumps[LUMP_LEAFS]
    if version != 1 or length % LEAF_BYTES:
        raise ValueError("unsupported leaf lump (version %d, %d bytes)" % (version, length))
    for i in range(length // LEAF_BYTES):
        contents, = struct.unpack_from("<i", data, offset + i * LEAF_BYTES)
        box = struct.unpack_from("<6h", data, offset + i * LEAF_BYTES + 8)
        yield contents, np.array(box[:3], float), np.array(box[3:], float)


def derive(data, volume):
    """(index payload, lighting payload, summary) of the new leaf ambient."""
    _, lumps = read_lumps(data)
    grid = volume.grids[0]
    # One normal bias inside the grid, so an axis sample biased outward from a
    # boundary probe still lands in the grid.
    margin = probe_volume.NORMAL_BIAS * float(grid.spacing.min()) * 1.01
    low = grid.origin + margin
    high = grid.origin + (np.asarray(grid.dims) - 1) * grid.spacing - margin
    probes = []
    for z in range(grid.dims[2]):
        for y in range(grid.dims[1]):
            for x in range(grid.dims[0]):
                position, i = volume.probe_position(grid, (x, y, z))
                if grid.state(i)[3] >= 0.5:
                    probes.append(position)
    probes = np.array(probes).reshape(-1, 3)
    index, samples = bytearray(), bytearray()
    count = 0
    lit_leaves = 0
    for contents, mins, maxs in leaves(data, lumps):
        if contents & CONTENTS_SOLID or (maxs <= mins).any():
            index += struct.pack("<HH", 0, 0)
            continue
        first = count
        inside = probes[((probes >= mins) & (probes <= maxs)).all(axis=1)]
        if len(inside) > MAX_SAMPLES_PER_LEAF:
            inside = inside[::int(math.ceil(len(inside) / MAX_SAMPLES_PER_LEAF))]
        if not len(inside):
            inside = [mins + (maxs - mins) * np.array([(0.25, 0.75)[(octant >> k) & 1]
                                                       for k in range(3)])
                      for octant in range(OCTANTS)]
        for position in inside:
            fraction = (np.asarray(position) - mins) / (maxs - mins)
            clamped = np.clip(position, low, high)
            cube = volume.ambient_cube(clamped, layer=0, visibility=True)
            if any(face is None for face in cube):
                continue
            quantized = np.clip(np.round(fraction * 255), 0, 255).astype(int)
            for face in cube:
                samples += struct.pack("<BBBb", *encode_rgbexp32(face))
            samples += struct.pack("<BBBB", *quantized, 0)
            count += 1
        if count - first > 0xFFFF or count > 0xFFFF:
            raise ValueError("leaf ambient exceeds the lump's 16-bit sample indices")
        index += struct.pack("<HH", count - first, first if count > first else 0)
        lit_leaves += count > first
    return bytes(index), bytes(samples), {"leaves": len(index) // 4, "lit_leaves": lit_leaves,
                                          "samples": count}


def rewrite(data, replacements):
    """Legacy BSP bytes with `replacements` {lump: (version, payload)} appended
    and their old ranges zeroed; every other lump stays where it is."""
    version, lumps = read_lumps(data)
    out = bytearray(data)
    for lump in replacements:
        offset, length, _, _ = lumps[lump]
        out[offset:offset + length] = b"\0" * length
    for lump, (lump_version, payload) in sorted(replacements.items()):
        out.extend(b"\0" * (-len(out) % 4))
        lumps[lump] = [len(out), len(payload), lump_version, b"\0\0\0\0"]
        out.extend(payload)
    for i, (offset, length, lump_version, fourcc) in enumerate(lumps):
        struct.pack_into("<iii4s", out, 8 + 16 * i, offset, length, lump_version, fourcc)
    return bytes(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--prbv", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    data = args.bsp.read_bytes()
    prbv = args.prbv.read_bytes()
    volume = probe_volume.Volume(prbv)
    index, samples, summary = derive(data, volume)
    out = rewrite(data, {
        LUMP_LEAF_AMBIENT_INDEX_HDR: (0, index), LUMP_LEAF_AMBIENT_INDEX: (0, index),
        LUMP_LEAF_AMBIENT_LIGHTING_HDR: (LEAF_AMBIENT_LIGHTING_VERSION, samples),
        LUMP_LEAF_AMBIENT_LIGHTING: (LEAF_AMBIENT_LIGHTING_VERSION, samples)})
    args.out.write_bytes(out)
    receipt = dict(summary, status="pass", scope="leaf-ambient-from-prbv",
                   source_bsp_sha256=sha256(data), prbv_sha256=sha256(prbv),
                   bsp_sha256=sha256(out), layer="total", visibility=True)
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "leaves", "lit_leaves", "samples")}))
    return 0


if __name__ == "__main__":
    sys.exit(main())
