#!/usr/bin/env python3
"""Compare matched playable legacy and WMSH world draws using the same assets."""

import argparse
import hashlib
import json
from pathlib import Path
import re

import numpy as np
from PIL import Image


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_boot(path, expected_mode):
    evidence = json.loads(path.read_text())
    if evidence["status"] != "pass" or len(evidence["screenshots"]) != 1:
        raise ValueError("Portal boot lacks one passing screenshot")
    screenshot = evidence["screenshots"][0]
    image_path = Path(screenshot["path"])
    if not screenshot["has_scene_detail"] or screenshot["sha256"] != sha256(image_path):
        raise ValueError("Portal screenshot is missing or changed")
    commands = (path.parent / "runtime/portal/cfg/portal_boot_commands.cfg").read_text().splitlines()
    if commands.count("r_worldmesh_draw " + str(expected_mode)) != 1:
        raise ValueError("Portal world draw mode was not selected exactly once")
    log = (path.parent / "runtime/portal/console.log").read_text()
    if log.count("WMSH materials ready (") != 1:
        raise ValueError("client did not resolve WMSH materials")
    if expected_mode == 0:
        if "WMSH draw path active" in log:
            raise ValueError("legacy capture submitted WMSH geometry")
    else:
        active = re.findall(r"WMSH draw path active \((\d+) material batches, "
                            r"(\d+) visible leaves, (\d+) queued meshlets\)", log)
        if len(active) != 1 or min(map(int, active[0])) < 1:
            raise ValueError("WMSH did not queue visible geometry")
    return evidence, np.asarray(Image.open(image_path).convert("RGB"), dtype=np.int16), commands


def compare(legacy_path, world_path):
    legacy, legacy_image, legacy_commands = load_boot(legacy_path, 0)
    world, world_image, world_commands = load_boot(world_path, 2)
    for key in ("map", "requested_resolution", "executables", "build_overrides",
                "display_environment", "tonemap_scale", "content_overrides"):
        if legacy[key] != world[key]:
            raise ValueError("Portal runtime conditions differ: " + key)
    if legacy["command"][1:] != world["command"][1:]:
        raise ValueError("Portal launch commands differ")
    if [command for command in legacy_commands if not command.startswith("r_worldmesh_draw ")] != [
            command for command in world_commands if not command.startswith("r_worldmesh_draw ")]:
        raise ValueError("Portal camera or other console commands differ")
    if legacy_image.shape != world_image.shape:
        raise ValueError("Portal screenshot dimensions differ")
    difference = np.abs(legacy_image - world_image)
    mae = float(difference.mean())
    changed = float(np.mean(np.max(difference, axis=2) > 5))
    if mae > 5.0 or changed > 0.45:
        raise ValueError("WMSH frame diverges from the matched legacy world")
    return {"status": "pass", "scope": "worldmesh-visible-draw-preview",
            "legacy_boot_sha256": sha256(legacy_path),
            "world_boot_sha256": sha256(world_path),
            "map_sha256": legacy["content_overrides"]["maps/" + legacy["map"] + ".bsp"]["sha256"],
            "same_executables_and_assets": True,
            "wmsh_visible_meshlets_queued": True,
            "frame_mean_absolute_rgb_difference": mae,
            "frame_changed_pixel_fraction_gt5": changed}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--legacy-boot", type=Path, required=True)
    parser.add_argument("--world-boot", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    result = compare(args.legacy_boot, args.world_boot)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
