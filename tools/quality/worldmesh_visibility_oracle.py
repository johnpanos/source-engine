#!/usr/bin/env python3
"""Prove WMSH meshlet visibility culling changes no pixels.

Runs the same camera sweep three times on the installed product through the
frame pacing harness (`frame_pacing.py`, which owns staging, launch and
command chaining):

    reference  r_worldmesh_cull 0: every meshlet drawn
    candidate  r_worldmesh_cull 1: leaf visibility and view frustum
    negative   r_worldmesh_cull 2: frustum culling with half-size spheres

Every candidate screenshot must be byte-identical to its reference, and the
negative control must differ in at least one view, or the sweep cannot see
culling errors. The sweep flies (noclip) through a grid of eye positions
inside the collision interior, looking in eight directions level and pitched,
plus straight up and down. Frame stats of each run are summarized so the
evidence also shows what culling saves.

    python3 tools/quality/worldmesh_visibility_oracle.py \\
        --runtime run/runtime-native --build build-rfc0008-staircase-lit-client \\
        --content-root quality-results/bedroom-map/content --map bedroom \\
        --collision-receipt quality-results/bedroom-map/collision/collision-receipt.json \\
        --out /tmp/claude-1000/wvo/bedroom
"""

import argparse
import hashlib
import json
from pathlib import Path
import statistics
import subprocess
import sys

HERE = Path(__file__).resolve().parent
MODES = {"reference": 0, "candidate": 1, "negative": 2}
EYE_HEIGHT = 64
MARGIN = 24


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def views(interior, grid):
    """(x, y, z origin, pitch, yaw) camera poses inside the interior box."""
    x0, y0, z0, x1, y1, z1 = interior
    z = min(z0 + EYE_HEIGHT * 0.75, (z0 + z1) / 2) - EYE_HEIGHT
    result = []
    for i in range(grid):
        for j in range(grid):
            x = x0 + MARGIN + (x1 - x0 - 2 * MARGIN) * (i + 0.5) / grid
            y = y0 + MARGIN + (y1 - y0 - 2 * MARGIN) * (j + 0.5) / grid
            for yaw in range(0, 360, 45):
                result.append((x, y, z, 0, yaw))
                result.append((x, y, z, 35, yaw))
            result.append((x, y, z, -89, 0))
            result.append((x, y, z, 89, 0))
    return result


def scenario(map_name, mode, poses):
    body = []
    for x, y, z, pitch, yaw in poses:
        body += ["setpos %.1f %.1f %.1f" % (x, y, z), "setang %d %d 0" % (pitch, yaw),
                 "wait 12", "screenshot", "wait 4"]
    return {"schema": "frame-pacing-scenario/v1", "id": "worldmesh-visibility-%s" % mode,
            "description": "WMSH visibility oracle sweep (r_worldmesh_cull %d)" % MODES[mode],
            "map": map_name, "passes": 1, "host_framerate": 60,
            "setup": ["r_worldmesh_draw 2", "r_worldmesh_cull %d" % MODES[mode], "net_graph 0",
                      "cl_showfps 0", "noclip", "notarget", "wait 90"],
            "body": body, "budgets": {}}


def frame_summary(stats):
    frames = [json.loads(line) for line in Path(stats).read_text().splitlines()
              if line.startswith('{"f"')]
    frames = frames[len(frames) // 4:]
    gpu = [frame["gpu"][1] for frame in frames if "gpu" in frame]
    return {"frames": len(frames),
            "median_interval_ms": round(statistics.median(f["interval"] for f in frames) / 1000, 3),
            "median_gpu_ms": round(statistics.median(gpu) / 1000, 3) if gpu else None,
            "median_draw_records": statistics.median(f["records"] for f in frames)}


def run(args, mode, poses):
    out = args.out / mode
    scenario_path = args.out / ("%s.json" % mode)
    scenario_path.write_text(json.dumps(scenario(args.map, mode, poses), indent=1) + "\n")
    command = [sys.executable, HERE / "frame_pacing.py", "--runtime", args.runtime,
               "--build", args.build, "--content-root", args.content_root,
               "--scenario", scenario_path, "--out", out, "--fps-max", "300",
               "--timeout", str(args.timeout)]
    result = subprocess.run([str(part) for part in command], capture_output=True, text=True)
    evidence = json.loads((out / "evidence.json").read_text()) if (
        out / "evidence.json").is_file() else {}
    shots = sorted((out / "runtime/portal/screenshots").glob("*.tga"))
    return {"returncode": result.returncode, "status": evidence.get("status"),
            "failures": evidence.get("failures", []), "screenshots": shots,
            "frames": frame_summary(out / "frame-stats.jsonl")
            if (out / "frame-stats.jsonl").is_file() else None}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--content-root", type=Path, required=True)
    parser.add_argument("--map", required=True)
    parser.add_argument("--collision-receipt", type=Path, required=True,
                        help="pbrt_collision_vmf receipt; its interior bounds the sweep")
    parser.add_argument("--grid", type=int, default=3, help="eye positions per axis")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    args.out = args.out.resolve()
    args.out.mkdir(parents=True, exist_ok=False)
    interior = json.loads(args.collision_receipt.read_text())["interior_source_units"]
    poses = views(interior, args.grid)
    runs = {mode: run(args, mode, poses) for mode in MODES}
    failures = ["%s run: %s" % (mode, "; ".join(item["failures"]) or "exit %d" % item["returncode"])
                for mode, item in runs.items() if item["status"] != "pass"]
    counts = {mode: len(item["screenshots"]) for mode, item in runs.items()}
    if any(count != len(poses) for count in counts.values()):
        failures.append("expected %d screenshots per run, got %s" % (len(poses), counts))
    pairs = list(zip(poses, runs["reference"]["screenshots"], runs["candidate"]["screenshots"],
                     runs["negative"]["screenshots"]))
    mismatched = [{"pose": pose, "reference": str(ref), "candidate": str(cand)}
                  for pose, ref, cand, _ in pairs if sha256(ref) != sha256(cand)]
    negative_differs = sum(1 for _, ref, _, neg in pairs if sha256(ref) != sha256(neg))
    if mismatched:
        failures.append("%d of %d views differ with culling on" % (len(mismatched), len(pairs)))
    if not negative_differs:
        failures.append("negative control changed no view: the sweep cannot detect culling errors")
    evidence = {"schema": "worldmesh-visibility-oracle/v1",
                "status": "fail" if failures else "pass", "failures": failures,
                "map": args.map, "build": str(args.build.resolve()),
                "content_root": str(args.content_root.resolve()),
                "interior_source_units": interior, "views": len(poses),
                "identical_views": len(pairs) - len(mismatched),
                "negative_control_differing_views": negative_differs,
                "first_mismatches": mismatched[:5],
                "frames": {mode: item["frames"] for mode, item in runs.items()}}
    (args.out / "evidence.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print(json.dumps({key: evidence[key] for key in
                      ("status", "failures", "views", "identical_views",
                       "negative_control_differing_views", "frames")}, indent=2))
    if failures:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
