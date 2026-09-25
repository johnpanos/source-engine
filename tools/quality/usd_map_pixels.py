#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Pixel oracle for a rendered frame of a USD-compiled map (RFC 0009 U1).
#
#   python3 tools/quality/usd_map_pixels.py check SCREENSHOT.tga \
#       --report authoring-report.json --origin -192 -128 0 --angles 10 20 0 \
#       [--fov 90] [--json out.json]
#
# The oracle is the authored stage, not the compiler or the renderer: each
# pixel block is labelled with the material of the first authored world solid
# its view ray meets (rays against the validator's Source-space face planes,
# through Source's camera: eye 64 units above the origin, horizontal FOV for
# the 4:3 frame, AngleVectors). Checks:
#   - every material the camera sees is lit (not black, not saturated);
#   - the frame's colour changes where the authored material boundaries
#     project: the median change between blocks on either side of a
#     predicted boundary is at least MIN_CONTRAST times the median change
#     within one material;
#   - labels from wrong cameras (yaw +-10 degrees, pitch +8, eye +48 units,
#     FOV -15) line up worse by at least MIN_RATIO: the frame shows the
#     authored geometry where the authored geometry projects.
# Props are not modelled; they cover few blocks. Prints the checks-v1 record.
# Needs numpy and Pillow.
#
# ============================================================================

import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from conformance_result import Checks  # noqa: E402

EYE_HEIGHT = 64.0
BLOCK = 8
MIN_CONTRAST = 3.0
MIN_RATIO = 1.5
DARK, BRIGHT = 8.0, 247.0
# Wrong cameras (origin offset, angle offset, FOV offset): each must line up
# worse with the frame than the authored camera.
CONTROLS = {"yaw+10": ((0, 0, 0), (0, 10, 0), 0), "yaw-10": ((0, 0, 0), (0, -10, 0), 0),
            "pitch+8": ((0, 0, 0), (8, 0, 0), 0), "eye+48": ((0, 0, 48), (0, 0, 0), 0),
            "fov-15": ((0, 0, 0), (0, 0, 0), -15)}


def angle_vectors(pitch, yaw, roll):
    """Source mathlib AngleVectors: forward, right, up."""
    sp, sy, sr = (math.sin(math.radians(a)) for a in (pitch, yaw, roll))
    cp, cy, cr = (math.cos(math.radians(a)) for a in (pitch, yaw, roll))
    forward = np.array([cp * cy, cp * sy, -sp])
    right = np.array([-sr * sp * cy + cr * sy, -sr * sp * sy - cr * cy, -sr * cp])
    up = np.array([cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp])
    return forward, right, up


def label_blocks(report, width, height, origin, angles, fov):
    """Material index per block (-1: no authored solid), and the names."""
    solids = [o for o in report["objects"] if o["role"] == "world_solid"]
    materials = sorted({s["material"] for o in solids for s in o["surfaces"]})
    eye = np.array([origin[0], origin[1], origin[2] + EYE_HEIGHT])
    forward, right, up = angle_vectors(*angles)
    half = math.tan(math.radians(fov) / 2.0)
    columns, rows = width // BLOCK, height // BLOCK
    u = (np.arange(columns) + 0.5) * BLOCK
    v = (np.arange(rows) + 0.5) * BLOCK
    uu, vv = np.meshgrid(u, v)
    x = (2.0 * uu / width - 1.0) * half
    y = (1.0 - 2.0 * vv / height) * half * height / width
    rays = forward[None, None, :] + x[..., None] * right + y[..., None] * up
    best = np.full(uu.shape, np.inf)
    label = np.full(uu.shape, -1, dtype=np.int64)
    for record in solids:
        enter = np.full(uu.shape, -np.inf)
        leave = np.full(uu.shape, np.inf)
        enter_face = np.zeros(uu.shape, dtype=np.int64)
        for index, face in enumerate(record["faces"]):
            n = np.array(face["plane"][:3])
            rate = rays @ n
            distance = face["plane"][3] - eye @ n
            with np.errstate(divide="ignore", invalid="ignore"):
                t = distance / rate
            entering = rate < 0
            update = entering & (t > enter)
            enter = np.where(update, t, enter)
            enter_face = np.where(update, index, enter_face)
            leave = np.where(~entering & (rate > 0), np.minimum(leave, t), leave)
            # A ray parallel to a face and in front of it misses the solid.
            leave = np.where((rate == 0) & (distance < 0), -np.inf, leave)
        hit = (enter <= leave) & (enter > 0) & (enter < best)
        best = np.where(hit, enter, best)
        face_material = np.array([materials.index(s["material"]) for s in record["surfaces"]])
        label = np.where(hit, face_material[enter_face], label)
    return label, materials


def block_colors(image):
    rows, columns = image.shape[0] // BLOCK, image.shape[1] // BLOCK
    trimmed = image[:rows * BLOCK, :columns * BLOCK, :3].astype(np.float64)
    return trimmed.reshape(rows, BLOCK, columns, BLOCK, 3).mean(axis=(1, 3))


def boundary_contrast(colors, label, gap=2):
    """Median colour change between blocks `gap` apart that the labels put
    on different materials, over the median change within one material. A
    frame that shows the authored geometry changes colour where the
    authored material boundaries project."""
    across, within = [], []
    for axis in (0, 1):
        first = [slice(None), slice(None)]
        second = [slice(None), slice(None)]
        first[axis], second[axis] = slice(None, -gap), slice(gap, None)
        a, b = colors[tuple(first)], colors[tuple(second)]
        la, lb = label[tuple(first)], label[tuple(second)]
        valid = (la >= 0) & (lb >= 0)
        change = np.linalg.norm(a - b, axis=-1)
        across.append(change[valid & (la != lb)])
        within.append(change[valid & (la == lb)])
    across, within = np.concatenate(across), np.concatenate(within)
    if len(across) < 16 or len(within) < 16:
        return 0.0
    return float(np.median(across) / max(np.median(within), 1e-6))


def check_frame(image, report, origin, angles, fov, checks):
    height, width = image.shape[:2]
    colors = block_colors(image)
    label, materials = label_blocks(report, width, height, origin, angles, fov)
    seen = [m for m in range(len(materials)) if (label == m).sum() >= 16]
    score = boundary_contrast(colors, label)
    record = {"materials": materials, "boundary_contrast": score,
              "blocks": {materials[m]: int((label == m).sum()) for m in range(len(materials))}}
    checks.check(len(seen) >= 2, "pixels.materials-in-view", repr(record["blocks"]))
    for m in seen:
        luminance = float(colors[label == m].mean(axis=0) @ np.array([0.2126, 0.7152, 0.0722]))
        record.setdefault("luminance", {})[materials[m]] = round(luminance, 2)
        checks.check(DARK < luminance < BRIGHT, "pixels.material-lit",
                     "%s mean luminance %.1f" % (materials[m], luminance))
    checks.check(score >= MIN_CONTRAST, "pixels.edges-where-authored",
                 "boundary contrast %.2f < %.2f" % (score, MIN_CONTRAST))
    record["controls"] = {}
    for name, (d_origin, d_angles, d_fov) in CONTROLS.items():
        control = boundary_contrast(colors, label_blocks(
            report, width, height, [o + d for o, d in zip(origin, d_origin)],
            [a + d for a, d in zip(angles, d_angles)], fov + d_fov)[0])
        record["controls"][name] = control
        checks.check(score >= MIN_RATIO * control, "pixels.wrong-camera-edges-weaker",
                     "%s: %.2f vs authored %.2f" % (name, control, score))
    return record


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    run = commands.add_parser("check")
    run.add_argument("screenshot")
    run.add_argument("--report", required=True)
    run.add_argument("--origin", type=float, nargs=3, required=True, help="player origin")
    run.add_argument("--angles", type=float, nargs=3, required=True, help="pitch yaw roll")
    run.add_argument("--fov", type=float, default=90.0, help="horizontal FOV (4:3 frame)")
    run.add_argument("--json")
    args = parser.parse_args(argv)
    from PIL import Image
    image = np.asarray(Image.open(args.screenshot).convert("RGB"))
    report = json.loads(Path(args.report).read_text())
    checks = Checks()
    record = check_frame(image, report, args.origin, args.angles, args.fov, checks)
    record.update({"screenshot": args.screenshot, "origin": args.origin, "angles": args.angles,
                   "fov": args.fov})
    if args.json:
        Path(args.json).write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: record.get(k) for k in ("boundary_contrast", "controls",
                                                 "luminance")}))
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
