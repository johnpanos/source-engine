#!/usr/bin/env python3
"""Check the installed Linux VBSP/VVIS/VRAD map compiler path."""

import argparse
import hashlib
import json
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

from bsp2_reader import Bsp2File, open_any
from legacy_lighting_audit import audit_lighting_lumps


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "quality/fixtures/vbsp-host"
LIGHT_LUMPS = (8, 15, 52, 56)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, timeout=60, check=False)


def lump(data, index):
    kind, header = open_any(data)
    if kind != "legacy" or header["version"] != 21:
        raise RuntimeError("compiler output is not a legacy v21 BSP")
    offset, length, *_ = header["lumps"][index]
    if offset < 0 or length < 0 or offset + length > len(data):
        raise RuntimeError(f"lump {index} extends outside the BSP")
    return data[offset : offset + length]


def require_success(label, result):
    if result.returncode:
        raise RuntimeError(f"{label} failed: " + result.stdout + result.stderr)


def check(vbsp, vvis, vrad, bsp2tool):
    with tempfile.TemporaryDirectory(prefix="rfc0008-vrad-host-") as temporary:
        root = Path(temporary)
        game = root / "game"
        shutil.copytree(FIXTURE / "game", game)
        vmf = root / "sealed_room.vmf"
        shutil.copy2(FIXTURE / "sealed_room.vmf", vmf)
        bsp = root / "sealed_room.bsp"

        require_success("VBSP", run(str(vbsp), "-game", str(game), str(vmf)))
        if not bsp.is_file() or not (root / "sealed_room.prt").is_file():
            raise RuntimeError("VBSP did not emit a BSP and portal file")
        require_success("VVIS", run(str(vvis), "-threads", "1", "-game", str(game), str(bsp)))
        pre_bake = bsp.read_bytes()
        if not lump(pre_bake, 4) or not lump(pre_bake, 29):
            raise RuntimeError("VVIS input lacks visibility or physics")

        baked = []
        lighting_layout = None
        for threads in (1, 2):
            case = root / f"threads-{threads}"
            case.mkdir()
            case_bsp = case / "sealed_room.bsp"
            case_bsp.write_bytes(pre_bake)
            result = run(
                str(vrad), "-bounce", "0", "-threads", str(threads),
                "-game", str(game), str(case_bsp),
            )
            require_success(f"VRAD with {threads} threads", result)
            data = case_bsp.read_bytes()
            if lump(data, 4) != lump(pre_bake, 4) or lump(data, 29) != lump(pre_bake, 29):
                raise RuntimeError("VRAD changed visibility or physics")
            lighting = lump(data, 8)
            if not lighting or sum(value != 0 for value in lighting) == 0:
                raise RuntimeError("VRAD produced no LDR lighting")
            lighting_layout = audit_lighting_lumps(lump(data, 7), lump(data, 6), lighting)
            if not lump(data, 15) or not lump(data, 52) or not lump(data, 56):
                raise RuntimeError("VRAD omitted world or ambient lighting")
            # The world lights are dworldlight_t records (100 bytes), so the
            # lump must say version 1: under version 0 the engine reads them
            # as 88-byte dworldlight_version0_t and crashed on the garbage
            # cluster (fixed 2026-09-25).
            worldlights_version = struct.unpack_from("<i", data, 8 + 16 * 15 + 8)[0]
            if worldlights_version != 1 or len(lump(data, 15)) % 100:
                raise RuntimeError("VRAD wrote world lights as version %d with %d bytes; expected "
                                   "whole 100-byte version 1 records"
                                   % (worldlights_version, len(lump(data, 15))))
            baked.append((case_bsp, data, {index: lump(data, index) for index in LIGHT_LUMPS}))

        for index in LIGHT_LUMPS:
            if baked[0][2][index] != baked[1][2][index]:
                raise RuntimeError(f"lighting lump {index} differs between worker counts")

        first_bsp, first_data, first_lumps = baked[0]
        prior = first_bsp.read_bytes()
        rejected = run(str(vrad), "-mpi", "-game", str(game), str(first_bsp))
        if rejected.returncode == 0 or "MPI is unavailable" not in rejected.stdout + rejected.stderr:
            raise RuntimeError("unsupported MPI option did not fail explicitly")
        if first_bsp.read_bytes() != prior:
            raise RuntimeError("unsupported MPI option mutated the BSP")

        converted = root / "sealed_room.bsp2"
        exported = root / "exported.bsp"
        require_success("BSP2 convert", run(str(bsp2tool), "convert", str(first_bsp), str(converted)))
        kind, container = open_any(converted.read_bytes())
        if kind != "bsp2" or not isinstance(container, Bsp2File):
            raise RuntimeError("independent reader rejected the baked BSP2")
        container.check_legacy_structures()
        require_success("BSP2 export", run(str(bsp2tool), "export", str(converted), str(exported)))
        if exported.read_bytes() != first_data:
            raise RuntimeError("BSP2 export changed the baked BSP")

        return {
            "status": "pass",
            "fixture_sha256": digest((FIXTURE / "sealed_room.vmf").read_bytes()),
            "vbsp_sha256": digest(vbsp.read_bytes()),
            "vvis_sha256": digest(vvis.read_bytes()),
            "vrad_sha256": digest(vrad.read_bytes()),
            "bsp2tool_sha256": digest(bsp2tool.read_bytes()),
            "worker_counts": [1, 2],
            "lighting_bytes": len(first_lumps[8]),
            "lighting_nonzero_bytes": sum(value != 0 for value in first_lumps[8]),
            "lighting_sha256": digest(first_lumps[8]),
            "lighting_layout": lighting_layout,
            "worldlights_bytes": len(first_lumps[15]),
            "worldlights_version": 1,
            "visibility_sha256": digest(lump(pre_bake, 4)),
            "bsp_sha256": digest(first_data),
            "byte_exact_export": True,
            "negative_cases": ["unsupported-mpi"],
        }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("vbsp", "vvis", "vrad", "bsp2tool"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    paths = [args.vbsp, args.vvis, args.vrad, args.bsp2tool]
    for path in paths:
        if not path.is_file():
            parser.error("required executable is missing: " + str(path))
    try:
        result = check(*(path.resolve() for path in paths))
    except (OSError, RuntimeError, subprocess.TimeoutExpired, ValueError) as error:
        result = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if result["status"] != "pass":
        print("VRAD host smoke failed: " + result["reason"], file=sys.stderr)
        return 1
    print("VRAD host smoke passed: LDR light, two worker counts, BSP2 round trip")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
