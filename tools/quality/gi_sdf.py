#!/usr/bin/env python3
"""RFC 0011 G6 evidence for the SDF-traced producer, in game.

    python3 tools/quality/gi_sdf.py door --out quality-results/rfc0011-g6/door

`door` (G6.2): the door fixture's map (quality-results/rfc0011-maps/door, or
--map-build) carries the SDFV and the door as a moving brush entity (`Door`,
a func_brush open at spawn). Each capture boots the map headless on native
Vulkan (gi_runtime.py capture), selects a producer, and screenshots the
indirect-light view of the far room; gi_runtime.py compare judges it against
the Cycles indirect-only reference of the state:

  sdf-open        SDF, door open: must match `open`.
  sdf-closed      SDF, `ent_fire Door Enable` CLOSE_FRAMES before the
                  screenshot (the declared 60 frames plus the input's
                  server-to-client latency): must match `closed`.
  radiosity-closed  radiosity, door closed: must NOT match `closed` (it
                  claims no GeometryMotion); the scenario tells them apart.
  baked-closed    baked, door closed: must NOT match `closed`.

The closed room is dark throughout (its reference is 0), so `closed` is
judged at the `open` state's light level (--level-state open) with the
producer's declared response tolerance (0.1) as the absolute allowance; the
relative tolerance is the world gate's 0.1. Dynamic-model regions are gated.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gi_runtime  # noqa: E402

MAPS = ROOT / "quality-results" / "rfc0011-maps"
# After the camera placement (gi_runtime.PLACEMENT_FRAMES), the producer is
# selected and runs WARM_FRAMES (past its warmupFrames, 72, and the switch's
# fade) before the door closes; the screenshot follows CLOSE_FRAMES later.
WARM_FRAMES = 150
CLOSE_FRAMES = 64
RESPONSE_TOLERANCE = 0.1  # SdfTracedProducer::Caps().responseTolerance


def door(args):
    map_build = Path(args.map_build or MAPS / "door")
    out = Path(args.out)
    close = "wait %d; ent_fire Door Enable" % WARM_FRAMES
    capture_wait = gi_runtime.PLACEMENT_FRAMES + WARM_FRAMES + CLOSE_FRAMES
    captures = {
        "sdf-open": ("open", "r_indirect_report 1; r_indirect_producer sdf; wait %d" %
                     WARM_FRAMES, True),
        "sdf-closed": ("closed", "r_indirect_report 1; r_indirect_producer sdf; " + close, True),
        "radiosity-closed": ("closed", "r_indirect_producer radiosity; " + close, False),
        "baked-closed": ("closed", "r_indirect_producer baked; " + close, False),
    }
    results, passed = {}, True
    for name, (state, command, should_pass) in captures.items():
        target = out / name
        capture = [sys.executable, HERE / "gi_runtime.py", "capture", "--fixture", "door",
                   "--map-build", map_build, "--out", target, "--build", args.build,
                   "--console-command", command, "--capture-wait", str(capture_wait)]
        if args.runtime:
            capture += ["--runtime", args.runtime]
        booted = subprocess.run([str(part) for part in capture], capture_output=True, text=True,
                                timeout=1200).returncode == 0
        compare = subprocess.run(
            [sys.executable, str(HERE / "gi_runtime.py"), "compare", "--fixture", "door",
             "--state", state, "--capture", str(target), "--out", str(target / "gate.json"),
             "--gate-models", "--level-state", "open",
             "--absolute-fraction", str(RESPONSE_TOLERANCE)],
            capture_output=True, text=True, timeout=600)
        gate = json.loads((target / "gate.json").read_text()) \
            if (target / "gate.json").is_file() else {}
        switched = producer_log(target)
        ok = booted and (gate.get("status") == "pass") == should_pass
        passed &= ok
        results[name] = {"state": state, "command": command, "booted": booted,
                         "expected": "pass" if should_pass else "fail",
                         "compare": gate.get("status"), "status": "pass" if ok else "fail",
                         "producer_log": switched, "compare_log": compare.stdout[-2000:]}
        print("%-17s %-6s expected %-4s compare %-4s -> %s" % (
            name, state, results[name]["expected"], gate.get("status"), results[name]["status"]))
    record = {"gate": "G6.2 door", "status": "pass" if passed else "fail",
              "build": args.build, "close_frames_before_capture": CLOSE_FRAMES,
              "response_tolerance": RESPONSE_TOLERANCE, "captures": results}
    out.mkdir(parents=True, exist_ok=True)
    (out / "door.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("G6.2 door: %s" % record["status"])
    return 0 if passed else 1


def producer_log(target):
    """The engine's indirect-light lines (producer switches, proxies)."""
    lines = []
    for log in sorted(Path(target).glob("*/runtime/engine.log")):
        for line in log.read_text(errors="replace").splitlines():
            if "indirect light:" in line and "indirect light: frame" not in line:
                lines.append(line.strip())
    return lines[-12:]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("door")
    command.add_argument("--map-build", type=Path)
    command.add_argument("--out", required=True)
    command.add_argument("--build", default="build",
                         help="the client build tree carrying the producer")
    command.add_argument("--runtime", help="base runtime (a private copy is booted)")
    args = parser.parse_args()
    return {"door": door}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
