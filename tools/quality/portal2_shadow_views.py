#!/usr/bin/env python3
"""Shadowed lights keep their shadows as the camera turns (RFC 0016 K7/K12).

Boots a Portal 2 map on build-p2 with the render core's world through
portal2_map_views.py (a private staged runtime, SDL's offscreen driver), with
cl_render_debug_stats on, and holds each view of a workload for a number of
frames. The core's stats report, once a second, how many shadowed lights a
view's shadow-atlas plan left without tiles; a light without a tile is drawn
unshadowed. Checks:

    - every view is captured and the map loads clean (portal2_map_views);
    - each held view has stats lines, and every one reports at most
      `unshadowed_lights_max` shadowed lights without tiles per frame;
    - the shadowed view is darker than the same view with the core's
      shadows off by at least `shadow_darkening_min` of its mean luminance
      (with the light's shadow lost, the view is nearly as bright as the
      unshadowed one).

    python3 tools/quality/portal2_shadow_views.py \\
        --workload quality/workloads/portal2-intro4-shadows-v1/views.json --out DIR

Ends with one checks-v1 record; <out>/shadow-views.json holds the numbers.
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import portal2_map_views  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
SCHEMA = "portal2-shadow-views/v1"
UNSHADOWED = re.compile(r"([0-9.]+) shadowed lights without tiles per frame")


def steps_for(workload):
    steps = ["do cl_render_debug_gpu_timers 1; cl_render_debug_stats 1", "wait 200"]
    for view in workload["views"]:
        if view.get("before"):
            steps.append("do " + view["before"])
        x, y, z = view["origin"]
        pitch, yaw = view["angles"]
        steps.append("view %s %g %g %g %g %g" % (view["name"], x, y, z, pitch, yaw))
        steps.append("wait %d" % workload["hold_frames"])
        steps.append("do echo QA_HELD %s" % view["name"])
    return steps


def held_windows(log, names):
    """The console lines from each view's QA_VIEW marker to its QA_HELD one."""
    # The console can print an echo's words around another thread's output
    # ("turned QA_HELD"): a marker is its two words in any order.
    windows = {}
    lines = log.splitlines()
    for name in names:
        start = next((i for i, line in enumerate(lines)
                      if set(line.split()) == {"QA_VIEW", name}), None)
        end = next((i for i, line in enumerate(lines)
                    if set(line.split()) == {"QA_HELD", name}), None)
        windows[name] = lines[start:end] if start is not None and end is not None else None
    return windows


def mean_luminance(path):
    from PIL import Image, ImageStat
    image = Image.open(path).convert("L")
    return ImageStat.Stat(image).mean[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--workload", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--build", type=Path, default=ROOT / "build-p2")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-shadow-views")
    parser.add_argument("--steam-root", type=Path, default=Path(os.environ.get(
        "SOURCE_PORTAL2_STEAM_ROOT",
        Path.home() / ".local/share/Steam/steamapps/common/Portal 2")))
    parser.add_argument("--timeout", type=int, default=600)
    args = parser.parse_args()
    workload = json.loads(args.workload.read_text())
    checks = Checks()
    if not checks.equal(workload.get("schema"), SCHEMA, "workload schema"):
        return checks.report()
    names = [view["name"] for view in workload["views"]]
    views_args = argparse.Namespace(
        map=workload["map"], out=args.out, step=steps_for(workload), build=args.build,
        runtime=args.runtime, steam_root=args.steam_root, physics="vphysics_box3d",
        engine_arg=workload["engine_args"], width=1600, height=900, start_frames=200,
        settle=90, shot_frames=60, timeout=args.timeout)
    result = portal2_map_views.run(views_args)
    checks.equal(result["status"], "pass", "the map boots and every view is captured")
    for failure in result["failures"]:
        print("INFO views: " + failure)
    log = (args.out.resolve() / "console.log").read_text(errors="replace") \
        if (args.out.resolve() / "console.log").is_file() else ""
    limit = workload["checks"]["unshadowed_lights_max"]
    record = {"schema": SCHEMA, "workload": str(args.workload), "views": {}}
    for name, window in held_windows(log, names).items():
        values = [float(m.group(1)) for line in (window or [])
                  for m in [UNSHADOWED.search(line)] if m]
        record["views"][name] = {"unshadowed_per_frame": values}
        if name == workload["checks"]["unshadowed_view"]:
            continue  # shadows off: no plan
        checks.check(bool(values), "%s: the core reports its shadow plan" % name,
                     "no stats line while the view was held")
        checks.check(all(v <= limit for v in values),
                     "%s: shadowed lights keep their tiles" % name,
                     "shadowed lights without tiles per frame: %s" % values)
    views = result.get("views", {})
    shadowed = views.get(workload["checks"]["shadowed_view"])
    unshadowed = views.get(workload["checks"]["unshadowed_view"])
    if checks.check(bool(shadowed and unshadowed), "the shadow comparison views exist"):
        lit = mean_luminance(unshadowed)
        shaded = mean_luminance(shadowed)
        darkening = 1.0 - shaded / lit if lit > 0 else 0.0
        record["luminance"] = {"shadowed": shaded, "unshadowed": lit, "darkening": darkening}
        checks.within(darkening, workload["checks"]["shadow_darkening_min"], None,
                      "the turned view keeps its shadows (darkening against shadows off)")
    (args.out.resolve() / "shadow-views.json").write_text(json.dumps(record, indent=2) + "\n")
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
