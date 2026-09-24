"""Render one GI fixture state in Cycles with separated light passes.

Run inside Blender (``blender -b --factory-startup --python-exit-code 9
--python tools/quality/gi_reference_blender.py -- ...``); `gi_reference.py`
does this for you. The normalized stage and scene model come from
`usd_scene.py extract`, so the reference renders exactly what the map
pipeline exports, with the pipeline's material policy (`pbrt_blender`).

Per camera it writes one uncompressed multilayer EXR holding Combined,
DiffDir, DiffInd, DiffCol, GlossDir, GlossInd, Emit, Env, IndexOB and Depth.
Denoising is off: the references are unbiased estimates, and the recorded
seed and sample count reproduce them on the recorded device.
"""

import argparse
import hashlib
import json
import math
import os
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_scene  # noqa: E402
import pbrt_blender  # noqa: E402

PASSES = ("use_pass_combined", "use_pass_diffuse_direct", "use_pass_diffuse_indirect",
          "use_pass_diffuse_color", "use_pass_glossy_direct", "use_pass_glossy_indirect",
          "use_pass_emit", "use_pass_environment", "use_pass_object_index", "use_pass_z")


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def add_camera(name, pose, film, horizontal_fov):
    data = bpy.data.cameras.new(name)
    camera = bpy.data.objects.new(name, data)
    bpy.context.scene.collection.objects.link(camera)
    forward = Vector(pose["forward"]).normalized()
    up = Vector(pose["up"])
    right = forward.cross(up).normalized()
    up = right.cross(forward).normalized()
    eye = pose["eye"]
    camera.matrix_world = Matrix(((right.x, up.x, -forward.x, eye[0]),
                                  (right.y, up.y, -forward.y, eye[1]),
                                  (right.z, up.z, -forward.z, eye[2]),
                                  (0.0, 0.0, 0.0, 1.0)))
    data.type = "PERSP"
    data.sensor_fit = "HORIZONTAL"
    data.angle_x = math.radians(horizontal_fov)
    data.clip_start = 0.01
    data.clip_end = 1000.0
    return camera


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--cameras", type=Path, required=True,
                        help="JSON {film, horizontal_fov_degrees, cameras: {name: pose}}")
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--samples", type=int, required=True)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("--device", choices=("cpu", "gpu", "auto"), default="auto")
    parser.add_argument("--light-paths", default="gi-reference")
    args = parser.parse_args(arguments)
    if not os.environ.get("OCIO") or not Path(os.environ["OCIO"]).is_file():
        parser.error("a readable OCIO configuration is required for color fidelity")
    scene = map_scene.parse(args.scene)
    views = json.loads(args.cameras.read_text())
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=False) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the normalized stage")
    meshes = pbrt_blender.source_meshes()
    if sorted(obj.name for obj in meshes) != sorted(s["name"] for s in scene["shapes"]):
        raise ValueError("normalized stage meshes differ from the scene model")
    pbrt_blender.rebind_materials(scene)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    # Object indices for region masks: 1..N over sorted mesh names, emitters after.
    objects = sorted((obj for obj in bpy.data.objects if obj.type == "MESH"),
                     key=lambda obj: obj.name)
    index = {}
    for number, obj in enumerate(objects, start=1):
        obj.pass_index = number
        index[obj.name] = number
    render = bpy.context.scene
    device = pbrt_blender.configure_cycles(args.samples, args.device)
    light_paths = pbrt_blender.configure_light_paths(args.light_paths)
    render.cycles.use_denoising = False
    render.cycles.seed = args.seed
    render.cycles.use_animated_seed = False
    render.cycles.use_adaptive_sampling = False
    render.render.film_transparent = False
    render.view_settings.view_transform = "Standard"
    film = views["film"]
    render.render.resolution_x, render.render.resolution_y = film["width"], film["height"]
    render.render.resolution_percentage = 100
    layer = render.view_layers[0]
    for name in PASSES:
        setattr(layer, name, True)
    settings = render.render.image_settings
    # Blender 5 selects multilayer EXR through the media type.
    if hasattr(settings, "media_type"):
        settings.media_type = "MULTI_LAYER_IMAGE"
        settings.file_format = "OPEN_EXR_MULTILAYER"
    else:
        settings.file_format = "OPEN_EXR_MULTILAYER"
    settings.color_depth = "32"
    settings.exr_codec = "NONE"
    args.out_dir.mkdir(parents=True, exist_ok=True)
    outputs = {}
    for name, pose in sorted(views["cameras"].items()):
        camera = add_camera("GiCamera_" + name, pose, film, views["horizontal_fov_degrees"])
        render.camera = camera
        path = args.out_dir / (name + ".exr")
        render.render.filepath = str(path.resolve())
        bpy.ops.render.render(write_still=True)
        if not path.is_file():
            raise RuntimeError("Cycles did not write " + str(path))
        outputs[name] = {"exr": path.name, "exr_sha256": sha256(path)}
    receipt = {"schema": "gi-cycles-render/v1", "status": "pass",
               "blender": bpy.app.version_string,
               "cycles_device": device, "samples": args.samples, "seed": args.seed,
               "denoising": False, "adaptive_sampling": False, "light_paths": light_paths,
               "film": film, "horizontal_fov_degrees": views["horizontal_fov_degrees"],
               "scene_sha256": scene["source_sha256"], "stage_sha256": sha256(args.stage),
               "environment_sha256": sha256(args.environment) if args.environment else None,
               "object_index": index, "renders": outputs,
               "renderer_sha256": sha256(__file__),
               "ocio_configuration_sha256": sha256(os.environ["OCIO"])}
    (args.out_dir / "render.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) +
                                              "\n")
    print("GI_REFERENCE_RENDER " + json.dumps({"status": "pass", "cameras": len(outputs)}))


if __name__ == "__main__":
    main()
