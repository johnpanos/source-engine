#!/usr/bin/env python3
"""Resolution sweep: matched baseline/candidate frame timings from 1024x768
through 3840x2160 (RFC 0016's optimization resolution sweep).

`run` plays the recorded gameplay demo workload (default:
quality/workloads/portal2-intro4-demo-v1) through tools/quality/demo_frames.py,
in its private headless mutter at each resolution, once per (repeat,
resolution, variant), on one private runtime staged beforehand. Variants are
named sets of extra engine arguments; the first is the baseline:

    --variant per-surface=+r_core_world_gpu_submit,0
    --variant gpu=+r_core_world_gpu_submit,1

Runs are interleaved: every repeat visits every resolution, and within one
resolution the variants run back to back, their order rotated by repeat, so
host drift lands on both sides of a comparison. --warmup plays the demo once
first (the baseline at the first resolution, not reported) so the driver's
shader cache is warm for every recorded run. --retries N plays a run again,
up to N times, while the host shows another game process during it (the
GPU is shared); the last attempt is kept either way. Each run keeps demo_frames's
whole evidence directory under OUT/runs/.

`report` reads the runs' evidence (demo_frames's playback-window metrics:
present interval, CPU critical-path time and GPU render time) and writes, per
resolution and variant, the median over runs of each run's median and p95,
with the runs' spread, and per resolution each later variant's absolute and
percentage change against the baseline. Each point is labelled from its
timings: "gpu" when the GPU render median is within 15% of the present
interval median, "cpu" when the CPU median is, "mixed" when both, else
"unresolved". These labels are hypotheses for the RFC's analysis, not proof.

A run that is not complete, has failures, or whose drawable extents differ
from the requested resolution is listed and left out of its point; a point
with no valid run is "unavailable". Runs that saw other game processes on
the host (the GPU is shared) are counted per point.

This is diagnostic evidence, never performance acceptance: High
qualification stays with frame_floor.py and its budget row.
"""

import argparse
import json
import shutil
import statistics
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_WORKLOAD = ROOT / "quality/workloads/portal2-intro4-demo-v1/workload.json"
DEFAULT_RESOLUTIONS = ("1024x768", "1920x1080", "2560x1440", "3840x2160")
BOUND_SHARE = 0.85
METRICS = ("interval", "cpu", "gpu_render")


class SweepError(Exception):
    pass


def parse_resolution(text):
    try:
        width, height = (int(part) for part in text.lower().split("x"))
    except ValueError:
        raise SweepError("resolution %r is not WIDTHxHEIGHT" % text)
    if width <= 0 or height <= 0:
        raise SweepError("resolution %r is not positive" % text)
    return width, height


def parse_variant(text):
    name, _, arguments = text.partition("=")
    if not name or not name.replace("-", "").replace("_", "").isalnum():
        raise SweepError("variant %r needs NAME=ARG,ARG..." % text)
    return name, [argument for argument in arguments.split(",") if argument]


def run_order(repeats, resolutions, variants):
    """(repeat, resolution, variant) in interleaved order."""
    order = []
    for repeat in range(repeats):
        for resolution in resolutions:
            shift = repeat % len(variants)
            for variant in variants[shift:] + variants[:shift]:
                order.append((repeat, resolution, variant))
    return order


def run_directory(out, repeat, resolution, variant):
    return Path(out) / "runs" / ("%dx%d" % resolution) / variant / ("r%d" % repeat)


def command_run(args):
    out = Path(args.out)
    if out.exists():
        raise SweepError("%s exists; use a new evidence directory" % out)
    if not (Path(args.runtime) / "portal2" / "gameinfo.txt").is_file():
        raise SweepError("%s is not a staged runtime (stage it once, then sweep)" % args.runtime)
    resolutions = [parse_resolution(text) for text in args.resolution or DEFAULT_RESOLUTIONS]
    variants = [parse_variant(text) for text in args.variant]
    if len(variants) < 2 or len({name for name, _ in variants}) != len(variants):
        raise SweepError("give at least two distinct --variant NAME=ARGS")
    arguments = dict(variants)
    names = [name for name, _ in variants]
    out.mkdir(parents=True)
    plan = {"workload": str(args.workload), "resolutions": ["%dx%d" % r for r in resolutions],
            "variants": arguments, "repeats": args.repeats, "warmup": args.warmup,
            "retries": args.retries,
            "kiln_profile": args.kiln_profile, "flavor": args.flavor, "runtime": str(args.runtime),
            "order": [["r%d" % r, "%dx%d" % res, v]
                      for r, res, v in run_order(args.repeats, resolutions, names)]}
    (out / "plan.json").write_text(json.dumps(plan, indent=2) + "\n")
    def play(directory, resolution, variant, log_path):
        command = [sys.executable, str(ROOT / "tools/quality/demo_frames.py"), "run",
                   "--workload", str(args.workload), "--no-stage",
                   "--width", str(resolution[0]), "--height", str(resolution[1]),
                   "--runtime", str(args.runtime), "--out", str(directory)]
        if args.kiln_profile:
            command += ["--kiln-profile", args.kiln_profile]
        command += ["--flavor", args.flavor]
        command += ["--extra-arg=" + argument for argument in arguments[variant]]
        with log_path.open("wb") as log:
            return subprocess.run(command, stdout=log, stderr=subprocess.STDOUT).returncode

    if args.warmup:
        print("sweep: warm-up %dx%d %s" % (resolutions[0] + (names[0],)), flush=True)
        play(out / "warmup", resolutions[0], names[0], out / "warmup.log")
    for repeat, resolution, variant in run_order(args.repeats, resolutions, names):
        directory = run_directory(out, repeat, resolution, variant)
        directory.parent.mkdir(parents=True, exist_ok=True)
        for attempt in range(args.retries + 1):
            print("sweep: r%d %dx%d %s%s" % (repeat, resolution[0], resolution[1], variant,
                                            " (retry %d)" % attempt if attempt else ""),
                  flush=True)
            if directory.exists():
                shutil.rmtree(directory)
            code = play(directory, resolution, variant,
                        directory.parent / ("r%d.log" % repeat))
            print("sweep:   exit %d" % code, flush=True)
            metrics, _ = run_metrics(directory, resolution)
            if metrics is None or metrics["other_games"] == 0:
                break
    return command_report(argparse.Namespace(out=str(out)))


def run_metrics(directory, resolution):
    """One run's medians and p95s, or the reason it is left out."""
    evidence = Path(directory) / "evidence.json"
    if not evidence.is_file():
        return None, "no evidence"
    record = json.loads(evidence.read_text())
    if record.get("status") != "complete" or record.get("failures"):
        return None, "run %s: %s" % (record.get("status"),
                                     "; ".join(record.get("failures", [])[:3]))
    profile = record.get("profile", {})
    if profile.get("render_extents") != [list(resolution)]:
        return None, "drawable extents %s, expected %dx%d" % (
            profile.get("render_extents"), resolution[0], resolution[1])
    metrics = {"frames": profile.get("frames", 0),
               "other_games": len(record.get("host", {}).get("other_game_processes", []))}
    for field in METRICS:
        values = profile.get("metrics", {}).get(field)
        if values:
            metrics[field + "_median_ms"] = values["median_ms"]
            metrics[field + "_p95_ms"] = values["p95_ms"]
    return metrics, None


def bound(point):
    interval = point.get("interval_median_ms")
    if not interval:
        return "unresolved"
    gpu_bound = (point.get("gpu_render_median_ms") or 0.0) >= BOUND_SHARE * interval
    cpu_bound = (point.get("cpu_median_ms") or 0.0) >= BOUND_SHARE * interval
    return "mixed" if gpu_bound and cpu_bound else "gpu" if gpu_bound else \
        "cpu" if cpu_bound else "unresolved"


def summarize_point(runs):
    point = {"runs": len(runs), "frames": sum(run["frames"] for run in runs),
             "runs_beside_other_games": sum(run["other_games"] > 0 for run in runs)}
    for key in sorted({key for run in runs for key in run if key.endswith("_ms")}):
        values = [run[key] for run in runs if key in run]
        if len(values) == len(runs):
            point[key] = round(statistics.median(values), 3)
            point[key[:-3] + "_spread_ms"] = round(max(values) - min(values), 3)
    point["bound"] = bound(point)
    return point


def build_report(out):
    plan = json.loads((Path(out) / "plan.json").read_text())
    variants = list(plan["variants"])
    report = {"plan": plan, "points": {}, "excluded": [], "changes": {}}
    for text in plan["resolutions"]:
        resolution = parse_resolution(text)
        report["points"][text] = {}
        for variant in variants:
            runs = []
            for directory in sorted((Path(out) / "runs" / text / variant).glob("r*")):
                if not directory.is_dir():
                    continue
                metrics, reason = run_metrics(directory, resolution)
                if metrics is None:
                    report["excluded"].append({"run": str(directory.relative_to(out)),
                                               "reason": reason})
                else:
                    runs.append(metrics)
            report["points"][text][variant] = (summarize_point(runs) if runs
                                               else {"runs": 0, "bound": "unavailable"})
        base = report["points"][text][variants[0]]
        report["changes"][text] = {}
        for variant in variants[1:]:
            candidate = report["points"][text][variant]
            change = {}
            for field in METRICS:
                for statistic in ("median", "p95"):
                    key = "%s_%s_ms" % (field, statistic)
                    if base.get(key) and key in candidate:
                        change[key] = {"delta_ms": round(candidate[key] - base[key], 3),
                                       "percent": round(100.0 * (candidate[key] - base[key]) /
                                                        base[key], 1)}
            report["changes"][text][variant] = change
    return report


def report_lines(report):
    columns = ("interval_median_ms", "interval_p95_ms", "cpu_median_ms", "gpu_render_median_ms",
               "gpu_render_p95_ms")
    lines = ["resolution\tvariant\truns\tbeside_games\tinterval_med\tinterval_p95\tcpu_med"
             "\tgpu_med\tgpu_p95\tbound\tinterval_med_change\tgpu_med_change"]
    for text, points in report["points"].items():
        for variant, point in points.items():
            change = report["changes"][text].get(variant, {})
            lines.append("\t".join(
                [text, variant, str(point["runs"]), str(point.get("runs_beside_other_games", 0))] +
                ["%.3f" % point[key] if key in point else "-" for key in columns] +
                [point["bound"]] +
                ["%+.3f ms (%+.1f%%)" % (change[key]["delta_ms"], change[key]["percent"])
                 if key in change else "-"
                 for key in ("interval_median_ms", "gpu_render_median_ms")]))
    for excluded in report["excluded"]:
        lines.append("excluded\t%s\t%s" % (excluded["run"], excluded["reason"]))
    return lines


def command_report(args):
    report = build_report(args.out)
    (Path(args.out) / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    lines = report_lines(report)
    (Path(args.out) / "report.tsv").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    run = commands.add_parser("run", help="play the sweep and report it")
    run.add_argument("--out", required=True, help="new evidence directory")
    run.add_argument("--variant", action="append", required=True,
                     help="NAME=ARG,ARG...; the first is the baseline")
    run.add_argument("--resolution", action="append",
                     help="WIDTHxHEIGHT (repeatable; default 1024x768 to 3840x2160)")
    run.add_argument("--repeats", type=int, default=2)
    run.add_argument("--warmup", action="store_true",
                     help="play the baseline once first, unrecorded (warm shader caches)")
    run.add_argument("--retries", type=int, default=0,
                     help="replay a run up to N times while another game process shares the host")
    run.add_argument("--workload", type=Path, default=DEFAULT_WORKLOAD)
    run.add_argument("--kiln-profile", help="kiln profile (default: the workload's)")
    run.add_argument("--flavor", default="dev", help="the profile's build flavor")
    run.add_argument("--runtime", type=Path, required=True,
                     help="private runtime, packaged beforehand (kiln package <profile> "
                          "--runtime DIR)")
    run.set_defaults(handler=command_run)
    report = commands.add_parser("report", help="report an existing sweep")
    report.add_argument("out")
    report.set_defaults(handler=command_report)
    args = parser.parse_args(argv)
    try:
        return args.handler(args)
    except SweepError as error:
        print("resolution_sweep: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
