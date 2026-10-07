#!/usr/bin/env python3
"""The lighting back end's gameplay identity gate.

The back end (`pbrt_map_build.py`) lights a compiled BSP from a front end and
packs it into a BSP2. Lighting may change only lighting: every legacy lump of
the input BSP must be carried byte for byte (entities, brushes, collision,
visibility, brush models, props, game lumps, the pak file, the vrad lightmaps
brush entities still use), except the lighting-only lumps the back end
rewrites: the world lights (the bake owns the lights it baked) and the leaf
ambient samples (derived from the probe volume).

    python3 tools/quality/gameplay_identity.py --bsp in.bsp --bsp2 out.bsp --out identity.json
"""

import argparse
import io
import json
import sys
import zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import bsp2_reader  # noqa: E402

SCHEMA = "legacy-relight-identity/v1"
# Lighting-only legacy lumps the back end may rewrite: world lights (15, 54)
# and leaf ambient index/samples, LDR and HDR (51, 52, 55, 56).
RELIT_LUMPS = {15: "worldlights", 54: "worldlights_hdr", 51: "leaf_ambient_index_hdr",
               52: "leaf_ambient_index", 55: "leaf_ambient_lighting_hdr",
               56: "leaf_ambient_lighting"}


# The pak lump may differ only in the static props' baked vertex light
# (sp_<n>.vhv and sp_hdr_<n>.vhv, prop_vertex_light.py): every other entry
# byte for byte.
PAK_LUMP = 40


def pak_differs_only_in_colour_meshes(original, carried):
    def entries(blob):
        with zipfile.ZipFile(io.BytesIO(bytes(blob))) as pak:
            return {info.filename: pak.read(info.filename) for info in pak.infolist()
                    if not (info.filename.lower().rsplit("/", 1)[-1].startswith("sp_") and
                            info.filename.lower().endswith(".vhv"))}
    try:
        return entries(original) == entries(carried)
    except zipfile.BadZipFile:
        return False


def gameplay_identity(source, bsp2):
    """Compare every legacy lump of the source BSP with the BSP2's copy."""
    header = bsp2_reader.parse_legacy_header(source, len(source))
    package = bsp2_reader.Bsp2File(bsp2, allow_unknown_required=True)
    if package.legacy is None:
        raise ValueError("the BSP2 carries no legacy map")
    identical, relit, differing = [], [], []
    for index, (offset, length, version, _) in enumerate(header["lumps"]):
        original = source[offset:offset + length]
        carried = package.legacy_lump(index)
        carried_version = package.legacy["lumps"][index][2]
        if original == carried and version == carried_version:
            identical.append(index)
        elif index == PAK_LUMP and version == carried_version and \
                pak_differs_only_in_colour_meshes(original, carried):
            relit.append({"lump": index, "name": "pakfile (static prop colour meshes)",
                          "bytes": [len(original), len(carried)]})
        elif index in RELIT_LUMPS:
            relit.append({"lump": index, "name": RELIT_LUMPS[index],
                          "bytes": [len(original), len(carried)]})
        else:
            differing.append({"lump": index, "bytes": [len(original), len(carried)],
                              "version": [version, carried_version]})
    added = sorted(entry["name"] for entry in package.entries
                   if bsp2_reader.legacy_index(entry["fourcc"]) is None and
                   entry["name"] not in ("LHDR", "LGAP"))
    return {"schema": SCHEMA, "status": "fail" if differing else "pass",
            "identical_lumps": len(identical), "relit_lumps": relit,
            "differing_lumps": differing, "added_lumps": added,
            "legacy_version": [header["version"], package.legacy["version"]],
            "revision": [header["revision"], package.legacy["revision"]]}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bsp", type=Path, required=True, help="the front end's compiled BSP")
    parser.add_argument("--bsp2", type=Path, required=True, help="the back end's packed BSP2")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    identity = gameplay_identity(args.bsp.read_bytes(), args.bsp2.read_bytes())
    identity.update(source=str(args.bsp), bsp2=str(args.bsp2))
    args.out.write_text(json.dumps(identity, indent=2) + "\n")
    print(json.dumps({k: identity[k] for k in ("status", "identical_lumps", "relit_lumps",
                                               "differing_lumps", "added_lumps")}))
    if identity["status"] != "pass":
        print("the lit map changed gameplay lumps: %s" % identity["differing_lumps"],
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
