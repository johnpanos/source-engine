#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Analyze native Vulkan frame stats and core pass reports; see render_profile.md.

GPU query results belong to gpu[0], not to the CPU frame that reads them.
CPU costs and nested core GPU scopes are inclusive; they must not be summed.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import sys

import frame_pacing


class ProfileError(ValueError):
    pass


def number(value):
    return (isinstance(value, (int, float)) and not isinstance(value, bool)
            and math.isfinite(value) and 0 <= value < 1e12)


def label(value):
    return (isinstance(value, str) and bool(value)
            and all(ord(character) >= 32 and ord(character) != 127 for character in value))


def count(value):
    return isinstance(value, int) and not isinstance(value, bool) and 0 <= value < 1e12


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ProfileError("duplicate JSON key: " + key)
        result[key] = value
    return result


def read_frames(path):
    """Strict completed JSONL, with delayed query results joined to their owner."""
    rows, gpu, header = [], {}, None
    with Path(path).open() as stream:
        for line_number, line in enumerate(stream, 1):
            if not line.endswith("\n"):
                raise ProfileError("truncated record at line %d" % line_number)
            try:
                row = json.loads(line, object_pairs_hook=unique_object)
            except json.JSONDecodeError as error:
                raise ProfileError("invalid JSON at line %d: %s" % (line_number, error)) from error
            if not isinstance(row, dict):
                raise ProfileError("record is not an object at line %d" % line_number)
            if header is None:
                if row.get("schema") != frame_pacing.STATS_SCHEMA:
                    raise ProfileError("missing " + frame_pacing.STATS_SCHEMA + " header")
                header = row
                continue
            if (not isinstance(row.get("f"), int) or isinstance(row["f"], bool)
                    or row["f"] < 1 or (rows and row["f"] != rows[-1]["f"] + 1)):
                raise ProfileError("missing, duplicated or unordered CPU frame at line %d" % line_number)
            if "mark" in row and not label(row["mark"]):
                raise ProfileError("invalid frame mark at line %d" % line_number)
            if "extent" in row and (not isinstance(row["extent"], list)
                    or len(row["extent"]) != 2
                    or not all(count(value) and value > 0 for value in row["extent"])):
                raise ProfileError("invalid drawable extent at line %d" % line_number)
            result = row.get("gpu")
            if result is not None:
                if (not isinstance(result, list) or len(result) != 3
                        or not isinstance(result[0], int) or isinstance(result[0], bool)
                        or result[0] < 1 or result[0] > row["f"]
                        or not all(number(value) for value in result[1:])
                        or result[2] > result[1]):
                    raise ProfileError("invalid GPU result in frame %s" % row["f"])
                passes = row.get("gpu_passes")
                if passes is not None:
                    names = set()
                    if not isinstance(passes, list) or not passes:
                        raise ProfileError("empty GPU pass result in frame %s" % row["f"])
                    for item in passes:
                        if (not isinstance(item, list) or len(item) != 3
                                or not label(item[0]) or item[0] in names
                                or not count(item[1]) or not number(item[2])):
                            raise ProfileError("invalid GPU pass in frame %s" % row["f"])
                        names.add(item[0])
                payload = {key: row[key] for key in ("gpu", "gpu_passes", "gpu_sequence") if key in row}
                old = gpu.get(result[0])
                if old is not None and old != payload:
                    raise ProfileError("conflicting GPU result for frame %s" % result[0])
                gpu[result[0]] = payload
            # Never leave a later frame's GPU result on this CPU row.
            rows.append({key: value for key, value in row.items()
                         if key not in ("gpu", "gpu_passes", "gpu_sequence")})
    if header is None or not rows:
        raise ProfileError("no frame records")
    for row in rows:
        row.update(gpu.get(row["f"], {}))
    return header, rows


def metric(values):
    if not values:
        return None
    return {"samples": len(values), "mean_ms": round(sum(values) / len(values), 4),
            "median_ms": round(frame_pacing.percentile(values, .5), 4),
            "p95_ms": round(frame_pacing.percentile(values, .95), 4),
            "p99_ms": round(frame_pacing.percentile(values, .99), 4),
            "max_ms": round(max(values), 4)}


def window(rows, begin, end):
    selected, phase, active, ended = [], "unmarked", begin is None, False
    for row in rows:
        marks = row.get("mark", "").split(",")
        if begin in marks:
            if active:
                raise ProfileError("duplicate begin mark " + begin)
            active = True
        for mark in marks:
            if mark and mark not in (begin, end):
                phase = mark
        if active:
            selected.append((phase, row))
        if end in marks and active:
            ended = True
            break
    if not selected or (end and not ended):
        raise ProfileError("empty or incomplete measurement bracket")
    return selected


def summarize(rows):
    for row in rows:
        for key in ("interval", "cpu", "engine", "backend"):
            if not number(row.get(key)):
                raise ProfileError("measured frame %s has no valid %s duration" % (row["f"], key))
        if not isinstance(row.get("cost"), dict):
            raise ProfileError("measured frame %s has no CPU costs" % row["f"])
        for name, cost in row["cost"].items():
            if (not label(name) or not isinstance(cost, list) or len(cost) != 2
                    or not count(cost[0]) or not number(cost[1])):
                raise ProfileError("frame %s has invalid cost %s" % (row["f"], name))
    metrics = {key: metric([row[key] / 1000 for row in rows])
               for key in ("interval", "cpu", "engine", "backend")}
    metrics["gpu_render"] = metric([row["gpu"][2] / 1000 for row in rows if "gpu" in row])
    metrics["gpu_including_present"] = metric([row["gpu"][1] / 1000 for row in rows if "gpu" in row])
    costs = {}
    for name in sorted({name for row in rows for name in row["cost"]}):
        values = [row["cost"].get(name, [0, 0]) for row in rows]
        costs[name] = {**metric([value[1] / 1000 for value in values]),
                       "count": sum(value[0] for value in values), "inclusive": True}
    return {"frames": len(rows), "first_frame": rows[0]["f"], "last_frame": rows[-1]["f"],
            "render_extents": sorted({tuple(row["extent"]) for row in rows
                                      if isinstance(row.get("extent"), list) and len(row["extent"]) == 2}),
            "metrics": metrics,
            "cpu_costs": dict(sorted(costs.items(), key=lambda item: -item[1]["mean_ms"])),
            "gpu_passes": frame_pacing.summarize_gpu_passes(rows),
            "missing_gpu_frames": [row["f"] for row in rows if "gpu" not in row],
            "missing_gpu_pass_frames": [row["f"] for row in rows if not row.get("gpu_passes")]}


CORE_HEADER = re.compile(r"cl_render_debug_stats: core GPU passes, mean of (\d+) frame\(s\):")
CORE_ROW = re.compile(r"^( +)(.*?)\s+([0-9.]+) ms  x([0-9.]+)$")


def core_reports(text):
    """Retain each aggregate window; its frame IDs/CPU-GPU correlation are unknown.

    The engine reports per-second means over completed frames, including warmup.
    Do not invent percentiles, exact phase assignments or exclusive parent time.
    """
    reports, current = [], None
    for line in text.splitlines():
        header = CORE_HEADER.search(line)
        if header:
            current = {"frames": int(header[1]), "passes": [], "overflowed": 0}
            reports.append(current)
            continue
        found = CORE_ROW.fullmatch(line)
        if current and found:
            name, ms, count = found[2].strip(), float(found[3]), float(found[4])
            if name == "(timestamps dropped)":
                current["overflowed"] += int(count)
            else:
                kind = ("cpu" if "CPU recording" in name else
                        "count" if name.startswith("shadow tiles drawn") else "gpu")
                current["passes"].append({"name": name, "depth": (len(found[1]) - 2) // 2,
                                          "mean_ms": ms, "per_frame": count, "kind": kind,
                                          "inclusive": True})
        elif current:
            current = None
    for report in reports:
        if not report["frames"] or not report["passes"]:
            raise ProfileError("empty core timer report")
    return reports


def analyze(path, console=None, begin="floor_begin", end="floor_end"):
    header, rows = read_frames(path)
    selected = window(rows, begin, end)
    phases = {}
    for phase, row in selected:
        phases.setdefault(phase, []).append(row)
    total = summarize([row for _, row in selected])
    reports = core_reports(Path(console).read_text()) if console else []
    failures = []
    if total["missing_gpu_frames"]:
        failures.append("missing GPU results for %d measured frames" % len(total["missing_gpu_frames"]))
    if total["missing_gpu_pass_frames"]:
        failures.append("missing GPU pass timers for %d measured frames; collect with --profile" %
                        len(total["missing_gpu_pass_frames"]))
    dropped = sum(report["overflowed"] for report in reports)
    if dropped:
        failures.append("core timer reports dropped %d timestamps" % dropped)
    if console and not reports:
        failures.append("no core GPU reports in the console log")
    return {"schema": "render-profile/v1", "status": "incomplete" if failures else "complete",
            "failures": failures, "device": header, "source": str(Path(path).resolve()),
            "sha256": hashlib.sha256(Path(path).read_bytes()).hexdigest(),
            "semantics": {"cpu_costs": "inclusive wall times; overlap; do not sum",
                          "gpu_passes": "non-overlapping backend segments, joined by GPU frame ID",
                          "core_reports": "nested inclusive scopes; per-window means, not percentiles; "
                                          "whole console including warmup; no exact phase correlation",
                          "cpu_gpu": "run concurrently; do not add CPU and GPU durations"},
            "summary": total, "phases": {phase: summarize(frames) for phase, frames in phases.items()},
            "core_reports": reports}


def tsv(report):
    lines = ["kind\tphase\tname\tmean_ms\tmedian_ms\tp99_ms\tcount\tdepth"]
    for phase, summary in [("all", report["summary"]), *report["phases"].items()]:
        for name, value in summary["metrics"].items():
            if value:
                lines.append("metric\t%s\t%s\t%.4f\t%.4f\t%.4f\t%d\t" %
                             (phase, name, value["mean_ms"], value["median_ms"],
                              value["p99_ms"], value["samples"]))
        for name, value in summary["cpu_costs"].items():
            lines.append("cpu_inclusive\t%s\t%s\t%.4f\t%.4f\t%.4f\t%d\t" %
                         (phase, name, value["mean_ms"], value["median_ms"],
                          value["p99_ms"], value["count"]))
        for name, value in (summary["gpu_passes"] or {}).get("segments", {}).items():
            lines.append("gpu_segment\t%s\t%s\t%.3f\t%.3f\t\t%.2f\t" %
                         (phase, name, value["mean_ms"], value["median_ms"], value["per_frame"]))
    for index, window_report in enumerate(report["core_reports"]):
        for value in window_report["passes"]:
            lines.append("core_%s_inclusive\twindow_%d\t%s\t%.3f\t\t\t%.2f\t%d" %
                         (value["kind"], index, value["name"], value["mean_ms"],
                          value["per_frame"], value["depth"]))
    return "\n".join(lines) + "\n"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("frames", type=Path)
    parser.add_argument("--console", type=Path, help="frame_floor's retained console.log")
    parser.add_argument("--begin", default="floor_begin")
    parser.add_argument("--end", default="floor_end")
    parser.add_argument("--all-frames", action="store_true", help="include loading and warmup")
    parser.add_argument("--json", type=Path, help="write reusable JSON report")
    args = parser.parse_args(argv)
    try:
        report = analyze(args.frames, args.console, None if args.all_frames else args.begin,
                         None if args.all_frames else args.end)
        if args.json:
            args.json.write_text(json.dumps(report, indent=2) + "\n")
        sys.stdout.write(tsv(report))
        for failure in report["failures"]:
            print("render_profile: " + failure, file=sys.stderr)
        return 0 if report["status"] == "complete" else 1
    except (OSError, ProfileError) as error:
        parser.exit(2, "render_profile: %s\n" % error)


if __name__ == "__main__":
    sys.exit(main())
