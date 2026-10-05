#!/usr/bin/env python3
"""Relight a compiled legacy map for the new lighting, keeping its gameplay.

    python3 tools/quality/legacy_bsp_relight.py testchmb_a_00 [--boot]
    python3 tools/quality/legacy_bsp_relight.py --bsp path/to/map.bsp --map-name my_map_relit
    tools/quality/vrad_cycles.py -game <gamedir> path/to/map   (a vrad drop-in)

The shipped-map front end of the one lighting back end: the map (by name
from the toolchain's game runtime, or any v20/v21 `--bsp`) goes to
`map_lighting.light` with no authored scene, which writes the back end's
`bsp` manifest to `<out>/manifest.json`. Only the light is new:

  * its world faces, materials and vrad's lights become a USD scene
    (`legacy_bsp_scene.py`; the receipt lists every conversion and
    approximation) that Cycles bakes into separated direct/indirect lightmap
    layers, a directional indirect page, a PRBV probe volume and RPRB
    reflection captures under the single `source2` production profile;
  * the output BSP2 carries every legacy lump of the input byte for byte
    (entities, brushes, collision, visibility, brush models, props, game
    lumps, the pak file and the vrad lightmaps the legacy renderer still uses
    for retained legacy draws), except the
    lighting-only lumps the relight rewrites: the leaf ambient samples
    (derived from the probe volume) and the world lights (vrad's baked ones
    removed so models are not lit twice). The back end's `identity` step
    (`gameplay_identity.py`) proves it in `gameplay-identity.json`; any other
    difference fails the run.

The map is published as `<map>_relit` (not over the shipped map), so
`./play testchmb_a_00_relit` runs it; its level changes lead to the shipped
maps. `--boot` also boots it headless on native Vulkan. Build steps are
cached in `<out>/steps.json` like any pipeline map; `--from STEP` forces one.
"""

import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gameplay_identity  # noqa: E402
import map_lighting  # noqa: E402
import pbrt_map_build  # noqa: E402

# The gate lives with the back end; re-exported for callers of this module.
SCHEMA = gameplay_identity.SCHEMA
RELIT_LUMPS = gameplay_identity.RELIT_LUMPS
gameplay_identity = gameplay_identity.gameplay_identity


load_toolchain = map_lighting.load_toolchain


def relight(bsp, name, out, toolchain, quality=pbrt_map_build.LEGACY_QUALITY, game=None,
            force_from=None, boot=False, keep_going=False, publish=True, device=None,
            runtime=None, max_seam_p99=None, probe_positions=None, probe_volumes=None):
    """Relight the compiled map `bsp` as map `name`, built in `out`: the
    lighting back end (`map_lighting.light`) with the scene derived from the
    BSP. See `map_lighting.light` for the arguments; `max_seam_p99` waives the
    stitched seam gate's default 99th percentile for this map (the manifest's
    lightmap.seam_gate). Returns the gameplay identity."""
    extra = {}
    if probe_positions is not None:
        extra["reflection_probe"] = {"positions": probe_positions}
    if probe_volumes is not None:
        extra.setdefault("reflection_probe", {})["volumes"] = probe_volumes
    return map_lighting.light(bsp, name, out, toolchain, None, quality, game, runtime, device,
                              force_from, boot, keep_going, publish, extra or None,
                              max_seam_p99=max_seam_p99)


def load_probe_positions(path, map_name):
    """Read the authored capture anchors for one shipped map, in meters."""
    data = json.loads(Path(path).read_text())
    if data.get("schema") != "map-probe-positions/v1" or data.get("map") != map_name:
        raise ValueError("probe positions must name the shipped map " + map_name)
    anchors = data.get("anchors")
    if not isinstance(anchors, list) or not anchors:
        raise ValueError("probe positions need one or more anchors")
    positions = []
    for anchor in anchors:
        position = anchor.get("position_m") if isinstance(anchor, dict) else None
        if (not isinstance(position, list) or len(position) != 3 or
                any(not isinstance(value, (int, float)) or not math.isfinite(value)
                    for value in position)):
            raise ValueError("each probe anchor needs a finite 3D position_m")
        positions.append(position)
    return positions


def default_out(name):
    return ROOT / "quality-results" / "relight" / name


def load_probe_volumes(path, map_name):
    """Read room proxies/influences in stage meters; placement owns validation."""
    import reflection_probe_set
    data = json.loads(Path(path).read_text())
    if data.get("schema") != "map-probe-volumes/v1" or data.get("map") != map_name:
        raise ValueError("probe volumes must name the shipped map " + map_name)
    volumes = data.get("volumes")
    if not isinstance(volumes, list) or not volumes:
        raise ValueError("probe volumes need one or more volumes")
    for volume in volumes:
        reflection_probe_set.authored_volume(volume, reflection_probe_set.PLACEMENT_DEFAULTS)
    return volumes


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
                        help="map export profile (default source2)")
    parser.add_argument("--device", choices=pbrt_map_build.cycles_device.DEVICES,
                        help="Cycles device for every bake (default: the profile's, gpu)")
    parser.add_argument("--toolchain", type=Path)
    parser.add_argument("--runtime", type=Path,
                        help="staged game runtime the materials come from (default: the "
                             "toolchain's; run/runtime-p2 for a Portal 2 map)")
    parser.add_argument("--from", dest="force_from", choices=pbrt_map_build.STEPS)
    parser.add_argument("--boot", action="store_true",
                        help="boot the relit map headless on native Vulkan")
    parser.add_argument("--keep-going", action="store_true")
    parser.add_argument("--no-publish", action="store_true")
    parser.add_argument("--max-seam-p99", type=float,
                        help="this map's stitched seam gate, 99th percentile (relative), in "
                             "place of lightmap_ktx2.py's default; recorded in the manifest")
    parser.add_argument("--probe-positions", type=Path,
                        help="authored reflection-probe anchors for this shipped map")
    parser.add_argument("--probe-volumes", type=Path,
                        help="authored proxy/influence boxes and priorities in stage meters")
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
    positions = load_probe_positions(args.probe_positions, stem) if args.probe_positions else None
    volumes = load_probe_volumes(args.probe_volumes, stem) if args.probe_volumes else None
    relight(bsp, name, args.out or default_out(name), toolchain, args.quality, args.game,
            args.force_from, args.boot, args.keep_going, not args.no_publish, args.device,
            args.runtime, args.max_seam_p99, positions, volumes)


if __name__ == "__main__":
    main()
