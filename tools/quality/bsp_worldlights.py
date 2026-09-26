#!/usr/bin/env python3
"""Remove the switchable-light controls' world lights from a legacy BSP.

    python3 tools/quality/bsp_worldlights.py --bsp in.bsp --out out.bsp \\
        --style 32 --style 33 [--receipt out.json]

RFC 0011 G4: a pipeline map's switchable baked lights (the radiosity
transfer's sources) are named zero-brightness `light` entities, so vbsp gives
each a light style and the game can switch it. vrad still writes a world
light for each, with zero intensity; the engine's model light cache must not
see those (they carry no light and are not real emitters). This removes
exactly the world lights with one of the given styles and zero intensity from
LUMP_WORLDLIGHTS (15) and LUMP_WORLDLIGHTS_HDR (54); any other world light
with such a style is an error. Like leaf_ambient_from_prbv.py, the new lumps
are appended and the old ranges zeroed, so every other lump keeps its offset.

    python3 tools/quality/bsp_worldlights.py --bsp in.bsp --out out.bsp \
        --keep-only [--style 32 ...]

A relit legacy map (`legacy_bsp_relight.py`) bakes vrad's lights into its
lightmap layers and probe volume, so the engine must not add them to models
again: `--keep-only` removes every world light except those of the given
styles (named lights that start dark, which the bake leaves out).
"""

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import leaf_ambient_from_prbv  # noqa: E402

LUMP_WORLDLIGHTS = 15
LUMP_WORLDLIGHTS_HDR = 54
# dworldlight_t (public/bspfile.h, lump version 1): origin, intensity, normal,
# shadow_cast_offset, cluster, type, style, stopdot, stopdot2, exponent,
# radius, three attenuations, flags, texinfo, owner.
RECORD = struct.Struct("<3f3f3f3fiiifffffffiii")
RECORD_V0 = struct.Struct("<3f3f3fiiifffffffiii")


def filter_lights(data, drop):
    """(bytes, removed per lump) without the world lights `drop(style,
    intensity)` selects, in both world-light lumps."""
    _, lumps = leaf_ambient_from_prbv.read_lumps(data)
    replacements, removed = {}, {}
    for lump in (LUMP_WORLDLIGHTS, LUMP_WORLDLIGHTS_HDR):
        offset, length, version, _ = lumps[lump]
        if not length:
            continue
        # The compile tools wrote version-1 records under lump version 0 until
        # 2026-09-25 (the engine misread every field after the normal); the
        # length decides which records these are, and a rewritten lump of
        # version-1 records is tagged version 1 (public/bspfile.h
        # WorldlightLumpLayout reads the old tag the same way).
        record = RECORD if version == 1 or length % RECORD_V0.size else RECORD_V0
        if length % record.size:
            raise ValueError("world light lump %d is not whole %d-byte records" %
                             (lump, record.size))
        kept, dropped = b"", 0
        for i in range(length // record.size):
            raw = data[offset + i * record.size:offset + (i + 1) * record.size]
            values = record.unpack(raw)
            intensity = values[3:6]
            style = values[14] if record is RECORD else values[11]
            if drop(style, intensity):
                dropped += 1
                continue
            kept += raw
        removed[lump] = dropped
        if dropped:
            replacements[lump] = (1 if record is RECORD else version, kept)
    return (leaf_ambient_from_prbv.rewrite(data, replacements) if replacements else data), removed


def strip(data, styles):
    """(bytes, removed per lump) without the controls' world lights."""
    def drop(style, intensity):
        if style not in styles:
            return False
        if any(abs(v) > 0 for v in intensity):
            raise ValueError("world light with control style %d carries light %s" %
                             (style, intensity))
        return True
    return filter_lights(data, drop)


def keep_only(data, styles):
    """(bytes, removed per lump) keeping only the world lights of `styles`."""
    return filter_lights(data, lambda style, _intensity: style not in styles)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--style", type=int, action="append", default=[])
    parser.add_argument("--keep-only", action="store_true",
                        help="relit legacy map: remove every world light except those of "
                             "the given styles (the bake now carries their light)")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    data = args.bsp.read_bytes()
    if args.keep_only:
        out, removed = keep_only(data, set(args.style))
    else:
        out, removed = strip(data, set(args.style))
        if sum(removed.values()) < len(args.style):
            raise SystemExit("expected a world light per control style %s, removed %s" %
                             (sorted(args.style), removed))
    args.out.write_bytes(out)
    receipt = {"schema": "bsp-worldlights-strip/v1", "styles": sorted(args.style),
               "mode": "keep-only" if args.keep_only else "strip-controls",
               "removed": {str(k): v for k, v in removed.items()},
               "in_sha256": hashlib.sha256(data).hexdigest(),
               "out_sha256": hashlib.sha256(out).hexdigest()}
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
