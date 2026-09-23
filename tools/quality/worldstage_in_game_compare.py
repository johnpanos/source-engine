#!/usr/bin/env python3
"""Check a World Stage lighting preview against the same Portal map with VRAD light."""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

from bsp2_reader import Bsp2File, open_any


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compare(baseline_bsp, preview_bsp, baseline_evidence, preview_evidence):
    before = baseline_bsp.read_bytes()
    after = preview_bsp.read_bytes()
    before_kind, before_header = open_any(before)
    after_kind, after_header = open_any(after)
    if before_kind != "legacy" or after_kind != "legacy" or len(before) != len(after):
        raise ValueError("comparison requires same-size legacy BSPs")
    if before[:8] != after[:8] or before_header["lumps"] != after_header["lumps"]:
        raise ValueError("BSP header or lump layout changed")
    changed_lumps = []
    for index, (offset, size, *_rest) in enumerate(before_header["lumps"]):
        if before[offset:offset + size] != after[offset:offset + size]:
            changed_lumps.append(index)
    if changed_lumps != [8]:
        raise ValueError(f"expected only LDR lightmap lump 8 to change: {changed_lumps}")

    a = json.loads(baseline_evidence.read_text())
    b = json.loads(preview_evidence.read_text())
    if a["status"] != "pass" or b["status"] != "pass":
        raise ValueError("both native Portal runs must pass")
    if a["map"] != b["map"] or a["executables"] != b["executables"]:
        raise ValueError("Portal map or executable differs between runs")
    if (a["command"][1:] != b["command"][1:] or
            a["command"][1:3] != ["-renderer", "native-vulkan"]):
        raise ValueError("native Vulkan launch settings differ between runs")
    for evidence, expected in ((a, before), (b, after)):
        map_path = "maps/" + evidence["map"] + ".bsp"
        installed = evidence["content_overrides"][map_path]
        container = Path(installed["source"])
        if (sha256(container) != installed["sha256"] or
                Bsp2File(container.read_bytes()).export_legacy() != expected):
            raise ValueError("staged BSP2 does not contain the expected legacy map")
    def commands(evidence):
        stage = Path(evidence["screenshots"][0]["path"]).parents[1]
        return sha256(stage / "cfg/portal_boot_commands.cfg")
    if commands(a) != commands(b):
        raise ValueError("in-game camera commands differ between runs")

    def materials(evidence):
        return {path: item["sha256"] for path, item in
                evidence["content_overrides"].items() if path.startswith("materials/")}

    if not materials(a) or materials(a) != materials(b):
        raise ValueError("Portal material assets differ between runs")
    if len(a["screenshots"]) != 1 or len(b["screenshots"]) != 1:
        raise ValueError("expected one in-game screenshot per run")

    def screenshot(evidence):
        entry = evidence["screenshots"][0]
        path = Path(entry["path"])
        if sha256(path) != entry["sha256"]:
            raise ValueError("in-game screenshot changed since its boot run")
        return np.asarray(Image.open(path).convert("RGB"), dtype=np.int16), entry

    first, first_entry = screenshot(a)
    second, second_entry = screenshot(b)
    if first.shape != second.shape or first.shape[0] < 64 or first.shape[1] < 64:
        raise ValueError("in-game screenshots have incompatible dimensions")
    delta = np.abs(second - first)
    changed_fraction = float(np.mean(np.max(delta, axis=2) > 5))
    mean_difference = float(np.mean(delta))
    if changed_fraction < 0.2 or mean_difference < 10:
        raise ValueError("Cycles-derived lighting did not materially change the frame")
    return {"status": "pass", "scope": "legacy-flat-style-preview-in-game",
            "baseline_bsp_sha256": sha256(baseline_bsp),
            "preview_bsp_sha256": sha256(preview_bsp),
            "baseline_boot_evidence_sha256": sha256(baseline_evidence),
            "preview_boot_evidence_sha256": sha256(preview_evidence),
            "baseline_frame_sha256": first_entry["sha256"],
            "preview_frame_sha256": second_entry["sha256"],
            "unchanged_executables": True, "unchanged_camera_commands": True,
            "unchanged_material_assets": len(materials(a)),
            "changed_lumps": changed_lumps, "changed_pixel_fraction_gt5": changed_fraction,
            "mean_absolute_rgb_difference": mean_difference,
            "baseline_mean_rgb": [float(x) for x in first.mean(axis=(0, 1))],
            "preview_mean_rgb": [float(x) for x in second.mean(axis=(0, 1))]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-bsp", type=Path, required=True)
    parser.add_argument("--preview-bsp", type=Path, required=True)
    parser.add_argument("--baseline-evidence", type=Path, required=True)
    parser.add_argument("--preview-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    result = compare(args.baseline_bsp, args.preview_bsp, args.baseline_evidence,
                     args.preview_evidence)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
