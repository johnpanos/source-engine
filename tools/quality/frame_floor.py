#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Frame-rate floor: play a scripted route through a map and fail at the
first frame below the floor.

The workload (quality/workloads/portal2-frame-floor-v1) is a portal2-scenarios
workload whose script flies a route through the map and fires its story
events (tools/quality/portal2_scenarios.py's QA driver). The game starts
through kiln.api (sepipe): a run request of the Portal 2 profile from a
private runtime that kiln packages, so it runs with exactly the arguments
`./kiln play portal2` gives a player, with the native Vulkan backend writing one line per
presented frame (-vkframestats). This tool reads that stream while the game
runs.

Every frame between the script's "floor_begin" and "floor_end" frame marks
is judged. The run fails, and the game is stopped at once, at the first
frame whose interval (present to present, what the player sees) is longer
than the row's floor. The stop names the frame, its
phase (the last frame mark) and the backend's costs in it. A run that
reaches "floor_end" is then judged on its lows:

  1% low     1000 / the mean interval of the slowest 1% of frames (fps)
  0.1% low   the same over the slowest 0.1%

against the workload's targets. p99 and p99.9 frame times are reported
beside them. --no-stop measures a whole run without stopping, to list every
frame below the floor at once.

Two display modes: "offscreen" (SDL's offscreen driver; it caps the back
buffer at 1024x768) and "compositor" (a private headless mutter with a
virtual monitor at --width x --height, as a player's window). Neither opens
a window on the desktop. The GPU is shared with anything else running on
the host; the evidence records the load and the other game processes seen.
The row-linked High workload rejects capped offscreen runs, lower floors and
quality overrides, applies and queries the product profile's High settings,
and checks actual back-buffer size, GPU and complete CPU/GPU timing records.
Timing success alone does not certify complete image or workload coverage.

Exit status: 0 pass, 1 floor or low failed (or the route did not complete),
2 usage or staging error.
"""

import argparse
import datetime
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

import conformance
import portal2_scenarios
import render_budgets
import product_profile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402


ROOT = Path(conformance.repo_root())
DEFAULT_WORKLOAD = ROOT / "quality/workloads/portal2-frame-floor-v1/workload.json"
EVIDENCE_SCHEMA = "frame-floor-evidence/v1"
BEGIN_MARK = "floor_begin"
END_MARK = "floor_end"
# The options file the tour script includes (qa/floor_options.nut).
OPTIONS_SCRIPT = "floor_options.nut"
# How many frames before a failing frame the evidence keeps.
CONTEXT_FRAMES = 30


class FloorError(Exception):
    pass


def floor_settings(workload, args):
    settings = dict(workload.get("frame_floor", {}))
    row = (render_budgets.find(render_budgets.load(), workload["render_budget_row"])
           if workload.get("render_budget_row") else None)
    if row:
        settings = {key: row["minimum_fps"]
                    for key in ("floor_fps", "low_1pct_fps", "low_01pct_fps")}
    for key, value in (("floor_fps", args.floor_fps), ("low_1pct_fps", args.low_1pct_fps),
                       ("low_01pct_fps", args.low_01pct_fps)):
        if value is not None:
            settings[key] = value
    for key in ("floor_fps", "low_1pct_fps", "low_01pct_fps"):
        if not render_budgets.valid_number(settings.get(key)) or settings[key] <= 0:
            raise FloorError("the workload's frame_floor needs a positive %s" % key)
        if row and settings[key] < row["minimum_fps"]:
            raise FloorError("%s cannot relax the hard budget %s" % (key, row["id"]))
    return settings


def configure_budget(workload, args):
    """Pin the declared High profile; diagnostic runs cannot lower its settings."""
    if not workload.get("render_budget_row"):
        return None
    row = render_budgets.find(render_budgets.load(), workload["render_budget_row"])
    conditions = row["conditions"]
    if (args.width, args.height) != (conditions["width"], conditions["height"]):
        raise FloorError("%s requires %dx%d" % (row["id"], conditions["width"], conditions["height"]))
    if args.display == "offscreen":
        raise FloorError("offscreen's capped drawable cannot certify %s" % row["id"])
    if args.set or args.extra_arg:
        raise FloorError("launch switches/extra arguments cannot override a hard High workload")
    profile = product_profile.load_profile(ROOT / row["profile"])
    quality = profile["intent"]["render_quality"][conditions["quality"]]
    return {"row": row["id"], "profile": row["profile"], "conditions": conditions,
            "settings": quality["settings"], "limits": row["modes"]["headroom"]}


def quality_receipt(log, header, budget, frames=None):
    """Read actual cvar values and back-buffer sizes, rather than launch intent."""
    sizes = [(int(w), int(h)) for w, h in re.findall(r"back buffer (\d+)x(\d+)", log)]
    observed = dict(re.findall(r'"([A-Za-z0-9_]+)"\s*=\s*"([^"\n]+)"', log))
    expected = budget["conditions"]
    failures = []
    extents = [frame.get("extent") for frame in frames] if frames is not None else None
    valid_extents = [e for e in (extents or []) if isinstance(e, list) and len(e) == 2
                     and all(isinstance(v, int) and not isinstance(v, bool) and v > 0 for v in e)]
    if extents is not None and (not extents or any(
            extent != [expected["width"], expected["height"]] for extent in extents)
            or len(valid_extents) != len(extents)):
        failures.append("per-frame drawable extents are missing or differ from %dx%d" %
                        (expected["width"], expected["height"]))
    if not sizes or any(size != (expected["width"], expected["height"]) for size in sizes):
        failures.append("actual back buffer is not verified at %dx%d" % (expected["width"], expected["height"]))
    if not header or header.get("device") != expected["device"]:
        failures.append("the budget's native GPU identity is not verified")
    for name, value in budget["settings"].items():
        if observed.get(name) != value:
            failures.append("High setting %s: expected %s, observed %s" % (name, value, observed.get(name)))
    return {"back_buffers": sizes, "measured_extents": sorted({tuple(e) for e in valid_extents})
            if extents is not None else None,
            "observed_settings": observed,
            "failures": failures, "status": "fail" if failures else "pass"}


def graphics_context():
    """Record current driver/API facts, not the profile's earlier observation."""
    try:
        probe = subprocess.run(["vulkaninfo", "--summary"], capture_output=True, text=True, timeout=15)
        if probe.returncode:
            return {"status": "unavailable", "reason": "vulkaninfo exited %d" % probe.returncode}
        devices = []
        for block in re.split(r"(?m)^GPU\d+:\s*$", probe.stdout)[1:]:
            fields = dict(re.findall(r"(?m)^\s*(deviceName|driverName|driverInfo|apiVersion)\s*=\s*(.+)$", block))
            if fields:
                devices.append({key: value.strip() for key, value in fields.items()})
        return {"status": "pass" if devices else "unavailable", "devices": devices}
    except (OSError, subprocess.SubprocessError) as error:
        return {"status": "unavailable", "reason": str(error)}


def metric_budget_failures(summary, budget):
    failures = []
    for field in ("cpu", "gpu_render", "submission"):
        for statistic in ("p99", "max"):
            metric = field + "_" + statistic + "_ms"
            value = summary.get(metric)
            if not render_budgets.valid_number(value):
                failures.append("the complete route has no valid %s" % metric)
            elif value > budget["limits"]["max_" + field + ("_p99_ms" if statistic == "p99" else "_ms")]:
                failures.append("%s %.3f ms exceeds the hard budget" % (metric, value))
    return failures


def valid_duration_us(value):
    # Negative int64 deltas wrapped into uint64 are not elapsed durations.
    return render_budgets.valid_number(value) and value < (1 << 63)


# --- Frame statistics ------------------------------------------------------------

def low_fps(intervals_ms, fraction):
    """1000 / the mean of the slowest <fraction> of the intervals."""
    ordered = sorted(intervals_ms)
    count = max(1, int(len(ordered) * fraction))
    worst = ordered[-count:]
    return 1000.0 / (sum(worst) / len(worst))


def percentile(intervals_ms, fraction):
    ordered = sorted(intervals_ms)
    return ordered[min(len(ordered) - 1, int(len(ordered) * fraction))]


def summarize(frames):
    intervals = [frame["interval_ms"] for frame in frames]
    if not intervals:
        return {"frames": 0}
    result = {
        "frames": len(intervals),
        "seconds": round(sum(intervals) / 1000.0, 2),
        "median_ms": round(percentile(intervals, 0.5), 3),
        "p95_ms": round(percentile(intervals, 0.95), 3),
        "p99_ms": round(percentile(intervals, 0.99), 3),
        "p999_ms": round(percentile(intervals, 0.999), 3),
        "max_ms": round(max(intervals), 3),
        "average_fps": round(1000.0 * len(intervals) / sum(intervals), 1),
        "low_1pct_fps": round(low_fps(intervals, 0.01), 1),
        "low_01pct_fps": round(low_fps(intervals, 0.001), 1),
    }
    for field in ("cpu", "gpu_render", "submission"):
        values = [frame[field + "_ms"] for frame in frames if frame.get(field + "_ms") is not None]
        if len(values) == len(frames):
            result[field + "_p99_ms"] = round(percentile(values, 0.99), 3)
            result[field + "_max_ms"] = round(max(values), 3)
    return result


def frame_brief(row, phase):
    """What a frame record says about where its time went."""
    cost = row.get("cost", {})
    heaviest = sorted(((name, value[1]) for name, value in cost.items()
                       if isinstance(value, list) and len(value) == 2),
                      key=lambda item: -item[1])[:6]
    brief = {"frame": row.get("f"), "phase": phase,
             "interval_ms": round(row.get("interval", 0) / 1000.0, 3),
             "engine_ms": round(row.get("engine", 0) / 1000.0, 3),
             "backend_ms": round(row.get("backend", 0) / 1000.0, 3),
             "upload_bytes": row.get("upload_bytes", 0),
             "heaviest_costs_us": dict(heaviest)}
    if "gpu" in row:
        brief["gpu"] = row["gpu"]
    if "mark" in row:
        brief["mark"] = row["mark"]
    return brief


class FrameWatcher:
    """Reads the -vkframestats stream as the game writes it and judges each
    frame between the begin and end marks against the floor."""

    def __init__(self, path, floor_ms):
        self.path = Path(path)
        self.floor_ms = floor_ms
        self.offset = 0
        self.partial = ""
        self.state = "waiting"  # waiting -> judging -> ended
        self.phase = None
        self.frames = []
        self.phases = {}
        self.recent = []
        self.below = []
        self.header = None
        self.invalid = []
        self.gpu_frames = {}

    def poll(self):
        """Reads new lines; returns the first frame below the floor seen in
        them, or None."""
        if not self.path.is_file():
            return None
        with self.path.open("r", errors="replace") as stream:
            stream.seek(self.offset)
            text = stream.read()
            self.offset = stream.tell()
        text = self.partial + text
        lines = text.split("\n")
        self.partial = lines.pop()
        first = None
        for line in lines:
            if not line.startswith("{"):
                if line.strip() and self.state == "judging":
                    self.invalid.append("unreadable frame stats during the route")
                continue
            try:
                row = json.loads(line)
            except json.JSONDecodeError:
                if self.state == "judging":
                    self.invalid.append("malformed frame stats during the route")
                continue
            if "f" not in row:
                self.header = row
                continue
            failed = self.judge(row)
            if failed is not None and first is None:
                first = failed
        return first

    def judge(self, row):
        gpu = row.get("gpu")
        if isinstance(gpu, list) and len(gpu) >= 3 and valid_duration_us(gpu[2]):
            self.gpu_frames[gpu[0]] = gpu[2] / 1000.0
        marks = row.get("mark", "").split(",") if row.get("mark") else []
        # The marks are attached to the frame being built when the script
        # sent them; a frame carrying floor_begin is the first one judged.
        if BEGIN_MARK in marks and self.state == "waiting":
            self.state = "judging"
        for mark in marks:
            if mark not in (BEGIN_MARK, END_MARK):
                self.phase = mark
        if self.state != "judging":
            return None
        if not isinstance(row.get("f"), int) or isinstance(row["f"], bool) or row["f"] < 1:
            self.invalid.append("frame stats have an invalid frame number")
            return None
        interval = row.get("interval")
        if not valid_duration_us(interval) or interval == 0:
            self.invalid.append("frame %s has no positive finite interval" % row.get("f"))
            return None
        if self.frames and row.get("f") != self.frames[-1]["frame"] + 1:
            self.invalid.append("missing, duplicated or unordered frame after %s" % self.frames[-1]["frame"])
        interval_ms = interval / 1000.0
        frame = {"frame": row.get("f"), "interval_ms": interval_ms, "phase": self.phase,
                 "extent": row.get("extent")}
        if valid_duration_us(row.get("cpu")):
            frame["cpu_ms"] = row["cpu"] / 1000.0
        elif "cpu" in row:
            self.invalid.append("frame %s has an invalid CPU duration (possible counter wrap)" % row["f"])
        emit = row.get("cost", {}).get("emit")
        if isinstance(emit, list) and len(emit) == 2 and valid_duration_us(emit[1]):
            frame["submission_ms"] = emit[1] / 1000.0
        self.frames.append(frame)
        self.phases.setdefault(self.phase or "-", []).append(frame)
        self.recent.append(frame_brief(row, self.phase))
        del self.recent[:-CONTEXT_FRAMES]
        failed = None
        if interval_ms > self.floor_ms:
            failed = {"frame": frame_brief(row, self.phase),
                      "context": list(self.recent[:-1]),
                      "index": len(self.frames) - 1,
                      "seconds_into_route": round(sum(f["interval_ms"] for f in self.frames) / 1000.0, 2)}
            self.below.append(failed["frame"])
        if END_MARK in marks:
            self.state = "ended"
        return failed


# --- Staging and launch ------------------------------------------------------------

def stage(args, workload):
    """Package the profile into the private runtime (kiln.api, the same
    packager and steps as `kiln play`)."""
    runtime = args.runtime.resolve()
    player = Path(args.session.plan(args.kiln_profile, flavor=args.flavor)["runtime"]).resolve()
    if runtime == player:
        raise FloorError("refusing the profile's own runtime (%s); use a private --runtime" % player)
    try:
        args.session.build(args.kiln_profile, flavor=args.flavor, up_to="package", runtime=str(runtime))
    except args.sepipe.KilnError as error:
        raise FloorError("packaging failed: %s" % error) from error
    for scenario in workload["scenarios"]:
        if not (runtime / "portal2/custom" / ("pbrt-" + scenario["map"])).exists() and \
                not (runtime / "portal2/maps" / (scenario["map"] + ".bsp")).exists():
            raise FloorError("map %s is neither published nor installed" % scenario["map"])
    return runtime


def game_command(args, scenario, stats_path, runtime):
    high = (["+mat_antialias", args.render_budget["settings"]["mat_antialias"],
             "+mat_vsync", "0", "+exec", "render_budget_high"] if args.render_budget else [])
    startup = [] if args.render_budget else ["+volume", "0", "+mat_vsync", "0",
                                            "+engine_no_focus_sleep", "0"]
    profiling = (["-vkgputimers"] + ([] if args.render_budget else ["+exec", "render_profile"])
                 if getattr(args, "profile", False) else [])
    return ["-multirun", "-novid", "-condebug", "-windowed", "-noborder",
            "-w", str(args.width), "-h", str(args.height),
            "-vkopaquebatch", "0" if getattr(args, "opaque_batching", "on") == "off" else "1",
            # The game starts in runtime; keep evidence paths beneath its
            # 512-character command-line limit even for a long output path.
            "-vkframestats", os.path.relpath(stats_path, runtime),
            *startup,
            *args.extra_arg,
            *high,
            *profiling,
            "+map", scenario["map"], "+wait", str(args.start_frames),
            "+exec", "qa_" + scenario["name"]]


def run_options(args, runtime, arguments, output):
    """The kiln run request's options; the run and its evidence use one copy."""
    refresh = args.render_budget["conditions"].get("refresh_hz", 60) if args.render_budget else 60
    return {"flavor": args.flavor, "switches": args.set, "arguments": arguments, "runtime": str(runtime),
            "display": "private" if args.display == "compositor" else "none",
            "display_mode": (args.width, args.height, float(refresh)), "log": str(output / "stdout.log")}


def launch(args, runtime, arguments, output):
    """A kiln run request on a thread: the profile's display session (a
    private compositor or offscreen) and run provider own the process."""
    tools = output / "tools"
    portal2_scenarios.write_fake_zenity(tools)
    os.environ["PATH"] = str(tools) + os.pathsep + os.environ.get("PATH", "")
    return sepipe_loader.Run(args.sepipe, args.session, "run", args.kiln_profile,
                             **run_options(args, runtime, arguments, output))


def host_context():
    load = os.getloadavg()
    others = subprocess.run(["pgrep", "-af", "hl2_launcher|portal2_linux|hl2_linux"],
                            capture_output=True, text=True).stdout.splitlines()
    return {"load_average": [round(value, 2) for value in load],
            "cpu_count": os.cpu_count(),
            "other_game_processes": [line for line in others if "frame_floor" not in line]}


# --- A run ------------------------------------------------------------------

def run(args, workload, settings, scenario, runtime):
    output = args.out / scenario["name"]
    output.mkdir(parents=True)
    stats_path = output / "frames.jsonl"
    console = runtime / "portal2/console.log"
    console.unlink(missing_ok=True)
    floor_ms = 1000.0 / settings["floor_fps"]
    watcher = FrameWatcher(stats_path, floor_ms)
    command = game_command(args, scenario, stats_path, runtime)
    context_before = host_context()
    started = time.monotonic()
    process = launch(args, runtime, command, output)
    first_failure = None
    timed_out = False
    try:
        deadline = started + scenario["timeout_seconds"]
        while process.poll() is None:
            failed = watcher.poll()
            if failed is not None and first_failure is None:
                first_failure = failed
                frame = failed["frame"]
                print("  FLOOR frame %s in %s: %.1f ms (%.1f fps) %.1f s into the route" % (
                    frame["frame"], frame["phase"], frame["interval_ms"],
                    1000.0 / frame["interval_ms"], failed["seconds_into_route"]), flush=True)
                if not args.no_stop:
                    break
            if time.monotonic() > deadline:
                timed_out = True
                break
            time.sleep(0.05)
    finally:
        process.stop()
    final_failure = watcher.poll()
    first_failure = first_failure or final_failure
    seconds = time.monotonic() - started
    log = console.read_text(errors="replace") if console.is_file() else ""
    stdout_log = output / "stdout.log"
    stdout = stdout_log.read_text(errors="replace") if stdout_log.is_file() else ""
    if process.error:
        stdout += "\nkiln: " + process.error
    (output / "console.log").write_text(log)
    qa = portal2_scenarios.evaluate(scenario, log, process.returncode, timed_out)
    for frame in watcher.frames:
        frame["gpu_render_ms"] = watcher.gpu_frames.get(frame["frame"])
    summary = summarize(watcher.frames)
    phases = {name: summarize(frames) for name, frames in watcher.phases.items()}
    stopped_early = first_failure is not None and not args.no_stop
    failures = list(watcher.invalid)
    if not watcher.frames:
        failures.append("the route measured no valid frames")
    receipt = None
    if args.render_budget:
        receipt = quality_receipt(log + "\n" + stdout, watcher.header, args.render_budget, watcher.frames)
        failures.extend(receipt["failures"])
        device_name = (watcher.header or {}).get("device")
        drivers = [device for device in args.graphics.get("devices", [])
                   if device.get("deviceName") == device_name]
        if not drivers or not all(device.get("driverInfo") and device.get("apiVersion") for device in drivers):
            failures.append("actual native GPU driver/API evidence is unavailable")
        failures.extend(metric_budget_failures(summary, args.render_budget))
    if watcher.state == "waiting":
        failures.append("no frame carried the %s mark" % BEGIN_MARK)
    if first_failure is not None:
        frame = first_failure["frame"]
        failures.append("frame %s in %s took %.1f ms, below the %g fps floor" % (
            frame["frame"], frame["phase"], frame["interval_ms"], settings["floor_fps"]))
    if not stopped_early:
        if watcher.state != "ended":
            failures.append("the route did not reach %s" % END_MARK)
        failures += ["route: " + failure for failure in qa["failures"]]
        if summary.get("frames"):
            if summary["low_1pct_fps"] < settings["low_1pct_fps"]:
                failures.append("1%% low %.1f fps is under %g" % (summary["low_1pct_fps"],
                                                                  settings["low_1pct_fps"]))
            if summary["low_01pct_fps"] < settings["low_01pct_fps"]:
                failures.append("0.1%% low %.1f fps is under %g" % (summary["low_01pct_fps"],
                                                                   settings["low_01pct_fps"]))
    return {
        "scenario": scenario["name"],
        "map": scenario["map"],
        "status": "fail" if failures else "pass",
        "failures": failures,
        "stopped_early": stopped_early,
        "seconds": round(seconds, 1),
        "summary": summary,
        "phases": phases,
        "first_below_floor": first_failure,
        "below_floor": watcher.below,
        "device": watcher.header,
        "quality_receipt": receipt,
        "route_checks": qa["checks"],
        "command": {"profile": args.kiln_profile, "options": run_options(args, runtime, command, output),
                    "plan": args.session.plan(args.kiln_profile,
                                              **run_options(args, runtime, command, output))},
        "host_before": context_before,
        "host_after": host_context(),
    }


def print_result(result, settings):
    summary = result["summary"]
    if summary.get("frames"):
        print("  %d frames over %.1f s: median %.2f ms, p99 %.2f, p99.9 %.2f, max %.1f | "
              "avg %.1f fps, 1%% low %.1f fps, 0.1%% low %.1f fps" % (
                  summary["frames"], summary["seconds"], summary["median_ms"], summary["p99_ms"],
                  summary["p999_ms"], summary["max_ms"], summary["average_fps"],
                  summary["low_1pct_fps"], summary["low_01pct_fps"]))
    for name, phase in result["phases"].items():
        if phase.get("frames"):
            print("    %-14s %6d frames  median %6.2f  p99 %6.2f  max %7.1f ms  1%% low %6.1f fps" % (
                name, phase["frames"], phase["median_ms"], phase["p99_ms"], phase["max_ms"],
                phase["low_1pct_fps"]))
    if len(result["below_floor"]) > 1:
        print("  %d frames below the floor" % len(result["below_floor"]))
    for failure in result["failures"]:
        print("  FAIL %s" % failure)
    print("  %s (floor %g fps, targets: 1%% low %g, 0.1%% low %g)" % (
        result["status"].upper(), settings["floor_fps"], settings["low_1pct_fps"],
        settings["low_01pct_fps"]), flush=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
    parser.add_argument("--kiln-profile", default="portal2",
                        help="kiln profile to build, package and run (default portal2)")
    parser.add_argument("--flavor", default="dev", help="the profile's build flavor")
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-p2-floor",
                        help="private runtime kiln packages into (on every run)")
    parser.add_argument("--out", type=Path, required=True, help="new evidence directory")
    parser.add_argument("--display", choices=("compositor", "offscreen"), default="compositor")
    parser.add_argument("--width", type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    parser.add_argument("--start-frames", type=int, default=60,
                        help="client frames after the map loads before the script starts")
    parser.add_argument("--floor-fps", type=float, help="override the workload's floor")
    parser.add_argument("--low-1pct-fps", type=float, help="override the 1%% low target")
    parser.add_argument("--low-01pct-fps", type=float, help="override the 0.1%% low target")
    parser.add_argument("--no-stop", action="store_true",
                        help="play the whole route and list every frame below the floor")
    parser.add_argument("--profile", action="store_true",
                        help="enable existing backend and core GPU pass timers; diagnostic "
                             "timings include instrumentation overhead, with quality unchanged")
    parser.add_argument("--opaque-batching", choices=("on", "off"), default="on",
                        help="same-binary opaque batching control; leaves High quality unchanged")
    parser.add_argument("--preview", action="store_true",
                        help="take a screenshot at each view of the route (a screenshot is "
                             "itself a hitch: implies --no-stop, and no verdict on lows)")
    parser.add_argument("--set", action="append", default=[], metavar="SWITCH",
                        help="kiln launch switch (kiln switches portal2), e.g. --set no-core-world")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="extra engine argument before +map (repeatable)")
    parser.add_argument("--skip-stage", action="store_true",
                        help="use the runtime as staged by an earlier run")
    args = parser.parse_args(argv)
    if args.preview:
        args.no_stop = True

    try:
        workload = portal2_scenarios.load_workload(args.workload)
        settings = floor_settings(workload, args)
        args.render_budget = configure_budget(workload, args)
    except (portal2_scenarios.ScenarioError, product_profile.ProfileError, FloorError, ValueError) as error:
        parser.exit(2, "frame_floor: %s\n" % error)
    try:
        args.sepipe = sepipe_loader.load()
        args.session = args.sepipe.Session(str(ROOT))
    except sepipe_loader.LoadError as error:
        parser.exit(2, "frame_floor: %s\n" % error)
    args.out = args.out.resolve()
    if (args.out / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    args.out.mkdir(parents=True, exist_ok=True)
    evidence = {"schema": EVIDENCE_SCHEMA, "status": "incomplete",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(str(ROOT)),
                "workload": str(args.workload), "settings": settings,
                "display": {"mode": args.display, "width": args.width, "height": args.height},
                "preview": args.preview, "no_stop": args.no_stop, "profile": args.profile,
                "opaque_batching": args.opaque_batching,
                "results": []}
    evidence["render_budget"] = args.render_budget
    args.graphics = graphics_context() if args.render_budget else {}
    evidence["graphics"] = args.graphics
    evidence["image_acceptance"] = "unverified: requires separate complete-image K11/K12/R91 evidence"
    evidence_path = args.out / "evidence.json"
    try:
        runtime = args.runtime.resolve() if args.skip_stage else stage(args, workload)
        portal2_scenarios.install_scripts(args.workload, workload, runtime)
        scripts = runtime / "portal2/scripts/vscripts" / portal2_scenarios.SCRIPT_DIRECTORY
        (scripts / OPTIONS_SCRIPT).write_text(
            "// Written by tools/quality/frame_floor.py for this run.\n"
            "::FLOOR_OPTIONS <- { preview = %s }\n" % ("true" if args.preview else "false"))
        if args.profile:
            (runtime / "portal2/cfg/render_profile.cfg").write_text(
                "cl_render_debug_gpu_timers 1\ncl_render_debug_stats 1\n")
        if args.render_budget:
            settings_commands = [name + " " + value for name, value in args.render_budget["settings"].items()]
            settings_commands.append("fps_max 1000")
            settings_commands.extend(["volume 0", "engine_no_focus_sleep 0"])
            if args.profile:
                settings_commands.extend(["cl_render_debug_gpu_timers 1", "cl_render_debug_stats 1"])
            settings_commands.extend(args.render_budget["settings"])
            cfg = runtime / "portal2/cfg/render_budget_high.cfg"
            cfg.parent.mkdir(parents=True, exist_ok=True)
            cfg.write_text("\n".join(settings_commands) + "\n")
    except (OSError, ValueError, FloorError) as error:
        evidence["status"] = "error"
        evidence["error"] = str(error)
        evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
        parser.exit(2, "frame_floor: %s\n" % error)

    status = "pass"
    for scenario in workload["scenarios"]:
        print("== %s (%s, %s %dx%d)" % (scenario["name"], scenario["map"], args.display,
                                        args.width, args.height), flush=True)
        result = run(args, workload, settings, scenario, runtime)
        if args.preview:
            shots = runtime / "portal2/screenshots"
            target = args.out / scenario["name"] / "shots"
            target.mkdir(exist_ok=True)
            for shot in sorted(shots.glob("*")) if shots.is_dir() else []:
                shutil.move(str(shot), target / shot.name)
            result["status"] = "preview"
        evidence["results"].append(result)
        evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
        print_result(result, settings)
        if result["status"] == "fail":
            status = "fail"
    evidence["status"] = "preview" if args.preview else status
    evidence["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    evidence_path.write_text(json.dumps(evidence, indent=2) + "\n")
    return 1 if status == "fail" else 0


if __name__ == "__main__":
    sys.exit(main())
