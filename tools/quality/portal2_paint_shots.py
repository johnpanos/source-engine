#!/usr/bin/env python3
"""Check the paint gel screenshots of a portal2-paint-v1 scenario run.

tools/quality/portal2_scenarios.py runs quality/workloads/portal2-paint-v1,
whose driver prints one ``QA_SHOT <scenario> <name>`` line per ``screenshot``
command: the view before the gel sprayers start (``empty``), the frozen gel
drawn as the blobulator's isosurface (``isosurface``) and as the fast-sphere
fallback (``spheres``). This tool pairs the screenshots the run wrote into the
runtime with those names, copies them into the evidence directory as PNG, and
checks what the gel must show:

  gel.visible           the isosurface shot differs from the empty view
  gel.repulsion_blue    blue gel pixels (paintblobs/blob_surface_bounce)
  gel.propulsion_orange orange gel pixels (paintblobs/blob_surface_speed)
  gel.isosurface_drawn  the isosurface shot differs from the sphere shot, so
                        the blobulator, not the sphere fallback, drew it
  gel.spheres_visible   the sphere fallback also draws the gel

It fails when the scenario did not pass, when the number of screenshots the
run wrote differs from its QA_SHOT lines, or when a named shot is missing.

    python3 tools/quality/portal2_paint_shots.py --evidence <portal2_scenarios --out>
"""

import argparse
import datetime
import json
from pathlib import Path
import re
import sys

import numpy as np
from PIL import Image


SCHEMA = "portal2-paint-shots/v1"
SHOT_LINE = re.compile(r"^QA_SHOT (\S+) (\S+)\s*$")
REQUIRED_SHOTS = ("empty", "isosurface", "spheres")

# A pixel changed when its RGB distance (sum of absolute differences) to the
# other shot exceeds this; well above the run-to-run noise of a static view.
CHANGE_THRESHOLD = 30
# Fractions of the frame (the gel covers ~2-4% of it at 1024x768).
MIN_GEL_FRACTION = 0.002
MIN_ISOSURFACE_VS_SPHERES_FRACTION = 0.0005
# Gel colour pixels among the changed ones (the base textures are pure blue
# (0, 165, 255) and orange (255, 109, 0); lighting scales them).
MIN_COLOUR_PIXELS = 150


class ShotError(Exception):
    pass


def changed_mask(a, b):
    return np.abs(a.astype(int) - b.astype(int)).sum(axis=2) > CHANGE_THRESHOLD


def blue_pixels(image, mask):
    r, g, b = (image[..., k].astype(int) for k in range(3))
    return int((mask & (b > 90) & (b > r + 40) & (b >= g)).sum())


def orange_pixels(image, mask):
    r, g, b = (image[..., k].astype(int) for k in range(3))
    # Saturated: an unlit or untextured (white-based) blob is pale, not orange.
    return int((mask & (r > 110) & (10 * g < 7 * r) & (10 * b < 4 * r)).sum())


def check_shots(shots):
    """shots: name -> HxWx3 uint8 array. Returns check -> (ok, detail)."""
    missing = [name for name in REQUIRED_SHOTS if name not in shots]
    if missing:
        raise ShotError("missing shots: " + ", ".join(missing))
    empty, iso, spheres = (shots[name] for name in REQUIRED_SHOTS)
    if not empty.shape == iso.shape == spheres.shape:
        raise ShotError("shots differ in size")
    pixels = empty.shape[0] * empty.shape[1]

    gel = changed_mask(iso, empty)
    sphere_gel = changed_mask(spheres, empty)
    iso_vs_spheres = changed_mask(iso, spheres)
    blue = blue_pixels(iso, gel)
    orange = orange_pixels(iso, gel)
    checks = {
        "gel.visible": (gel.sum() >= MIN_GEL_FRACTION * pixels,
                        "%d changed pixels (need %d)" % (gel.sum(), MIN_GEL_FRACTION * pixels)),
        "gel.repulsion_blue": (blue >= MIN_COLOUR_PIXELS,
                               "%d blue gel pixels (need %d)" % (blue, MIN_COLOUR_PIXELS)),
        "gel.propulsion_orange": (orange >= MIN_COLOUR_PIXELS,
                                  "%d orange gel pixels (need %d)" % (orange, MIN_COLOUR_PIXELS)),
        "gel.isosurface_drawn": (
            iso_vs_spheres.sum() >= MIN_ISOSURFACE_VS_SPHERES_FRACTION * pixels,
            "%d pixels differ from the sphere fallback (need %d)"
            % (iso_vs_spheres.sum(), MIN_ISOSURFACE_VS_SPHERES_FRACTION * pixels)),
        "gel.spheres_visible": (sphere_gel.sum() >= MIN_GEL_FRACTION * pixels,
                                "%d changed pixels (need %d)"
                                % (sphere_gel.sum(), MIN_GEL_FRACTION * pixels)),
    }
    return {name: (bool(ok), detail) for name, (ok, detail) in checks.items()}


def collect_shots(evidence_dir, scenario):
    """Pairs the run's screenshots with its QA_SHOT names; returns name -> path."""
    evidence_dir = Path(evidence_dir)
    evidence = json.loads((evidence_dir / "evidence.json").read_text())
    results = [r for r in evidence.get("results", []) if r.get("scenario") == scenario]
    if len(results) != 1:
        raise ShotError("evidence has no single result for %s" % scenario)
    if results[0].get("status") != "pass":
        raise ShotError("scenario %s did not pass" % scenario)
    log = (evidence_dir / scenario / "console.log").read_text(errors="replace")
    names = [m.group(2) for m in map(SHOT_LINE.match, log.splitlines())
             if m and m.group(1) == scenario]
    if len(set(names)) != len(names):
        raise ShotError("duplicate QA_SHOT names: %s" % names)
    started = datetime.datetime.fromisoformat(evidence["started_utc"]).timestamp()
    directory = Path(evidence["runtime"]) / "portal2/screenshots"
    files = sorted(p for p in directory.glob("*.tga") if p.stat().st_mtime >= started)
    if len(files) != len(names):
        raise ShotError("%d screenshots written since the run started, %d QA_SHOT lines"
                        % (len(files), len(names)))
    return dict(zip(names, files))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--evidence", type=Path, required=True,
                        help="the portal2_scenarios.py --out directory of a portal2-paint-v1 run")
    parser.add_argument("--scenario", default="sp_a3_speed_ramp_gel")
    args = parser.parse_args(argv)

    report = {"schema": SCHEMA, "scenario": args.scenario, "status": "fail", "checks": {}}
    out = args.evidence / args.scenario / "shots"
    try:
        paths = collect_shots(args.evidence, args.scenario)
        out.mkdir(parents=True, exist_ok=True)
        shots = {}
        for name, path in paths.items():
            image = Image.open(path).convert("RGB")
            image.save(out / (name + ".png"))
            shots[name] = np.asarray(image)
        checks = check_shots(shots)
    except (OSError, ValueError, KeyError, ShotError) as error:
        report["error"] = str(error)
        print("portal2_paint_shots: FAIL %s" % error)
    else:
        report["checks"] = {name: {"outcome": "PASS" if ok else "FAIL", "detail": detail}
                            for name, (ok, detail) in checks.items()}
        report["status"] = "pass" if all(ok for ok, _ in checks.values()) else "fail"
        for name, (ok, detail) in checks.items():
            print("  %-4s %s %s" % ("PASS" if ok else "FAIL", name, detail))
        print("portal2_paint_shots: %s" % report["status"].upper())
    out.mkdir(parents=True, exist_ok=True)
    (out / "shots.json").write_text(json.dumps(report, indent=2) + "\n")
    return 0 if report["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
