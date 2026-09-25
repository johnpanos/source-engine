#!/usr/bin/env python3
"""RFC 0011 G10 evidence: light through an open portal pair (fixture
`portal-light`), in game.

    python3 tools/quality/gi_portal.py --out quality-results/rfc0011-g10/gate

The map (quality-results/rfc0011-maps/portal-light, or --map-build) has room
A, lit by a ceiling panel, and room B, with no light of its own, sealed and
6 m apart; a map-placed, linked, activated prop_portal pair (`PortalA` on A's
east wall, `PortalB` on B's west wall) is their only connection. Room B's
bake is black. The Cycles reference joins the rooms at one wall plane with a
portal-sized opening (`open`), or plugs it (`closed`).

Each capture views room B from the `room` camera (judged against the joined
`joined` reference camera) in the diffuse view (mat_indirect_view 3: the
bake, the producer's change and unbaked lights, no albedo), after the
producer's warm-up and its 64-frame convergence:

  <producer>-open             the pair open: every world region of room B
                              must match Cycles `open` (DiffDir + DiffInd)
  <producer>-closed           `ent_fire PortalA SetActivatedState 0`: room B
                              must be dark, judged at `open`'s light level
  radiosity-open, -closed     radiosity's portal links (the patches and the
                              SDFV lights beyond, sampled at their images):
                              must match `open` and go dark closed
  baked-open                  the bake carries no light through portals:
                              must fail `open`
  <producer>-open-no-portals  r_indirect_portals 0 (the producer is not given
                              the pair): must fail `open`

The producers under test are sdf and, with --producer rayquery, the
ray-query one. Models are measured, not gated (a model's view is its ambient
cube, which carries the same probe light).
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
WARM_FRAMES = 150
CHANGE_FRAMES = 64
RESPONSE_TOLERANCE = 0.1


def select(producer):
    return "r_indirect_report 1; r_indirect_producer %s" % producer


def capture(args, name, command):
    target = Path(args.out) / name
    run = [sys.executable, HERE / "gi_runtime.py", "capture", "--fixture", "portal-light",
           "--camera", "room", "--map-build", Path(args.map_build or MAPS / "portal-light"),
           "--out", target, "--build", args.build, "--console-command", command,
           "--capture-wait", str(gi_runtime.PLACEMENT_FRAMES + WARM_FRAMES + CHANGE_FRAMES),
           "--view", "3"]
    booted = subprocess.run([str(part) for part in run], capture_output=True, text=True,
                            timeout=1800).returncode == 0
    return target, booted


def compare(target, state):
    command = [sys.executable, str(HERE / "gi_runtime.py"), "compare", "--fixture",
               "portal-light", "--state", state, "--capture", str(target), "--out",
               str(target / "gate.json"), "--absolute-fraction", str(RESPONSE_TOLERANCE),
               "--reference-light", "diffuse", "--gate-world", "--level-state", "open"]
    subprocess.run(command, capture_output=True, text=True, timeout=600)
    return json.loads((target / "gate.json").read_text()) \
        if (target / "gate.json").is_file() else {}


def regions(gate):
    return {region: {"measured": entry["measured_luminance"],
                     "reference": entry["reference_luminance"], "status": entry["status"]}
            for view in gate.get("views", {}).values() for region, entry in view.items()
            if "measured_luminance" in entry}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--map-build", type=Path)
    parser.add_argument("--out", required=True)
    parser.add_argument("--build", default="build")
    parser.add_argument("--producer", choices=("sdf", "rayquery"), default="sdf")
    args = parser.parse_args()
    producer = args.producer
    settle = "wait %d" % (WARM_FRAMES + CHANGE_FRAMES)
    close = "wait %d; ent_fire PortalA SetActivatedState 0; wait %d" % (WARM_FRAMES,
                                                                         CHANGE_FRAMES)
    captures = {
        producer + "-open": ("open", "%s; %s" % (select(producer), settle), True),
        producer + "-closed": ("closed", "%s; %s" % (select(producer), close), True),
        "baked-open": ("open", "%s; %s" % (select("baked"), settle), False),
        "radiosity-open": ("open", "%s; %s" % (select("radiosity"), settle), True),
        "radiosity-closed": ("closed", "%s; %s" % (select("radiosity"), close), True),
        producer + "-open-no-portals": (
            "open", "r_indirect_portals 0; %s; %s" % (select(producer), settle), False),
    }
    results, passed = {}, True
    for name, (state, command, should_pass) in captures.items():
        target, booted = capture(args, name, command)
        gate = compare(target, state)
        ok = booted and bool(gate) and (gate.get("status") == "pass") == should_pass
        passed &= ok
        log = sorted(Path(target).glob("*/runtime/engine.log"))
        portals = [line.strip() for line in (log[0].read_text(errors="replace").splitlines()
                                             if log else []) if "open portal" in line]
        results[name] = {"state": state, "command": command, "booted": booted,
                         "expected": "pass" if should_pass else "fail",
                         "compare": gate.get("status"), "regions": regions(gate),
                         "failures": gate.get("failures", []), "portal_log": portals[-4:],
                         "status": "pass" if ok else "fail"}
        print("%-26s %-6s expected %-4s compare %-4s -> %s" % (
            name, state, results[name]["expected"], gate.get("status"), results[name]["status"]))
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    gate = "G10 portal-light (%s)" % producer
    record = {"gate": gate, "status": "pass" if passed else "fail", "producer": producer,
              "build": args.build, "captures": results, "warm_frames": WARM_FRAMES,
              "change_frames": CHANGE_FRAMES, "response_tolerance": RESPONSE_TOLERANCE}
    (out / "gate.json").write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("%s: %s" % (gate, record["status"]))
    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())
