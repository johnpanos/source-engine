#!/usr/bin/env python3
"""Exercise BSP2 WMSH carriage and structural failure without licensed maps."""

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory

from bsp2_reader import Bsp2File, fourcc
from worldstage_mesh_compare import read_payload
from worldstage_mesh_pack import tangent_frame, write_payload


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def fixture():
    points = [(0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0)]
    uv = [(0.0, 0.0), (1.0, 0.0), (0.0, 1.0)]
    normal = (0.0, 0.0, 1.0)
    tangent, sign = tangent_frame(points, uv, normal)
    faces = {7: {"material": "test/é", "triangles": [
        (points, uv, uv, normal, tangent, sign)]}}
    wmsh = write_payload(faces, [{7}, set()])[0]
    legacy = b"VBSP" + struct.pack("<i", 21) + b"\0" * (16 * 64) + struct.pack("<i", 1)
    return legacy, wmsh


def run(tool, legacy_path, wmsh_path, output_path):
    return subprocess.run([str(tool), "pack-world", str(legacy_path), str(wmsh_path),
                           str(output_path)], capture_output=True, text=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tool", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    tool = args.tool.resolve(strict=True)
    legacy, wmsh = fixture()
    with TemporaryDirectory(prefix="worldmesh-tool-") as directory:
        root = Path(directory)
        legacy_path = root / "minimal.bsp"
        wmsh_path = root / "minimal.wmsh"
        output_path = root / "minimal.bsp2"
        legacy_path.write_bytes(legacy)
        wmsh_path.write_bytes(wmsh)
        completed = run(tool, legacy_path, wmsh_path, output_path)
        if completed.returncode or not output_path.exists():
            raise ValueError("pack-world rejected the valid synthetic fixture: " +
                             completed.stderr)
        container_bytes = output_path.read_bytes()
        container = Bsp2File(container_bytes, known=(fourcc("WMSH"),))
        entry = container.by_id[fourcc("WMSH")]
        if (entry["version"] != 1 or entry["flags"] or entry["alignment"] != 4096 or
                container.lump(entry) != wmsh or container.export_legacy() != legacy or
                read_payload(container.lump(entry))["faces"] != (7,)):
            raise ValueError("WMSH carriage or legacy export differs")
        mutations = {"truncated": wmsh[:-1], "bad_magic": b"BAD!" + wmsh[4:],
                     "bad_version": wmsh[:4] + struct.pack("<I", 2) + wmsh[8:],
                     "bad_section_offset": wmsh[:56] + struct.pack("<Q", 144) + wmsh[64:]}
        for name, section_field, replacement in (
                ("bad_index", 64, struct.pack("<I", 9)),
                ("bad_leaf_reference", 104, struct.pack("<I", 9)),
                ("nonfinite_vertex", 56, struct.pack("<I", 0x7FC00000)),
                ("bad_tangent", 56, b"\0\0\0\0"),
                ("bad_meshlet_count", 88, struct.pack("<I", 66)),
                ("bad_meshlet_bound", 88, struct.pack("<f", 100.0)),
                ("bad_material_path", 112, b"/"),
                ("bad_material_utf8", 112, b"\xff")):
            changed = bytearray(wmsh)
            offset = struct.unpack_from("<Q", changed, section_field)[0]
            if name == "bad_meshlet_count":
                offset += 4
            elif name == "bad_meshlet_bound":
                offset += 16
            elif name == "bad_material_path":
                offset += 4
            elif name == "bad_material_utf8":
                offset += 4 + struct.unpack_from("<I", changed, offset)[0] - 1
            elif name == "bad_tangent":
                offset += 16
            changed[offset:offset + len(replacement)] = replacement
            mutations[name] = bytes(changed)
        for name, payload in mutations.items():
            wmsh_path.write_bytes(payload)
            output_path.unlink(missing_ok=True)
            completed = run(tool, legacy_path, wmsh_path, output_path)
            if completed.returncode != 2 or output_path.exists():
                raise ValueError("pack-world accepted malformed " + name)
    evidence = {"status": "pass", "scope": "worldmesh-wmsh-v1-tool-carriage",
                "tool_sha256": sha256(tool.read_bytes()),
                "legacy_sha256": sha256(legacy), "wmsh_sha256": sha256(wmsh),
                "container_sha256": sha256(container_bytes),
                "legacy_export_byte_identical": True, "wmsh_byte_identical": True,
                "malformed_rejected": sorted(mutations)}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
