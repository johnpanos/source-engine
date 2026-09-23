#!/usr/bin/env python3
"""Compare the stable semantic fields of v21 and v25 compiler outputs."""

import argparse
import hashlib
import json
import sys
from pathlib import Path

from bsp2_reader import open_any
from legacy_lighting_audit import audit_lighting_lumps


EXACT_LUMPS = {
    0: "entities",
    4: "visibility",
    55: "HDR leaf ambient samples",
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def read_bsp(path, expected_version):
    data = path.read_bytes()
    kind, header = open_any(data)
    if kind != "legacy" or header["version"] != expected_version:
        raise ValueError(f"{path} is not a legacy v{expected_version} BSP")
    lumps = {}
    for index, entry in enumerate(header["lumps"]):
        offset, length, *_ = entry
        if offset < 0 or length < 0 or offset + length > len(data):
            raise ValueError(f"{path} lump {index} extends outside the BSP")
        lumps[index] = data[offset : offset + length]
    return data, lumps


def compare(reference, candidate):
    reference_data, reference_lumps = read_bsp(reference, 25)
    candidate_data, candidate_lumps = read_bsp(candidate, 21)
    exact = {}
    for index, name in EXACT_LUMPS.items():
        expected = reference_lumps[index]
        actual = candidate_lumps[index]
        if not expected or expected != actual:
            raise ValueError(f"{name} lump {index} differs or is empty")
        exact[name] = {"bytes": len(actual), "sha256": digest(actual)}

    reference_light = reference_lumps[54]
    candidate_light = candidate_lumps[54]
    if len(reference_light) != 100 or len(candidate_light) != 100:
        raise ValueError("the fixture does not contain one 100-byte HDR world light in each BSP")
    if reference_light[:24] != candidate_light[:24]:
        raise ValueError("HDR world-light origin or intensity differs")

    reference_map = reference_lumps[53]
    candidate_map = candidate_lumps[53]
    if not reference_map or not candidate_map:
        raise ValueError("either compiler omitted the HDR lightmap")
    if not any(reference_map) or not any(candidate_map):
        raise ValueError("either HDR lightmap contains no nonzero lighting")
    candidate_layout = audit_lighting_lumps(
        candidate_lumps[58] or candidate_lumps[7], candidate_lumps[6], candidate_map
    )

    return {
        "status": "pass",
        "scope": "v25-v21 cross-version semantic baseline; not same-revision byte parity",
        "reference_bsp_sha256": digest(reference_data),
        "candidate_bsp_sha256": digest(candidate_data),
        "exact_lumps": exact,
        "worldlight_origin_intensity_sha256": digest(candidate_light[:24]),
        "worldlight_full_bytes_equal": reference_light == candidate_light,
        "reference_hdr_lighting_bytes": len(reference_map),
        "candidate_hdr_lighting_bytes": len(candidate_map),
        "reference_hdr_lighting_sha256": digest(reference_map),
        "candidate_hdr_lighting_sha256": digest(candidate_map),
        "candidate_hdr_lighting_layout": candidate_layout,
        "reference_physics_bytes": len(reference_lumps[29]),
        "candidate_physics_bytes": len(candidate_lumps[29]),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True, help="Portal 2 CE v25 BSP")
    parser.add_argument("--candidate", type=Path, required=True, help="Linux port v21 BSP")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    try:
        result = compare(args.reference, args.candidate)
    except (OSError, ValueError) as error:
        result = {"status": "fail", "reason": str(error)}
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if result["status"] != "pass":
        print("cross-version comparison failed: " + result["reason"], file=sys.stderr)
        return 1
    print("cross-version comparison passed: entities, visibility, ambient, light origin/intensity")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
