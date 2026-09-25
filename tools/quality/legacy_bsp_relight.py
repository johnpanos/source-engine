#!/usr/bin/env python3
"""Relight a compiled legacy map for the new lighting, keeping its gameplay.

    python3 tools/quality/legacy_bsp_relight.py testchmb_a_00 [--boot]
    python3 tools/quality/legacy_bsp_relight.py --bsp path/to/map.bsp --map-name my_map_relit
    tools/quality/vrad_cycles.py -game <gamedir> path/to/map   (a vrad drop-in)

The map (by name from the toolchain's game runtime, or any v20/v21 `--bsp`)
is rebuilt by `pbrt_map_build.py` from a `legacy_bsp` manifest this script
writes to `<out>/manifest.json`. Only the light is new:

  * its world faces, materials and vrad's lights become a USD scene
    (`legacy_bsp_scene.py`; the receipt lists every conversion and
    approximation) that Cycles bakes into separated direct/indirect lightmap
    layers, a PRBV probe volume, an RTRN radiosity transfer and an SDFV
    distance volume (profile `legacy-relight`), so every indirect-light
    producer, the SDF-traced one included, can run on the map;
  * the output BSP2 carries every legacy lump of the input byte for byte
    (entities, brushes, collision, visibility, brush models, props, game
    lumps, the pak file and the vrad lightmaps the legacy renderer still uses
    for brush entities, displacements and translucent faces), except the
    lighting-only lumps the relight rewrites: the leaf ambient samples
    (derived from the probe volume) and the world lights (vrad's baked ones
    removed so models are not lit twice). `gameplay-identity.json` proves it;
    any other difference fails the run.

The map is published as `<map>_relit` (not over the shipped map), so
`./play testchmb_a_00_relit` runs it; its level changes lead to the shipped
maps. `--boot` also boots it headless on native Vulkan. Build steps are
cached in `<out>/steps.json` like any pipeline map; `--from STEP` forces one.
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import bsp2_reader  # noqa: E402
import pbrt_map_build  # noqa: E402
import pbrt_map_toolchain  # noqa: E402

SCHEMA = "legacy-relight-identity/v1"
# Lighting-only legacy lumps the relight may rewrite: world lights (15, 54)
# and leaf ambient index/samples, LDR and HDR (51, 52, 55, 56).
RELIT_LUMPS = {15: "worldlights", 54: "worldlights_hdr", 51: "leaf_ambient_index_hdr",
               52: "leaf_ambient_index", 55: "leaf_ambient_lighting_hdr",
               56: "leaf_ambient_lighting"}


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


def load_toolchain(path=None):
    """The map pipeline's toolchain file (default: the provisioned one)."""
    profile, _ = pbrt_map_toolchain.load_profiles()
    return pbrt_map_toolchain.load(path or ROOT / profile["layout"]["toolchain_file"])


def relight(bsp, name, out, toolchain, quality=pbrt_map_build.LEGACY_QUALITY, game=None,
            force_from=None, boot=False, keep_going=False, publish=True, device=None):
    """Relight the compiled map `bsp` as map `name`, built in `out`.

    `game` is the directory the map was compiled against (vbsp/vrad
    `-game`); its loose materials are found before the game runtime's.
    `device` overrides the profile's Cycles device for every bake
    (`cycles_device.DEVICES`; `cpu` and `auto` are explicit opt-ins).
    Returns the gameplay identity; raises SystemExit when a step or the
    identity check fails (the input BSP is never written)."""
    bsp, out = Path(bsp).resolve(), Path(out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    manifest_path = out / "manifest.json"
    manifest = {"schema": "pbrt-map-manifest/v1", "map": name, "legacy_bsp": str(bsp),
                "quality": quality,
                "credit": "Relight of %s; its gameplay lumps are carried unchanged" % bsp.name}
    if game:
        manifest["legacy_game"] = str(Path(game).resolve())
    if device:
        manifest["lightmap"] = {"device": device}
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")
    manifest = pbrt_map_build.load_manifest(manifest_path)
    pipeline = pbrt_map_build.Pipeline(manifest, toolchain, out, force_from, boot,
                                       keep_going, publish=False)
    failure = None
    try:
        pipeline.build()
    except SystemExit as error:
        # Gate failures under --keep-going still finish the map.
        if not (keep_going and pipeline.failed_gates):
            raise
        failure = error
    identity = gameplay_identity(bsp.read_bytes(), pipeline.paths["bsp2"].read_bytes())
    identity.update(source=str(bsp), bsp2=str(pipeline.paths["bsp2"]))
    (out / "gameplay-identity.json").write_text(json.dumps(identity, indent=2) + "\n")
    print(json.dumps({k: identity[k] for k in ("status", "identical_lumps", "relit_lumps",
                                               "differing_lumps", "added_lumps")}))
    if identity["status"] != "pass":
        raise SystemExit("the relit map changed gameplay lumps: %s" %
                         identity["differing_lumps"])
    if publish:
        import playable_maps
        playable_maps.publish(json.loads((out / "build.json").read_text()))
        print("published; play it with ./play " + name)
    if failure:
        raise failure
    return identity


def default_out(name):
    return ROOT / "quality-results" / "relight" / name


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("map", nargs="?", help="shipped map name (portal/maps/<map>.bsp)")
    parser.add_argument("--bsp", type=Path, help="a BSP file instead of a shipped map")
    parser.add_argument("--game", type=Path,
                        help="the game directory --bsp was compiled against (vrad -game); "
                             "its materials are found first")
    parser.add_argument("--map-name", help="output map name (default <map>_relit)")
    parser.add_argument("--out", type=Path,
                        help="build directory (default quality-results/relight/<map name>)")
    parser.add_argument("--quality", default=pbrt_map_build.LEGACY_QUALITY,
                        help="map export profile (default legacy-relight)")
    parser.add_argument("--device", choices=pbrt_map_build.cycles_device.DEVICES,
                        help="Cycles device for every bake (default: the profile's, gpu)")
    parser.add_argument("--toolchain", type=Path)
    parser.add_argument("--from", dest="force_from", choices=pbrt_map_build.STEPS)
    parser.add_argument("--boot", action="store_true",
                        help="boot the relit map headless on native Vulkan")
    parser.add_argument("--keep-going", action="store_true")
    parser.add_argument("--no-publish", action="store_true")
    args = parser.parse_args()
    toolchain = load_toolchain(args.toolchain)
    if bool(args.map) == bool(args.bsp):
        parser.error("name a shipped map or give --bsp, not both")
    if args.map:
        runtime = Path(toolchain["runtime"])
        runtime = runtime if runtime.is_absolute() else ROOT / runtime
        bsp = runtime / "portal" / "maps" / (args.map + ".bsp")
        if not bsp.is_file():
            parser.error("no shipped map " + str(bsp))
        stem = args.map
    else:
        bsp = args.bsp
        stem = bsp.stem
    name = args.map_name or (stem.lower() + "_relit")
    relight(bsp, name, args.out or default_out(name), toolchain, args.quality, args.game,
            args.force_from, args.boot, args.keep_going, not args.no_publish, args.device)


if __name__ == "__main__":
    main()
