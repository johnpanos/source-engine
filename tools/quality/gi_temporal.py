#!/usr/bin/env python3
"""Temporal stability of the RFC 0011 indirect-light producers, in game.

    python3 tools/quality/gi_temporal.py capture --fixture door \\
        --producer baked --producer sdf --out quality-results/rfc0011-temporal/door
    python3 tools/quality/gi_temporal.py score --frames <dir with *.tga> [--out score.json]

`capture` boots a fixture map (quality-results/rfc0011-maps/<fixture>, or
--map-build) headless on native Vulkan once per producer, places the camera
at a fixture camera (gi_runtime's camera commands), selects the producer,
waits --warm frames (past the producer's warm-up and first convergence), then
screenshots --frames frames --stride frames apart of a scene in which nothing
changes. Every frame of a still scene should be the same frame: whatever
differs between them is flicker. (Screenshots cannot be consecutive: the
engine drops one requested while it is still writing the last.)

`score` measures a burst in linear light (sRGB decoded), on luminance, each
figure relative to the burst's mean luminance so dark and bright views
compare:

  flicker_rms        sqrt(mean over pixels of each pixel's temporal variance)
  step_max           the largest mean |frame[i+1] - frame[i]| over the burst
  pixel_range_p99    the 99th percentile over pixels of (max - min) over time

A burst passes when flicker_rms <= FLICKER_RMS_LIMIT and step_max <=
STEP_LIMIT. Eight-bit screenshots quantize sRGB to 1/255, so a still image
scores 0 and any real change in a dark pixel scores at least that step; the
limits are fractions of the mean, well above a single quantization step on a
mid-grey view. `baked` is the control: it publishes one volume and must score
0 (any nonzero figure there is the capture, not a producer).
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gi_reference  # noqa: E402
import gi_runtime  # noqa: E402
import pbrt_map_toolchain  # noqa: E402
import reference_compare  # noqa: E402

SCHEMA = "gi-temporal/v1"
MAPS = ROOT / "quality-results" / "rfc0011-maps"
FLICKER_RMS_LIMIT = 0.002
STEP_LIMIT = 0.002
# Frames a screenshot costs the quit countdown: writing the TGA stalls the
# engine for about 0.7 s, which the countdown sees as many frames.
SHOT_FRAMES = 60


def luminance_frames(paths):
    """(T, H, W) linear luminance of TGA screenshots, in capture order."""
    frames = []
    for path in paths:
        rgb = gi_runtime.srgb_decode(np.asarray(Image.open(path).convert("RGB")))
        frames.append(0.2126 * rgb[..., 0] + 0.7152 * rgb[..., 1] + 0.0722 * rgb[..., 2])
    return np.stack(frames)


def score(frames):
    """Temporal stability figures of a (T, H, W) luminance burst."""
    if len(frames) < 3:
        raise ValueError("a burst needs at least three frames")
    mean = float(frames.mean())
    scale = max(mean, 1e-6)
    variance = frames.var(axis=0)
    steps = np.abs(np.diff(frames, axis=0)).mean(axis=(1, 2)) / scale
    span = frames.max(axis=0) - frames.min(axis=0)
    result = {"frames": int(len(frames)), "mean_luminance": mean,
              "flicker_rms": float(np.sqrt(variance.mean())) / scale,
              "step_max": float(steps.max()), "step_mean": float(steps.mean()),
              "steps": [float(step) for step in steps],
              "pixel_range_p99": float(np.percentile(span, 99)) / scale,
              "changed_pixels": float((span > 0).mean())}
    result["status"] = "pass" if (result["flicker_rms"] <= FLICKER_RMS_LIMIT and
                                  result["step_max"] <= STEP_LIMIT) else "fail"
    return result


def capture_burst(args, producer, target):
    fixture = gi_reference.load_fixture(args.fixture)
    camera = args.camera or sorted(fixture["cameras"])[0]
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(ROOT / profile["layout"]["toolchain_file"])
    runtime = Path(args.runtime or toolchain["runtime"])
    map_build = Path(args.map_build or MAPS / args.fixture)
    manifest = json.loads((fixture["directory"] / fixture["map_manifest"]).read_text())
    placement, _ = reference_compare.camera_commands(gi_runtime.camera_scene(fixture, camera))
    # The burst as a chain of short aliases, one per cfg line: a console line
    # is cut at about 512 characters, and `wait` inside an alias holds the
    # rest of the chain.
    aliases = ['alias gt_shot%d "screenshot; wait %d; gt_shot%d"' % (k, args.stride, k - 1)
               for k in range(args.frames, 0, -1)] + ['alias gt_shot0 "echo gi_temporal done"']
    view = ["mat_indirect_view %d" % (1 if args.view == "indirect" else 0)]
    line = "; ".join(["r_drawvgui 0"] + placement + view + list(args.console_command) +
                     ["r_indirect_producer %s" % producer, "wait %d" % args.warm,
                      "gt_shot%d" % args.frames])
    frames = (gi_runtime.PLACEMENT_FRAMES + args.warm +
              args.frames * (args.stride + SHOT_FRAMES) + 30)
    result = subprocess.run(
        [sys.executable, HERE / "portal_boot.py", "--runtime", runtime, "--build", args.build,
         "--content-root", map_build / "content", "--renderer", "native-vulkan", "--headless",
         "--map", manifest["map"], "--width", str(args.width), "--height", str(args.height),
         "--capture-wait", str(frames)] +
        [item for command in aliases + [line] for item in ("--console-command", command)] +
        ["--out", target],
        cwd=ROOT, capture_output=True, text=True)
    evidence = json.loads((target / "evidence.json").read_text()) \
        if (target / "evidence.json").is_file() else {}
    shots = [shot["path"] for shot in evidence.get("screenshots", [])]
    # The last screenshot is portal_boot's own, after the burst.
    return shots[:args.frames], result.returncode, evidence.get("status")


def capture(args):
    out = Path(args.out)
    results, passed = {}, True
    for producer in args.producer:
        target = out / producer
        shots, returncode, booted = capture_burst(args, producer, target)
        if len(shots) != args.frames:
            record = {"status": "fail", "failure": "captured %d of %d frames (boot %s, exit %d)"
                      % (len(shots), args.frames, booted, returncode)}
        else:
            record = score(luminance_frames(shots))
        record["producer"] = producer
        results[producer] = record
        passed &= record["status"] == "pass"
        print("%-10s %s  flicker_rms %s  step_max %s  range_p99 %s" % (
            producer, record["status"], *("%.5f" % record[key] if key in record else "-"
                                          for key in ("flicker_rms", "step_max",
                                                      "pixel_range_p99"))))
    summary = {"schema": SCHEMA, "fixture": args.fixture, "camera": args.camera,
               "view": args.view, "frames": args.frames, "stride": args.stride,
               "warm": args.warm, "build": args.build,
               "limits": {"flicker_rms": FLICKER_RMS_LIMIT, "step_max": STEP_LIMIT},
               "status": "pass" if passed else "fail", "producers": results}
    out.mkdir(parents=True, exist_ok=True)
    (out / "temporal.json").write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n")
    print("temporal stability %s: %s" % (args.fixture, summary["status"]))
    return 0 if passed else 1


def score_command(args):
    paths = sorted(Path(args.frames).rglob("*.tga"))
    record = score(luminance_frames(paths))
    text = json.dumps(record, indent=2, sort_keys=True) + "\n"
    if args.out:
        Path(args.out).write_text(text)
    print(text, end="")
    return 0 if record["status"] == "pass" else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    c = commands.add_parser("capture")
    c.add_argument("--fixture", default="door")
    c.add_argument("--camera")
    c.add_argument("--map-build", type=Path)
    c.add_argument("--producer", action="append", required=True,
                   help="r_indirect_producer value (repeatable; include baked as the control)")
    c.add_argument("--frames", type=int, default=24)
    c.add_argument("--stride", type=int, default=45,
                   help="frames between screenshots (the engine drops a screenshot requested "
                        "while it is still writing the last, about 0.7 s)")
    c.add_argument("--warm", type=int, default=300,
                   help="frames from selecting the producer to the first screenshot")
    c.add_argument("--view", choices=("shaded", "indirect"), default="shaded")
    c.add_argument("--console-command", action="append", default=[],
                   help="extra console commands before the producer is selected")
    c.add_argument("--width", type=int, default=1024)
    c.add_argument("--height", type=int, default=768)
    c.add_argument("--build", default="build")
    c.add_argument("--runtime")
    c.add_argument("--out", required=True)
    s = commands.add_parser("score")
    s.add_argument("--frames", required=True, help="a directory of the burst's TGA screenshots")
    s.add_argument("--out")
    args = parser.parse_args()
    return {"capture": capture, "score": score_command}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
