#!/usr/bin/env python3
"""render.lab.transmission (RFC 0016 K11/K12): scene color in render_lab's map renderer.

    python3 tools/render/lab_transmission.py [--tree build-rc-lab] [--out DIR]
        [--runtime run/runtime-p2/portal2] [--map run/maps/sp_a1_intro4_relit]

The map renderer (render_lab --map) draws its opaque world and models, takes
the view's scene color through render.graph's capture owner
(graph::RecordSceneColor), binds it to the transmitting programs' view group
and then draws the transmitting surfaces, in render.pass.world's order.

The oracle: a zero-warp Refract pane (a mover box, $refractamount 0,
$refracttint [0.5 0.75 1], a flat normal map, no env map) in front of the
camera over the published sp_a1_intro4_relit world. Inside the pane every
pixel is the same frame drawn without the pane times the tint as Source
decodes a gamma tint (the 8-bit table: round(v * 255) / 255, to the 2.2), per
channel within 1 percent + 2e-5. Outside the pane the frame is unchanged.
The frames use the game's shipped terms (no SSR, GTAO or projector bounce),
with validation and synchronization validation on. The pane's screen rectangle is projected here from the camera, independently
of the renderer.

Seeded defects that must fail the interior check (render_lab
--transmission-defect): the capture skipped, the capture taken before the
opaque draws, and the transmitting draws rasterized at a viewport offset by
an eighth of the target that the frame's viewport term does not describe.

The content is the user's installed Portal 2 runtime (run/runtime-p2) and the
published relit map; neither is in the repository. The suite builds a private
loose-file overlay of both (render_lab reads loose files from one game root)
and extracts the few VPK-only files the frame reads (flashlight cookies, the
flat normal map) with tools/render/vmt_corpus.py's VPK reader.
"""
import argparse
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "render"))
from conformance_result import Checks  # noqa: E402
from lab import lab_binary, lab_env  # noqa: E402

MAP = "sp_a1_intro4_relit"
WIDTH, HEIGHT = 320, 240
EYE = (-60.0, 260.0, 134.0)
HFOV = 90.0
# The pane: a box one unit deep, its front face (normal -y) toward the eye.
PANE_MIN = (-100.0, 320.0, 110.0)
PANE_MAX = (-20.0, 321.0, 160.0)
TINT = (0.5, 0.75, 1.0)
MATERIAL = "lab_transmission/zero_warp"
VMT = """Refract
{
$model 1
$refractamount 0
$refracttint "[%g %g %g]"
$normalmap "dev/flat_normal"
}
""" % TINT
# VPK-only files the frame reads.
EXTRACT = ["materials/dev/flat_normal.vtf", "materials/effects/flashlight003.vtf",
           "materials/effects/flashlight004.vtf"]
DEFECTS = ("skip-capture", "before-opaque", "viewport-offset")
REL_TOLERANCE = 0.01
ABS_TOLERANCE = 2e-5
INTERIOR_PASS_FRACTION = 0.99


def source_gamma_to_linear(value):
    """Source's 8-bit gamma table (independent of render/material)."""
    if value > 1.0:
        return value
    if value >= 0.95:
        return 1.0
    index = round(value * 255.0)  # Python rounds half to even, as lrint does
    return (index / 255.0) ** 2.2


def read_pfm(path):
    with open(path, "rb") as f:
        if f.readline().strip() != b"PF":
            raise ValueError("%s is not an RGB PFM" % path)
        width, height = map(int, f.readline().split())
        scale = float(f.readline())
        data = np.frombuffer(f.read(), dtype="<f4" if scale < 0 else ">f4")
    return np.flipud(data.reshape(height, width, 3))  # row 0 at the top


def project(point):
    """Pixel (x, y) of a world point for the camera facing +y, up +z."""
    focal = (WIDTH / 2.0) / math.tan(math.radians(HFOV) / 2.0)
    depth = point[1] - EYE[1]
    return (WIDTH / 2.0 + (point[0] - EYE[0]) / depth * focal,
            HEIGHT / 2.0 - (point[2] - EYE[2]) / depth * focal)


def build_overlay(runtime, map_dir, root):
    """Loose files of the runtime and the map under one game root."""
    from vmt_corpus import Game  # the VPK reader, imported only when needed
    for entry in runtime.iterdir():
        if entry.name not in ("materials", "maps"):
            (root / entry.name).symlink_to(entry)
    for tree in (runtime / "materials", map_dir / "materials"):
        if tree.is_dir():
            shutil.copytree(tree, root / "materials", symlinks=True, dirs_exist_ok=True,
                            copy_function=os.symlink)
    (root / "maps").mkdir()
    (root / "maps" / (MAP + ".bsp")).symlink_to(map_dir / "maps" / (MAP + ".bsp"))
    game = Game("portal2")
    missing = []
    for relative in EXTRACT:
        data = game.read(relative)
        if data is None:
            missing.append(relative)
            continue
        target = root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists():
            target.write_bytes(data)
    pane = root / "materials" / (MATERIAL + ".vmt")
    pane.parent.mkdir(parents=True, exist_ok=True)
    pane.write_text(VMT)
    return missing


def render(tree, root, out, extra):
    command = [str(lab_binary(tree)), "--game", str(root), "--map",
               str(root / "maps" / (MAP + ".bsp")), "--eye", "%g,%g,%g" % EYE,
               "--forward", "0,1,0", "--up", "0,0,1", "--hfov", str(HFOV),
               "--size", "%dx%d" % (WIDTH, HEIGHT), "--no-bounce", "--no-ssr", "--no-ao", "--validate",
               "--out", str(out)] + extra
    result = subprocess.run(command, capture_output=True, text=True, env=lab_env(tree),
                            timeout=300)
    log = result.stdout + result.stderr
    out.with_suffix(".log").write_text(log)
    messages = re.search(r"validation messages (\d+)", result.stdout)
    return result.returncode, log, int(messages.group(1)) if messages else None


def interior_error(control, pane, interior):
    """Fraction of interior pixels within the tint oracle, and the worst one."""
    tint = np.array([source_gamma_to_linear(t) for t in TINT], dtype=np.float64)
    expected = control[interior].astype(np.float64) * tint
    actual = pane[interior].astype(np.float64)
    error = np.abs(actual - expected)
    bound = REL_TOLERANCE * np.abs(expected) + ABS_TOLERANCE
    within = np.all(error <= bound, axis=-1)
    return float(within.mean()), float((error / bound).max())


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--tree", default="build-rc-lab")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run" / "runtime-p2" / "portal2")
    parser.add_argument("--map", type=Path, default=ROOT / "run" / "maps" / MAP)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    checks = Checks()
    binary = lab_binary(args.tree)
    if not checks.check(binary.exists(), "lab.binary", "%s is not built" % binary):
        return checks.report()
    if not checks.check(args.runtime.is_dir() and (args.map / "maps" / (MAP + ".bsp")).is_file(),
                        "content.present", "%s or %s is missing" % (args.runtime, args.map)):
        return checks.report()
    # Short paths: the overlay's absolute paths reach the renderer's file reads.
    work = Path(tempfile.mkdtemp(prefix="lab-tx-", dir=args.out or None))
    root = work / "game"
    root.mkdir()
    missing = build_overlay(args.runtime, args.map, root)
    checks.check(not missing, "content.extracted", "absent from the VPKs: %s" % missing)

    control_path, pane_path = work / "control.pfm", work / "pane.pfm"
    mover = ["--mover", "%g,%g,%g,%g,%g,%g,%s" % (PANE_MIN + PANE_MAX + (MATERIAL,))]
    status, log, control_messages = render(args.tree, root, control_path, [])
    checks.check(status == 0, "control.exit", log.strip()[-300:])
    status, log, pane_messages = render(args.tree, root, pane_path, mover)
    checks.check(status == 0, "pane.exit", log.strip()[-300:])
    checks.check("scene color captured for transmission" in log, "pane.capture-recorded")
    checks.check("scene color captured" not in control_path.with_suffix(".log").read_text(),
                 "control.no-capture-without-transmission")
    # The game's terms: SSR, GTAO and the projector bounce are off in its
    # shipped profile (portal2-linux-native-vulkan-high.json).
    checks.check(control_messages == 0 and pane_messages == 0, "validation.silent",
                 "control %s, pane %s" % (control_messages, pane_messages))
    if not (control_path.exists() and pane_path.exists()):
        return checks.report()
    control, pane = read_pfm(control_path), read_pfm(pane_path)
    checks.check(np.isfinite(control).all() and np.isfinite(pane).all(), "image.finite")
    checks.check(float(control.mean()) > 1e-4, "control.lit", "mean %g" % control.mean())

    x0, y0 = project((PANE_MIN[0], PANE_MIN[1], PANE_MAX[2]))
    x1, y1 = project((PANE_MAX[0], PANE_MIN[1], PANE_MIN[2]))
    rows, cols = np.mgrid[0:HEIGHT, 0:WIDTH]
    centers_x, centers_y = cols + 0.5, rows + 0.5
    margin = 2.0
    interior = ((centers_x > x0 + margin) & (centers_x < x1 - margin) &
                (centers_y > y0 + margin) & (centers_y < y1 - margin))
    exterior = ((centers_x < x0 - margin) | (centers_x > x1 + margin) |
                (centers_y < y0 - margin) | (centers_y > y1 + margin))
    checks.check(interior.sum() > 5000, "pane.interior-size", "%d pixels" % interior.sum())
    changed = np.any(control != pane, axis=-1)
    checks.check(not changed[exterior].any(), "pane.exterior-unchanged",
                 "%d exterior pixels changed" % changed[exterior].sum())
    fraction, worst = interior_error(control, pane, interior)
    checks.check(fraction >= INTERIOR_PASS_FRACTION, "pane.background-times-tint",
                 "%.4f within, worst %.2f bounds" % (fraction, worst))
    checks.check(float(np.abs(pane[interior] - control[interior]).max()) > 1e-4,
                 "pane.visible", "the pane does not change the frame")

    for defect in DEFECTS:
        path = work / ("defect-%s.pfm" % defect)
        status, log, _ = render(args.tree, root, path, mover + ["--transmission-defect", defect])
        if not checks.check(status == 0, "seeded.%s.renders" % defect, log.strip()[-300:]):
            continue
        seeded_fraction, seeded_worst = interior_error(control, read_pfm(path), interior)
        checks.check(seeded_fraction < INTERIOR_PASS_FRACTION, "seeded.%s.detected" % defect,
                     "%.4f within, worst %.2f bounds" % (seeded_fraction, seeded_worst))
    if not args.out:
        shutil.rmtree(work, ignore_errors=True)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
