#!/usr/bin/env python3
"""Compile the vbsp-host fixtures with vbsp2 and check their World Stage geometry."""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from bsp2_reader import Bsp2File, open_any


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


# Each fixture: (name, whether welding folds some face to no surface).
# folded_sliver adds a detail brush whose ~0.03-unit edge bevels are thinner
# than the VBSP weld distance, so their welded windings enclose no area.
FIXTURES = (("sealed_room", False), ("folded_sliver", True))


def check_fixture(args, environment, folder, name, expect_folded):
    shutil.copytree(FIXTURE / "game", folder / "game")
    shutil.copy2(FIXTURE / (name + ".vmf"), folder / (name + ".vmf"))
    bsp = folder / (name + ".bsp")
    bsp2 = folder / (name + ".bsp2")
    stage = folder / (name + ".geometry.usda")
    invoke([str(args.vbsp2.resolve()), "-game", str(folder / "game"),
            str(folder / (name + ".vmf"))], environment)
    if not bsp.is_file() or not bsp2.is_file() or not stage.is_file():
        raise RuntimeError("vbsp2 did not publish BSP, BSP2 and World Stage geometry")
    kind, container = open_any(bsp2.read_bytes())
    if kind != "bsp2" or not isinstance(container, Bsp2File):
        raise RuntimeError("independent reader rejected the direct BSP2 output")
    container.check_legacy_structures()
    if container.export_legacy() != bsp.read_bytes():
        raise RuntimeError("direct BSP2 export changed the compiled BSP")
    legacy = folder / "legacy"
    legacy.mkdir()
    shutil.copytree(FIXTURE / "game", legacy / "game")
    shutil.copy2(FIXTURE / (name + ".vmf"), legacy / (name + ".vmf"))
    invoke([str(args.vbsp.resolve()), "-game", str(legacy / "game"),
            str(legacy / (name + ".vmf"))], environment)
    if (not (legacy / (name + ".bsp")).is_file() or
            (legacy / (name + ".bsp")).read_bytes() != bsp.read_bytes()):
        raise RuntimeError("vbsp2 changed the legacy %s BSP bytes" % name)
    checker_output = invoke([str(args.usdchecker.resolve()), str(stage)], environment)
    if "Success!" not in checker_output:
        raise RuntimeError("usdchecker did not report success")
    comparator_output = invoke([
        str(args.python.resolve()), str(COMPARATOR), "--bsp", str(bsp),
        "--stage", str(stage), "--negative-self-test"], environment)
    comparison = json.loads(comparator_output)
    required = ["negative_missing_face_rejected", "negative_bad_normal_rejected",
                "negative_missing_entity_rejected", "negative_missing_light_rejected",
                "negative_bad_light_style_rejected", "negative_emptied_surface_rejected"]
    if expect_folded:
        required.append("negative_folded_face_triangulated_rejected")
    if comparison.get("status") != "pass" or not all(comparison.get(key) for key in required):
        raise RuntimeError("semantic comparator did not pass its negative control")
    if bool(comparison.get("faces_without_surface")) != expect_folded:
        raise RuntimeError("%s: expected folded faces %s, comparator found %s" % (
            name, expect_folded, comparison.get("faces_without_surface")))
    return {"fixture_sha256": digest(FIXTURE / (name + ".vmf")),
            "bsp_sha256": digest(bsp), "legacy_bsp_byte_exact": True,
            "bsp2_sha256": digest(bsp2), "bsp2_export_byte_exact": True,
            "stage_sha256": digest(stage), "usdchecker_success": True,
            "comparison": comparison}


def check(args):
    for required in (args.vbsp, args.vbsp2, args.usdchecker, args.python, args.plugin_root,
                     args.openusd_python, COMPARATOR, FIXTURE / "game") + tuple(
                         FIXTURE / (name + ".vmf") for name, _ in FIXTURES):
        if not required.exists():
            raise ValueError("required input is missing: " + str(required))
    environment = os.environ.copy()
    environment["PXR_PLUGINPATH_NAME"] = str(args.plugin_root.resolve())
    environment["PYTHONPATH"] = str(args.openusd_python.resolve())
    fixtures = {}
    for name, expect_folded in FIXTURES:
        with tempfile.TemporaryDirectory(prefix="rfc0008-worldstage-") as temporary:
            fixtures[name] = check_fixture(args, environment, Path(temporary), name,
                                           expect_folded)
    return {"status": "pass", "vbsp_sha256": digest(args.vbsp),
            "vbsp2_sha256": digest(args.vbsp2), "fixtures": fixtures}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vbsp", type=Path, required=True)
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
