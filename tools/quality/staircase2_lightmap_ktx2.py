#!/usr/bin/env python3
"""Package a checked staircase2 Cycles diffuse atlas as linear RGBA16F KTX2."""

import argparse
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path

import imageio.v3 as iio
import numpy as np


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command):
    return subprocess.run(command, check=True, capture_output=True, text=True).stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exr", type=Path, required=True)
    parser.add_argument("--bake-evidence", type=Path, required=True)
    parser.add_argument("--lighting-stage", type=Path, required=True)
    parser.add_argument("--ktx-tool", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    evidence = json.loads(args.bake_evidence.read_text())
    if (evidence.get("status") != "pass" or evidence.get("scope") !=
            "staircase2-shared-lightmap-uv-and-cycles-bake" or
            evidence.get("atlas_exr_sha256") != sha256(args.exr) or
            evidence.get("lighting_stage_sha256") != sha256(args.lighting_stage)):
        raise ValueError("Cycles atlas differs from its authored USD receipt")
    pixels = iio.imread(args.exr)
    if (pixels.shape != (evidence["size"], evidence["size"], 4) or
            not np.isfinite(pixels).all() or np.min(pixels[:, :, :3]) < 0):
        raise ValueError("Cycles atlas has invalid dimensions or pixels")
    rgba = np.empty(pixels.shape, dtype="<f2")
    rgba[:, :, :3] = pixels[::-1, :, :3].astype("<f2")
    rgba[:, :, 3] = 1.0
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="staircase2-lmap-", dir=args.out.parent) as name:
        temporary = Path(name)
        raw = temporary / "atlas.rgba16f"
        package = temporary / "atlas.ktx2"
        extracted = temporary / "extracted.rgba16f"
        raw.write_bytes(rgba.tobytes())
        tool = str(args.ktx_tool.resolve())
        run([tool, "create", "--format", "R16G16B16A16_SFLOAT", "--raw",
             "--width", str(evidence["size"]), "--height", str(evidence["size"]),
             "--assign-tf", "linear", "--assign-texcoord-origin", "top-left",
             str(raw), str(package)])
        run([tool, "validate", str(package)])
        run([tool, "extract", "--raw", str(package), str(extracted)])
        if extracted.read_bytes() != raw.read_bytes():
            raise ValueError("KTX2 changed the authored half-float texels")
        os.replace(package, args.out)
    result = {"status": "pass", "scope": "staircase2-cycles-l0-ktx2",
              "atlas_exr_sha256": sha256(args.exr),
              "bake_evidence_sha256": sha256(args.bake_evidence),
              "lighting_stage_sha256": sha256(args.lighting_stage),
              "ktx2_sha256": sha256(args.out), "format": "R16G16B16A16_SFLOAT",
              "orientation": "top-left", "width": evidence["size"],
              "height": evidence["size"],
              "max_half_quantization_error": float(np.max(np.abs(
                  rgba[::-1, :, :3].astype(np.float32) - pixels[:, :, :3])))}
    args.out.with_suffix(".json").write_text(json.dumps(result, indent=2,
                                                      sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
