#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Exercise 64-bit BSP2 tool I/O with a sparse, SHA-pinned Portal map.

The private converted and sparse files are removed after the run. Evidence
contains hashes and results, not licensed map bytes.
"""

import argparse
import filecmp
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

import conformance


SOURCE_NAME = "testchmb_a_01.bsp"
SHIFT = 1 << 32
SCHEMA = "bsp2-sparse-export-evidence/v1"


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run_tool(tool, command, source, output=None):
    args = [str(tool), command, str(source)]
    if output is not None:
        args.append(str(output))
    run = subprocess.run(args, capture_output=True, text=True, timeout=120)
    return {"exit_code": run.returncode, "stdout": run.stdout, "stderr": run.stderr}


def make_sparse(base, output):
    data = base.read_bytes()
    if data[:8] != b"SRCBSP2\x1a":
        raise ValueError("converted file is not BSP2")
    directory, count = struct.unpack_from("<QI", data, 24)
    if count == 0 or directory + count * 64 != len(data):
        raise ValueError("converted directory is incomplete")
    entries = bytearray(data[directory:])
    candidate = None
    for index in range(count):
        at = index * 64
        fourcc = entries[at:at + 4]
        offset, size = struct.unpack_from("<QQ", entries, at + 16)
        if fourcc == b"L001" and size:
            candidate = offset
        struct.pack_into("<Q", entries, at + 16, offset + SHIFT)
    if candidate is None:
        raise ValueError("pinned map has no L001 payload for the negative control")
    header = bytearray(data[:64])
    struct.pack_into("<Q", header, 24, directory + SHIFT)
    header[48:64] = hashlib.blake2b(entries, digest_size=16).digest()
    with output.open("wb") as sparse:
        sparse.write(header)
        sparse.seek(SHIFT + 64)
        sparse.write(data[64:directory])
        sparse.write(entries)
    return SHIFT + candidate, count


def run(args):
    source = Path(args.legacy_map).resolve()
    tool = Path(args.tool).resolve()
    inventory = Path(args.inventory).resolve()
    output = Path(args.out).resolve()
    if source.parent == output or source.parent in output.parents:
        print("BSP2 sparse export: fail (evidence must be outside the source map directory)")
        return 2
    if output.exists() and (not output.is_dir() or any(output.iterdir())):
        print("BSP2 sparse export: fail (use a fresh evidence directory)")
        return 2
    output.mkdir(parents=True, exist_ok=True)
    evidence = {"schema": SCHEMA, "source": conformance.source_identity(Path(conformance.repo_root())),
                "inputs": {"legacy_map": str(source), "tool": str(tool),
                           "inventory": str(inventory)}, "checks": {}, "failures": []}
    try:
        expected = json.loads(inventory.read_text())
        entry = next((item for item in expected["maps"] if item["name"] == SOURCE_NAME), None)
        if entry is None or source.name != SOURCE_NAME:
            raise ValueError("pinned source map is absent from the inventory")
        if not tool.is_file() or not source.is_file():
            raise ValueError("required tool or source map is missing")
        evidence["source_map_sha256"] = sha256(source)
        evidence["tool_sha256"] = sha256(tool)
        evidence["inventory_sha256"] = sha256(inventory)
        if evidence["source_map_sha256"] != entry["sha256"] or \
                struct.unpack_from("<I", source.read_bytes(), 4)[0] != entry["version"]:
            raise ValueError("source map hash or version differs from the pinned inventory")
        with tempfile.TemporaryDirectory(prefix="bsp2-sparse-export-") as directory:
            private = Path(directory)
            base = private / "base.bsp2"
            sparse = private / "sparse.bsp2"
            exported = private / "exported.bsp"
            evidence["checks"]["convert"] = run_tool(tool, "convert", source, base)
            if evidence["checks"]["convert"]["exit_code"] != 0:
                raise ValueError("conversion failed")
            corrupt_offset, count = make_sparse(base, sparse)
            evidence["logical_size"] = sparse.stat().st_size
            evidence["allocated_bytes"] = sparse.stat().st_blocks * 512
            evidence["lump_count"] = count
            for command in ("info", "verify"):
                evidence["checks"][command] = run_tool(tool, command, sparse)
                if evidence["checks"][command]["exit_code"] != 0:
                    raise ValueError("sparse %s failed" % command)
            if "offset=429" not in evidence["checks"]["info"]["stdout"]:
                raise ValueError("info omitted the offset above 4 GiB")
            evidence["checks"]["export"] = run_tool(tool, "export", sparse, exported)
            if evidence["checks"]["export"]["exit_code"] != 0 or \
                    not filecmp.cmp(source, exported, shallow=False):
                raise ValueError("sparse export differs from pinned legacy map")
            with sparse.open("r+b") as stream:
                stream.seek(corrupt_offset)
                byte = stream.read(1)
                if len(byte) != 1:
                    raise ValueError("cannot read the selected corruption byte")
                stream.seek(corrupt_offset)
                stream.write(bytes([byte[0] ^ 1]))
            evidence["checks"]["corrupt_verify"] = run_tool(tool, "verify", sparse)
            exported.write_bytes(b"retain previous output")
            evidence["checks"]["corrupt_export"] = run_tool(tool, "export", sparse, exported)
            if any(evidence["checks"][name]["exit_code"] != 1 or
                   "content-hash-mismatch" not in evidence["checks"][name]["stderr"]
                   for name in ("corrupt_verify", "corrupt_export")):
                raise ValueError("corrupted payload was not rejected")
            if exported.read_bytes() != b"retain previous output" or \
                    (private / "exported.bsp.tmp").exists():
                raise ValueError("failed export replaced the prior output")
    except (OSError, ValueError, KeyError, struct.error, subprocess.TimeoutExpired) as error:
        evidence["failures"].append(str(error))
    evidence["status"] = "fail" if evidence["failures"] else "pass"
    path = output / "evidence.json"
    path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("BSP2 sparse export: %s (%s)" % (evidence["status"], path))
    for failure in evidence["failures"]:
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--legacy-map", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--inventory", required=True)
    parser.add_argument("--out", required=True)
    return run(parser.parse_args(argv))


if __name__ == "__main__":
    sys.exit(main())
