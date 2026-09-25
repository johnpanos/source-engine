#!/usr/bin/env python3
"""Run the versioned supplemental SH directions through Blender Cycles."""

import argparse
import hashlib
import json
import math
import os
import subprocess
from pathlib import Path

import cycles_device
from worldstage_sh_l1_plan import load_plan


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--plan", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--blender", default="blender")
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    parser.add_argument("--samples", type=int, required=True)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--device", choices=cycles_device.DEVICES,
                        default=cycles_device.CHECK_DEVICE)
    args = parser.parse_args()
    if min(args.width, args.height) < 64 or args.samples < 1 or args.timeout < 1:
        parser.error("invalid atlas dimensions, samples or timeout")
    plan, plan_hash = load_plan(args.plan)
    script = Path(__file__).with_name("worldstage_cycles_bake_preview.py")
    args.out_dir.mkdir(parents=True, exist_ok=True)
    evidence_path = args.out_dir / "supplemental-bakes.json"
    if evidence_path.exists():
        parser.error("supplemental bake evidence already exists")
    evidence = {"status": "incomplete", "scope": "cycles-supplemental-sh-bakes",
                "plan_sha256": plan_hash, "stage_sha256": sha256(args.stage),
                "manifest_sha256": sha256(args.manifest),
                "width": args.width, "height": args.height, "samples": args.samples,
                "device": args.device, "bakes": {}}
    for entry in plan["additional"]:
        identity = entry["id"]
        target = args.out_dir / (identity + ".exr")
        log = args.out_dir / (identity + ".log")
        if target.exists() or target.with_suffix(".json").exists():
            raise ValueError("supplemental bake output already exists: " + identity)
        command = [args.blender, "-b", "--factory-startup", "--python-exit-code", "9",
                   "--python", str(script), "--", "--stage", str(args.stage),
                   "--manifest", str(args.manifest), "--direction-id", identity,
                   "--normal-local", *[str(value) for value in entry["normal"]],
                   "--require-all-charts-lit", "--out", str(target),
                   "--width", str(args.width), "--height", str(args.height),
                   "--samples", str(args.samples), "--device", args.device]
        try:
            with log.open("w") as handle:
                result = subprocess.run(command, stdout=handle, stderr=subprocess.STDOUT,
                                        env=os.environ.copy(), timeout=args.timeout, check=False)
        except subprocess.TimeoutExpired as error:
            evidence["failure"] = identity + " timed out after " + str(args.timeout) + "s"
            evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
            raise RuntimeError(evidence["failure"]) from error
        if result.returncode:
            evidence["failure"] = identity + " exited " + str(result.returncode)
            evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
            raise RuntimeError(evidence["failure"])
        bake = json.loads(target.with_suffix(".json").read_text())
        if (bake["status"] != "pass" or bake["source_basis"] != identity or
                any(not math.isclose(actual, expected, abs_tol=1e-6)
                    for actual, expected in zip(bake["source_local_direction"],
                                                entry["normal"])) or
                bake["stage_sha256"] != evidence["stage_sha256"] or
                bake["exr_sha256"] != sha256(target) or
                bake["width"] != args.width or bake["height"] != args.height or
                bake["samples"] != args.samples):
            evidence["failure"] = identity + " evidence mismatch"
            evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
            raise ValueError(evidence["failure"])
        evidence["bakes"][identity] = {"exr_sha256": sha256(target),
                                       "evidence_sha256": sha256(target.with_suffix(".json")),
                                       "log_sha256": sha256(log), "command": command}
        evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
        print(identity + " " + evidence["bakes"][identity]["exr_sha256"], flush=True)
    evidence["status"] = "pass"
    evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("supplemental Cycles bakes passed: " + str(evidence_path))


if __name__ == "__main__":
    main()
