#!/usr/bin/env python3
"""Build the Portal 2 portal traversal test map, `qa_portal_walk`.

One lit chamber for the portal fixtures in
quality/workloads/portal2-portals-v1 (run by tools/quality/portal2_scenarios.py):

* a 1280x1280x448 room, x and y in [-640, 640], z in [0, 448];
* the north (y = 640), south (y = -640) and east (x = 640) walls, the floor
  and the ceiling are Portal 2 white tile, so portals fit on them;
* the west wall (x = -640) is black metal (`%noportal`), where a shot must not
  leave a portal;
* two black pillars stand in the room as landmarks, so views through a portal
  are easy to tell apart;
* the player spawns at the room's center facing north, on a dual portal gun
  (`weapon_portalgun`, both portals enabled).

The fixtures aim the gun at points the scenario names, so the room's sizes
are part of their contract: change them together.

    python3 tools/quality/portal2_portal_map.py --runtime run/runtime-p2 \\
        --install-game-dir run/runtime-p2/portal2
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import vmf_map_build  # noqa: E402
from gyro_lab_map import Vmf, vec  # noqa: E402  (the VMF writer)

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
NAME = "qa_portal_walk"

WALL = "tile/white_wall_tile003a"
FLOOR = "tile/white_floor_tile002a"
CEILING = "tile/white_ceiling_tile002a"
NO_PORTAL = "metal/black_wall_metal_002c"

HALF = 640
HEIGHT = 448
THICK = 16
PILLARS = [(-320, 320), (320, -320)]
PILLAR_HALF = 32


def build_vmf():
    vmf = Vmf()
    t = THICK / 2
    outer = HALF + THICK
    vmf.world += [
        vmf.box((0, 0, -t), (outer, outer, t), FLOOR),
        vmf.box((0, 0, HEIGHT + t), (outer, outer, t), CEILING),
        vmf.box((0, HALF + t, HEIGHT / 2), (outer, t, HEIGHT / 2), WALL),
        vmf.box((0, -HALF - t, HEIGHT / 2), (outer, t, HEIGHT / 2), WALL),
        vmf.box((HALF + t, 0, HEIGHT / 2), (t, HALF, HEIGHT / 2), WALL),
        vmf.box((-HALF - t, 0, HEIGHT / 2), (t, HALF, HEIGHT / 2), NO_PORTAL),
    ]
    vmf.world += [vmf.box((x, y, HEIGHT / 2), (PILLAR_HALF, PILLAR_HALF, HEIGHT / 2), NO_PORTAL)
                  for x, y in PILLARS]
    for x in (-400, 0, 400):
        for y in (-400, 0, 400):
            vmf.entity("light", {"origin": vec((x, y, HEIGHT - 48)),
                                 "_light": "255 250 240 18", "_lightHDR": "-1 -1 -1 1",
                                 "_lightscaleHDR": "1", "_quadratic_attn": "0",
                                 "_linear_attn": "1", "_constant_attn": "0"})
    vmf.entity("info_player_start", {"origin": "0 0 8", "angles": "0 90 0"})
    vmf.entity("weapon_portalgun", {"origin": "0 0 24", "angles": "0 90 0",
                                    "CanFirePortal1": "1", "CanFirePortal2": "1"})
    return vmf.text()


def build(out, tools, runtime, install_game_dir=None):
    """Write and compile the map under `out`; returns vmf_map_build's record."""
    out.mkdir(parents=True, exist_ok=True)
    source = out / (NAME + ".vmf")
    source.write_text(build_vmf())
    record = vmf_map_build.build(source, out, tools, runtime, quality="full", name=NAME)
    if record["status"] != "pass":
        raise RuntimeError("%s: %s (%s)" % (NAME, record["status"], out / "build.json"))
    if install_game_dir:
        print("installed " + str(vmf_map_build.install(record, install_game_dir)))
    return record


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/portal2-maps/" / NAME)
    parser.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2",
                        help="staged Portal 2 runtime the materials come from")
    parser.add_argument("--install-game-dir", type=Path,
                        help="copy the map into this game directory's maps/ (e.g. <runtime>/portal2)")
    parser.add_argument("--vmf-only", action="store_true", help="write the VMF and stop")
    args = parser.parse_args()
    out = args.out.resolve()
    if args.vmf_only:
        out.mkdir(parents=True, exist_ok=True)
        (out / (NAME + ".vmf")).write_text(build_vmf())
        print("wrote " + str(out / (NAME + ".vmf")))
        return 0
    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    build(out, tools, args.runtime.resolve(), args.install_game_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
