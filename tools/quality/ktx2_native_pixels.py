#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the RFC 0008 F3 native BC pixel gate against a selected Vulkan device."""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
import time

import conformance


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "texture"))
import ktx2_select as selector  # noqa: E402


DEVICE = re.compile(r"^KTX2_DEVICE vendorID=(\d+) deviceID=(\d+) name=(.+)$", re.MULTILINE)
RESULT = re.compile(r"^CONFORMANCE (\d+) (\d+)$", re.MULTILINE)
MIN_CHECKS = 19


class GateFailure(ValueError):
    pass


def classify(process, plan):
    if process.returncode == 77:
        raise GateFailure("native Vulkan device or display was unavailable")
    if process.returncode != 0:
        raise GateFailure("native pixel process exited %d" % process.returncode)
    records = RESULT.findall(process.stdout)
    if len(records) != 1 or int(records[0][0]) < MIN_CHECKS or int(records[0][1]) != 0:
        raise GateFailure("native pixel result is missing, incomplete or failing")
    devices = DEVICE.findall(process.stdout)
    if len(devices) != 1:
        raise GateFailure("native pixel result lacks one device identity")
    observed = {"vendorID": int(devices[0][0]), "deviceID": int(devices[0][1]),
                "deviceName": devices[0][2]}
    expected = plan["device"]
    if any(observed[key] != expected[key] for key in observed):
        raise GateFailure("native pixel device differs from the queried package device")
    return {"checks": int(records[0][0]), "failures": int(records[0][1]),
            "device": observed}


def validate_plan(plan):
    if plan.get("outcome") != "pass" or plan.get("missing_required_classes"):
        raise GateFailure("format selection plan did not pass")
    product, host, product_path, host_path = selector.load_profiles(
        selector.DEFAULT_PRODUCT_PROFILE)
    if plan.get("product_profile") != product["id"] or \
            plan.get("product_profile_sha256") != selector.sha256(product_path) or \
            plan.get("host_tool_profile") != host["id"] or \
            plan.get("host_tool_profile_sha256") != selector.sha256(host_path):
        raise GateFailure("format selection plan does not match current profiles")
    if set(plan["selections"]) != set(host["texture_classes"]) or \
            any(value.get("selected") is None for value in plan["selections"].values()):
        raise GateFailure("format selection plan lacks a required texture class")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--selection-plan", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args(argv)
    evidence = {"outcome": "fail"}
    process = None
    try:
        plan = json.loads(args.selection_plan.read_text())
        validate_plan(plan)
        started = time.monotonic()
        process = subprocess.run([str(args.binary.resolve())], cwd=ROOT,
                                 capture_output=True, text=True, timeout=120)
        evidence.update(classify(process, plan))
        evidence["duration_seconds"] = round(time.monotonic() - started, 3)
        evidence["selection_plan_sha256"] = selector.sha256(args.selection_plan)
        evidence["binary_sha256"] = selector.sha256(args.binary)
        evidence["source_identity"] = conformance.source_identity(str(ROOT))
        evidence["outcome"] = "pass"
    except (OSError, KeyError, TypeError, ValueError, subprocess.SubprocessError) as error:
        evidence["error"] = str(error)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    if process is not None:
        stdout_path = args.out.with_suffix(".stdout.log")
        stderr_path = args.out.with_suffix(".stderr.log")
        stdout_path.write_text(process.stdout)
        stderr_path.write_text(process.stderr)
        evidence["stdout_log"] = str(stdout_path)
        evidence["stderr_log"] = str(stderr_path)
        evidence["exit_code"] = process.returncode
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("KTX2 native pixels: %s (%s)" % (evidence["outcome"], args.out))
    return 0 if evidence["outcome"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
