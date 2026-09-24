#!/usr/bin/env python3
"""Drop-test the collision of a PBRT map in the running game.

The collision step records every horizontal brush top ("walkable tops") and
the spawn floor. This drops the player (gravity on, noclip off) a short height
above the highest tops and the spawn, waits, and reads the resting eye height
with `getpos`. Each probe must come to rest on its top, so a stair tread
without collision (the player falls to the floor below) or a floor that lets
the player through fails.

`commands()` builds one console line: the engine honors `wait` only within a
single script line, so a multi-line cfg would run every `getpos` at once.
"""

import argparse
import json
import re
from pathlib import Path

EYE_HEIGHT = 64.0
DROP_HEIGHT = 12.0
SETTLE_FRAMES = 240
PROBES = 3
TOLERANCE = 2.0


def probes(receipt, count=PROBES):
    """The `count` highest walkable tops plus the spawn floor."""
    tops = sorted(receipt.get("walkable_tops", []), key=lambda top: -top["z"])[:count]
    spawn = receipt["spawn"]
    result = [{"name": "top-%d" % index, "x": top["x"], "y": top["y"], "z": top["z"]}
              for index, top in enumerate(tops)]
    result.append({"name": "spawn-floor", "x": spawn["origin"][0], "y": spawn["origin"][1],
                   "z": spawn["floor_z"]})
    return result


def commands(receipt):
    parts = ["r_worldmesh_draw 2", "wait 60"]
    for probe in probes(receipt):
        parts += ["cmd setpos %.2f %.2f %.2f" % (probe["x"], probe["y"], probe["z"] + DROP_HEIGHT),
                  "wait %d" % SETTLE_FRAMES, "getpos"]
    return ["; ".join(parts)]


def evaluate(console, receipt):
    """Match getpos lines after the settle waits to the probes, in order."""
    positions = [tuple(float(value) for value in match) for match in re.findall(
        r"^setpos (-?[\d.]+) (-?[\d.]+) (-?[\d.]+);setang", console, re.M)]
    expected = probes(receipt)
    # The first getpos lines may be the spawn view printed before the drops
    # settle; the last len(expected) readings belong to the probes.
    readings = positions[-len(expected):] if len(positions) >= len(expected) else []
    results = []
    for probe, reading in zip(expected, readings):
        feet = reading[2] - EYE_HEIGHT
        near = abs(reading[0] - probe["x"]) < 1.0 and abs(reading[1] - probe["y"]) < 1.0
        rested = abs(feet - probe["z"]) <= TOLERANCE
        results.append({**probe, "resting_feet_z": round(feet, 2), "horizontal_match": near,
                        "rested_on_top": rested and near})
    passed = len(results) == len(expected) and all(item["rested_on_top"] for item in results)
    return {"scope": "pbrt-collision-drop-test", "status": "pass" if passed else "fail",
            "probes": results, "getpos_lines": len(positions)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--collision-receipt", type=Path, required=True)
    parser.add_argument("--boot-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    receipt = json.loads(args.collision_receipt.read_text())
    boot = json.loads(args.boot_evidence.read_text())
    if boot.get("status") != "pass":
        raise SystemExit("traversal boot did not pass")
    config = args.boot_evidence.parent / "runtime/portal/cfg/portal_boot_commands.cfg"
    if config.read_text().splitlines() != commands(receipt):
        raise SystemExit("traversal boot did not run the probe commands for this receipt")
    console = (args.boot_evidence.parent / "runtime/portal/console.log").read_text(
        errors="replace")
    result = evaluate(console, receipt)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"status": result["status"],
                      "probes": [(p["name"], p["z"], p["resting_feet_z"]) for p in
                                 result["probes"]]}))
    if result["status"] != "pass":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
