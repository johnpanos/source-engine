#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Per-frame timing of a recorded Portal 2 demo.

A workload (quality/workloads/portal2-*-demo-v*/workload.json) names a demo
fixture, its map, the build tree and the settings it was recorded with. This
tool stages a private runtime, copies the demo into it, and plays it back in
real time through ./play_p2 in a private headless mutter (no window on the
desktop), with the native Vulkan backend writing one line per presented frame
(-vkframestats). The game quits when playback ends.

The demo carries no frame marks, so the playback window is found in the
stream: it starts at the first run of settled frames after level load (the
workload's playback_window rule) and excludes the final (quit) frame.

Reported per run: frame-interval percentiles, 1% and 0.1% lows, hitch counts
over the workload's thresholds, the share of frames bound by GPU time, the
strict render_profile summary (CPU costs, GPU render time and, with
--profile, backend GPU passes and the render core's per-pass reports), and the
worst frames with their heaviest named costs. The effective settings are
queried in-game and must equal the workload's; a run whose settings drifted is
not comparable and fails.

Diagnostic only: no budget row, no acceptance claim. Interleave runs when
comparing builds; the host and its GPU are shared.

  python3 tools/quality/demo_frames.py run --runtime /tmp/claude-1000/i4rt --out /tmp/claude-1000/i4/a1
  python3 tools/quality/demo_frames.py analyze /tmp/claude-1000/i4/a1

Exit status: 0 complete, 1 incomplete (playback, window, settings or
records), 2 usage or staging error.
"""

import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

import conformance
import frame_floor
import frame_pacing
import launch_sandbox
import portal2_scenarios
import private_session
import render_profile

ROOT = Path(conformance.repo_root())
DEFAULT_WORKLOAD = ROOT / "quality/workloads/portal2-intro4-demo-v1/workload.json"
EVIDENCE_SCHEMA = "demo-frames-evidence/v1"
QUERY_CFG = "demo_frames.cfg"
WORST_FRAMES = 15
GPU_BOUND_SHARE = 0.9


class DemoError(Exception):
    pass


def load_workload(path):
    workload = json.loads(Path(path).read_text())
    if workload.get("schema") != "portal2-demo/v1":
        raise DemoError("%s is not a portal2-demo/v1 workload" % path)
    demo = (Path(path).parent / workload["demo"]).resolve()
    if hashlib.sha256(demo.read_bytes()).hexdigest() != workload["demo_sha256"]:
        raise DemoError("demo %s does not match the workload's sha256" % demo)
    workload["demo_path"] = demo
    return workload


# --- Analysis ------------------------------------------------------------------

def playback_window(rows, rule):
    """The settled playback frames: level load before, the quit frame after."""
    settle, limit_us = rule["settle_frames"], rule["settle_interval_ms"] * 1000
    # A load frame can carry a wrapped CPU counter; it is not playback.
    settled = [0 < row["interval"] < limit_us and row.get("records", 0) > rule["min_records"]
               and frame_floor.valid_duration_us(row.get("cpu"))
               for row in rows]
    for start in range(1, len(rows) - settle):
        if all(settled[start:start + settle]):
            selected = rows[start:-1]
            if len(selected) < settle:
                break
            return selected
    raise DemoError("no run of %d settled frames: the demo did not reach playback" % settle)


def queried_settings(console):
    return dict(re.findall(r'(?m)^"([A-Za-z0-9_]+)" = "([^"]*)"', console))


def setting_failures(console, expected):
    found = queried_settings(console)
    return ["setting %s is %s, the workload's recording used %s"
            % (name, found.get(name, "unreported"), value)
            for name, value in expected.items() if found.get(name) != value]


def frame_stats(rows, thresholds):
    intervals = [row["interval"] / 1000 for row in rows]
    gpu = [row for row in rows if "gpu" in row]
    return {
        "frames": len(rows),
        "seconds": round(sum(intervals) / 1000, 2),
        "average_fps": round(1000 * len(intervals) / sum(intervals), 1),
        "median_ms": round(frame_pacing.percentile(intervals, .5), 3),
        "p90_ms": round(frame_pacing.percentile(intervals, .9), 3),
        "p99_ms": round(frame_pacing.percentile(intervals, .99), 3),
        "p999_ms": round(frame_pacing.percentile(intervals, .999), 3),
        "max_ms": round(max(intervals), 3),
        "low_1pct_fps": round(frame_floor.low_fps(intervals, .01), 1),
        "low_01pct_fps": round(frame_floor.low_fps(intervals, .001), 1),
        "hitches": {"over_%g_ms" % limit: sum(value > limit for value in intervals)
                    for limit in thresholds},
        # GPU time of the frame's own work against its interval: near the
        # interval means the GPU, not a CPU thread, set the frame rate.
        "gpu_bound_frames": sum(row["gpu"][1] >= GPU_BOUND_SHARE * row["interval"] for row in gpu),
        "gpu_result_frames": len(gpu),
    }


def worst_frames(rows, count=WORST_FRAMES):
    start = rows[0]["t"]
    worst = sorted(rows, key=lambda row: -row["interval"])[:count]
    return [dict(frame_floor.frame_brief(row, None),
                 seconds_into_playback=round((row["t"] - start) / 1e6, 2),
                 cpu_ms=round(row["cpu"] / 1000, 3)) for row in worst]


def core_pass_means(reports):
    """Frame-weighted per-frame means of each core report row (inclusive)."""
    totals, frames = {}, sum(report["frames"] for report in reports)
    for report in reports:
        path = []
        for item in report["passes"]:
            if item["depth"] > len(path):
                # The engine prints some sections deeper than the row above
                # them (a family split of an earlier scope): no parent is known.
                path = ["(unplaced)"] * item["depth"] + [item["name"]]
            else:
                path = path[:item["depth"]] + [item["name"]]
            entry = totals.setdefault(" > ".join(path), {"ms": 0.0, "per_frame": 0.0, "kind": item["kind"]})
            entry["ms"] += item["mean_ms"] * report["frames"]
            entry["per_frame"] += item["per_frame"] * report["frames"]
    means = [{"pass": name, "mean_ms": round(entry["ms"] / frames, 3),
              "per_frame": round(entry["per_frame"] / frames, 2), "kind": entry["kind"]}
             for name, entry in totals.items()]
    return sorted(means, key=lambda item: -item["mean_ms"])


def analyze(out, workload, extent=None):
    """Judges a run directory's frames.jsonl and console.log."""
    width, height = extent or (workload["width"], workload["height"])
    failures = []
    header, rows = render_profile.read_frames(out / "frames.jsonl")
    console_path = out / "console.log"
    console = console_path.read_text(errors="replace") if console_path.is_file() else ""
    if not re.search(r"Playing demo from \S*%s" % re.escape(Path(workload["demo_name"]).name), console):
        failures.append("the console does not show the demo playing")
    failures += setting_failures(console, workload["settings"])
    selected = playback_window(rows, workload["playback_window"])
    extents = {tuple(row["extent"]) for row in selected if "extent" in row}
    if extents != {(width, height)}:
        failures.append("drawable extents %s, expected %dx%d" % (sorted(extents), width, height))
    reports = render_profile.core_reports(console)
    return {"failures": failures, "device": header,
            "window": {"first_frame": selected[0]["f"], "last_frame": selected[-1]["f"],
                       "load_frames": selected[0]["f"] - rows[0]["f"]},
            "frames": frame_stats(selected, workload["hitch_thresholds_ms"]),
            "worst_frames": worst_frames(selected),
            "profile": render_profile.summarize(selected),
            "core_passes": core_pass_means(reports) if reports else [],
            "settings": queried_settings(console)}


# --- A run ---------------------------------------------------------------------

def stage(args, workload):
    runtime = args.runtime.resolve()
    if runtime in ((ROOT / "run/runtime-p2").resolve(), (ROOT / "run/runtime-p2-fsr").resolve()):
        raise DemoError("refusing a ./play_p2 runtime; use a private --runtime")
    if not args.no_stage:
        log = args.out / "stage.log"
        with log.open("wb") as stream:
            code = subprocess.run([sys.executable, str(ROOT / "tools/quality/stage_portal2_runtime.py"),
                                   "--steam-root", str(args.steam_root), "--runtime", str(runtime),
                                   "--build", str(args.build.resolve()), "--mount-published"],
                                  stdout=stream, stderr=subprocess.STDOUT).returncode
        if code != 0:
            raise DemoError("staging failed (%s)" % log)
    if not (runtime / "hl2_launcher").is_file():
        raise DemoError("%s has no staged launcher" % runtime)
    game = runtime / "portal2"
    shutil.copyfile(workload["demo_path"], game / workload["demo_name"])
    queries = ["%s" % name for name in workload["settings"]]
    # Console variables set with --extra-arg +name value are queried too, so
    # the console log records the value each run actually used.
    queries += [argument[1:] for argument in args.extra_arg
                if argument.startswith("+") and argument[1:] not in ("exec", "map") and
                argument[1:].replace("_", "").isalnum()]
    timers = ["cl_render_debug_gpu_timers 1", "cl_render_debug_stats 1"] if args.profile else []
    # Startup cvars live here, not on the command line: ./play_p2 adds its own
    # switches, and the engine refuses command lines over 512 characters.
    startup = ["volume 0", "mat_vsync 0", "engine_no_focus_sleep 0", "demo_quitafterplayback 1"]
    (game / "cfg" / QUERY_CFG).write_text("\n".join(startup + timers + queries) + "\n")
    console = game / "console.log"
    if console.exists():
        console.unlink()
    return runtime


def renderdoc_prefix(args):
    """renderdoccmd around ./play_p2. It follows children (the launcher
    re-execs itself) and runs the game on mutter's Xwayland, because RenderDoc
    hides VK_KHR_wayland_surface."""
    if not args.renderdoc_frames:
        return []
    renderdoccmd = shutil.which("renderdoccmd")
    if not renderdoccmd:
        raise DemoError("--renderdoc-frames needs renderdoccmd on PATH")
    (args.out / "renderdoc").mkdir(exist_ok=True)
    return ["env", "SDL_VIDEODRIVER=x11", renderdoccmd, "capture", "--opt-hook-children", "-w",
            "-c", str(args.out / "renderdoc" / "frame")]


def game_command(args, workload, stats_path, runtime):
    frames = (["-vkrenderdocframes", ",".join(str(frame) for frame in args.renderdoc_frames)]
              if args.renderdoc_frames else [])
    return [*renderdoc_prefix(args), str(ROOT / "play_p2"),
            "-multirun", "-novid", "-condebug", "-windowed", "-noborder",
            "-w", str(args.width), "-h", str(args.height),
            # The game starts in the runtime; a relative path keeps long output
            # directories under the engine's 512-character command line.
            "-vkframestats", os.path.relpath(stats_path, runtime),
            *(["-vkgputimers"] if args.profile else []),
            *frames,
            *args.extra_arg,
            "+exec", Path(QUERY_CFG).stem,
            "+playdemo", Path(workload["demo_name"]).stem]


def launch(args, workload, runtime, command):
    """./play_p2 in a private headless mutter, in the launch sandbox."""
    sandbox = launch_sandbox.Sandbox(args.out / "sandbox", write_paths=[runtime])
    environment = sandbox.environment(os.environ)
    environment.update({"P2_RUNTIME": str(runtime), "P2_NO_BUILD": "1",
                        "P2_BUILD_DIR": str(args.build.resolve()),
                        "P2_FSR": "1" if workload.get("fsr") else "0",
                        "SDL_VIDEODRIVER": "wayland"})
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    tools = args.out / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    environment["PATH"] = str(tools) + os.pathsep + environment.get("PATH", "")
    command = private_session.dbus_run_session(args.out / "dbus") + [
        "mutter", "--headless", "--virtual-monitor",
        "%dx%d@%g" % (args.width, args.height, workload.get("refresh_hz", 60)),
        "--wayland-display", "demo-%d" % os.getpid(), "--"] + command
    stream = (args.out / "stdout.log").open("wb")
    process = subprocess.Popen(command, cwd=ROOT, env=environment, stdout=stream,
                               stderr=subprocess.STDOUT, start_new_session=True)
    return process, stream, sandbox


def run(args, workload):
    args.out.mkdir(parents=True, exist_ok=True)
    runtime = stage(args, workload)
    stats = args.out / "frames.jsonl"
    command = game_command(args, workload, stats, runtime)
    if len(" ".join(command[command.index(str(ROOT / "play_p2")) + 1:])) >= 512:
        raise DemoError("command line exceeds the engine's 512 characters")
    context = {"host": frame_floor.host_context(), "graphics": frame_floor.graphics_context()}
    started = time.monotonic()
    process, stream, sandbox = launch(args, workload, runtime, command)
    timed_out = False
    try:
        process.wait(timeout=workload["timeout_seconds"])
    except subprocess.TimeoutExpired:
        timed_out = True
    finally:
        frame_floor.stop(process)
        stream.close()
    seconds = round(time.monotonic() - started, 1)
    console = runtime / "portal2/console.log"
    if console.is_file():
        shutil.copyfile(console, args.out / "console.log")
    evidence = {"schema": EVIDENCE_SCHEMA, "workload": str(args.workload),
                "demo_sha256": workload["demo_sha256"],
                "revision": subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT,
                                           capture_output=True, text=True).stdout.strip(),
                "dirty": bool(subprocess.run(["git", "status", "--porcelain", "-uno"], cwd=ROOT,
                                             capture_output=True, text=True).stdout.strip()),
                "build": str(args.build), "runtime": str(runtime), "command": command,
                "profile": args.profile, "extent": [args.width, args.height],
                "started": datetime.datetime.now().isoformat(timespec="seconds"),
                "wall_seconds": seconds, "exit_status": process.returncode,
                "sandbox": sandbox.finish(), **context}
    failures = []
    output = (args.out / "stdout.log").read_text(errors="replace")
    if "command line too long" in output:
        failures.append("the engine refused the command line (over 512 characters)")
    if timed_out:
        failures.append("playback did not finish within %d s" % workload["timeout_seconds"])
    elif process.returncode != 0:
        failures.append("the game exited with status %s" % process.returncode)
    try:
        result = analyze(args.out, workload, (args.width, args.height))
        failures += result.pop("failures")
        evidence.update(result)
    except (DemoError, render_profile.ProfileError, OSError) as error:
        failures.append(str(error))
    if args.renderdoc_frames:
        failures += renderdoc_failures(args, evidence)
    evidence["status"] = "incomplete" if failures else "complete"
    evidence["failures"] = failures
    (args.out / "evidence.json").write_text(json.dumps(evidence, indent=1) + "\n")
    return evidence


def renderdoc_failures(args, evidence):
    """Every requested frame was captured: tagged in the stream, one .rdc each."""
    captures = sorted(str(path) for path in (args.out / "renderdoc").glob("*.rdc"))
    tagged = []
    stats = args.out / "frames.jsonl"
    if stats.is_file():
        for line in stats.read_text(errors="replace").splitlines():
            try:
                row = json.loads(line)
            except json.JSONDecodeError:
                continue  # a killed run's last line; read_frames reports it
            if row.get("renderdoc_capture") is True:
                tagged.append(row["f"])
    evidence["renderdoc"] = {"requested_frames": args.renderdoc_frames, "captured_frames": tagged,
                             "captures": captures,
                             "note": "RenderDoc changes timing; this run is a capture, not a measurement"}
    failures = []
    missing = sorted(set(args.renderdoc_frames) - set(tagged))
    if missing:
        failures.append("RenderDoc did not capture frames %s" % missing)
    if len(captures) != len(tagged):
        failures.append("%d frames tagged but %d .rdc files written" % (len(tagged), len(captures)))
    return failures


def print_result(evidence):
    frames = evidence.get("frames")
    if frames:
        window = evidence["window"]
        print("playback frames %d-%d (%d load frames before): %d frames, %.1f s, %.1f fps"
              % (window["first_frame"], window["last_frame"], window["load_frames"],
                 frames["frames"], frames["seconds"], frames["average_fps"]))
        print("interval ms: p50 %.2f  p90 %.2f  p99 %.2f  p99.9 %.2f  max %.1f"
              % (frames["median_ms"], frames["p90_ms"], frames["p99_ms"], frames["p999_ms"], frames["max_ms"]))
        print("lows: 1%% %.1f fps  0.1%% %.1f fps" % (frames["low_1pct_fps"], frames["low_01pct_fps"]))
        print("hitches: " + "  ".join("%s %d" % item for item in frames["hitches"].items()))
        print("GPU-bound frames: %d of %d with GPU results"
              % (frames["gpu_bound_frames"], frames["gpu_result_frames"]))
        metrics = evidence["profile"]["metrics"]
        for name in ("cpu", "engine", "backend", "gpu_including_present"):
            if metrics.get(name):
                print("%-22s p50 %7.2f  p99 %7.2f ms" % (name, metrics[name]["median_ms"], metrics[name]["p99_ms"]))
        print("worst frames:")
        for frame in evidence["worst_frames"][:8]:
            costs = " ".join("%s %.1f" % (name, value / 1000) for name, value in frame["heaviest_costs_us"].items())
            print("  f%-6d +%6.2f s  %7.1f ms  cpu %7.1f  %s" % (frame["frame"], frame["seconds_into_playback"],
                                                               frame["interval_ms"], frame["cpu_ms"], costs))
        for item in evidence.get("core_passes", [])[:12]:
            print("  core %7.2f ms  x%-5.1f %s" % (item["mean_ms"], item["per_frame"], item["pass"]))
    for path in evidence.get("renderdoc", {}).get("captures", []):
        print("renderdoc capture: " + path)
    print("status: %s" % evidence["status"])
    for failure in evidence["failures"]:
        print("  FAIL " + failure)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    play = commands.add_parser("run", help="stage, play the demo and analyze it")
    play.add_argument("--runtime", type=Path, required=True, help="private runtime to stage")
    play.add_argument("--out", type=Path, required=True, help="new evidence directory")
    play.add_argument("--build", type=Path, help="Waf tree (default: the workload's)")
    play.add_argument("--steam-root", type=Path, default=frame_floor.DEFAULT_STEAM_ROOT)
    play.add_argument("--no-stage", action="store_true", help="reuse the staged runtime as is")
    play.add_argument("--profile", action="store_true",
                      help="backend GPU pass timers and the core's per-pass reports (adds overhead)")
    play.add_argument("--width", type=int, help="drawable width (default: the workload's)")
    play.add_argument("--height", type=int, help="drawable height (default: the workload's)")
    play.add_argument("--renderdoc-frames", type=lambda text: sorted({int(item) for item in text.split(",")}),
                      default=[], metavar="N[,N...]",
                      help="capture these -vkframestats frame numbers with RenderDoc (frame numbers "
                           "drift by a few between runs; the capture run's stream is authoritative)")
    play.add_argument("--extra-arg", action="append", default=[], help="engine argument (repeatable)")
    again = commands.add_parser("analyze", help="re-analyze an existing evidence directory")
    again.add_argument("out", type=Path)
    for sub in (play, again):
        sub.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
    args = parser.parse_args(argv)
    try:
        workload = load_workload(args.workload)
        workload["demo_name"] = workload["map"] + ".dem"
        if args.command == "analyze":
            result = analyze(args.out, workload)
            evidence = {"status": "incomplete" if result["failures"] else "complete", **result}
        else:
            if args.out.exists() and any(args.out.iterdir()):
                raise DemoError("%s is not empty; use a new evidence directory" % args.out)
            args.build = args.build or ROOT / workload["build"]
            args.width = args.width or workload["width"]
            args.height = args.height or workload["height"]
            evidence = run(args, workload)
    except (DemoError, render_profile.ProfileError, OSError, KeyError) as error:
        print("demo_frames: %s" % error, file=sys.stderr)
        return 2
    print_result(evidence)
    return 0 if evidence["status"] == "complete" else 1


if __name__ == "__main__":
    sys.exit(main())
