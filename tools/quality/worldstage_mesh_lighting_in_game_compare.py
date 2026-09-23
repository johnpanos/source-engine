#!/usr/bin/env python3
"""Verify that the playable WMSH view responds to the World Stage Cycles bake."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct

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
    map_name = "maps/" + evidence["map"] + ".bsp"
    if evidence["content_overrides"][map_name]["sha256"] != sha256(map_path):
        raise ValueError("Portal boot used a different BSP2 map")
    screenshot = evidence["screenshots"][0]
    image_path = Path(screenshot["path"])
    if not screenshot["has_scene_detail"] or screenshot["sha256"] != sha256(image_path):
        raise ValueError("Portal screenshot is missing or changed")
    commands = (path.parent / "runtime/portal/cfg/portal_boot_commands.cfg").read_bytes()
    if commands.count(b"r_worldmesh_draw 2") != 1:
        raise ValueError("Portal boot did not select the WMSH world path")
    log = (path.parent / "runtime/portal/console.log").read_text()
    draws = re.findall(r"WMSH draw path active \((\d+) material batches, "
                       r"(\d+) visible leaves, (\d+) queued meshlets\)", log)
    if len(draws) != 1 or min(map(int, draws[0])) < 1 or "WMSH rejected" in log:
        raise ValueError("Portal client did not submit valid visible WMSH geometry")
    image = np.asarray(Image.open(image_path).convert("RGB"), dtype=np.int16)
    return evidence, image, commands, tuple(map(int, draws[0]))


def image_difference(left, right, minimum_mean, minimum_changed):
    if left.shape != right.shape:
        raise ValueError("Portal screenshot dimensions differ")
    delta = np.abs(left - right)
    mean = float(delta.mean())
    changed = float(np.mean(np.max(delta, axis=2) > 5))
    if mean < minimum_mean or changed < minimum_changed:
        raise ValueError("WMSH image did not respond to the controlled input change")
    return mean, changed


def legacy_lighting(path, map_file):
    source = path.read_bytes()
    offset, size, _version, _uncompressed = map_file.legacy["lumps"][8]
    if offset + size > len(source):
        raise ValueError("legacy source lighting escapes the file")
    return source[offset:offset + size]


def compare(args):
    bridge = json.loads(args.bridge_evidence.read_text())
    mesh = json.loads(args.mesh_comparison.read_text())
    bake = json.loads(args.cycles_bake_evidence.read_text())
    variant = json.loads(args.portal_variant.read_text())
    if (bridge["status"] != "pass" or mesh["status"] != "pass" or
            bake["status"] != "pass" or bake["renderer"] != "Blender Cycles bake preview" or
            bake["source_basis"] != "flat" or
            bridge["source_bsp_sha256"] != sha256(args.vrad_bsp) or
            bridge["output_bsp_sha256"] != sha256(args.cycles_bsp) or
            bridge["geometry_stage_sha256"] != sha256(args.geometry_stage) or
            bridge["material_stage_sha256"] != sha256(args.material_stage) or
            bridge["cycles_exr_sha256"] != sha256(args.cycles_exr) or
            bridge["portal_manifest_sha256"] != sha256(args.portal_manifest) or
            bake["stage_sha256"] != bridge["material_stage_sha256"] or
            bake["exr_sha256"] != bridge["cycles_exr_sha256"] or
            json.loads(args.portal_manifest.read_text())["stage_sha256"] !=
            bridge["geometry_stage_sha256"] or
            mesh["stage_sha256"] != bridge["geometry_stage_sha256"] or
            mesh["wmsh_sha256"] != sha256(args.wmsh) or
            mesh["bsp2_sha256"] != sha256(args.cycles_bsp2) or
            mesh["bsp_sha256"] != bridge["source_bsp_sha256"] or
            mesh["container_legacy_sha256"] != bridge["output_bsp_sha256"]):
        raise ValueError("World Stage, Cycles bake, WMSH or BSP2 provenance differs")
    vrad = Bsp2File(args.vrad_bsp2.read_bytes(), known=(fourcc("WMSH"),))
    cycles = Bsp2File(args.cycles_bsp2.read_bytes(), known=(fourcc("WMSH"),))
    if set(vrad.by_id) != set(cycles.by_id):
        raise ValueError("BSP2 lump sets differ")
    changed_lumps = {identity for identity in vrad.by_id if
                     vrad.lump(vrad.by_id[identity]) != cycles.lump(cycles.by_id[identity])}
    if changed_lumps != {fourcc("L008")}:
        raise ValueError("VRAD and Cycles BSP2 maps differ outside legacy lighting")
    if (vrad.legacy_lump(8) == cycles.legacy_lump(8) or
            vrad.legacy_lump(8) != legacy_lighting(args.vrad_bsp, vrad) or
            cycles.legacy_lump(8) != legacy_lighting(args.cycles_bsp, cycles)):
        raise ValueError("packaged lightmaps differ from their verified legacy sources")
    wmsh = cycles.lump(cycles.by_id[fourcc("WMSH")])
    if sha256(args.wmsh) != hashlib.sha256(wmsh).hexdigest():
        raise ValueError("BSP2 WMSH differs from the checked World Stage mesh")
    read_payload(wmsh)
    vrad_boot, vrad_image, vrad_commands, vrad_draws = load_boot(
        args.vrad_boot, args.vrad_bsp2)
    cycles_boot, cycles_image, cycles_commands, cycles_draws = load_boot(
        args.cycles_boot, args.cycles_bsp2)
    for key in ("map", "requested_resolution", "executables", "build_overrides",
                "display_environment", "tonemap_scale"):
        if vrad_boot[key] != cycles_boot[key]:
            raise ValueError("Portal runtime conditions differ: " + key)
    if (vrad_boot["command"][1:] != cycles_boot["command"][1:] or
            vrad_commands != cycles_commands or vrad_draws != cycles_draws):
        raise ValueError("Portal camera or WMSH submission differs")
    for name, record in vrad_boot["content_overrides"].items():
        other = cycles_boot["content_overrides"].get(name)
        if not name.startswith("maps/") and (other is None or
                                             record["sha256"] != other["sha256"]):
            raise ValueError("Portal material assets differ: " + name)
    if set(vrad_boot["content_overrides"]) != set(cycles_boot["content_overrides"]):
        raise ValueError("Portal content override sets differ")
    for material in ("portal_pbr/metal/metalwall_bts_006b",
                     "portal_pbr/tile/observation_tilefloor001a"):
        source = args.portal_assets / "textures" / material / "basecolor.png"
        converted = variant["converted_portal_textures"][material + "/basecolor"]
        texture_name = "materials/" + material + "/basecolor.vtf"
        vmt_name = "materials/" + material + ".vmt"
        if (converted["png_sha256"] != sha256(source) or
                converted["vtf_sha256"] != cycles_boot["content_overrides"][texture_name]["sha256"]):
            raise ValueError("playable base texture differs from the upsampled Portal source")
        vmt = cycles_boot["content_overrides"][vmt_name]
        vmt_path = Path(vmt["source"])
        if (sha256(vmt_path) != vmt["sha256"] or
                not vmt_path.read_text().lstrip().lower().startswith('"lightmappedgeneric"') or
                ('"$basetexture" "' + material + '/basecolor"').lower() not in
                vmt_path.read_text().lower()):
            raise ValueError("controlled material does not bind the upsampled base texture")
    mean, changed = image_difference(vrad_image, cycles_image, 10.0, 0.5)
    uv_mean = None
    uv_changed = None
    if args.uvzero_bsp2 or args.uvzero_boot:
        if not args.uvzero_bsp2 or not args.uvzero_boot:
            raise ValueError("UV negative control needs both BSP2 and boot evidence")
        uv_map = Bsp2File(args.uvzero_bsp2.read_bytes(), known=(fourcc("WMSH"),))
        if set(uv_map.by_id) != set(cycles.by_id):
            raise ValueError("UV negative map lump set differs")
        uv_differences = {identity for identity in cycles.by_id if
                          cycles.lump(cycles.by_id[identity]) != uv_map.lump(
                              uv_map.by_id[identity])}
        if uv_differences != {fourcc("WMSH")}:
            raise ValueError("UV negative map changed more than WMSH")
        expected = bytearray(wmsh)
        vertex_count = struct.unpack_from("<I", expected, 16)[0]
        vertex_offset = struct.unpack_from("<Q", expected, 56)[0]
        if not any(expected[vertex_offset + 40 * index + 32:
                            vertex_offset + 40 * index + 40] != b"\0" * 8
                   for index in range(vertex_count)):
            raise ValueError("World Stage mesh lacks nonzero lightmap coordinates")
        for index in range(vertex_count):
            expected[vertex_offset + 40 * index + 32:
                     vertex_offset + 40 * index + 40] = b"\0" * 8
        if bytes(expected) != uv_map.lump(uv_map.by_id[fourcc("WMSH")]):
            raise ValueError("UV negative map changed more than lightmap UVs")
        read_payload(bytes(expected))
        uv_boot, uv_image, uv_commands, uv_draws = load_boot(
            args.uvzero_boot, args.uvzero_bsp2)
        for key in ("map", "requested_resolution", "executables", "build_overrides",
                    "display_environment", "tonemap_scale"):
            if cycles_boot[key] != uv_boot[key]:
                raise ValueError("UV negative runtime conditions differ: " + key)
        if (cycles_boot["command"][1:] != uv_boot["command"][1:] or
                cycles_commands != uv_commands or cycles_draws != uv_draws):
            raise ValueError("UV negative camera or WMSH submission differs")
        for name, record in cycles_boot["content_overrides"].items():
            other = uv_boot["content_overrides"].get(name)
            if not name.startswith("maps/") and (other is None or
                                                 record["sha256"] != other["sha256"]):
                raise ValueError("UV negative material assets differ: " + name)
        uv_mean, uv_changed = image_difference(cycles_image, uv_image, 10.0, 0.5)
    if args.negative_self_test:
        try:
            image_difference(cycles_image, cycles_image, 10.0, 0.5)
        except ValueError:
            pass
        else:
            raise ValueError("identical-frame negative control was accepted")
    return {"status": "pass", "scope": "worldstage-cycles-wmsh-in-game-preview",
            "geometry_stage_sha256": bridge["geometry_stage_sha256"],
            "material_stage_sha256": bridge["material_stage_sha256"],
            "cycles_exr_sha256": bridge["cycles_exr_sha256"],
            "cycles_bake_evidence_sha256": sha256(args.cycles_bake_evidence),
            "portal_manifest_sha256": sha256(args.portal_manifest),
            "portal_variant_sha256": sha256(args.portal_variant),
            "upsampled_portal_base_textures_verified": 2,
            "wmsh_sha256": mesh["wmsh_sha256"],
            "bridge_evidence_sha256": sha256(args.bridge_evidence),
            "mesh_comparison_sha256": sha256(args.mesh_comparison),
            "vrad_boot_sha256": sha256(args.vrad_boot),
            "cycles_boot_sha256": sha256(args.cycles_boot),
            "only_legacy_lighting_lump_changed": True,
            "same_executables_materials_camera_and_visible_meshlets": True,
            "visible_meshlets": vrad_draws[2],
            "frame_mean_absolute_rgb_difference": mean,
            "frame_changed_pixel_fraction_gt5": changed,
            "uvzero_frame_mean_absolute_rgb_difference": uv_mean,
            "uvzero_frame_changed_pixel_fraction_gt5": uv_changed,
            "only_lightmap_uvs_changed_in_uv_control": uv_mean is not None,
            "identical_frame_negative_rejected": args.negative_self_test}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("vrad-bsp", "cycles-bsp", "vrad-bsp2", "cycles-bsp2", "wmsh",
                 "bridge-evidence", "mesh-comparison", "geometry-stage", "material-stage",
                 "cycles-exr", "cycles-bake-evidence", "portal-manifest", "vrad-boot",
                 "cycles-boot", "portal-variant", "portal-assets", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--negative-self-test", action="store_true")
    parser.add_argument("--uvzero-bsp2", type=Path)
    parser.add_argument("--uvzero-boot", type=Path)
    args = parser.parse_args()
    result = compare(args)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
