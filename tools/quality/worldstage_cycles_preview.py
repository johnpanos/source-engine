"""Render compiled World Stage geometry and UsdLux lights with Blender Cycles.

This is an inspection render. Neutral diffuse materials stand in for unresolved
Source VMTs; the RFC 0007 pinned standalone Cycles reference gate remains open.
Run with: blender -b --factory-startup --python this_file -- --stage ... --out ...
"""

import argparse
import hashlib
import json
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    parser.add_argument("--samples", type=int, default=32)
    parser.add_argument("--exposure", type=float, default=-5.0)
    parser.add_argument("--material-manifest", type=Path,
                        help="Portal PBR PNG mapping resolved from the USD and VMTs")
    parser.add_argument("--import-usd-materials", action="store_true",
                        help="use UsdPreviewSurface bindings from the composed World Stage")
    parser.add_argument("--disable-lights", action="store_true",
                        help="negative control: zero imported light energy")
    args = parser.parse_args(arguments)
    if args.width < 64 or args.height < 64 or args.samples < 1 or not args.stage.is_file():
        parser.error("stage, dimensions and samples must be valid")
    if args.material_manifest and args.import_usd_materials:
        parser.error("choose either a material manifest or composed USD materials")

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()),
                             import_materials=args.import_usd_materials,
                             import_lights=True) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the compiled World Stage")
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
    lights = [obj for obj in bpy.data.objects if obj.type == "LIGHT"]
    if not meshes or not lights:
        raise RuntimeError("USD import omitted world meshes or lights")
    if args.disable_lights:
        for light in lights:
            light.data.energy = 0.0

    material_policy = "neutral diffuse for unresolved Source VMTs"
    material_hashes = {}
    if args.import_usd_materials:
        images = {}
        for mesh in meshes:
            if len(mesh.data.materials) != 1:
                raise ValueError("USD material binding missing for " + mesh.name)
            material = mesh.data.materials[0]
            if not material.use_nodes:
                raise ValueError("USD material has no shader network: " + material.name)
            textures = [node.image for node in material.node_tree.nodes
                        if node.type == "TEX_IMAGE" and node.image]
            if len(textures) < 2:
                raise ValueError("USD material omitted Portal basecolor or MRAO")
            for texture in textures:
                path = Path(bpy.path.abspath(texture.filepath))
                if not path.is_file():
                    raise ValueError("USD texture is missing: " + str(path))
                images[str(path)] = sha256(path)
        material_policy = "UsdPreviewSurface bindings imported from composed World Stage"
        material_hashes = images
    elif args.material_manifest:
        manifest = json.loads(args.material_manifest.read_text())
        if manifest["stage_sha256"] != sha256(args.stage):
            raise ValueError("material manifest was generated for another World Stage")
        if set(manifest["meshes"]) != {mesh.name for mesh in meshes}:
            raise ValueError("material manifest does not cover all imported USD meshes")
        built = {}
        for path, spec in manifest["materials"].items():
            material = bpy.data.materials.new(path)
            material.use_nodes = True
            nodes = material.node_tree.nodes
            links = material.node_tree.links
            principled = nodes.get("Principled BSDF")
            uv = nodes.new("ShaderNodeUVMap")
            uv.uv_map = "st"
            base = nodes.new("ShaderNodeTexImage")
            base.image = bpy.data.images.load(spec["basecolor"]["path"], check_existing=True)
            mrao = nodes.new("ShaderNodeTexImage")
            mrao.image = bpy.data.images.load(spec["mrao"]["path"], check_existing=True)
            mrao.image.colorspace_settings.name = "Non-Color"
            for channel, node in (("basecolor", base), ("mrao", mrao)):
                if sha256(Path(spec[channel]["path"])) != spec[channel]["sha256"]:
                    raise ValueError("Portal texture changed after manifest generation")
                links.new(uv.outputs["UV"], node.inputs["Vector"])
            split = nodes.new("ShaderNodeSeparateColor")
            links.new(base.outputs["Color"], principled.inputs["Base Color"])
            links.new(mrao.outputs["Color"], split.inputs["Color"])
            links.new(split.outputs["Red"], principled.inputs["Metallic"])
            links.new(split.outputs["Green"], principled.inputs["Roughness"])
            if "normal" in spec:
                if sha256(Path(spec["normal"]["path"])) != spec["normal"]["sha256"]:
                    raise ValueError("Portal normal texture changed after manifest generation")
                normal_image = nodes.new("ShaderNodeTexImage")
                normal_image.image = bpy.data.images.load(spec["normal"]["path"],
                                                           check_existing=True)
                normal_image.image.colorspace_settings.name = "Non-Color"
                normal = nodes.new("ShaderNodeNormalMap")
                normal.uv_map = "st"
                links.new(uv.outputs["UV"], normal_image.inputs["Vector"])
                links.new(normal_image.outputs["Color"], normal.inputs["Color"])
                links.new(normal.outputs["Normal"], principled.inputs["Normal"])
            built[path] = material
            material_hashes[path] = spec["vmt_sha256"]
        for mesh in meshes:
            material_path = manifest["meshes"][mesh.name]
            if material_path not in built or "st" not in mesh.data.uv_layers:
                raise ValueError("Portal material or source UVs missing for " + mesh.name)
            mesh.data.materials.clear()
            mesh.data.materials.append(built[material_path])
        material_policy = "Portal PBR PNG masters with metallic R and roughness G"
    else:
        preview_material = bpy.data.materials.new("Unresolved_Source_VMT_Preview")
        preview_material.use_nodes = True
        principled = preview_material.node_tree.nodes.get("Principled BSDF")
        principled.inputs["Base Color"].default_value = (0.65, 0.65, 0.65, 1.0)
        principled.inputs["Roughness"].default_value = 0.9
        for mesh in meshes:
            mesh.data.materials.clear()
            mesh.data.materials.append(preview_material)

    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = args.samples
    scene.cycles.use_denoising = False
    scene.render.resolution_x = args.width
    scene.render.resolution_y = args.height
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.view_settings.exposure = args.exposure
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs["Strength"].default_value = 0.0
    if "Standard" in {item.name for item in scene.view_settings.bl_rna.properties[
            "view_transform"].enum_items}:
        scene.view_settings.view_transform = "Standard"

    camera_data = bpy.data.cameras.new("Compiled_Room_Camera")
    camera = bpy.data.objects.new("Compiled_Room_Camera", camera_data)
    scene.collection.objects.link(camera)
    scene.camera = camera
    source_unit_meters = 0.0254
    camera.location = Vector((-160, -160, 92)) * source_unit_meters
    target = Vector((-16, -16, 135)) * source_unit_meters
    camera.rotation_euler = (target - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera_data.lens = 24

    args.out.parent.mkdir(parents=True, exist_ok=True)
    scene.render.filepath = str(args.out.resolve())
    bpy.ops.render.render(write_still=True)
    if not args.out.is_file():
        raise RuntimeError("Cycles did not write the requested image")
    evidence = {
        "status": "pass", "renderer": "Blender Cycles preview", "blender": bpy.app.version_string,
        "stage_sha256": sha256(args.stage), "image_sha256": sha256(args.out),
        "width": args.width, "height": args.height, "samples": args.samples,
        "exposure": args.exposure,
        "lights_disabled": args.disable_lights,
        "meshes": len(meshes), "lights": len(lights),
        "material_policy": material_policy,
        "material_inputs_sha256": material_hashes,
    }
    args.out.with_suffix(".json").write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("WORLDSTAGE_CYCLES_PREVIEW " + json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
