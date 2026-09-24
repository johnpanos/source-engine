"""Convert a PBRT-v4 scene to a Z-up USD stage and optionally render it in Cycles.

Run inside Blender (``blender -b --factory-startup --python-exit-code 9
--python tools/quality/pbrt_usd_stage.py -- ...``); the map pipeline
(`pbrt_map_build.py`) does this for you. Every PLY becomes a USD mesh named
after its file stem with a faceVarying `st` primvar and its PBRT material; area
emitters become `LightQuadNN`/`LightDiskNN` meshes. The optional reference
render re-imports the written stage, so it measures what was exported.
"""

import argparse
import hashlib
import json
import os
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbrt_blender  # noqa: E402
import pbrt_scene  # noqa: E402

CAMERA_NAME = "ReferenceCamera"


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def import_scene(scene):
    root = Path(scene["source"]).parent
    materials = {name: pbrt_blender.build_material(scene, name)
                 for name in sorted({shape["material"] for shape in scene["shapes"]})}
    for shape in scene["shapes"]:
        path = (root / shape["filename"]).resolve()
        if bpy.ops.wm.ply_import(filepath=str(path)) != {"FINISHED"}:
            raise RuntimeError("Blender could not import " + shape["filename"])
        obj = bpy.context.object
        obj.name = obj.data.name = shape["name"]
        mesh = obj.data
        attributes = {attribute.name: attribute for attribute in mesh.attributes}
        if "u" not in attributes or "v" not in attributes:
            raise ValueError("PLY mesh lacks PBRT UVs: " + shape["filename"])
        layer = mesh.uv_layers.new(name="st")
        u = [value.value for value in attributes["u"].data]
        v = [value.value for value in attributes["v"].data]
        for loop in mesh.loops:
            layer.data[loop.index].uv = (u[loop.vertex_index], v[loop.vertex_index])
        for name in ("u", "v"):
            mesh.attributes.remove(mesh.attributes[name])
        mesh.materials.append(materials[shape["material"]])
        obj.matrix_world = pbrt_blender.PBRT_TO_USD @ pbrt_blender.matrix(
            shape["world_from_object"])
    for index, shape in enumerate(scene["emitters"]):
        pbrt_blender.add_emitter(index, shape)
    pbrt_blender.add_camera(scene, CAMERA_NAME)


def export_stage(path, scene):
    path.parent.mkdir(parents=True, exist_ok=True)
    if bpy.ops.wm.usd_export(filepath=str(path.resolve()), export_materials=True,
                             export_uvmaps=True, rename_uvmaps=False,
                             export_normals=True, export_cameras=True,
                             export_lights=True, triangulate_meshes=True,
                             export_textures_mode="NEW") != {"FINISHED"}:
        raise RuntimeError("Blender could not export the USD stage")
    root = Path(scene["source"]).parent
    for texture in scene["textures"].values():
        copied = path.parent / texture["filename"]
        if copied.is_file() and sha256(copied) != sha256(root / texture["filename"]):
            raise ValueError("USD texture copy differs from PBRT source: " + texture["filename"])


def render_reference(scene, stage, environment, out, samples, scale, device):
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the written USD stage")
    meshes = pbrt_blender.source_meshes()
    if len(meshes) != len(scene["shapes"]):
        raise ValueError("USD round trip lost PBRT meshes: %d of %d" %
                         (len(meshes), len(scene["shapes"])))
    pbrt_blender.rebind_materials(scene)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, environment)
    camera = bpy.data.objects.get(CAMERA_NAME)
    if not camera:
        raise ValueError("USD round trip lost the reference camera")
    render = bpy.context.scene
    render.camera = camera
    used = pbrt_blender.configure_cycles(samples, device)
    render.cycles.use_denoising = True
    render.render.resolution_x = max(16, round(scene["film"]["width"] * scale))
    render.render.resolution_y = max(16, round(scene["film"]["height"] * scale))
    render.render.resolution_percentage = 100
    render.render.film_transparent = False
    render.view_settings.view_transform = "Standard"
    render.render.image_settings.file_format = "PNG"
    out.parent.mkdir(parents=True, exist_ok=True)
    render.render.filepath = str(out.resolve())
    bpy.ops.render.render(write_still=True)
    exr = out.with_suffix(".exr")
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    bpy.data.images["Render Result"].save_render(filepath=str(exr.resolve()), scene=render)
    if not out.is_file() or not exr.is_file():
        raise RuntimeError("Cycles did not write the reference render")
    return {"image_sha256": sha256(out), "exr_sha256": sha256(exr),
            "width": render.render.resolution_x, "height": render.render.resolution_y,
            "samples": samples, "device": used}


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path,
                        help="equirect EXR produced by the pipeline's environment step")
    parser.add_argument("--render", type=Path, help="optional reference PNG (EXR beside it)")
    parser.add_argument("--samples", type=int, default=64)
    parser.add_argument("--scale", type=float, default=1.0)
    parser.add_argument("--device", choices=("cpu", "gpu", "auto"), default="auto")
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args(arguments)
    if not os.environ.get("OCIO") or not Path(os.environ["OCIO"]).is_file():
        parser.error("a readable OCIO configuration is required for color fidelity")
    scene = pbrt_scene.parse(args.scene)
    pbrt_blender.clear_scene()
    import_scene(scene)
    export_stage(args.stage, scene)
    receipt = {"status": "pass", "scope": "pbrt-usd-stage",
               "adapter_sha256": sha256(__file__), "source_sha256": scene["source_sha256"],
               "stage_sha256": sha256(args.stage),
               "environment_sha256": sha256(args.environment) if args.environment else None,
               "mesh_count": len(scene["shapes"]), "emitter_count": len(scene["emitters"]),
               "material_assignments": {shape["name"]: shape["material"]
                                        for shape in scene["shapes"]},
               "materials": {name: pbrt_scene.material_summary(scene, name)
                             for name in sorted({shape["material"]
                                                 for shape in scene["shapes"]})},
               "blender": bpy.app.version_string,
               "ocio_configuration_sha256": sha256(os.environ["OCIO"])}
    if args.render:
        receipt["reference_render"] = render_reference(
            scene, args.stage, args.environment, args.render, args.samples, args.scale,
            args.device)
    args.receipt.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print("PBRT_USD_STAGE " + json.dumps({key: receipt[key] for key in
                                          ("status", "mesh_count", "emitter_count")}))


if __name__ == "__main__":
    main()
