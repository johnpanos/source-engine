#!/usr/bin/env python3
"""Author shared staircase2 lightmap UVs in USD and bake Cycles diffuse light.

Run inside Blender. The output stage keeps the original mesh/material/camera
data and adds one packed `lightmap_st` primvar to each source mesh.
"""

import argparse
import hashlib
import json
import math
import os
import re
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
from staircase2_usd_cycles import emitter_material, parse_scene


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--out-stage", type=Path, required=True)
    parser.add_argument("--out-exr", type=Path, required=True)
    parser.add_argument("--size", type=int, default=2048)
    parser.add_argument("--samples", type=int, default=32)
    args = parser.parse_args(arguments)
    if args.size < 512 or args.samples < 1 or not os.environ.get("OCIO"):
        parser.error("valid atlas size, samples and OCIO are required")
    if args.stage.resolve() == args.out_stage.resolve():
        parser.error("output stage must differ from input stage")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import staircase2 USD")
    meshes = sorted((obj for obj in bpy.data.objects
                     if obj.type == "MESH" and re.fullmatch(r"Mesh\d{3}", obj.name)),
                    key=lambda obj: obj.name)
    if len(meshes) != 19:
        raise ValueError("USD source mesh inventory changed")
    bpy.ops.object.select_all(action="DESELECT")
    baked_meshes = []
    for obj in meshes:
        if not obj.data.uv_layers.get("st"):
            raise ValueError("source mesh lacks material UVs: " + obj.name)
        obj.data.uv_layers.active = obj.data.uv_layers.new(name="lightmap_st")
        material_names = {slot.material.name.lower() for slot in obj.material_slots
                          if slot.material}
        if len(material_names) != 1:
            raise ValueError("source mesh has ambiguous material: " + obj.name)
        if material_names.isdisjoint({"spotholder", "glass"}):
            obj.select_set(True)
            baked_meshes.append(obj)
        else:
            for loop in obj.data.uv_layers.active.data:
                loop.uv = (0.0, 0.0)
    if not baked_meshes:
        raise ValueError("no architectural meshes selected for baking")
    bpy.context.view_layer.objects.active = baked_meshes[0]
    result = bpy.ops.uv.lightmap_pack(PREF_CONTEXT="ALL_FACES", PREF_PACK_IN_ONE=True,
                                      PREF_NEW_UVLAYER=False, PREF_BOX_DIV=12,
                                      PREF_MARGIN_DIV=0.1)
    if result != {"FINISHED"}:
        raise RuntimeError("Blender could not pack shared lightmap UVs")
    uv_extents = {}
    for obj in meshes:
        layer = obj.data.uv_layers.get("lightmap_st")
        if not layer:
            raise ValueError("lightmap pack did not create a separate UV layer for " +
                             obj.name + ": " + str([uv.name for uv in obj.data.uv_layers]))
        obj.data.uv_layers.active = layer
        values = [tuple(loop.uv) for loop in layer.data]
        if len(values) != len(obj.data.loops) or any(
                not (0 <= value[0] <= 1 and 0 <= value[1] <= 1) for value in values):
            raise ValueError("packed lightmap UVs escaped their atlas: " + obj.name +
                             " loops " + str((len(values), len(obj.data.loops))) +
                             " range " + str((min(v[0] for v in values),
                                              max(v[0] for v in values),
                                              min(v[1] for v in values),
                                              max(v[1] for v in values))))
        uv_extents[obj.name] = [min(value[axis] for value in values)
                                for axis in range(2)] + [
                                    max(value[axis] for value in values)
                                    for axis in range(2)]
    args.out_stage.parent.mkdir(parents=True, exist_ok=True)
    if bpy.ops.wm.usd_export(filepath=str(args.out_stage.resolve()),
                             export_materials=True, export_uvmaps=True,
                             export_cameras=True, export_lights=True,
                             export_textures_mode="NEW") != {"FINISHED"}:
        raise RuntimeError("could not export authored lightmap USD")
    source = args.scene.read_text()
    _, _, _, _, lights = parse_scene(source)
    for index, spec in enumerate(lights):
        name = ("LightDisk" if spec["shape"] == "disk" else "LightQuad") + "%02d" % index
        obj = bpy.data.objects.get(name)
        if not obj or obj.type != "MESH":
            raise ValueError("staircase2 area emitter disappeared: " + name)
        obj.data.materials.clear()
        obj.data.materials.append(emitter_material(index, spec, 1.0))
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = args.samples
    scene.render.bake.use_clear = True
    scene.render.bake.margin = 4
    scene.render.bake.use_pass_color = False
    scene.render.bake.use_pass_direct = True
    scene.render.bake.use_pass_indirect = True
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs["Strength"].default_value = 0.0
    atlas = bpy.data.images.new("Staircase2Lightmap", width=args.size, height=args.size,
                                alpha=True, float_buffer=True)
    for material in {slot.material for obj in baked_meshes for slot in obj.material_slots}:
        if not material or not material.use_nodes:
            raise ValueError("source mesh has no Cycles material")
        node = material.node_tree.nodes.new("ShaderNodeTexImage")
        node.name = "BakeTarget"
        node.image = atlas
        material.node_tree.nodes.active = node
    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"DIRECT", "INDIRECT"})
    args.out_exr.parent.mkdir(parents=True, exist_ok=True)
    # Image.save() applies the image display transform to this generated bake,
    # even when the destination is EXR. save_render() writes scene-linear texels.
    scene.render.image_settings.file_format = "OPEN_EXR"
    scene.render.image_settings.color_depth = "32"
    atlas.save_render(filepath=str(args.out_exr.resolve()), scene=scene)
    if not args.out_exr.is_file():
        raise RuntimeError("Cycles did not save the lightmap atlas")
    # Catch a display transform entering EXR export. Compare sparse, nontrivial
    # texels against Blender's in-memory scene-linear bake, not a PNG preview.
    sampled = []
    stride = max(1, args.size // 16)
    for y in range(stride // 2, args.size, stride):
        for x in range(stride // 2, args.size, stride):
            offset = (y * args.size + x) * 4
            rgb = tuple(atlas.pixels[offset + channel] for channel in range(3))
            peak = max(rgb)
            if all(math.isfinite(value) for value in rgb) and 0.05 < peak < 5 and abs(peak - 1) > 0.1:
                sampled.append((offset, rgb))
    if not sampled:
        raise ValueError("Cycles lightmap has no nontrivial linear validation texels")
    saved = bpy.data.images.load(str(args.out_exr.resolve()), check_existing=False)
    max_sample_error = max(abs(saved.pixels[offset + channel] - rgb[channel])
                           for offset, rgb in sampled for channel in range(3))
    if max_sample_error > 1e-4:
        raise ValueError("Cycles lightmap EXR differs from its linear bake buffer")
    evidence = {"status": "pass", "scope": "staircase2-shared-lightmap-uv-and-cycles-bake",
                "source_stage_sha256": sha256(args.stage),
                "lighting_stage_sha256": sha256(args.out_stage),
                "scene_sha256": sha256(args.scene), "atlas_exr_sha256": sha256(args.out_exr),
                "size": args.size, "samples": args.samples,
                "mesh_count": len(meshes), "baked_mesh_count": len(baked_meshes),
                "emitter_count": len(lights),
                "linear_exr_sample_count": len(sampled),
                "linear_exr_max_sample_error": max_sample_error,
                "uv_extents": uv_extents, "blender": bpy.app.version_string,
                "ocio_configuration_sha256": sha256(Path(os.environ["OCIO"]))}
    args.out_exr.with_name(args.out_exr.name + ".json").write_text(
        json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("STAIRCASE2_LIGHTMAP_BAKE " + json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
