#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Collect P2:CE netconsole markers and analyze per-present MangoHud CSV.

This measures presentation intervals, never GPU execution or shader/pass time.
The native application and its private runtime must already be running.
See render_profile.md for setup and timing limitations.
"""

import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import socket
import sys
import time

import render_profile


def collect(args):
    args.out.mkdir(parents=True, exist_ok=False)
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as control:
        control.settimeout(2)
        control.connect("\0" + args.control)
        with socket.create_connection(("127.0.0.1", args.port), timeout=2) as net:
            net.settimeout(.1)
            start = time.time_ns()
            control.sendall(b":logging=1;")
            clock = {"schema": "present-clock/v1", "logging_start_command_ns": start,
                     "clock": "CLOCK_REALTIME", "port": args.port, "control": args.control,
                     "script": args.script, "settings": args.settings,
                     "semantics": "logger starts on a subsequent present; no GPU clock correlation"}
            (args.out / "clock.json").write_text(json.dumps(clock, indent=2) + "\n")
            net.sendall(("exec %s\ncvarlist r_clustered\nscript_execute %s\n" %
                         (args.settings, args.script)).encode())
            pending = b""
            deadline = time.monotonic() + args.timeout
            with (args.out / "marks.jsonl").open("w") as marks, (args.out / "netconsole.log").open("wb") as log:
                while time.monotonic() < deadline:
                    try:
                        chunk = net.recv(65536)
                    except TimeoutError:
                        continue
                    if not chunk:
                        break
                    log.write(chunk)
                    log.flush()
                    pending += chunk
                    lines = pending.split(b"\n")
                    pending = lines.pop()
                    for line in lines:
                        text = line.decode("utf-8", errors="replace").strip()
                        if "PROFILE_MARK" in text or "QA_" in text:
                            marks.write(json.dumps({"observed_ns": time.time_ns(), "line": text}) + "\n")
                            marks.flush()
                            if "QA_DONE" in text:
                                print(text)
                                if not re.search(r"checks=4 failures=0\b", text):
                                    raise render_profile.ProfileError("camera checks failed")
                                return
            raise render_profile.ProfileError("no completed camera workload")


def analyze(csv_path, marks_path, clock_path, guard=.5):
    clock = json.loads(clock_path.read_text(), object_pairs_hook=render_profile.unique_object)
    if (not isinstance(clock, dict) or clock.get("schema") != "present-clock/v1" or not isinstance(clock.get("logging_start_command_ns"), int)
            or isinstance(clock["logging_start_command_ns"], bool)
            or clock["logging_start_command_ns"] <= 0):
        raise render_profile.ProfileError("missing logging clock receipt")
    if not .1 <= guard <= 2:
        raise render_profile.ProfileError("phase edge guard must be between 0.1 and 2 seconds")
    marks, checks, done, previous = {}, set(), False, 0
    mark_text = marks_path.read_text()
    if not mark_text.endswith("\n"):
        raise render_profile.ProfileError("truncated console observation")
    for line in mark_text.splitlines():
        row = json.loads(line, object_pairs_hook=render_profile.unique_object)
        if not isinstance(row, dict):
            raise render_profile.ProfileError("invalid console observation")
        stamp = row.get("observed_ns")
        if not isinstance(stamp, int) or isinstance(stamp, bool) or stamp < previous:
            raise render_profile.ProfileError("invalid or unordered marker clock")
        previous = stamp
        text = row.get("line", "")
        if not isinstance(text, str):
            raise render_profile.ProfileError("invalid console observation")
        mark = re.search(r"PROFILE_MARK (\w+)", text)
        if mark:
            if mark[1] in marks:
                raise render_profile.ProfileError("duplicate phase marker")
            marks[mark[1]] = stamp
        check = re.search(r"QA_CHECK .*\.(map.loaded|view.arrival|view.reverse|view.return) (PASS|FAIL)\b", text)
        if check:
            if check[2] != "PASS" or check[1] in checks:
                raise render_profile.ProfileError("failed or duplicated camera check")
            checks.add(check[1])
        if re.search(r"QA_CHECK .* FAIL\b", text):
            raise render_profile.ProfileError("failed camera check")
        if "QA_DONE" in text:
            if done:
                raise render_profile.ProfileError("duplicate completion record")
            done = bool(re.search(r"checks=4 failures=0\b", text))
    required = ("floor_begin", "arrival", "reverse", "return", "floor_end")
    if (not done or len(checks) != 4 or any(name not in marks for name in required)
            or any(marks[b] < marks[a] for a, b in zip(required, required[1:]))):
        raise render_profile.ProfileError("incomplete camera checks or phase markers")
    start = clock["logging_start_command_ns"]
    samples = []
    previous = -1
    if not csv_path.read_bytes().endswith(b"\n"):
        raise render_profile.ProfileError("truncated CSV record")
    with csv_path.open(newline="") as stream:
        info_keys = next(csv.reader(stream))
        info_values = next(csv.reader(stream))
        if (not info_keys or len(info_keys) != len(info_values)
                or len(set(info_keys)) != len(info_keys)
                or not {"gpu", "driver"}.issubset(info_keys)):
            raise render_profile.ProfileError("invalid device metadata")
        reader = csv.DictReader(stream)
        columns = ("frametime", "elapsed", "gpu_core_clock", "gpu_temp", "gpu_vram_used")
        if (not reader.fieldnames or len(set(reader.fieldnames)) != len(reader.fieldnames)
                or any(name not in reader.fieldnames for name in columns)):
            raise render_profile.ProfileError("not a per-present MangoHud CSV")
        for row in reader:
            if None in row or any(value is None for value in row.values()):
                raise render_profile.ProfileError("truncated CSV row")
            values = {name: float(row[name]) for name in columns}
            elapsed = values["elapsed"]
            if (not all(render_profile.number(value) for value in values.values())
                    or values["frametime"] <= 0 or elapsed <= previous or elapsed != int(elapsed)):
                raise render_profile.ProfileError("invalid duration or unordered presentation clock")
            previous = elapsed
            samples.append((start + int(elapsed), values))
    phases = {}
    for name, end in (("arrival", "reverse"), ("reverse", "return"), ("return", "floor_end")):
        selected = [row for stamp, row in samples if marks[name] + guard * 1e9 <= stamp <= marks[end] - guard * 1e9]
        if not selected:
            raise render_profile.ProfileError("missing presentation samples in " + name)
        phases[name] = {"presentation_interval": render_profile.metric([row["frametime"] for row in selected]),
                        "gpu_clock_mean_mhz": sum(row["gpu_core_clock"] for row in selected) / len(selected),
                        "gpu_temperature_mean_c": sum(row["gpu_temp"] for row in selected) / len(selected),
                        "gpu_vram_mean_gib": sum(row["gpu_vram_used"] for row in selected) / len(selected)}
    return {"schema": "external-present-profile/v1", "status": "complete", "phases": phases,
            "device": dict(zip(info_keys, info_values)), "phase_edge_guard_seconds": guard,
            "semantics": "per-present intervals; GPU execution and pass breakdown unavailable; "
                         "host-observed console marks; logger command clock; excludes phase edges; "
                         "contextual comparison, not equivalent-output or hard-budget acceptance",
            "sources": {str(path.resolve()): hashlib.sha256(path.read_bytes()).hexdigest()
                        for path in (csv_path, marks_path, clock_path)}}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    collect_parser = sub.add_parser("collect")
    collect_parser.add_argument("--port", type=int, required=True)
    collect_parser.add_argument("--control", required=True)
    collect_parser.add_argument("--script", required=True)
    collect_parser.add_argument("--settings", required=True)
    collect_parser.add_argument("--out", type=Path, required=True)
    collect_parser.add_argument("--timeout", type=float, default=90)
    analyze_parser = sub.add_parser("analyze")
    analyze_parser.add_argument("csv", type=Path)
    analyze_parser.add_argument("--marks", type=Path, required=True)
    analyze_parser.add_argument("--clock", type=Path, required=True)
    analyze_parser.add_argument("--guard", type=float, default=.5)
    analyze_parser.add_argument("--json", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        if args.mode == "collect":
            if not all(re.fullmatch(r"[A-Za-z0-9_/.-]+", value) for value in (args.script, args.settings)):
                raise render_profile.ProfileError("invalid script or settings path")
            collect(args)
        else:
            report = analyze(args.csv, args.marks, args.clock, args.guard)
            args.json.write_text(json.dumps(report, indent=2) + "\n")
            print("phase\tmetric\tmean_ms\tmedian_ms\tp99_ms\tframes")
            for name, phase in report["phases"].items():
                value = phase["presentation_interval"]
                print("%s\tpresent_interval\t%.4f\t%.4f\t%.4f\t%d" %
                      (name, value["mean_ms"], value["median_ms"], value["p99_ms"], value["samples"]))
        return 0
    except (OSError, ValueError, StopIteration) as error:
        parser.exit(2, "p2ce_present: %s\n" % error)


if __name__ == "__main__":
    sys.exit(main())
