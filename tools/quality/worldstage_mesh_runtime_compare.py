#!/usr/bin/env python3
"""Check map-scoped WMSH accept/reject behavior in playable Portal captures."""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

from bsp2_reader import Bsp2File, fourcc
from worldstage_mesh_compare import read_payload


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_boot(path, map_path):
    evidence = json.loads(path.read_text())
    if evidence["status"] != "pass" or len(evidence["screenshots"]) != 1:
        raise ValueError("Portal boot lacks one passing screenshot")
    override = evidence["content_overrides"]["maps/" + evidence["map"] + ".bsp"]
    if override["sha256"] != sha256(map_path):
        raise ValueError("Portal boot used a different map")
    screenshot = evidence["screenshots"][0]
    image_path = Path(screenshot["path"])
    if not screenshot["has_scene_detail"] or screenshot["sha256"] != sha256(image_path):
        raise ValueError("Portal screenshot is missing or changed")
    image = np.asarray(Image.open(image_path).convert("RGB"), dtype=np.int16)
    return evidence, image, (path.parent / "runtime/portal/console.log").read_text()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--valid-bsp2", type=Path, required=True)
    parser.add_argument("--invalid-bsp2", type=Path, required=True)
    parser.add_argument("--valid-boot", type=Path, required=True)
    parser.add_argument("--invalid-boot", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    valid_map = Bsp2File(args.valid_bsp2.read_bytes(), known=(fourcc("WMSH"),))
    invalid_map = Bsp2File(args.invalid_bsp2.read_bytes(), known=(fourcc("WMSH"),))
    valid_ids = {entry["fourcc"] for entry in valid_map.entries}
    if valid_ids != {entry["fourcc"] for entry in invalid_map.entries}:
        raise ValueError("map lump sets differ")
    for identity in valid_ids - {fourcc("WMSH")}:
        if valid_map.lump(valid_map.by_id[identity]) != invalid_map.lump(
                invalid_map.by_id[identity]):
            raise ValueError("legacy or other render lump changed")
    valid_wmsh = valid_map.lump(valid_map.by_id[fourcc("WMSH")])
    invalid_wmsh = invalid_map.lump(invalid_map.by_id[fourcc("WMSH")])
    read_payload(valid_wmsh)
    try:
        read_payload(invalid_wmsh)
    except ValueError:
        pass
    else:
        raise ValueError("independent reader accepted corrupt WMSH")
    valid, valid_image, valid_log = load_boot(args.valid_boot, args.valid_bsp2)
    invalid, invalid_image, invalid_log = load_boot(args.invalid_boot, args.invalid_bsp2)
    for key in ("map", "requested_resolution", "executables", "build_overrides",
                "display_environment", "tonemap_scale"):
        if valid[key] != invalid[key]:
            raise ValueError("Portal runtime conditions differ: " + key)
    if valid["command"][1:] != invalid["command"][1:]:
        raise ValueError("Portal launch commands differ")
    valid_materials = {name: record["sha256"] for name, record in
                       valid["content_overrides"].items() if name.startswith("materials/")}
    invalid_materials = {name: record["sha256"] for name, record in
                         invalid["content_overrides"].items() if name.startswith("materials/")}
    if not valid_materials or valid_materials != invalid_materials:
        raise ValueError("Portal material assets differ")
    if (valid_log.count("WMSH ready (") != 1 or "WMSH rejected" in valid_log or
            invalid_log.count("WMSH rejected (ok, invalid-vertices)") != 1 or
            "WMSH ready (" in invalid_log):
        raise ValueError("client did not accept/reject the expected WMSH")
    if valid_image.shape != invalid_image.shape:
        raise ValueError("Portal screenshot dimensions differ")
    difference = np.abs(valid_image - invalid_image)
    mae = float(difference.mean())
    changed = float(np.mean(np.max(difference, axis=2) > 5))
    if mae > 1.0 or changed > 0.02:
        raise ValueError("legacy render frames diverged after optional WMSH rejection")
    evidence = {"status": "pass", "scope": "worldmesh-client-validated-borrow",
                "valid_bsp2_sha256": sha256(args.valid_bsp2),
                "invalid_bsp2_sha256": sha256(args.invalid_bsp2),
                "valid_boot_sha256": sha256(args.valid_boot),
                "invalid_boot_sha256": sha256(args.invalid_boot),
                "legacy_lumps_byte_identical": True,
                "client_accepts_valid_wmsh": True,
                "client_rejects_invalid_wmsh": True,
                "same_material_assets": len(valid_materials),
                "frame_mean_absolute_rgb_difference": mae,
                "frame_changed_pixel_fraction_gt5": changed}
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
