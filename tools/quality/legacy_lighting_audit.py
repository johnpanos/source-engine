#!/usr/bin/env python3
"""Independently check v21 face lightmap spans against their lighting lump."""

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

from bsp2_reader import load_expected_inventory, open_any


FACE_BYTES = 56
TEXINFO_BYTES = 72
SURF_NOLIGHT = 0x0400
SURF_BUMPLIGHT = 0x0800
SURF_SKY = 0x0004


def audit_lighting_lumps(faces, texinfos, lighting):
    """Validate the vrad face order, average colors, styles, and luxel spans."""
    if not faces or len(faces) % FACE_BYTES:
        raise ValueError("face lump is empty or has a partial face")
    if not texinfos or len(texinfos) % TEXINFO_BYTES:
        raise ValueError("texinfo lump is empty or has a partial record")
    if not lighting or len(lighting) % 4:
        raise ValueError("lighting lump is empty or has a partial RGBExp32 value")

    cursor = 0
    lit_faces = 0
    bumped_faces = 0
    styles_seen = set()
    for index in range(len(faces) // FACE_BYTES):
        face = faces[index * FACE_BYTES : (index + 1) * FACE_BYTES]
        texinfo = struct.unpack_from("<h", face, 10)[0]
        if texinfo < 0 or texinfo >= len(texinfos) // TEXINFO_BYTES:
            raise ValueError(f"face {index} has an invalid texinfo")
        flags = struct.unpack_from("<i", texinfos, texinfo * TEXINFO_BYTES + 64)[0]
        styles = face[16:20]
        count = next((i for i, style in enumerate(styles) if style == 255), 4)
        if any(style != 255 for style in styles[count:]):
            raise ValueError(f"face {index} has a style after the sentinel")
        offset = struct.unpack_from("<i", face, 20)[0]
        width, height = struct.unpack_from("<ii", face, 36)
        special = flags & (SURF_NOLIGHT | SURF_SKY)
        if special or not count:
            if offset != -1:
                raise ValueError(f"unlit face {index} has a lighting offset")
            continue
        if width < 0 or height < 0:
            raise ValueError(f"face {index} has a negative lightmap size")
        if offset != cursor + count * 4:
            raise ValueError(f"face {index} has a noncontiguous lighting offset")
        basis_count = 4 if flags & SURF_BUMPLIGHT else 1
        end = offset + (width + 1) * (height + 1) * count * basis_count * 4
        if end > len(lighting):
            raise ValueError(f"face {index} lightmap extends past the lighting lump")
        cursor = end
        lit_faces += 1
        bumped_faces += basis_count == 4
        styles_seen.update(styles[:count])

    if not lit_faces:
        raise ValueError("no faces have lighting")
    if cursor != len(lighting):
        raise ValueError("lighting lump contains unreferenced bytes")
    return {
        "faces": len(faces) // FACE_BYTES,
        "lit_faces": lit_faces,
        "bumped_faces": bumped_faces,
        "styles": sorted(styles_seen),
        "lighting_bytes": len(lighting),
    }


def audit_corpus(source_root, inventory):
    expected, inventory_info = load_expected_inventory(inventory)
    actual = {path.name for path in source_root.glob("*.bsp")}
    if actual != set(expected):
        raise ValueError(
            f"corpus differs: missing={sorted(set(expected) - actual)} "
            f"extra={sorted(actual - set(expected))}"
        )
    maps = []
    for name, (expected_hash, expected_version) in sorted(expected.items()):
        if expected_version != 21:
            raise ValueError(f"{name} is not a declared v21 map")
        data = (source_root / name).read_bytes()
        if hashlib.sha256(data).hexdigest() != expected_hash:
            raise ValueError(f"{name} differs from the pinned inventory")
        kind, header = open_any(data)
        if kind != "legacy" or header["version"] != 21:
            raise ValueError(f"{name} is not a legacy v21 BSP")

        def lump(index):
            offset, size, *_ = header["lumps"][index]
            if offset < 0 or size < 0 or offset + size > len(data):
                raise ValueError(f"{name} lump {index} extends outside the BSP")
            return data[offset : offset + size]

        hdr = bool(lump(53))
        faces = lump(58) or lump(7) if hdr else lump(7)
        layout = audit_lighting_lumps(faces, lump(6), lump(53 if hdr else 8))
        maps.append({"name": name, "sha256": expected_hash, "mode": "HDR" if hdr else "LDR",
                     "layout": layout})
    return {"status": "pass", "schema": "rfc0008-v21-lighting-layout/v1",
            "inventory": inventory_info, "maps": maps}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    try:
        result = audit_corpus(args.source_root, args.inventory)
    except (OSError, ValueError) as error:
        result = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if result["status"] != "pass":
        print("v21 lighting audit failed: " + result["reason"], file=sys.stderr)
        return 1
    print(f"v21 lighting audit passed: {len(result['maps'])} pinned maps")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
