#!/usr/bin/env python3
"""Check installed Linux VBSP/VVIS and the BSP2 handoff on a sealed map."""

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


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "quality/fixtures/vbsp-host"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, timeout=45, check=False)


def visibility(path):
    data = path.read_bytes()
    kind, header = open_any(data)
    if kind != "legacy" or header["version"] != 21:
        raise RuntimeError("compiler output is not a legacy v21 BSP")
    offset, length, *_ = header["lumps"][4]
    lump = data[offset : offset + length]
    if len(lump) < 4:
        raise RuntimeError("visibility lump is empty")
    clusters = struct.unpack_from("<i", lump)[0]
    if clusters != 4 or len(lump) < 4 + 8 * clusters:
        raise RuntimeError("sealed fixture did not produce four visibility clusters")
    for cluster in range(clusters):
        pvs, _ = struct.unpack_from("<ii", lump, 4 + 8 * cluster)
        if pvs < 0 or pvs >= len(lump) or lump[pvs] != 0x0F:
            raise RuntimeError("visibility PVS does not include all four connected clusters")
    return data, lump


def compile_case(folder, vbsp, vvis, threads):
    game = folder / "game"
    shutil.copytree(FIXTURE / "game", game)
    vmf = folder / "sealed_room.vmf"
    shutil.copy2(FIXTURE / "sealed_room.vmf", vmf)
    compiled = run(str(vbsp), "-game", str(game), str(vmf))
    bsp = folder / "sealed_room.bsp"
    prt = folder / "sealed_room.prt"
    if compiled.returncode or not bsp.is_file() or not prt.is_file():
        raise RuntimeError("VBSP failed to emit a sealed BSP and portal file: " + compiled.stdout + compiled.stderr)
    before = bsp.read_bytes()
    visible = run(str(vvis), "-threads", str(threads), "-game", str(game), str(bsp))
    if visible.returncode:
        raise RuntimeError("VVIS failed: " + visible.stdout + visible.stderr)
    after, lump = visibility(bsp)
    if after == before:
        raise RuntimeError("VVIS left the BSP unchanged")
    return bsp, prt, game, lump


def check(vbsp, vvis, bsp2tool):
    with tempfile.TemporaryDirectory(prefix="rfc0008-vvis-host-") as temporary:
        root = Path(temporary)
        single = root / "single"
        parallel = root / "parallel"
        single.mkdir()
        parallel.mkdir()
        bsp, prt, game, serial_lump = compile_case(single, vbsp, vvis, 1)
        _, _, _, parallel_lump = compile_case(parallel, vbsp, vvis, 2)
        if serial_lump != parallel_lump:
            raise RuntimeError("VVIS visibility differs between one and two worker threads")

        missing = single / "missing.bsp"
        shutil.copy2(bsp, missing)
        prior = missing.read_bytes()
        result = run(str(vvis), "-game", str(game), str(missing))
        if result.returncode == 0 or "couldn't read" not in result.stdout + result.stderr or missing.read_bytes() != prior:
            raise RuntimeError("missing portal file did not fail without mutating its BSP")
        result = run(str(vvis), "-mpi", "-game", str(game), str(bsp))
        if result.returncode == 0 or "MPI is unavailable" not in result.stdout + result.stderr:
            raise RuntimeError("unsupported MPI option did not fail explicitly")

        converted = root / "sealed_room.bsp2"
        exported = root / "exported.bsp"
        result = run(str(bsp2tool), "convert", str(bsp), str(converted))
        if result.returncode or not converted.is_file():
            raise RuntimeError("BSP2 conversion failed: " + result.stdout + result.stderr)
        kind, container = open_any(converted.read_bytes())
        if kind != "bsp2" or not isinstance(container, Bsp2File):
            raise RuntimeError("independent reader rejected the VVIS BSP2")
        container.check_legacy_structures()
        result = run(str(bsp2tool), "export", str(converted), str(exported))
        if result.returncode or exported.read_bytes() != bsp.read_bytes():
            raise RuntimeError("BSP2 export changed the VVIS BSP")
        return {
            "status": "pass",
            "fixture_sha256": digest((FIXTURE / "sealed_room.vmf").read_bytes()),
            "vbsp_sha256": digest(vbsp.read_bytes()),
            "vvis_sha256": digest(vvis.read_bytes()),
            "bsp2tool_sha256": digest(bsp2tool.read_bytes()),
            "visibility_bytes": len(serial_lump),
            "visibility_sha256": digest(serial_lump),
            "worker_counts": [1, 2],
            "negative_cases": ["missing-portal", "unsupported-mpi"],
            "byte_exact_export": True,
        }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vbsp", type=Path, required=True)
    parser.add_argument("--vvis", type=Path, required=True)
    parser.add_argument("--bsp2tool", type=Path, required=True)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    for path in (args.vbsp, args.vvis, args.bsp2tool):
        if not path.is_file():
            parser.error("required executable is missing: " + str(path))
    try:
        result = check(args.vbsp.resolve(), args.vvis.resolve(), args.bsp2tool.resolve())
    except (OSError, RuntimeError, subprocess.TimeoutExpired, ValueError, struct.error) as error:
        result = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if result["status"] != "pass":
        print("VVIS host smoke failed: " + result["reason"], file=sys.stderr)
        return 1
    print("VVIS host smoke passed: four clusters, two worker counts, failure cases, BSP2 round trip")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
