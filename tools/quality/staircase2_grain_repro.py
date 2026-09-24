#!/usr/bin/env python3
"""Verify a staircase2 grain fix with an LMAP-only native Vulkan ablation."""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import numpy as np
from scipy.ndimage import gaussian_filter

from bsp2_reader import Bsp2File, fourcc
from staircase2_runtime_compare import validated_frame


WALL_PATCH = (700, 80, 970, 390)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def lightmap_only_change(before, after):
    known = (fourcc("WMSH"), fourcc("LMAP"))
    old = Bsp2File(before.read_bytes(), known=known)
    new = Bsp2File(after.read_bytes(), known=known)
    if len(old.entries) != len(new.entries):
        raise ValueError("BSP2 lump inventory changed")
    changed = []
    for first, second in zip(old.entries, new.entries):
        for field in ("name", "fourcc", "version", "flags", "alignment"):
            if first[field] != second[field]:
                raise ValueError("BSP2 lump metadata changed")
        if old.lump(first) != new.lump(second):
            changed.append(first["name"])
    if changed != ["LMAP"]:
        raise ValueError("expected only LMAP to change, got " + str(changed))
    return len(old.entries)


def patch_metrics(image):
    x0, y0, x1, y1 = WALL_PATCH
    pixels = np.asarray(image, dtype=np.float32)[y0:y1, x0:x1]
    smooth = gaussian_filter(pixels, sigma=(3, 3, 0))
    return {"mean_rgb": float(pixels.mean()),
            "high_frequency_mad": float(np.abs(pixels - smooth).mean())}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--before-bsp2", type=Path, required=True)
    parser.add_argument("--after-bsp2", type=Path, required=True)
    parser.add_argument("--before-boot", type=Path, required=True)
    parser.add_argument("--after-boot", type=Path, required=True)
    parser.add_argument("--max-noise-ratio", type=float, default=0.3)
    parser.add_argument("--max-mean-drift", type=float, default=0.01)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if not 0 < args.max_noise_ratio < 1 or not 0 <= args.max_mean_drift < 1:
        parser.error("noise ratio and mean drift limits are invalid")

    lump_count = lightmap_only_change(args.before_bsp2, args.after_bsp2)
    before_receipt, before_capture, before_image, before_crop = validated_frame(
        args.before_boot, 1024)
    after_receipt, after_capture, after_image, after_crop = validated_frame(
        args.after_boot, 1024)
    if (before_crop != after_crop or
            before_receipt.get("build_overrides") != after_receipt.get("build_overrides")):
        raise ValueError("camera crop or native Vulkan build changed")
    first_content = before_receipt.get("content_overrides", {})
    second_content = after_receipt.get("content_overrides", {})
    map_name = "maps/staircase2_playable.bsp"
    if (set(first_content) != set(second_content) or
            {name for name in first_content if first_content[name]["sha256"] !=
             second_content[name]["sha256"]} != {map_name} or
            first_content[map_name]["sha256"] != sha256(args.before_bsp2) or
            second_content[map_name]["sha256"] != sha256(args.after_bsp2)):
        raise ValueError("runtime content differs outside the two checked BSP2 maps")

    before = patch_metrics(before_image)
    after = patch_metrics(after_image)
    ratio = after["high_frequency_mad"] / before["high_frequency_mad"]
    drift = abs(after["mean_rgb"] - before["mean_rgb"]) / before["mean_rgb"]
    result = {"schema": "staircase2-lightmap-grain-ablation/v1",
              "status": "pass" if ratio <= args.max_noise_ratio and
              drift <= args.max_mean_drift else "fail",
              "changed_lumps": ["LMAP"], "bsp2_lump_count": lump_count,
              "wall_patch": list(WALL_PATCH),
              "before_bsp2_sha256": sha256(args.before_bsp2),
              "after_bsp2_sha256": sha256(args.after_bsp2),
              "before_boot_sha256": sha256(args.before_boot),
              "after_boot_sha256": sha256(args.after_boot),
              "before_capture_sha256": sha256(before_capture),
              "after_capture_sha256": sha256(after_capture),
              "before": before, "after": after,
              "noise_ratio": ratio, "mean_rgb_drift": drift,
              "max_noise_ratio": args.max_noise_ratio,
              "max_mean_drift": args.max_mean_drift}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({key: result[key] for key in
                      ("status", "noise_ratio", "mean_rgb_drift")}, sort_keys=True))
    if result["status"] != "pass":
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
