#!/usr/bin/env python3
"""Prove a directional preview changes only the selected style's RNM luxels."""

import argparse
import hashlib
import json
import struct
from pathlib import Path

from bsp2_reader import open_any
from legacy_lighting_audit import FACE_BYTES, SURF_BUMPLIGHT, TEXINFO_BYTES, audit_lighting_lumps


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def compare(flat, directional, style):
    if len(flat) != len(directional):
        raise ValueError("BSP lengths differ")
    kind, header = open_any(flat)
    other_kind, other_header = open_any(directional)
    if kind != "legacy" or other_kind != kind or header != other_header:
        raise ValueError("BSP headers differ")

    def lump(index):
        offset, length, *_ = header["lumps"][index]
        return flat[offset:offset + length]

    faces, texinfos, lighting = lump(7), lump(6), lump(8)
    audit_lighting_lumps(faces, texinfos, lighting)
    light_offset, light_length, *_ = header["lumps"][8]
    audit_lighting_lumps(faces, texinfos,
                         directional[light_offset:light_offset + light_length])
    permitted = bytearray(len(flat))
    basis_changed = [0, 0, 0]
    bumped_faces = 0
    for index in range(len(faces) // FACE_BYTES):
        face = faces[index * FACE_BYTES:(index + 1) * FACE_BYTES]
        texinfo = struct.unpack_from("<h", face, 10)[0]
        flags = struct.unpack_from("<i", texinfos, texinfo * TEXINFO_BYTES + 64)[0]
        styles = face[16:20]
        if style not in styles or not flags & SURF_BUMPLIGHT:
            continue
        bumped_faces += 1
        style_index = styles.index(style)
        face_offset = struct.unpack_from("<i", face, 20)[0]
        width, height = struct.unpack_from("<ii", face, 36)
        span = (width + 1) * (height + 1) * 4
        start = light_offset + face_offset + style_index * 4 * span
        for basis in range(3):
            basis_start = start + (basis + 1) * span
            permitted[basis_start:basis_start + span] = b"\1" * span
            basis_changed[basis] += sum(flat[position] != directional[position]
                                          for position in range(basis_start,
                                                                basis_start + span))
    unexpected = [index for index, (before, after) in enumerate(zip(flat, directional))
                  if before != after and not permitted[index]]
    if not bumped_faces or any(count == 0 for count in basis_changed) or unexpected:
        raise ValueError("directional delta is absent or escapes RNM spans: " +
                         str(unexpected[:8]))
    return {"bumped_faces": bumped_faces, "changed_bytes_by_basis": basis_changed,
            "unexpected_changed_bytes": 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--flat", type=Path, required=True)
    parser.add_argument("--directional", type=Path, required=True)
    parser.add_argument("--style", type=int, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    flat = args.flat.read_bytes()
    directional = args.directional.read_bytes()
    counts = compare(flat, directional, args.style)
    mutated = bytearray(directional)
    mutated[header_offset(flat)] ^= 1
    try:
        compare(flat, bytes(mutated), args.style)
    except ValueError:
        negative_rejected = True
    else:
        negative_rejected = False
    if not negative_rejected:
        raise ValueError("flat-byte negative control was accepted")
    evidence = {"status": "pass", "scope": "cycles-rnm-only-bsp-delta",
                "flat_sha256": sha256(flat), "directional_sha256": sha256(directional),
                "style": args.style, "negative_flat_change_rejected": negative_rejected,
                **counts}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


def header_offset(data):
    """Select a lighting average byte outside every permitted RNM span."""
    _, header = open_any(data)
    light_offset, *_ = header["lumps"][8]
    return light_offset


if __name__ == "__main__":
    main()
