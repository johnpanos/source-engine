#!/usr/bin/env python3
"""Compare a camera-matched Source staircase2 frame with its Cycles render.

The headless SDL display can ignore the requested square window size. The
camera uses an 86-degree Source 4:3 FOV, equivalent to the reference's
70-degree square FOV; a centered square crop then has the same projection.
"""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

from staircase2_compare import ssim


CAMERA_COMMANDS = ["r_worldmesh_draw 2", "default_fov 86", "fov_desired 86",
                   "setpos 272.119 -100.557 5.025", "setang 0 154.1814 0"]
REGIONS = {
    "ceiling": (0, 0, 1024, 260),
    "wall": (530, 0, 1024, 550),
    "stairs": (320, 260, 1024, 800),
    "floor": (0, 720, 1024, 1024),
    "glass": (330, 480, 550, 740),
}
EXPECTED_MATERIAL_CHANGES = {
    "materials/staircase2/chrome/basecolor.vtf",
    "materials/staircase2/metal/basecolor.vtf",
    "materials/staircase2/wall/basecolor.vtf",
}


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validated_frame(receipt_path, expected_size):
    receipt = json.loads(receipt_path.read_text())
    if receipt.get("schema") != "portal-boot-evidence/v1" or receipt.get("status") != "pass":
        raise ValueError("runtime boot did not pass")
    if receipt.get("map") != "staircase2_playable" or receipt.get("tonemap_scale") != 1.0:
        raise ValueError("runtime map or tonemap scale differs from the fixture")
    if receipt.get("display_environment", {}).get("SDL_VIDEODRIVER") != "offscreen":
        raise ValueError("runtime capture did not use the headless display")
    command = receipt.get("command", [])
    if not ("-renderer" in command and command[command.index("-renderer") + 1] ==
            "native-vulkan"):
        raise ValueError("runtime capture did not use native Vulkan")
    config = receipt_path.parent / "runtime/portal/cfg/portal_boot_commands.cfg"
    if config.read_text().splitlines() != CAMERA_COMMANDS:
        raise ValueError("runtime camera commands differ from the reference projection")
    screenshots = receipt.get("screenshots", [])
    if len(screenshots) != 1:
        raise ValueError("runtime boot must capture exactly one frame")
    capture = Path(screenshots[0]["path"])
    if not capture.is_file() or sha256(capture) != screenshots[0]["sha256"]:
        raise ValueError("runtime screenshot differs from its boot receipt")
    with Image.open(capture) as opened:
        image = opened.convert("RGB")
    if image.size != (screenshots[0]["width"], screenshots[0]["height"]):
        raise ValueError("runtime screenshot dimensions differ from its boot receipt")
    width, height = image.size
    if width < height:
        raise ValueError("runtime capture does not contain a centered square view")
    left = (width - height) // 2
    square = image.crop((left, 0, left + height, height))
    if square.size != (expected_size, expected_size):
        square = square.resize((expected_size, expected_size), Image.Resampling.LANCZOS)
    return receipt, capture, square, [left, 0, left + height, height]


def metrics(reference, candidate):
    first = np.asarray(reference, dtype=np.float64)
    second = np.asarray(candidate, dtype=np.float64)
    result = {"mean_absolute_rgb": float(np.abs(first - second).mean()),
              "luminance_ssim": ssim(first, second), "regions": {}}
    for name, (x0, y0, x1, y1) in REGIONS.items():
        a = first[y0:y1, x0:x1]
        b = second[y0:y1, x0:x1]
        result["regions"][name] = {"mean_absolute_rgb": float(np.abs(a - b).mean()),
                                    "luminance_ssim": ssim(a, b)}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--reference-receipt", type=Path, required=True)
    parser.add_argument("--candidate-receipt", type=Path, required=True)
    parser.add_argument("--before-receipt", type=Path)
    parser.add_argument("--minimum-improvement", type=float, default=0.0)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    render = json.loads(args.reference_receipt.read_text())
    if (render.get("status") != "pass" or render.get("fixture") != "staircase2" or
            render.get("renderer") != "Blender Cycles" or
            render.get("image_sha256") != sha256(args.reference) or
            render.get("camera_fov_degrees") != 70.0 or render.get("size") != 1024):
        raise ValueError("Cycles reference or its camera differs from the render receipt")
    with Image.open(args.reference) as opened:
        reference = opened.convert("RGB")
    if reference.size != (1024, 1024):
        raise ValueError("Cycles reference is not 1024 squared")
    candidate_receipt, capture, candidate, crop = validated_frame(
        args.candidate_receipt, 1024)
    after = metrics(reference, candidate)
    result = {"status": "pass", "scope": "staircase2-native-pbr-cycles-camera-comparison",
              "reference_sha256": sha256(args.reference),
              "reference_receipt_sha256": sha256(args.reference_receipt),
              "candidate_boot_sha256": sha256(args.candidate_receipt),
              "candidate_capture_sha256": sha256(capture),
              "crop_source_pixels": crop, "after": after}
    if args.before_receipt:
        before_receipt, old_capture, before, old_crop = validated_frame(
            args.before_receipt, 1024)
        if (old_crop != crop or
                before_receipt.get("build_overrides") !=
                candidate_receipt.get("build_overrides")):
            raise ValueError("before and after do not share the camera, build and BSP2 map")
        old_content = before_receipt.get("content_overrides", {})
        new_content = candidate_receipt.get("content_overrides", {})
        if set(old_content) != set(new_content):
            raise ValueError("before and after stage different content inventories")
        changed = {path for path in old_content if
                   old_content[path]["sha256"] != new_content[path]["sha256"]}
        if changed != EXPECTED_MATERIAL_CHANGES:
            raise ValueError("material ablation changed unexpected content: " + str(sorted(changed)))
        old = metrics(reference, before)
        improvement = old["mean_absolute_rgb"] - after["mean_absolute_rgb"]
        result["before_boot_sha256"] = sha256(args.before_receipt)
        result["before_capture_sha256"] = sha256(old_capture)
        result["before"] = old
        result["mean_absolute_rgb_improvement"] = improvement
        result["changed_content"] = sorted(changed)
        if improvement < args.minimum_improvement:
            result["status"] = "fail"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    candidate.save(args.out.with_suffix(".png"))
    result["candidate_square_sha256"] = sha256(args.out.with_suffix(".png"))
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps({"status": result["status"], "after_mae": after["mean_absolute_rgb"],
                      "after_ssim": after["luminance_ssim"],
                      "improvement": result.get("mean_absolute_rgb_improvement")},
                     sort_keys=True))
    if result["status"] != "pass":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
