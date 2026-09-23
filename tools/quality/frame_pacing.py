#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Measure frame pacing of a scripted Portal workload on the native Vulkan renderer.

The installed product runs a versioned scenario (quality/workloads/*.json) in a
private staged runtime, with the backend writing one line per presented frame
(-vkframestats, materialsystem/shaderapivulkan/vulkan_frame_stats.h). The
analysis reports frame-interval percentiles, hitches and what the backend was
doing in each hitch frame, per scenario phase and per pass. A scenario body
repeated in one process separates first-use costs (pass 1) from costs that
recur (later passes): a stutter that survives warm-up is the one players feel
every time.

Staging and launch reuse portal_boot (the owner of runtime staging); nothing
here copies that logic.
"""

import argparse
import datetime
import json
import math
from pathlib import Path
import re
import sys

import conformance
import portal_boot


SCENARIO_SCHEMA = "frame-pacing-scenario/v1"
STATS_SCHEMA = "vulkan-frame-stats/v1"
EVIDENCE_SCHEMA = "frame-pacing-evidence/v1"
CFG_PREFIX = "frame_pacing_scenario"
# The console parses at most COMMAND_MAX_LENGTH (512) characters of a line and
# runs the remainder as further, out-of-order commands; each cfg stays below it.
CFG_LINE_LIMIT = 400
# A frame is a hitch when its interval exceeds both a multiple of the window's
# median and the median plus a floor: the multiple follows the workload's own
# cadence; the floor keeps sub-millisecond jitter of a fast frame out.
DEFAULT_HITCH_RATIO = 2.0
DEFAULT_HITCH_FLOOR_MS = 4.0
MARK_PATTERN = re.compile(r"^[A-Za-z0-9_.-]+$")


class ScenarioError(ValueError):
    pass


def load_scenario(path):
    scenario = json.loads(Path(path).read_text())
    if not isinstance(scenario, dict) or scenario.get("schema") != SCENARIO_SCHEMA:
        raise ScenarioError("scenario must be a %s object" % SCENARIO_SCHEMA)
    for key in ("id", "map"):
        if not isinstance(scenario.get(key), str) or not re.fullmatch(r"[A-Za-z0-9_.-]+", scenario[key]):
            raise ScenarioError("scenario %s must be a simple name" % key)
    for key in ("setup", "body"):
        steps = scenario.get(key)
        if not isinstance(steps, list) or not all(isinstance(step, str) and step for step in steps):
            raise ScenarioError("scenario %s must be a list of console commands" % key)
        for step in steps:
            if any(character in step for character in ';"\n\r'):
                raise ScenarioError("one console command per step (no ; \" or newlines): " + step)
            words = step.split()
            if words[0] == "vk_frame_mark" and (len(words) != 2 or not MARK_PATTERN.match(words[1])):
                raise ScenarioError("vk_frame_mark takes one [A-Za-z0-9_.-] label: " + step)
    if not scenario["body"]:
        raise ScenarioError("scenario body is empty")
    passes = scenario.get("passes", 1)
    if not isinstance(passes, int) or not 1 <= passes <= 16:
        raise ScenarioError("passes must be an integer in [1, 16]")
    # Simulated frames per second. `wait` counts frames, so without a fixed
    # simulation step the same script walks further on a slower build and the
    # measured workload changes with the thing being measured.
    rate = scenario.get("host_framerate")
    if not isinstance(rate, int) or not 10 <= rate <= 1000:
        raise ScenarioError("host_framerate must be an integer in [10, 1000]")
    return scenario


def scenario_commands(scenario, passes):
    """The whole scenario as an ordered list of console commands.

    Each pass is bracketed by marks so the analysis can split the frame stream
    without trusting frame counts.
    """
    commands = list(scenario["setup"])
    for index in range(1, passes + 1):
        commands.append("vk_frame_mark pass_%d_begin" % index)
        commands.extend(scenario["body"])
        commands.append("vk_frame_mark pass_%d_end" % index)
    commands.append("wait 10")
    commands.append("quit")
    return commands


def scenario_cfgs(commands):
    """Chained cfg files, each one line: {name: text}; the first is exec'd.

    Cmd_Exec_f runs separate lines of a file at once, while one ;-separated
    line keeps every `wait` in sequence. A line must also stay under the
    console's length limit, so the sequence continues in the next file by an
    `exec` at the end of each line, which runs after that line's waits.
    """
    files, line = [], []
    for command in commands:
        chained = ";".join(line + [command, "exec %s_%03d.cfg" % (CFG_PREFIX, len(files) + 1)])
        if line and len(chained) > CFG_LINE_LIMIT:
            files.append(line + ["exec %s_%03d.cfg" % (CFG_PREFIX, len(files) + 1)])
            line = []
        if len(command) + 32 > CFG_LINE_LIMIT:
            raise ScenarioError("console command too long: " + command)
        line.append(command)
    files.append(line)
    return {"%s_%03d.cfg" % (CFG_PREFIX, index): ";".join(text) + "\n"
            for index, text in enumerate(files)}


def read_stats(path):
    """Header and frame records; a truncated final line (killed process) is dropped."""
    header, frames, truncated = None, [], 0
    with Path(path).open() as stream:
        for line in stream:
            line = line.strip()
            if not line:
                continue
            try:
                record = json.loads(line)
            except json.JSONDecodeError:
                truncated += 1
                continue
            if header is None:
                if record.get("schema") != STATS_SCHEMA:
                    raise ScenarioError("frame stats do not start with a %s header" % STATS_SCHEMA)
                header = record
            else:
                frames.append(record)
    if header is None:
        raise ScenarioError("frame stats are empty")
    return header, frames, truncated


def percentile(values, fraction):
    """Nearest-rank percentile; values must be non-empty."""
    ordered = sorted(values)
    rank = max(1, math.ceil(fraction * len(ordered)))
    return ordered[rank - 1]


def summarize(intervals_ms):
    if not intervals_ms:
        return {"frames": 0}
    return {
        "frames": len(intervals_ms),
        "mean_ms": round(sum(intervals_ms) / len(intervals_ms), 3),
        "median_ms": round(percentile(intervals_ms, 0.5), 3),
        "p90_ms": round(percentile(intervals_ms, 0.9), 3),
        "p99_ms": round(percentile(intervals_ms, 0.99), 3),
        "max_ms": round(max(intervals_ms), 3),
    }


def frame_costs(frame):
    return {kind: {"count": value[0], "ms": round(value[1] / 1000.0, 3)}
            for kind, value in frame.get("cost", {}).items()}


def analyze(frames, passes, hitch_ratio=DEFAULT_HITCH_RATIO, hitch_floor_ms=DEFAULT_HITCH_FLOOR_MS):
    """Split the stream into passes/phases by marks and find hitches in each pass."""
    gpu_by_frame = {}
    for frame in frames:
        if "gpu" in frame:
            gpu_by_frame[frame["gpu"][0]] = frame["gpu"][1] / 1000.0
    failures = []
    report = {"passes": [], "cost_totals": {}}
    # A mark lands on the frame being built when the command ran.
    marks = {}
    for position, frame in enumerate(frames):
        for mark in filter(None, frame.get("mark", "").split(",")):
            marks.setdefault(mark, position)
    for index in range(1, passes + 1):
        begin, end = marks.get("pass_%d_begin" % index), marks.get("pass_%d_end" % index)
        if begin is None or end is None or end <= begin:
            failures.append("pass %d marks missing from the frame stream" % index)
            continue
        window = frames[begin + 1:end + 1]
        intervals = [frame["interval"] / 1000.0 for frame in window]
        stats = summarize(intervals)
        if not intervals:
            failures.append("pass %d measured no frames" % index)
            report["passes"].append({"pass": index, "summary": stats})
            continue
        threshold = max(stats["median_ms"] * hitch_ratio, stats["median_ms"] + hitch_floor_ms)
        phase, phases, hitches = "pass_start", {}, []
        for frame in window:
            for mark in filter(None, frame.get("mark", "").split(",")):
                if not mark.startswith("pass_"):
                    phase = mark
            interval = frame["interval"] / 1000.0
            bucket = phases.setdefault(phase, [])
            bucket.append(interval)
            for kind, value in frame.get("cost", {}).items():
                total = report["cost_totals"].setdefault(kind, {"count": 0, "ms": 0.0, "frames": 0})
                total["count"] += value[0]
                total["ms"] += value[1] / 1000.0
                total["frames"] += 1
            if interval > threshold:
                costs = frame_costs(frame)
                hitches.append({
                    "frame": frame["f"], "phase": phase, "interval_ms": round(interval, 3),
                    "engine_ms": round(frame["engine"] / 1000.0, 3),
                    "backend_ms": round(frame["backend"] / 1000.0, 3),
                    "gpu_ms": round(gpu_by_frame[frame["f"]], 3) if frame["f"] in gpu_by_frame else None,
                    "records": frame["records"], "upload_bytes": frame["upload_bytes"],
                    "costs": costs,
                    # The largest named blocking cost, excluding the frame's own
                    # recording/fence (which every frame has).
                    "dominant": max(((kind, cost["ms"]) for kind, cost in costs.items()
                                     if kind not in {"record", "submit", "present", "acquire"}),
                                    key=lambda item: item[1], default=(None, 0.0))[0],
                })
        report["passes"].append({
            "pass": index, "summary": stats, "hitch_threshold_ms": round(threshold, 3),
            "hitch_count": len(hitches),
            "hitch_excess_ms": round(sum(h["interval_ms"] - stats["median_ms"] for h in hitches), 3),
            "phases": {name: {**summarize(values),
                              "hitches": sum(1 for h in hitches if h["phase"] == name)}
                       for name, values in phases.items()},
            "hitches": hitches,
        })
    for total in report["cost_totals"].values():
        total["ms"] = round(total["ms"], 3)
    return report, failures


def check_budgets(report, budgets):
    """Budgets apply to the last (warm) pass; the first pass is reported, not gated."""
    failures = []
    if not report["passes"]:
        return ["no measured pass"]
    warm = report["passes"][-1]
    if warm["summary"].get("frames", 0) == 0:
        return ["warm pass measured no frames"]
    if budgets.get("max_hitches") is not None and warm["hitch_count"] > budgets["max_hitches"]:
        failures.append("warm pass has %d hitches (budget %d)" % (warm["hitch_count"], budgets["max_hitches"]))
    if budgets.get("max_p99_ms") is not None and warm["summary"]["p99_ms"] > budgets["max_p99_ms"]:
        failures.append("warm pass p99 %.3f ms exceeds budget %.3f ms" % (warm["summary"]["p99_ms"], budgets["max_p99_ms"]))
    if budgets.get("max_frame_ms") is not None and warm["summary"]["max_ms"] > budgets["max_frame_ms"]:
        failures.append("warm pass max %.3f ms exceeds budget %.3f ms" % (warm["summary"]["max_ms"], budgets["max_frame_ms"]))
    return failures


def compare(report, baseline):
    """Per-pass deltas against an earlier evidence file (same scenario)."""
    rows = []
    for current, previous in zip(report["passes"], baseline.get("analysis", {}).get("passes", [])):
        row = {"pass": current["pass"]}
        for key in ("median_ms", "p99_ms", "max_ms"):
            if key in current["summary"] and key in previous.get("summary", {}):
                row[key] = [previous["summary"][key], current["summary"][key]]
        row["hitch_count"] = [previous.get("hitch_count"), current.get("hitch_count")]
        rows.append(row)
    return rows


def print_report(report):
    for entry in report["passes"]:
        summary = entry["summary"]
        if not summary.get("frames"):
            print("  pass %d: no frames" % entry["pass"])
            continue
        print("  pass %d: %d frames, median %.2f ms, p99 %.2f ms, max %.2f ms, %d hitches (> %.2f ms, %.1f ms excess)"
              % (entry["pass"], summary["frames"], summary["median_ms"], summary["p99_ms"], summary["max_ms"],
                 entry["hitch_count"], entry["hitch_threshold_ms"], entry["hitch_excess_ms"]))
        for name, phase in entry["phases"].items():
            print("    %-16s %5d frames  median %6.2f  p99 %6.2f  max %6.2f  hitches %d"
                  % (name, phase["frames"], phase["median_ms"], phase["p99_ms"], phase["max_ms"], phase["hitches"]))
        for hitch in sorted(entry["hitches"], key=lambda h: -h["interval_ms"])[:8]:
            costs = ", ".join("%s %dx %.1fms" % (kind, cost["count"], cost["ms"])
                              for kind, cost in sorted(hitch["costs"].items(), key=lambda item: -item[1]["ms"])
                              if cost["ms"] >= 0.5)
            print("      frame %d [%s] %.1f ms (engine %.1f, backend %.1f, gpu %s): %s"
                  % (hitch["frame"], hitch["phase"], hitch["interval_ms"], hitch["engine_ms"],
                     hitch["backend_ms"], "%.1f" % hitch["gpu_ms"] if hitch["gpu_ms"] is not None else "-",
                     costs or "no named backend cost"))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runtime", type=Path, required=True, help="installed Portal runtime")
    parser.add_argument("--build", type=Path, required=True, help="Waf output tree to overlay")
    parser.add_argument("--scenario", type=Path,
                        default=Path(conformance.repo_root()) / "quality/workloads/portal-frame-pacing-v1.json")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--renderer", default="native-vulkan")
    parser.add_argument("--physics", default="vphysics")
    parser.add_argument("--passes", type=int, help="override the scenario's pass count")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--fps-max", type=int, default=1000,
                        help="engine frame cap; the default leaves each frame's cost uncapped "
                             "(fps_max 0 stops this engine presenting frames at all)")
    parser.add_argument("--timeout", type=float, default=240)
    parser.add_argument("--windowed", action="store_true",
                        help="use the ambient display instead of the SDL offscreen driver")
    parser.add_argument("--hitch-ratio", type=float, default=DEFAULT_HITCH_RATIO)
    parser.add_argument("--hitch-floor-ms", type=float, default=DEFAULT_HITCH_FLOOR_MS)
    parser.add_argument("--baseline", type=Path, help="earlier evidence.json to compare against")
    parser.add_argument("--extra-arg", action="append", default=[],
                        help="extra engine command-line argument (repeatable), e.g. an A/B switch")
    args = parser.parse_args(argv)

    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    evidence = {"schema": EVIDENCE_SCHEMA, "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "runtime": str(args.runtime.resolve()), "build": str(args.build.resolve())}
    failures = []
    try:
        scenario = load_scenario(args.scenario)
        passes = args.passes or scenario.get("passes", 1)
        evidence["scenario"] = {"path": str(args.scenario.resolve()), "id": scenario["id"],
                                "sha256": portal_boot.sha256(args.scenario), "passes": passes}
        stage = output / "runtime"
        evidence["staging"] = portal_boot.stage_runtime(args.runtime, stage)
        evidence["build_overrides"] = portal_boot.install_build(args.build, stage)
        (stage / "portal/cfg").mkdir(parents=True, exist_ok=True)
        cfgs = scenario_cfgs(scenario_commands(scenario, passes))
        for name, text in cfgs.items():
            (stage / "portal/cfg" / name).write_text(text)
        stats_path = output / "frame-stats.jsonl"
        command = [str(stage / "hl2_launcher"), "-game", "portal", "-renderer", args.renderer,
                   "-windowed", "-w", str(args.width), "-h", str(args.height), "-multirun",
                   "-novid", "-insecure", "-console", "-condebug", "-dev", "-physics", args.physics,
                   "-vkframestats", str(stats_path)] + args.extra_arg + [
                   "+sv_cheats", "1", "+mat_queue_mode", "0", "+fps_max", str(args.fps_max),
                   "+host_framerate", str(scenario["host_framerate"]),
                   "+volume", "0", "+map", scenario["map"],
                   "+wait", "120", "+exec", next(iter(cfgs))]
        environment = dict(portal_boot.os.environ)
        environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        environment["SteamAppId"] = environment["SteamGameId"] = "400"
        if not args.windowed:
            # Real GPU rendering through VK_EXT_headless_surface: no window on
            # anyone's desktop and no compositor pacing in the measurement.
            environment["SDL_VIDEODRIVER"] = environment["SDL_VIDEO_DRIVER"] = "offscreen"
            for variable in ("WAYLAND_DISPLAY", "DISPLAY"):
                environment.pop(variable, None)
        evidence["command"] = command
        code, timed_out, _, seconds = portal_boot.run_product(
            command, stage, environment, args.timeout, output / "stdout.log")
        evidence.update(returncode=code, timed_out=timed_out, elapsed_seconds=round(seconds, 1))
        if timed_out:
            failures.append("product timed out after %.0f s" % args.timeout)
        elif code != 0:
            failures.append("product exited with %d" % code)
        if not stats_path.is_file():
            failures.append("no frame stats were written (is the renderer native-vulkan?)")
        else:
            header, frames, truncated = read_stats(stats_path)
            evidence["device"] = header
            evidence["frames_recorded"] = len(frames)
            evidence["truncated_lines"] = truncated
            report, analysis_failures = analyze(frames, passes, args.hitch_ratio, args.hitch_floor_ms)
            failures.extend(analysis_failures)
            evidence["analysis"] = report
            budgets = scenario.get("budgets", {})
            evidence["budgets"] = budgets
            evidence["budget_failures"] = check_budgets(report, budgets)
            if args.baseline:
                evidence["baseline"] = {"path": str(args.baseline.resolve()),
                                        "passes": compare(report, json.loads(args.baseline.read_text()))}
    except (OSError, ValueError) as error:
        failures.append(str(error))
    evidence["failures"] = failures
    # A run that could not measure is a failure; a run over budget is a
    # measured result with a verdict, reported separately.
    if failures:
        evidence["status"] = "fail"
    else:
        evidence["status"] = "over-budget" if evidence.get("budget_failures") else "pass"
    (output / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print("Frame pacing: %s (%s)" % (evidence["status"], output / "evidence.json"))
    if "analysis" in evidence:
        print_report(evidence["analysis"])
    for row in evidence.get("baseline", {}).get("passes", []):
        print("  vs baseline pass %d: %s" % (row["pass"], ", ".join(
            "%s %s -> %s" % (key, value[0], value[1]) for key, value in row.items() if key != "pass")))
    for failure in failures + evidence.get("budget_failures", []):
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
