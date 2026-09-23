#!/usr/bin/env python3
"""Compile the sealed room with vbsp2 and check its World Stage geometry."""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "quality/fixtures/vbsp-host"
COMPARATOR = ROOT / "tools/quality/worldstage_geometry_compare.py"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def invoke(command, environment, timeout=60):
    result = subprocess.run(command, env=environment, capture_output=True,
                            text=True, timeout=timeout, check=False)
    if result.returncode:
        raise RuntimeError("command failed (exit %d): %s\n%s" % (
            result.returncode, " ".join(map(str, command)),
            (result.stdout + result.stderr)[-4000:]))
    return result.stdout + result.stderr


def check(args):
    for required in (args.vbsp2, args.usdchecker, args.python, args.plugin_root,
                     args.openusd_python, COMPARATOR, FIXTURE / "sealed_room.vmf",
                     FIXTURE / "game"):
        if not required.exists():
            raise ValueError("required input is missing: " + str(required))
    environment = os.environ.copy()
    environment["PXR_PLUGINPATH_NAME"] = str(args.plugin_root.resolve())
    environment["PYTHONPATH"] = str(args.openusd_python.resolve())
    with tempfile.TemporaryDirectory(prefix="rfc0008-worldstage-") as temporary:
        folder = Path(temporary)
        shutil.copytree(FIXTURE / "game", folder / "game")
        shutil.copy2(FIXTURE / "sealed_room.vmf", folder / "sealed_room.vmf")
        bsp = folder / "sealed_room.bsp"
        stage = folder / "sealed_room.geometry.usda"
        invoke([str(args.vbsp2.resolve()), "-game", str(folder / "game"),
                str(folder / "sealed_room.vmf")], environment)
        if not bsp.is_file() or not stage.is_file():
            raise RuntimeError("vbsp2 did not publish both BSP and World Stage geometry")
        checker_output = invoke([str(args.usdchecker.resolve()), str(stage)], environment)
        if "Success!" not in checker_output:
            raise RuntimeError("usdchecker did not report success")
        comparator_output = invoke([
            str(args.python.resolve()), str(COMPARATOR), "--bsp", str(bsp),
            "--stage", str(stage), "--negative-self-test"], environment)
        comparison = json.loads(comparator_output)
        if (comparison.get("status") != "pass" or not comparison.get(
                "negative_missing_face_rejected") or not comparison.get(
                    "negative_missing_entity_rejected")):
            raise RuntimeError("semantic comparator did not pass its negative control")
        return {
            "status": "pass", "fixture_sha256": digest(FIXTURE / "sealed_room.vmf"),
            "vbsp2_sha256": digest(args.vbsp2), "bsp_sha256": digest(bsp),
            "stage_sha256": digest(stage), "usdchecker_success": True,
            "comparison": comparison,
        }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vbsp2", type=Path, required=True)
    parser.add_argument("--usdchecker", type=Path, required=True)
    parser.add_argument("--python", type=Path, required=True,
                        help="interpreter used to build the OpenUSD Python bindings")
    parser.add_argument("--openusd-python", type=Path, required=True)
    parser.add_argument("--plugin-root", type=Path, required=True)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    try:
        evidence = check(args)
    except (OSError, ValueError, RuntimeError, subprocess.TimeoutExpired) as error:
        evidence = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
