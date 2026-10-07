#!/usr/bin/env python3
"""Build the 3DS test map, `n3ds_chamber`, with the shared bake pipeline.

A small clean-Aperture chamber for the 3DS Portal 2 client
(quality/product_profiles/portal2-3ds-pica.json): one 768x512x320 room, a
raised platform with steps, a black-metal pillar and a back alcove, lit by
five lights. It uses only opaque LightmappedGeneric materials from Portal 2's
sp_a2_triple_laser palette, so every surface has one base texture the
3DS backend encodes to ETC1 (no alpha, no translucency, no water or
reflections; nothing the fullbright PICA path skips). Few, large faces keep
the vertex and lightmap counts small for the 3DS's memory.

The VMF is compiled by tools/quality/vmf_map_build.py (vbsp, vvis, vrad
full quality) against the Portal 2 runtime's materials, so the map carries
baked lightmaps for the PICA's texture combiner.

    python3 tools/n3ds/n3ds_test_map.py [--runtime run/runtime-p2] [--toolchain ...]
"""

import argparse
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT / "tools/quality"))

import gyro_lab_map  # noqa: E402  (the VMF writer)
import vmf_map_build  # noqa: E402
from gyro_lab_map import Vmf, vec  # noqa: E402

NAME = "n3ds_chamber"
THICK = 16

WALL = "tile/white_wall_tile003a"
WALL_ACCENT = "tile/white_wall_tile003f"
CEILING = "tile/white_wall_tile003c"
FLOOR = "tile/white_floor_tile002a"
METAL = "metal/black_wall_metal_002a"
METAL_FLOOR = "metal/black_floor_metal_001c"


def box(vmf, x0, y0, z0, x1, y1, z1, material):
    center = ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2)
    half = ((x1 - x0) / 2, (y1 - y0) / 2, (z1 - z0) / 2)
    vmf.world.append(vmf.box(center, half, material))


def build_vmf():
    vmf = Vmf()
    x0, y0, z0, x1, y1, z1 = -384, -256, 0, 384, 256, 320
    t = THICK
    # Shell: floor, ceiling, four walls (the +x wall in the accent tile).
    box(vmf, x0 - t, y0 - t, z0 - t, x1 + t, y1 + t, z0, FLOOR)
    box(vmf, x0 - t, y0 - t, z1, x1 + t, y1 + t, z1 + t, CEILING)
    box(vmf, x0 - t, y0, z0, x0, y1, z1, WALL)
    box(vmf, x1, y0, z0, x1 + t, y1, z1, WALL_ACCENT)
    box(vmf, x0, y0 - t, z0, x1, y0, z1, WALL)
    box(vmf, x0, y1, z0, x1, y1 + t, z1, WALL)
    # A raised black-metal platform along the +x wall with two steps.
    box(vmf, 160, -256, 0, 384, 256, 64, METAL_FLOOR)
    box(vmf, 128, -96, 0, 160, 96, 42, METAL_FLOOR)
    box(vmf, 96, -96, 0, 128, 96, 21, METAL_FLOOR)
    # A pillar in the room and a black band around the walls at eye height.
    box(vmf, -96, -32, 0, -32, 32, 320, METAL)
    box(vmf, x0, y0, 120, x0 + 8, y1, 136, METAL)
    box(vmf, x0 + 8, y1 - 8, 120, x1, y1, 136, METAL)
    box(vmf, x0 + 8, y0, 120, x1, y0 + 8, 136, METAL)

    vmf.entity("info_player_start", {"origin": vec((-300, 0, 8)), "angles": "0 0 0"})
    for origin in ((-256, -160, 296), (-256, 160, 296), (0, 0, 296), (256, -160, 296), (256, 160, 296)):
        vmf.entity("light", {"origin": vec(origin), "_light": "255 248 236 260",
                             "_quadratic_attn": "1", "_linear_attn": "0", "_constant_attn": "0"})
    return vmf.text()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=ROOT / "quality-results/n3ds-maps" / NAME)
    parser.add_argument("--toolchain", type=Path, default=ROOT / "build/toolchains/pbrt-map-toolchain.json")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2",
                        help="staged Portal 2 runtime the materials come from")
    parser.add_argument("--vmf-only", action="store_true")
    args = parser.parse_args()
    out = args.out.resolve()
    work = out / "compile"
    work.mkdir(parents=True, exist_ok=True)
    vmf = work / (NAME + ".vmf")
    vmf.write_text(build_vmf())
    print("wrote %s" % vmf)
    if args.vmf_only:
        return 0
    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    record = vmf_map_build.build(vmf, out, tools, args.runtime.resolve(), quality="full", name=NAME)
    print("status %s (%s)" % (record["status"], out / "build.json"))
    return 0 if record["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
