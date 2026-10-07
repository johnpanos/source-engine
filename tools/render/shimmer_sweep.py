#!/usr/bin/env python3
"""Specular shimmer under camera motion (RFC 0012 A2's shimmer and reference
oracles, in the game).

    python3 tools/render/shimmer_sweep.py run --runtime DIR --out DIR
        [--build DIR] [--map NAME] [--views N] [--steps N] [--step UNITS]
    python3 tools/render/shimmer_sweep.py analyze DIR
    python3 tools/render/shimmer_sweep.py selftest

`run` boots ./play_p2 twice on the map (headless mutter, as
term_sweep.py does), noclips to evenly spaced views from the intro4 demo and
strafes the camera in small steps, pausing for a screenshot at each:

- `pair`: 1920x1080, at each pose r_core_specular_aa 1 then 0 in the same
  paused frame state (separate boots differ by a few percent in scene
  state, which swamps the filter);
- `ref`: 3840x2160, r_core_specular_aa 0, box-downsampled 2x2 in linear
  light: the supersampled reference of the same poses.

`analyze` works in linear luminance. Per frame, the error is the 1080p frame
minus the reference at the same pose. Shimmer is the per-pixel temporal
standard deviation of that error over a view's steps (the camera motion
itself cancels; what remains is aliasing that flickers), and the reference
error is its mean absolute value. Both are reported over the whole frame and
over the filter's footprint (pixels where `on` and `off` differ by more than
one 8-bit level in some frame). Declared before the first measurement:

- shimmer.footprint: on <= 0.9 x off;
- reference.footprint: on <= off;
- energy: on's mean luminance within 1 % of off's.

Animated content (screens, particles) is paused with the game; a residual
floor is reported as the reference run's own flicker.
"""

import argparse
import json
import math
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "render"))

MARK = "SHIMMER_SWEEP"
SCRIPT = "shimmer_sweep.cfg"
# pair: both settings at each paused pose in one session (on, then off), so
# scene state that differs between boots cannot enter the comparison.
PASSES = {"pair": (1920, 1080, ("1", "0")), "ref": (3840, 2160, ("0",))}
LIMITS = {"shimmer_ratio": 0.9, "reference_ratio": 1.0, "energy": 0.01}


def console_script(views, settings, steps, step, first_wait=300, settle=12):
    lines = ["sv_cheats 1", "cl_drawhud 0", "r_drawviewmodel 0", "crosshair 0",
             # A fixed exposure: adaptation would shift whole frames between
             # passes and swamp the specular signal.
             "mat_dynamic_tonemapping 0", "mat_force_tonemap_scale 1",
             "noclip", "wait %d" % first_wait, "setpause"]
    for v, (x, y, z, pitch, yaw) in enumerate(views):
        # Strafe: perpendicular to the view's yaw.
        sx, sy = -math.sin(math.radians(yaw)), math.cos(math.radians(yaw))
        for k in range(steps):
            lines += ["unpause", "setpos %.3f %.3f %.3f" % (x + sx * step * k, y + sy * step * k, z),
                      "setang %.1f %.1f 0" % (pitch, yaw), "wait 2", "setpause"]
            for setting in settings:
                lines += ["r_core_specular_aa %s" % setting,
                          "wait %d" % (120 if k == 0 else settle),
                          "echo %s.SHOT.%d.%d.%s" % (MARK, v, k, setting), "screenshot",
                          "wait 10"]
    lines += ["echo %s.DONE" % MARK, "quit"]
    out = []
    for index, s in enumerate(lines):
        follow = "; ss_step%d" % (index + 1) if index + 1 < len(lines) else ""
        out.append('alias ss_step%d "%s%s"' % (index, s, follow))
    out.append("ss_step0")
    return out


def run_pass(args, name, views):
    import demo_frames
    import frame_floor
    width, height, settings = PASSES[name]
    out = args.out / name
    out.mkdir(parents=True, exist_ok=True)
    if args.steam_root is None:
        args.steam_root = frame_floor.DEFAULT_STEAM_ROOT
    stage_args = argparse.Namespace(runtime=args.runtime, no_stage=args.no_stage, out=out,
                                    steam_root=args.steam_root, build=args.build, extra_arg=[],
                                    profile=False)
    workload = demo_frames.load_workload(demo_frames.DEFAULT_WORKLOAD)
    workload["demo_name"] = workload["map"] + ".dem"
    runtime = demo_frames.stage(stage_args, workload)
    args.no_stage = True  # later passes reuse the staged runtime
    game = runtime / "portal2"
    (game / "cfg" / SCRIPT).write_text(
        "\n".join(console_script(views, settings, args.steps, args.step)) + "\n")
    shots = game / "screenshots"
    if shots.is_dir():
        shutil.rmtree(shots)
    command = [str(ROOT / "play_p2"), "-multirun", "-novid", "-condebug", "-windowed",
               "-noborder", "-w", str(width), "-h", str(height),
               "+exec", Path(demo_frames.QUERY_CFG).stem, "+ai_norebuildgraph", "1",
               "+map", args.map, "+exec", Path(SCRIPT).stem]
    workload = dict(workload, timeout_seconds=args.timeout)
    run_args = argparse.Namespace(out=out, build=args.build, width=width, height=height)
    process, stream, sandbox = demo_frames.launch(run_args, workload, runtime, command)
    try:
        process.wait(timeout=args.timeout)
    except subprocess.TimeoutExpired:
        pass
    finally:
        frame_floor.stop(process)
        stream.close()
        sandbox.finish()
    if (game / "console.log").is_file():
        shutil.copyfile(game / "console.log", out / "console.log")
    if shots.is_dir():
        shutil.copytree(shots, out / "screenshots", dirs_exist_ok=True)


def run(args):
    import term_sweep
    views = term_sweep.demo_views(count=args.views)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "views.json").write_text(json.dumps(
        {"views": views, "steps": args.steps, "step": args.step, "map": args.map}))
    for name in PASSES:
        run_pass(args, name, views)
    report = analyze(args.out)
    print(json.dumps(report, indent=1))
    return 0 if report.get("verdict") == "pass" else 1


def to_linear(rgb):
    import numpy as np
    c = rgb.astype(np.float64) / 255.0
    return np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)


def luminance(linear):
    return linear[..., 0] * 0.2126 + linear[..., 1] * 0.7152 + linear[..., 2] * 0.0722


def load_pass(directory, expected):
    import numpy as np
    from PIL import Image
    shots = sorted((directory / "screenshots").glob("*.tga"))
    if len(shots) != expected:
        raise ValueError("%s: %d screenshots, expected %d" % (directory, len(shots), expected))
    frames = []
    for path in shots:
        lin = to_linear(np.asarray(Image.open(path).convert("RGB")))
        h, w = lin.shape[:2]
        if w > 1920:
            lin = lin.reshape(h // 2, 2, w // 2, 2, 3).mean(axis=(1, 3))
        frames.append(lin)
    return frames


def measure(on, off, ref, views, steps):
    """The oracle's numbers from linear frames, view-major then step."""
    import numpy as np
    footprint_on, footprint_off, whole_on, whole_off = [], [], [], []
    l1 = {"on": [], "off": []}
    ref_flicker = []
    energy = {"on": 0.0, "off": 0.0}
    for v in range(views):
        sl = slice(v * steps, (v + 1) * steps)
        Lon = np.stack([luminance(f) for f in on[sl]])
        Loff = np.stack([luminance(f) for f in off[sl]])
        Lref = np.stack([luminance(f) for f in ref[sl]])
        energy["on"] += Lon.mean()
        energy["off"] += Loff.mean()
        mask = (np.abs(np.stack(on[sl]) - np.stack(off[sl])).max(axis=(0, 3)) > 1.0 / 255.0 / 12.92)
        e_on, e_off = Lon - Lref, Loff - Lref
        s_on, s_off = e_on.std(axis=0), e_off.std(axis=0)
        whole_on.append(s_on.mean())
        whole_off.append(s_off.mean())
        if mask.any():
            footprint_on.append(s_on[mask].mean())
            footprint_off.append(s_off[mask].mean())
            l1["on"].append(np.abs(e_on)[:, mask].mean())
            l1["off"].append(np.abs(e_off)[:, mask].mean())
        ref_flicker.append(np.abs(np.diff(Lref, axis=0)).mean())
    mean = lambda xs: float(sum(xs) / len(xs)) if xs else float("nan")
    result = {
        "shimmer": {"footprint_on": mean(footprint_on), "footprint_off": mean(footprint_off),
                    "whole_on": mean(whole_on), "whole_off": mean(whole_off)},
        "reference_l1": {"footprint_on": mean(l1["on"]), "footprint_off": mean(l1["off"])},
        "energy_change": float(energy["on"] / energy["off"] - 1.0) if energy["off"] else 0.0,
        "reference_step_change": mean(ref_flicker),
        "per_view_shimmer_ratio": [a / b if b else float("nan")
                                   for a, b in zip(footprint_on, footprint_off)],
    }
    s, r = result["shimmer"], result["reference_l1"]
    result["shimmer_ratio"] = s["footprint_on"] / s["footprint_off"] if s["footprint_off"] else float("nan")
    result["reference_ratio"] = r["footprint_on"] / r["footprint_off"] if r["footprint_off"] else float("nan")
    checks = {
        "shimmer.footprint": result["shimmer_ratio"] <= LIMITS["shimmer_ratio"],
        "reference.footprint": result["reference_ratio"] <= LIMITS["reference_ratio"],
        "energy": abs(result["energy_change"]) <= LIMITS["energy"],
    }
    result["checks"] = checks
    result["verdict"] = "pass" if all(checks.values()) else "fail"
    return result


def analyze(out):
    meta = json.loads((out / "views.json").read_text())
    views, steps = len(meta["views"]), meta["steps"]
    pair = load_pass(out / "pair", 2 * views * steps)
    ref = load_pass(out / "ref", views * steps)
    report = dict(measure(pair[0::2], pair[1::2], ref, views, steps),
                  schema="shimmer-sweep/v1", limits=LIMITS, **meta)
    (out / "shimmer_sweep.json").write_text(json.dumps(report, indent=1))
    return report


def selftest():
    """The oracle on synthetic sequences: a flickering highlight against a
    stable reference must fail on, pass when on is stable, and a brightened
    on must fail energy."""
    import numpy as np
    rng = np.random.default_rng(7)
    views, steps = 1, 8
    base = np.full((16, 16, 3), 0.2)
    # The reference holds the sparkle's true filtered value, its mean.
    ref = [base.copy() for _ in range(steps)]
    for f in ref:
        f[4:8, 4:8] += 0.25
    off = []
    for k in range(steps):
        f = base.copy()
        f[4:8, 4:8] += rng.uniform(0, 0.5, (4, 4, 1))  # flickering sparkle
        off.append(f)
    calm = [base.copy() for _ in range(steps)]
    for f in calm:
        f[4:8, 4:8] += 0.25
    failures = 0
    good = measure(calm, off, ref, views, steps)
    failures += not good["checks"]["shimmer.footprint"]
    failures += not good["checks"]["reference.footprint"]
    same = measure(off, [f + 1e-3 for f in off], ref, views, steps)
    failures += same["checks"]["shimmer.footprint"]  # no improvement must fail
    bright = measure([f * 1.5 for f in calm], off, ref, views, steps)
    failures += bright["checks"]["energy"]
    print("CONFORMANCE %d %d" % (4, failures))
    return 1 if failures else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    play = sub.add_parser("run")
    play.add_argument("--runtime", type=Path, required=True)
    play.add_argument("--out", type=Path, required=True)
    play.add_argument("--build", type=Path, default=ROOT / "build-p2-fsr")
    play.add_argument("--steam-root", type=Path, default=None)
    play.add_argument("--no-stage", action="store_true")
    play.add_argument("--map", default="sp_a1_intro4_relit")
    play.add_argument("--views", type=int, default=4)
    play.add_argument("--steps", type=int, default=12)
    play.add_argument("--step", type=float, default=0.25)
    play.add_argument("--timeout", type=int, default=1800)
    again = sub.add_parser("analyze")
    again.add_argument("out", type=Path)
    sub.add_parser("selftest")
    args = parser.parse_args()
    if args.command == "run":
        return run(args)
    if args.command == "analyze":
        print(json.dumps(analyze(args.out), indent=1))
        return 0
    return selftest()


if __name__ == "__main__":
    sys.exit(main())
