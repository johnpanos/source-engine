#!/usr/bin/env python3
"""Compare the testchmb_a_01 view set with the legacy shader ports on and off.

R32-LEGACY-SHADERS' default-on gate asks that the map ./kiln play portal boots look the
same with the ports as without them, except where a reviewed port improvement
is expected. This boots the installed Portal product (through portal_boot,
which owns staging) three times at the portal profile's launch settings: the ports on (the
default), off (-novklegacyports), and on again. Each run captures the same
views: eight yaws and two upward views from the spawn point, then the pause
menu.

For each view the metric is the fraction of pixels whose largest channel
difference exceeds 16/255. The second ports-on run measures run-to-run noise
(dust, flicker, auto-exposure). A view passes when on-versus-off stays within
FRACTION_LIMIT, or within NOISE_FACTOR times that view's own noise. A negative
control compares two different views; it must exceed CONTROL_MINIMUM, or the
comparison could not see a difference at all.

  legacy_ports_views.py --runtime run/runtime --build build --out quality-results/ports-views

Keep --out short: the engine refuses command lines over 512 characters.
"""

import argparse
import datetime
import json
from pathlib import Path
import subprocess
import sys

import numpy
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402

QUALITY = Path(__file__).resolve().parent
PORTAL_BOOT = QUALITY / "portal_boot.py"
SCHEMA = "legacy-ports-views/v1"

MAP = "testchmb_a_01"
# (name, pitch, yaw) from the spawn point; negative pitch looks up.
VIEWS = tuple(("yaw%03d" % yaw, 0, yaw) for yaw in range(0, 360, 45)) + (
    ("up000", -50, 0), ("up180", -50, 180))
PAUSE = "pause_menu"
# the portal profile's launch settings (its mat_args, job_args, queue and physics variables).
# Console variables go in portal_boot's startup cfg, which keeps the command
# line under the engine's 512 characters (argument lengths summed, the program
# path included). The cfg runs after portal_boot's own +mat_queue_mode 0, so
# its mat_queue_mode 2 wins.
STARTUP_COMMANDS = ("mat_queue_mode 2", "mat_bumpmap 1", "mat_picmip -1", "r_lod 0",
                    "r_indirect_producer auto", "cl_render_start_graph 2",
                    "sv_querycache_job_graph 2", "portal_carve_job_graph 2")
ENGINE_ARGS = ("-vkemitparallel", "1", "-physics_shape_inertia")
WIDTH, HEIGHT = 1920, 1080
# Frames after each camera move before its screenshot, and after it.
SETTLE_FRAMES = 45
AFTER_FRAMES = 20

PIXEL_THRESHOLD = 16
FRACTION_LIMIT = 0.005
NOISE_FACTOR = 3.0
CONTROL_MINIMUM = 0.05

RUNS = (("on", ()), ("off", ("-novklegacyports",)), ("on2", ()))


def capture_script():
    """One console line: waits delay only the commands after them on that line."""
    steps = []
    for _, pitch, yaw in VIEWS:
        steps += ["cmd setang %d %d 0" % (pitch, yaw), "wait %d" % SETTLE_FRAMES, "screenshot",
                  "wait %d" % AFTER_FRAMES]
    steps += ["gameui_activate", "wait %d" % (SETTLE_FRAMES * 2), "screenshot",
              "wait %d" % AFTER_FRAMES]
    return "; ".join(steps)


def capture_frames():
    return (len(VIEWS) + 2) * (SETTLE_FRAMES + AFTER_FRAMES) + 120


def boot(args, name, extra, out):
    command = [sys.executable, str(PORTAL_BOOT), *sepipe_loader.boot_arguments(args),
               "--out", str(out), "--headless", "--map", MAP,
               "--renderer", "native-vulkan", "--physics", "vphysics_box3d", "--require-vulkan",
               "--width", str(WIDTH), "--height", str(HEIGHT),
               "--capture-wait", str(capture_frames()), "--timeout", str(args.timeout),
               "--console-command", capture_script()]
    for argument in ENGINE_ARGS + tuple(extra):
        command.append("--engine-arg=" + argument)
    for line in STARTUP_COMMANDS:
        command += ["--startup-command", line]
    completed = subprocess.run(command, capture_output=True, text=True)
    shots = sorted((out / "runtime/portal/screenshots").glob("*.tga"))
    names = [view[0] for view in VIEWS] + [PAUSE]
    return {"run": name, "extra": list(extra), "returncode": completed.returncode,
            "boot_tail": completed.stdout[-400:] + completed.stderr[-400:],
            "screenshots": {n: str(path) for n, path in zip(names, shots)},
            "screenshot_count": len(shots)}


def load(path):
    return numpy.asarray(Image.open(path).convert("RGB"), dtype=numpy.int16)


def differing_fraction(a, b, threshold=PIXEL_THRESHOLD):
    """Fraction of pixels whose largest channel difference exceeds `threshold`."""
    if a.shape != b.shape:
        return 1.0
    return float((numpy.abs(a - b).max(axis=2) > threshold).mean())


def judge(on_off, noise):
    limit = max(FRACTION_LIMIT, NOISE_FACTOR * noise)
    return on_off <= limit, limit


def write_difference(a, b, path):
    """The differing pixels in white over a dimmed ports-on frame."""
    mask = numpy.abs(a - b).max(axis=2) > PIXEL_THRESHOLD
    image = (a // 3).astype(numpy.uint8)
    image[mask] = 255
    Image.fromarray(image).save(path)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sepipe_loader.add_arguments(parser, "portal")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=600)
    args = parser.parse_args(argv)
    out = args.out.resolve()
    if (out / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    out.mkdir(parents=True, exist_ok=True)
    evidence = {"schema": SCHEMA, "status": "fail", "map": MAP,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "engine_args": list(ENGINE_ARGS), "startup_commands": list(STARTUP_COMMANDS),
                "size": [WIDTH, HEIGHT],
                "pixel_threshold": PIXEL_THRESHOLD, "fraction_limit": FRACTION_LIMIT,
                "noise_factor": NOISE_FACTOR, "control_minimum": CONTROL_MINIMUM,
                "runs": [], "views": [], "failures": []}
    runs = {}
    for name, extra in RUNS:
        result = boot(args, name, extra, out / name)
        evidence["runs"].append(result)
        runs[name] = result
        if result["returncode"] != 0 or result["screenshot_count"] < len(VIEWS) + 1:
            evidence["failures"].append("run %s: exit %s with %d screenshots"
                                        % (name, result["returncode"],
                                           result["screenshot_count"]))
    if not evidence["failures"]:
        images = {name: {view: load(path) for view, path in run["screenshots"].items()}
                  for name, run in runs.items()}
        views = out / "views"
        views.mkdir(exist_ok=True)
        for view in [v[0] for v in VIEWS] + [PAUSE]:
            on, off, on2 = images["on"][view], images["off"][view], images["on2"][view]
            on_off = differing_fraction(on, off)
            noise = differing_fraction(on, on2)
            passed, limit = judge(on_off, noise)
            for label, image in (("on", on), ("off", off)):
                Image.fromarray(image.astype(numpy.uint8)).save(views / ("%s-%s.png"
                                                                         % (view, label)))
            write_difference(on, off, views / ("%s-diff.png" % view))
            evidence["views"].append({"view": view, "on_off": on_off, "noise": noise,
                                      "limit": limit, "pass": passed})
            if not passed:
                evidence["failures"].append("%s: %.4f of pixels differ on vs off (limit %.4f, "
                                            "noise %.4f)" % (view, on_off, limit, noise))
        control = differing_fraction(images["on"][VIEWS[0][0]], images["off"][VIEWS[2][0]])
        evidence["negative_control"] = {"views": [VIEWS[0][0], VIEWS[2][0]], "fraction": control}
        if control < CONTROL_MINIMUM:
            evidence["failures"].append("negative control: different views differ in only %.4f "
                                        "of pixels; the comparison cannot see a change" % control)
    evidence["status"] = "pass" if not evidence["failures"] else "fail"
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    for view in evidence["views"]:
        print("%-10s on/off %.4f  noise %.4f  limit %.4f  %s"
              % (view["view"], view["on_off"], view["noise"], view["limit"],
                 "pass" if view["pass"] else "FAIL"))
    if "negative_control" in evidence:
        print("negative control %.4f" % evidence["negative_control"]["fraction"])
    print("legacy ports views: %s (%s)" % (evidence["status"], out / "evidence.json"))
    for failure in evidence["failures"]:
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
