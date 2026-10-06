#!/usr/bin/env python3
"""Compare a release-flavor build's frames with its dev build (RFC 0023 R2).

A release tree (`--product-flavor=release`: -O3, -march, LTO, instrumentation
compiled out) must draw the same image as the dev tree built from the same
source. This boots the product through portal_boot three times -- dev, release,
dev again -- and captures the same views each time: eight yaws and two upward
views from the spawn point, then the pause menu (legacy_ports_views' set).

Per view the metric is the fraction of pixels whose largest channel difference
exceeds 16/255. The second dev run measures run-to-run noise (dust, flicker,
auto-exposure). A view passes when dev-versus-release stays within the fraction
limit or within NOISE_FACTOR times that view's own noise. The negative control
compares two different views and must exceed CONTROL_MINIMUM.

  release_views.py --game portal --dev-build build --release-build build-release \\
      --runtime run/runtime --out /tmp/claude-1000/rv-p1
  release_views.py --game portal2 --map sp_a1_intro4 --dev-build build-p2 \\
      --release-build build-p2-release --dev-runtime RT_DEV --release-runtime RT_REL \\
      --out /tmp/claude-1000/rv-p2

Portal 2 runtimes come from stage_portal2_runtime.py (retail content). Keep --out
short: the engine refuses command lines over 512 characters.
"""

import argparse
import datetime
import json
from pathlib import Path
import subprocess
import sys

import numpy
from PIL import Image

import legacy_ports_views as views

SCHEMA = "release-views/v1"
DEFAULT_MAP = {"portal": "testchmb_a_01", "portal2": "sp_a1_intro4"}
RUNS = ("dev", "release", "dev2")
# The view model's idle sway follows the time since load, and release loads
# faster than dev, so its pose differs by a few pixels while the world matches
# (first Portal 2 run, 2026-10-06: only the portal gun differed). Hide it.
EXTRA_STARTUP = ("r_drawviewmodel 0",)


def temporal_script():
    """The view script with each screenshot replaced by an FSR output capture."""
    names = iter([view[0] for view in views.VIEWS] + [views.PAUSE])
    return "; ".join("r_temporal_capture rvc_" + next(names) if step == "screenshot" else step
                     for step in views.capture_script().split("; "))


def load_temporal(prefix):
    """FSR output (.output.rgba16f) clamped to [0, 1] and scaled to 0-255 like a frame."""
    meta = json.loads(Path(str(prefix) + ".json").read_text())
    width, height = meta["output"]
    data = numpy.fromfile(str(prefix) + ".output.rgba16f", dtype=numpy.float16)
    rgb = data.reshape(height, width, 4)[:, :, :3].astype(numpy.float32)
    return (numpy.clip(rgb, 0.0, 1.0) * 255.0 + 0.5).astype(numpy.int16)


def boot(args, name, out):
    flavor = "release" if name == "release" else "dev"
    build = args.release_build if flavor == "release" else args.dev_build
    runtime = (args.release_runtime if flavor == "release" else args.dev_runtime) or args.runtime
    command = [sys.executable, str(views.PORTAL_BOOT), "--runtime", str(runtime),
               *(("--build", str(build)) if build else ()), "--out", str(out), "--headless", "--game", args.game,
               "--map", args.map, "--renderer", "native-vulkan", "--physics", "vphysics_box3d",
               "--require-vulkan", "--width", str(views.WIDTH), "--height", str(views.HEIGHT),
               "--capture-wait", str(views.capture_frames()), "--timeout", str(args.timeout),
               ]
    # One --console-command per step: portal_boot chains them in cfg scripts that
    # keep each wait ordered, where one joined line can exceed the parser's limit.
    script = temporal_script() if args.capture == "temporal" else views.capture_script()
    for step in script.split("; "):
        command += ["--console-command", step]
    for argument in views.ENGINE_ARGS + tuple(args.engine_arg):
        command.append("--engine-arg=" + argument)
    for line in views.STARTUP_COMMANDS + EXTRA_STARTUP + tuple(args.startup_command):
        command += ["--startup-command", line]
    completed = subprocess.run(command, capture_output=True, text=True)
    names = [view[0] for view in views.VIEWS] + [views.PAUSE]
    if args.capture == "temporal":
        shots = [out / "runtime" / ("rvc_" + n) for n in names
                 if (out / "runtime" / ("rvc_" + n + ".output.rgba16f")).exists()]
    else:
        shots = sorted((out / "runtime" / args.game / "screenshots").glob("*.tga"))
    return {"run": name, "build": str(build), "returncode": completed.returncode,
            "boot_tail": completed.stdout[-400:] + completed.stderr[-400:],
            "screenshots": {n: str(path) for n, path in zip(names, shots)},
            "screenshot_count": len(shots)}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--game", choices=("portal", "portal2"), default="portal")
    parser.add_argument("--map")
    parser.add_argument("--dev-build", type=Path,
                        help="omit when --dev-runtime is already staged from the dev tree")
    parser.add_argument("--release-build", type=Path,
                        help="omit when --release-runtime is already staged from the release tree")
    parser.add_argument("--runtime", type=Path, help="runtime for both flavors")
    parser.add_argument("--dev-runtime", type=Path)
    parser.add_argument("--release-runtime", type=Path)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument("--engine-arg", action="append", default=[],
                        help="extra engine argument for every run, e.g. --engine-arg=-fsr")
    parser.add_argument("--ignore-exit", action="store_true",
                        help="judge captures from runs that fail only at exit (recorded in the "
                             "evidence; FSR trees trap at shutdown in dev and release alike)")
    parser.add_argument("--capture", choices=("screenshot", "temporal"), default="screenshot",
                        help="temporal reads FSR's output with r_temporal_capture; the engine's "
                             "screenshot reads back garbage while FSR runs (2026-10-06)")
    parser.add_argument("--startup-command", action="append", default=[],
                        help="extra startup cfg line for every run, e.g. 'r_core_world 1'")
    args = parser.parse_args(argv)
    args.map = args.map or DEFAULT_MAP[args.game]
    if not args.dev_build and not args.dev_runtime or \
            not args.release_build and not args.release_runtime:
        parser.error("each flavor needs its build or its own staged runtime")
    if not args.runtime and not (args.dev_runtime and args.release_runtime):
        parser.error("give --runtime, or both --dev-runtime and --release-runtime")
    out = args.out.resolve()
    if (out / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    out.mkdir(parents=True, exist_ok=True)
    evidence = {"schema": SCHEMA, "status": "fail", "game": args.game, "map": args.map,
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "size": [views.WIDTH, views.HEIGHT], "pixel_threshold": views.PIXEL_THRESHOLD,
                "fraction_limit": views.FRACTION_LIMIT, "noise_factor": views.NOISE_FACTOR,
                "control_minimum": views.CONTROL_MINIMUM, "capture": args.capture,
                "ignore_exit": args.ignore_exit, "runs": [], "views": [],
                "failures": []}
    runs = {}
    for name in RUNS:
        result = boot(args, name, out / name)
        evidence["runs"].append(result)
        runs[name] = result
        bad_exit = result["returncode"] != 0 and not args.ignore_exit
        if bad_exit or result["screenshot_count"] < len(views.VIEWS) + 1:
            evidence["failures"].append("run %s: exit %s with %d screenshots"
                                        % (name, result["returncode"], result["screenshot_count"]))
    if not evidence["failures"]:
        load = load_temporal if args.capture == "temporal" else views.load
        images = {name: {view: load(path) for view, path in run["screenshots"].items()}
                  for name, run in runs.items()}
        frames = out / "views"
        frames.mkdir(exist_ok=True)
        for view in [v[0] for v in views.VIEWS] + [views.PAUSE]:
            dev, release, dev2 = (images[name][view] for name in RUNS)
            difference = views.differing_fraction(dev, release)
            noise = views.differing_fraction(dev, dev2)
            passed, limit = views.judge(difference, noise)
            for label, image in (("dev", dev), ("release", release)):
                Image.fromarray(image.astype(numpy.uint8)).save(frames / ("%s-%s.png"
                                                                          % (view, label)))
            views.write_difference(dev, release, frames / ("%s-diff.png" % view))
            evidence["views"].append({"view": view, "dev_release": difference, "noise": noise,
                                      "limit": limit, "pass": passed})
            if not passed:
                evidence["failures"].append("%s: %.4f of pixels differ dev vs release (limit "
                                            "%.4f, noise %.4f)" % (view, difference, limit, noise))
        first, third = views.VIEWS[0][0], views.VIEWS[2][0]
        control = views.differing_fraction(images["dev"][first], images["release"][third])
        evidence["negative_control"] = {"views": [first, third], "fraction": control}
        if control < views.CONTROL_MINIMUM:
            evidence["failures"].append("negative control: different views differ in only %.4f "
                                        "of pixels; the comparison cannot see a change" % control)
    evidence["status"] = "pass" if not evidence["failures"] else "fail"
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    for view in evidence["views"]:
        print("%-10s dev/release %.4f  noise %.4f  limit %.4f  %s"
              % (view["view"], view["dev_release"], view["noise"], view["limit"],
                 "pass" if view["pass"] else "FAIL"))
    if "negative_control" in evidence:
        print("negative control %.4f" % evidence["negative_control"]["fraction"])
    print("release views: %s (%s)" % (evidence["status"], out / "evidence.json"))
    for failure in evidence["failures"]:
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
