#!/usr/bin/env python3
"""Compare deterministic DXVK frames that differ only in Cycles RNM luxels."""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

from worldstage_rnm_delta_compare import compare as compare_bsp


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_boot(path, expected_bsp):
    evidence = json.loads(path.read_text())
    if evidence["status"] != "pass" or len(evidence["screenshots"]) != 1:
        raise ValueError("DXVK boot did not pass with one screenshot")
    map_path = "maps/" + evidence["map"] + ".bsp"
    map_record = evidence["content_overrides"][map_path]
    if (map_record["sha256"] != sha256(expected_bsp) or
            sha256(Path(map_record["source"])) != map_record["sha256"]):
        raise ValueError("staged legacy BSP does not match the expected file")
    log = path.parent / "runtime/hl2_launcher_d3d9.log"
    if "Found config env: dxvk.enableGraphicsPipelineLibrary = False" not in log.read_text():
        raise ValueError("DXVK clip-plane configuration was not applied")
    if "provider=vulkan-compat" not in (path.parent / "stdout.log").read_text():
        raise ValueError("DXVK renderer provider was not selected")
    screenshot = evidence["screenshots"][0]
    image_path = Path(screenshot["path"])
    if sha256(image_path) != screenshot["sha256"]:
        raise ValueError("DXVK screenshot changed since boot")
    image = np.asarray(Image.open(image_path).convert("RGB"), dtype=np.int16)
    if image.shape[0] < 64 or image.shape[1] < 64:
        raise ValueError("DXVK screenshot is too small")
    return evidence, image


def compare(flat_bsp, directional_bsp, flat_paths, directional_path,
            delta_evidence_path):
    delta = json.loads(delta_evidence_path.read_text())
    if (delta["status"] != "pass" or not delta["negative_flat_change_rejected"] or
            delta["flat_sha256"] != sha256(flat_bsp) or
            delta["directional_sha256"] != sha256(directional_bsp)):
        raise ValueError("RNM-only BSP delta evidence does not match these files")
    spans = compare_bsp(flat_bsp.read_bytes(), directional_bsp.read_bytes(),
                        delta["style"])
    if spans != {key: delta[key] for key in spans}:
        raise ValueError("RNM-only BSP spans changed since delta comparison")
    records = [load_boot(path, flat_bsp) for path in flat_paths]
    records.append(load_boot(directional_path, directional_bsp))
    first = records[0][0]
    for evidence, image in records:
        if (evidence["map"] != first["map"] or
                evidence["executables"] != first["executables"] or
                evidence["build_overrides"] != first["build_overrides"] or
                evidence["shader_overrides"] != first["shader_overrides"] or
                evidence["command"][1:] != first["command"][1:] or
                "-nomouse" not in evidence["command"] or
                evidence["display_environment"] != first["display_environment"] or
                evidence["tonemap_scale"] != first["tonemap_scale"] or
                image.shape != records[0][1].shape):
            raise ValueError("DXVK runs differ in runtime, camera, or display settings")
        materials = {name: value["sha256"] for name, value in
                     evidence["content_overrides"].items() if name.startswith("materials/")}
        initial_materials = {name: value["sha256"] for name, value in
                             first["content_overrides"].items()
                             if name.startswith("materials/")}
        if not materials or materials != initial_materials:
            raise ValueError("DXVK runs differ in Portal material assets")
        cfg = (Path(evidence["screenshots"][0]["path"]).parents[1] /
               "cfg/portal_boot_commands.cfg")
        first_cfg = (Path(first["screenshots"][0]["path"]).parents[1] /
                     "cfg/portal_boot_commands.cfg")
        if sha256(cfg) != sha256(first_cfg):
            raise ValueError("DXVK camera or exposure commands differ")
    if not np.array_equal(records[0][1], records[1][1]):
        raise ValueError("repeated flat DXVK captures are not deterministic")
    difference = np.abs(records[2][1] - records[0][1])
    mae = float(difference.mean())
    changed_fraction = float(np.mean(np.max(difference, axis=2) > 5))
    if mae < 0.5 or changed_fraction < 0.01:
        raise ValueError("DXVK frame did not respond to the RNM-only BSP delta")
    return {"status": "pass", "scope": "cycles-rnm-dxvk-in-game",
            "flat_bsp_sha256": sha256(flat_bsp),
            "directional_bsp_sha256": sha256(directional_bsp),
            "delta_evidence_sha256": sha256(delta_evidence_path),
            "boot_evidence_sha256": [sha256(path) for path in
                                     [*flat_paths, directional_path]],
            "flat_repeat_exact": True,
            "flat_frame_sha256": first["screenshots"][0]["sha256"],
            "directional_frame_sha256": records[2][0]["screenshots"][0]["sha256"],
            "same_material_assets": len(initial_materials),
            "mean_absolute_rgb_difference": mae,
            "changed_pixel_fraction_gt5": changed_fraction,
            "changed_bytes_by_basis": spans["changed_bytes_by_basis"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--flat-bsp", type=Path, required=True)
    parser.add_argument("--directional-bsp", type=Path, required=True)
    parser.add_argument("--flat-boot", type=Path, action="append", required=True)
    parser.add_argument("--directional-boot", type=Path, required=True)
    parser.add_argument("--delta-evidence", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if len(args.flat_boot) != 2:
        parser.error("exactly two repeated --flat-boot captures are required")
    evidence = compare(args.flat_bsp, args.directional_bsp, args.flat_boot,
                       args.directional_boot, args.delta_evidence)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
