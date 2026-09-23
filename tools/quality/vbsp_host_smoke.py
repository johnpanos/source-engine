#!/usr/bin/env python3
"""Exercise the installed Linux VBSP and its BSP2 container handoff."""

import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from bsp2_reader import Bsp2File, open_any


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "quality/fixtures/vbsp-host/game"
MAP = ROOT / "hammer/gtk/samples/room.vmf"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(*args):
    result = subprocess.run(args, capture_output=True, text=True, timeout=45, check=False)
    return {"exit_code": result.returncode, "output": result.stdout + result.stderr}


def compile_map(vbsp, folder, alteration=None):
    game = folder / "game"
    shutil.copytree(FIXTURE, game)
    if alteration == "missing-manifest":
        (game / "scripts/surfaceproperties_manifest.txt").unlink()
    elif alteration == "missing-default":
        (game / "scripts/surfaceproperties.txt").write_text(
            '"other"\n{\n    "density" "1000"\n}\n', encoding="utf-8"
        )
    vmf = folder / "room.vmf"
    shutil.copy2(MAP, vmf)
    return run(str(vbsp), "-game", str(game), str(vmf)), folder / "room.bsp"


def check(vbsp, bsp2tool):
    with tempfile.TemporaryDirectory(prefix="rfc0008-vbsp-host-") as temporary:
        folder = Path(temporary)
        positive = folder / "positive"
        positive.mkdir()
        compiled, bsp = compile_map(vbsp, positive)
        if compiled["exit_code"] != 0 or not bsp.is_file():
            raise RuntimeError("VBSP did not produce the sample BSP: " + compiled["output"])

        data = bsp.read_bytes()
        kind, header = open_any(data)
        physics_size = header["lumps"][29][1]
        if kind != "legacy" or header["version"] != 21 or physics_size == 0:
            raise RuntimeError("VBSP output lacks v21 geometry or physics data")

        converted = folder / "room.bsp2"
        exported = folder / "exported.bsp"
        conversion = run(str(bsp2tool), "convert", str(bsp), str(converted))
        if conversion["exit_code"] != 0 or not converted.is_file():
            raise RuntimeError("BSP2 conversion failed: " + conversion["output"])
        kind, container = open_any(converted.read_bytes())
        if kind != "bsp2" or not isinstance(container, Bsp2File):
            raise RuntimeError("independent reader rejected the converted BSP2")
        container.check_legacy_structures()
        export = run(str(bsp2tool), "export", str(converted), str(exported))
        if export["exit_code"] != 0 or exported.read_bytes() != data:
            raise RuntimeError("BSP2 export changed the compiled BSP")

        negatives = {}
        expected = {
            "missing-manifest": "Required surface properties manifest is missing",
            "missing-default": "Surface properties manifest does not define 'default'",
        }
        for name, diagnostic in expected.items():
            case = folder / name
            case.mkdir()
            result, output = compile_map(vbsp, case, name)
            if result["exit_code"] == 0 or diagnostic not in result["output"] or output.exists():
                raise RuntimeError(name + " was not rejected before writing a BSP: " + result["output"])
            negatives[name] = {"exit_code": result["exit_code"], "diagnostic": diagnostic}

        return {
            "status": "pass",
            "fixture_sha256": digest(MAP),
            "vbsp_sha256": digest(vbsp),
            "bsp2tool_sha256": digest(bsp2tool),
            "vbsp_version": header["version"],
            "physics_lump_bytes": physics_size,
            "bsp_sha256": hashlib.sha256(data).hexdigest(),
            "bsp2_sha256": digest(converted),
            "byte_exact_export": True,
            "negative_cases": negatives,
            "compiler_output": compiled["output"],
        }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vbsp", type=Path, required=True, help="installed VBSP executable")
    parser.add_argument("--bsp2tool", type=Path, required=True)
    parser.add_argument("--out", type=Path, help="write JSON evidence here")
    args = parser.parse_args()
    for path in (args.vbsp, args.bsp2tool):
        if not path.is_file():
            parser.error("required executable is missing: " + str(path))
    try:
        result = check(args.vbsp.resolve(), args.bsp2tool.resolve())
    except (OSError, RuntimeError, subprocess.TimeoutExpired, ValueError) as error:
        result = {"status": "fail", "reason": str(error)}
    serialized = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(serialized, encoding="utf-8")
    if result["status"] != "pass":
        print("VBSP host smoke failed: " + result["reason"], file=sys.stderr)
        return 1
    print("VBSP host smoke passed: v21, physics, BSP2 round trip, two failures rejected")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
