#!/usr/bin/env python3
"""Hammer map-building loop suite: author -> save -> compile -> play.

Each case drives the headless Hammer host (hammer_cli) with a versioned command
script from quality/workloads/hammer-loop-v1, then compiles the saved VMF and
boots it with tools/quality/vmf_map_build.py. The same command layer backs the
GTK editor, UI-driven tests and an MCP server, so this suite checks the
authority every front end uses.

  hammer_loop.py --cli build-r03-tools/hammer/hammer_cli --out quality-results/hammer-loop

Cases:
  room  a sealed room with a player start and a light: it must save, compile
        without a leak, and boot with the map active, a player spawned and a
        lit scene captured.
  leak  the same room without its north wall: the compile must stop at the
        leak gate (negative control for the leak check).
  dark  the room without a light: it compiles and boots, and the room's capture
        must be at least 1.3x brighter than this one, so the placed light
        demonstrably reaches the rendered scene.

Results are reported as checks-v1 (tools/quality/conformance_result.py), with
the elapsed author -> playable time of the room case.
"""

import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import vmf_map_build  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = HERE.parents[1]
WORKLOAD = ROOT / "quality/workloads/hammer-loop-v1"


LIT_RATIO = 1.3  # the lit room's capture must be this much brighter than the unlit one


def capture_mean(record):
    """Mean channel value of a boot's last screenshot (uncompressed TGA), or None."""
    shots = ( record or {} ).get("boot", {}).get("screenshots") or []
    if not shots:
        return None
    data = open(shots[-1]["path"], "rb").read()
    if len(data) < 18 or data[2] != 2:  # uncompressed true-colour only
        return None
    pixels = data[18 + data[0]:]
    return sum(pixels) / max(len(pixels), 1)


def author(cli, script, work):
    """Run a command script through hammer_cli with `work` as its file root."""
    work.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(cli), "--script", str(script), "--root", str(work)],
                            capture_output=True, text=True)
    (work / "hammer_cli.log").write_text(result.stdout + result.stderr)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--cli", type=Path, required=True, help="the built hammer_cli")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--workload", type=Path, default=WORKLOAD)
    parser.add_argument("--toolchain", type=Path, default=vmf_map_build.TOOLCHAIN)
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime")
    parser.add_argument("--boot-runtime", type=Path, default=ROOT / "run/runtime-native")
    parser.add_argument("--no-boot", action="store_true",
                        help="compile only (no GPU runtime); the boot checks are skipped")
    args = parser.parse_args()

    checks = Checks()
    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    out = args.out.resolve()
    results = {}

    def case(name, vmf_name, boot):
        work = out / name
        started = time.monotonic()
        authored = author(args.cli.resolve(), args.workload / (name + ".hcmd"), work / "author")
        checks.check(authored.returncode == 0, name + ".authored", authored.stderr.strip())
        vmf = work / "author" / vmf_name
        checks.check(vmf.is_file(), name + ".saved", str(vmf))
        if not vmf.is_file():
            return None
        record = vmf_map_build.build(vmf, work / "map", tools, args.runtime.resolve(),
                                     name="hammer_loop_" + name)
        if boot and record["status"] == "pass" and not args.no_boot:
            record["boot"] = vmf_map_build.boot(record, work / "map", args.boot_runtime.resolve())
            vmf_map_build.finish(work / "map", record)
        record["elapsed_seconds"] = round(time.monotonic() - started, 2)
        results[name] = record
        return record

    room = case("room", "room.vmf", boot=True)
    if room:
        checks.equal(room["status"], "pass", "room.compiled")
        checks.equal(room.get("leaked"), False, "room.sealed")
        if not args.no_boot:
            boot = room.get("boot", {})
            checks.equal(boot.get("status"), "pass", "room.booted")
            checks.check(not boot.get("failures"), "room.boot-clean", "; ".join(boot.get("failures", [])))

    leak = case("leak", "leak.vmf", boot=False)
    if leak:
        checks.equal(leak["status"], "leak", "leak.detected")
        checks.check(not (out / "leak/map/content").exists(), "leak.not-packaged")

    if not args.no_boot:
        dark = case("dark", "dark.vmf", boot=True)
        if dark:
            checks.equal(dark["status"], "pass", "dark.compiled")
            # The light the room script places must reach the captured scene:
            # the unlit control has the same geometry and view.
            lit, unlit = capture_mean(room), capture_mean(dark)
            checks.check(lit is not None and unlit is not None and lit >= LIT_RATIO * unlit,
                         "room.lit-vs-dark", "lit %s, unlit %s, need x%.1f" % (lit, unlit, LIT_RATIO))
            if room is not None:
                room.setdefault("capture_mean", lit)
            dark["capture_mean"] = unlit

    summary = {"schema": "hammer-loop/v1",
               "cases": {name: {k: r.get(k) for k in ("status", "failed_gates", "leaked",
                                                      "elapsed_seconds", "capture_mean", "boot")}
                         for name, r in results.items()}}
    (out / "hammer-loop.json").parent.mkdir(parents=True, exist_ok=True)
    (out / "hammer-loop.json").write_text(json.dumps(summary, indent=2) + "\n")
    if room:
        print("hammer_loop: room author -> playable in %.1f s" % room["elapsed_seconds"])
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
