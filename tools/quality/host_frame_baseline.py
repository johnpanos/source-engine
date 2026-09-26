#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Versioned legacy host-frame capture (RFC 0003 Phase A, roadmap R10).

The fixture in quality/fixtures/host-frame/ is a -hostframetrace capture of the
legacy hand-ordered host frame (host_frame_graph 0) on a fixed, deterministic
run, with a manifest of how it was taken. `check` boots the installed Portal
product in both host frame modes and compares each run to the fixture with
tools/quality/host_frame_capture.py (exact, except the manifest's declared
numeric tolerances), reporting checks-v1. `record` takes a new capture for a
reviewed fixture update; never re-record merely to make `check` pass.

  host_frame_baseline.py check  --build build-r03-portal-native/install --out DIR
  host_frame_baseline.py record --build build-r03-portal-native/install --out DIR

The first host frame runs before any command executes, in either mode, so
frame 0 of every capture takes the default path; both paths produce the same
trace.
"""

import argparse
import gzip
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import host_frame_capture  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
FIXTURE = ROOT / "quality/fixtures/host-frame/testchmb_a_00-v1.json"
SCHEMA = "host-frame-baseline/v1"


def load_fixture(manifest_path):
    """Returns (manifest, parsed frames) and checks the manifest's counts."""
    manifest = json.loads(Path(manifest_path).read_text())
    if manifest.get("schema") != SCHEMA:
        raise host_frame_capture.CaptureError("%s: schema is not %s" % (manifest_path, SCHEMA))
    trace_path = Path(manifest_path).parent / manifest["trace"]
    text = gzip.decompress(trace_path.read_bytes()).decode()
    frames = host_frame_capture.parse(text, str(trace_path))
    events = sum(len(calls) for _, calls in frames)
    if len(frames) != manifest["frames"] or events != manifest["events"]:
        raise host_frame_capture.CaptureError(
            "%s: %d frames / %d events, manifest says %d / %d"
            % (trace_path, len(frames), events, manifest["frames"], manifest["events"]))
    return manifest, frames


def capture(manifest, build, runtime, out, graph_mode):
    """Boots the product once and returns (boot returncode, capture path)."""
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(runtime),
               "--build", str(build), "--out", str(out), "--headless",
               "--renderer", manifest["renderer"], "--map", manifest["map"],
               "--engine-arg=-hostframetrace", "--engine-arg=h.t"]
    for startup in manifest["startup_commands"] + ["host_frame_graph %d" % graph_mode]:
        command.append("--startup-command=" + startup)
    result = subprocess.run(command, capture_output=True, text=True)
    (Path(out).parent / (Path(out).name + ".log")).write_text(result.stdout + result.stderr)
    return result.returncode, Path(out) / "runtime" / "h.t"


def compare_to_fixture(manifest, frames, capture_path):
    actual = host_frame_capture.parse(Path(capture_path).read_text(), str(capture_path))
    return host_frame_capture.compare(frames, actual, 0, manifest.get("tolerances", {}))


def git_revision():
    result = subprocess.run(["git", "-C", str(ROOT), "rev-parse", "--short", "HEAD"],
                            capture_output=True, text=True)
    dirty = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain"],
                           capture_output=True, text=True).stdout.strip()
    return result.stdout.strip() + ("+dirty" if dirty else "")


def check(args):
    checks = Checks()
    try:
        manifest, frames = load_fixture(args.fixture)
    except (OSError, ValueError, KeyError) as error:
        checks.check(False, "fixture.loads", str(error))
        return checks.report()
    checks.check(True, "fixture.loads")
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    summary = {"schema": "host-frame-baseline-check/v1", "fixture": str(args.fixture), "modes": {}}
    for label, mode in (("legacy", 0), ("graph", 1)):
        code, path = capture(manifest, args.build, args.runtime, out / label, mode)
        checks.equal(code, 0, label + ".booted")
        if code != 0 or not path.is_file():
            checks.check(False, label + ".matches-fixture", "no capture at %s" % path)
            continue
        report = compare_to_fixture(manifest, frames, path)
        summary["modes"][label] = {k: report.get(k) for k in
                                   ("identical", "compared_frames", "compared_events",
                                    "tolerated_differences", "first_divergence")}
        checks.check(report["identical"], label + ".matches-fixture",
                     json.dumps(report.get("first_divergence")))
    (out / "host-frame-baseline.json").write_text(json.dumps(summary, indent=2) + "\n")
    return checks.report()


def record(args):
    manifest = json.loads(args.fixture.read_text()) if args.fixture.is_file() else {
        "schema": SCHEMA, "map": "testchmb_a_00", "renderer": "native-vulkan",
        "startup_commands": ["host_framerate 0.015", "cl_clock_correction 0"],
        "tolerances": {"ia": 0.0001}, "trace": args.fixture.stem + ".trace.gz"}
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    code, path = capture(manifest, args.build, args.runtime, out / "legacy", 0)
    if code != 0 or not path.is_file():
        print("host_frame_baseline: the legacy capture failed (see %s)" % out, file=sys.stderr)
        return 1
    text = path.read_text()
    frames = host_frame_capture.parse(text, str(path))
    manifest.update({
        "frames": len(frames), "events": sum(len(calls) for _, calls in frames),
        "recorded_revision": git_revision(),
        "note": "Legacy host frame (host_frame_graph 0) from frame 1; frame 0 precedes command "
                "execution in either mode. Update only after reviewing a behavior change; the "
                "ia tolerance absorbs the wall-clock tick remainder of frames run before "
                "host_framerate applies."})
    args.fixture.parent.mkdir(parents=True, exist_ok=True)
    (args.fixture.parent / manifest["trace"]).write_bytes(gzip.compress(text.encode(), 9, mtime=0))
    args.fixture.write_text(json.dumps(manifest, indent=2) + "\n")
    print("host_frame_baseline: recorded %d frames, %d events" % (manifest["frames"], manifest["events"]))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("check", "record"):
        p = sub.add_parser(name)
        p.add_argument("--build", type=Path, required=True, help="the installed Portal product")
        p.add_argument("--runtime", type=Path, default=ROOT / "run/runtime-native")
        p.add_argument("--out", type=Path, required=True)
        p.add_argument("--fixture", type=Path, default=FIXTURE)
    args = parser.parse_args(argv)
    return check(args) if args.command == "check" else record(args)


if __name__ == "__main__":
    sys.exit(main())
