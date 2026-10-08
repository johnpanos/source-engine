#!/usr/bin/env python3
"""Lighting-term cost and quality sweep (RFC 0016 Source 2 lighting defaults).

    python3 tools/render/term_sweep.py run --runtime DIR --out DIR [--width W --height H]
        [--no-stage] [--build DIR] [--terms a,b,...] [--map NAME]
    python3 tools/render/term_sweep.py script --out FILE [--terms a,b,...]
    python3 tools/render/term_sweep.py analyze DIR [--screenshots DIR] [--console FILE]
    python3 tools/render/term_sweep.py selftest

Nothing is recompiled: each setting is a runtime control. A lighting term
switched off with cl_render_debug_term becomes a specialization constant of
the surface program (RFC 0014 D1), so the driver compiles that variant on
first use and the term's code is gone from it; the sweep then reads the
per-pass GPU timers (cl_render_debug_gpu_timers) and takes a screenshot.

`run` boots the portal2-fsr profile on the map through kiln (headless mutter, the launch sandbox, as
tools/quality/demo_frames.py does), stands at the player's spawn, pauses
the game, hides the HUD and view model, and for each view (the spawn at
four headings) and each setting (the baseline first and last, then every
term off by itself): sets it, waits for the compile and a settle, marks a
measurement window in the console (TERM_SWEEP BEGIN/END), screenshots the
frame, and restores the baseline. `script` writes that console script alone,
for a run on another machine (the Bazzite RTX 3070 box). `analyze` joins
the console's GPU timer reports to the windows and compares each
screenshot with its view's baseline:

- cost: the time the term saves in the world surfaces' shading
  (`world / pbr`) and in the whole core view (`core world view`), the
  baseline being the mean of the first and last baseline windows (their
  difference is reported as the run's drift);
- quality: per pixel CIELAB difference from the baseline (sRGB D65, CIE76
  delta E): mean, 95th percentile and the share of pixels over 2.3 (about a
  just-noticeable difference), and the mean luminance change.

The ranking table is written to DIR/term_sweep.json and DIR/term_sweep.md,
with difference images (DIR/diff/<view>-<term>.png, delta E 0..10 mapped to
black..white). Diagnostic: compiler and driver behaviour, not a gate.
"""

import argparse
import json
import math
import os
import re
import shutil
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/quality"))

# The terms cl_render_debug_term names, in the shader's order
# (render/shaderlib/debug_view.cpp kTermNames). area, ssr and ao are off by
# default (RFC 0016 Source 2 defaults) and volumetric needs a fog volume:
# they are swept too, and report no difference where they draw nothing.
TERMS = ["clustered", "sun", "area", "projected", "baked", "probes", "ibl", "ssr", "ao",
         "specular_occlusion", "emission", "volumetric", "shadow_visibility", "directional",
         "normal_map", "bounce", "soft_shadows"]
DEMO = ROOT / "quality/fixtures/demos/sp_a1_intro4_relit.dem"
VIEW_COUNT = 6


def demo_views(path=DEMO, count=VIEW_COUNT):
    """Evenly spaced camera positions from a recorded demo's packets
    (protocol 3: a 76-byte view record per packet): (x, y, z, pitch, yaw) of
    the player (its origin, as setpos takes it), the first and last tenth (the arrival and departure elevators)
    left out."""
    import struct
    data = Path(path).read_bytes()
    off, frames = 1072, []
    while off + 5 <= len(data):
        cmd = data[off]
        off += 5
        if cmd in (1, 2):
            _, x, y, z, pitch, yaw, _ = struct.unpack_from("<i3f3f", data, off)
            off += 76 + 8
            n, = struct.unpack_from("<i", data, off)
            off += 4 + n
            if cmd == 2:
                frames.append((x, y, z, pitch, yaw))
        elif cmd == 3:
            continue
        elif cmd in (4, 6, 8):
            n, = struct.unpack_from("<i", data, off)
            off += 4 + n
        elif cmd == 5:
            off += 4
            n, = struct.unpack_from("<i", data, off)
            off += 4 + n
        else:
            break
    if not frames:
        raise ValueError("%s has no view records" % path)
    lo, hi = len(frames) // 10, len(frames) - len(frames) // 10
    step = (hi - lo) / count
    return [frames[lo + int(step * (i + 0.5))] for i in range(count)]
MARK = "TERM_SWEEP"
SCRIPT = "term_sweep.cfg"


def settings(terms):
    """The sweep's settings: (name, cl_render_debug_term value)."""
    return [("baseline", "")] + [(term, term) for term in terms] + [("baseline-end", "")]


def console_script(terms, views=None, compile_frames=150, measure_frames=240,
                   settle_frames=300):
    """Aliases that each run one step and call the next (one exec'd line runs
    at once; `wait` counts frames)."""
    steps = ["sv_cheats 1", "cl_drawhud 0", "r_drawviewmodel 0", "crosshair 0",
             "cl_render_debug_gpu_timers 1", "cl_render_debug_stats 1", "wait %d" % settle_frames,
             "setpause"]
    for v, (x, y, z, pitch, yaw) in enumerate(views if views is not None else demo_views()):
        # The demo's view record holds the player's origin (the feet), as setpos.
        steps += ["unpause", "setpos %.1f %.1f %.1f" % (x, y, z),
                  "setang %.1f %.1f 0" % (pitch, yaw), "wait 60", "setpause", "wait 30"]
        for name, value in settings(terms):
            # A lone comma names no term (the parser skips empty names).
            steps += ["cl_render_debug_term %s" % ( value or "," ),
                      "wait %d" % compile_frames, "echo %s.BEGIN.%d.%s" % (MARK, v, name),
                      "wait %d" % measure_frames, "echo %s.SHOT.%d.%s" % (MARK, v, name),
                      "screenshot", "wait 20", "echo %s.END.%d.%s" % (MARK, v, name)]
    steps += ["cl_render_debug_term ,", "echo %s.DONE" % MARK, "quit"]
    lines = []
    for index, step in enumerate(steps):
        follow = "; ts_step%d" % (index + 1) if index + 1 < len(steps) else ""
        lines.append('alias ts_step%d "%s%s"' % (index, step, follow))
    lines.append("ts_step0")
    return lines


def read_windows(console_text):
    """Per (view, setting): the GPU timer reports printed inside its window."""
    windows, current, report = {}, None, None
    for line in console_text.splitlines():
        m = re.search(r"%s\.(BEGIN|SHOT|END)\.(\d+)\.(\S+)" % MARK, line)
        if m:
            key = (int(m.group(2)), m.group(3))
            if m.group(1) == "BEGIN":
                current = key
                windows[key] = {"reports": []}
            elif m.group(1) == "END":
                current = None
            continue
        m = re.match(r"cl_render_debug_stats: core GPU passes, mean of (\d+) frame", line)
        if m:
            report = {"frames": int(m.group(1))}
            if current is not None:
                windows[current]["reports"].append(report)
            continue
        if report is not None:
            m = re.match(r"\s*(.+?)\s{2,}([\d.]+) ms\s+x[\d.]+\s*$", line)
            if m:
                report.setdefault(m.group(1).strip(), float(m.group(2)))
            elif line.strip() and not line.startswith(" "):
                report = None
    return windows


def window_ms(window, label):
    """Frame-weighted mean of one pass over a window's reports."""
    total = weight = 0.0
    for report in window["reports"]:
        if label in report:
            total += report[label] * report["frames"]
            weight += report["frames"]
    return total / weight if weight else None


def srgb_to_lab(rgb):
    import numpy as np
    c = rgb.astype(np.float64) / 255.0
    linear = np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)
    m = np.array([[0.4124564, 0.3575761, 0.1804375], [0.2126729, 0.7151522, 0.0721750],
                  [0.0193339, 0.1191920, 0.9503041]])
    xyz = linear @ m.T / np.array([0.95047, 1.0, 1.08883])
    f = np.where(xyz > (6 / 29) ** 3, np.cbrt(xyz), xyz / (3 * (6 / 29) ** 2) + 4 / 29)
    return np.stack([116 * f[..., 1] - 16, 500 * (f[..., 0] - f[..., 1]),
                     200 * (f[..., 1] - f[..., 2])], axis=-1), linear @ m[1]


def compare(reference, test):
    """Delta E statistics and the mean luminance change of `test`."""
    import numpy as np
    lab_a, luminance_a = srgb_to_lab(reference)
    lab_b, luminance_b = srgb_to_lab(test)
    delta = np.sqrt(((lab_a - lab_b) ** 2).sum(axis=-1))
    mean_luminance = float(luminance_a.mean())
    return {"delta_e_mean": float(delta.mean()), "delta_e_p95": float(np.percentile(delta, 95)),
            "noticeable_share": float((delta > 2.3).mean()),
            "luminance_change": (float(luminance_b.mean()) - mean_luminance) /
                                max(mean_luminance, 1e-6)}, delta


def load_image(path):
    import numpy as np
    from PIL import Image
    return np.asarray(Image.open(path).convert("RGB"))


def analyze(out, screenshots=None, console=None):
    import numpy as np
    from PIL import Image
    console = Path(console) if console else out / "console.log"
    text = console.read_text(errors="replace")
    windows = read_windows(text)
    order = [(m.group(1), m.group(2)) for m in
             re.finditer(r"%s\.SHOT\.(\d+)\.(\S+)" % MARK, text)]
    shots_dir = Path(screenshots) if screenshots else out / "screenshots"
    shots = sorted(shots_dir.glob("*.tga")) if shots_dir.is_dir() else []
    # Screenshots are numbered in the order the script takes them.
    shot_of = {}
    if len(shots) >= len(order):
        for (view, name), path in zip(order, shots[-len(order):]):
            shot_of[(int(view), name)] = path
    views = sorted({view for view, _ in windows})
    names = []
    for _, name in windows:
        if name not in names:
            names.append(name)
    rows = {}
    drift = []
    (out / "diff").mkdir(parents=True, exist_ok=True)
    for view in views:
        base = [windows.get((view, b)) for b in ("baseline", "baseline-end")]
        values = [window_ms(b, "core world view") for b in base if b]
        if len(values) == 2 and None not in values:
            drift.append(values[1] - values[0])
    for name in names:
        if name.startswith("baseline"):
            continue
        row = {"term": name, "views": 0, "world_saved_ms": [], "view_saved_ms": [],
               "delta_e_mean": [], "delta_e_p95": [], "noticeable_share": [],
               "luminance_change": []}
        for view in views:
            base = [windows.get((view, b)) for b in ("baseline", "baseline-end")]
            test = windows.get((view, name))
            if not test or not all(base):
                continue
            for label, key in (("world / pbr", "world_saved_ms"),
                               ("core world view", "view_saved_ms")):
                values = [window_ms(b, label) for b in base]
                measured = window_ms(test, label)
                if None not in values and measured is not None:
                    row[key].append(sum(values) / 2 - measured)
            if (view, "baseline") in shot_of and (view, name) in shot_of:
                stats, delta = compare(load_image(shot_of[(view, "baseline")]),
                                       load_image(shot_of[(view, name)]))
                for key, value in stats.items():
                    row[key].append(value)
                Image.fromarray((np.clip(delta / 10.0, 0, 1) * 255).astype(np.uint8)).save(
                    out / "diff" / ("%d-%s.png" % (view, name)))
            row["views"] += 1
        mean = lambda values: sum(values) / len(values) if values else None
        rows[name] = {key: (mean(value) if isinstance(value, list) else value)
                      for key, value in row.items()}
    ranked = sorted(rows.values(), key=lambda r: -(r["view_saved_ms"] or 0))
    report = {"schema": "term-sweep/v1", "views": len(views),
              "baseline_drift_ms": drift, "terms": ranked,
              "quality_rank": [r["term"] for r in sorted(
                  rows.values(), key=lambda r: -(r["delta_e_mean"] or 0))]}
    (out / "term_sweep.json").write_text(json.dumps(report, indent=2) + "\n")
    lines = ["| term | core view saved (ms) | world shading saved (ms) | mean dE | p95 dE | "
             "pixels over 2.3 | luminance change |", "|---|---|---|---|---|---|---|"]
    fmt = lambda v, f: f % v if v is not None else "-"
    for r in ranked:
        lines.append("| %s | %s | %s | %s | %s | %s | %s |" % (
            r["term"], fmt(r["view_saved_ms"], "%.2f"), fmt(r["world_saved_ms"], "%.2f"),
            fmt(r["delta_e_mean"], "%.2f"), fmt(r["delta_e_p95"], "%.1f"),
            fmt(None if r["noticeable_share"] is None else r["noticeable_share"] * 100, "%.1f%%"),
            fmt(None if r["luminance_change"] is None else r["luminance_change"] * 100,
                "%+.1f%%")))
    lines.append("")
    lines.append("Baseline drift between the first and last baseline windows (core view, "
                 "ms): %s" % ", ".join("%+.2f" % d for d in drift))
    (out / "term_sweep.md").write_text("\n".join(lines) + "\n")
    return report


def run(args):
    import demo_frames
    args.out.mkdir(parents=True, exist_ok=True)
    stage_args = argparse.Namespace(runtime=args.runtime, no_stage=args.no_stage, out=args.out,
                                    kiln_profile=args.kiln_profile, flavor=args.flavor,
                                    extra_arg=[], profile=False)
    workload = demo_frames.load_workload(demo_frames.DEFAULT_WORKLOAD)
    workload["demo_name"] = workload["map"] + ".dem"
    runtime = demo_frames.stage(stage_args, workload)
    game = runtime / "portal2"
    (game / "cfg" / SCRIPT).write_text("\n".join(console_script(args.terms)) + "\n")
    shots = game / "screenshots"
    if shots.is_dir():
        shutil.rmtree(shots)
    arguments = ["-multirun", "-novid", "-condebug", "-windowed",
               "-noborder", "-w", str(args.width), "-h", str(args.height), "-vkgputimers",
               "+exec", Path(demo_frames.QUERY_CFG).stem, "+ai_norebuildgraph", "1", "+map", args.map,
               "+exec", Path(SCRIPT).stem]
    workload = dict(workload, timeout_seconds=args.timeout)
    run_args = argparse.Namespace(out=args.out, kiln_profile=args.kiln_profile,
                                  flavor=args.flavor, width=args.width, height=args.height,
                                  renderdoc_frames=None)
    process, sandbox = demo_frames.launch(run_args, workload, runtime, arguments)
    deadline = time.monotonic() + args.timeout
    try:
        while process.poll() is None and time.monotonic() < deadline:
            time.sleep(0.5)
    finally:
        if process.poll() is None:
            process.stop()
        sandbox.finish()
    if (game / "console.log").is_file():
        shutil.copyfile(game / "console.log", args.out / "console.log")
    if shots.is_dir():
        shutil.copytree(shots, args.out / "screenshots", dirs_exist_ok=True)
    report = analyze(args.out)
    print((args.out / "term_sweep.md").read_text())
    return 0 if report["terms"] else 1


def selftest():
    import numpy as np
    failures = 0
    text = "\n".join([
        "TERM_SWEEP.BEGIN.0.baseline",
        "cl_render_debug_stats: core GPU passes, mean of 10 frame(s):",
        "  core world view                   10.000 ms  x2.0",
        "        world / pbr                  6.000 ms  x1.0",
        "TERM_SWEEP.END.0.baseline",
        "TERM_SWEEP.BEGIN.0.probes",
        "cl_render_debug_stats: core GPU passes, mean of 10 frame(s):",
        "  core world view                    9.000 ms  x2.0",
        "        world / pbr                  5.000 ms  x1.0",
        "TERM_SWEEP.END.0.probes"])
    windows = read_windows(text)
    failures += window_ms(windows[(0, "baseline")], "world / pbr") != 6.0
    failures += window_ms(windows[(0, "probes")], "core world view") != 9.0
    image = np.full((8, 8, 3), 128, np.uint8)
    same, _ = compare(image, image)
    failures += same["delta_e_mean"] != 0.0
    darker, _ = compare(image, image // 2)
    failures += not (darker["delta_e_mean"] > 10 and darker["luminance_change"] < -0.5)
    lines = console_script(["probes"])
    failures += not any("cl_render_debug_term probes" in line for line in lines)
    failures += lines[-1] != "ts_step0"
    print("CONFORMANCE %d %d" % (5, failures))
    return 1 if failures else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    terms = lambda text: [t for t in text.split(",") if t]
    play = commands.add_parser("run")
    play.add_argument("--runtime", type=Path, required=True)
    play.add_argument("--out", type=Path, required=True)
    play.add_argument("--kiln-profile", default="portal2-fsr", help="kiln profile")
    play.add_argument("--flavor", default="dev", help="the profile's build flavor")
    play.add_argument("--no-stage", action="store_true")
    play.add_argument("--width", type=int, default=1920)
    play.add_argument("--height", type=int, default=1080)
    play.add_argument("--map", default="sp_a1_intro4_relit")
    play.add_argument("--terms", type=terms, default=TERMS)
    play.add_argument("--timeout", type=int, default=3600)
    script = commands.add_parser("script")
    script.add_argument("--out", type=Path, required=True)
    script.add_argument("--terms", type=terms, default=TERMS)
    script.add_argument("--views", type=int, default=VIEW_COUNT)
    script.add_argument("--compile-frames", type=int, default=150)
    script.add_argument("--measure-frames", type=int, default=240)
    again = commands.add_parser("analyze")
    again.add_argument("out", type=Path)
    again.add_argument("--screenshots", type=Path)
    again.add_argument("--console", type=Path)
    commands.add_parser("selftest")
    args = parser.parse_args()
    if args.command == "run":
        return run(args)
    if args.command == "script":
        args.out.write_text("\n".join(console_script(
            args.terms, demo_views(count=args.views), args.compile_frames,
            args.measure_frames)) + "\n")
        return 0
    if args.command == "analyze":
        analyze(args.out, args.screenshots, args.console)
        print((args.out / "term_sweep.md").read_text())
        return 0
    return selftest()


if __name__ == "__main__":
    sys.exit(main())
