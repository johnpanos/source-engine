"""Render the six cube faces of a reflection probe in Cycles.

Run inside Blender; `pbrt_map_build.py` drives it. The probe sits at the PBRT
horizontal center of the scene's bounds at the reference eye height unless
`--position` (USD stage units) is given; the eye itself can sit against a wall
(the living room camera is 1.4 cm from one), which would fill half the probe. The scene is the
baked lighting stage with the PBRT material policy, emitters and sky, so the
probe sees what the lightmap was baked from. Face orientation comes from
`reflection_probe.FACES`; faces are written as linear EXRs `<face>.exr`.
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
import pbrt_blender  # noqa: E402
import pbrt_scene  # noqa: E402
import reflection_probe  # noqa: E402


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--face-size", type=int, default=256)
    parser.add_argument("--samples", type=int, default=512)
    parser.add_argument("--device", choices=("cpu", "gpu", "auto"), default="auto")
    parser.add_argument("--position", type=float, nargs=3)
    args = parser.parse_args(arguments)
    if not os.environ.get("OCIO"):
        parser.error("OCIO is required")
    scene = pbrt_scene.parse(args.scene)
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import the lighting stage")
    pbrt_blender.rebind_materials(scene)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    device = pbrt_blender.configure_cycles(args.samples, args.device)
    render = bpy.context.scene
    render.cycles.use_denoising = True
    render.render.resolution_x = render.render.resolution_y = args.face_size
    render.render.resolution_percentage = 100
    render.render.film_transparent = False
    render.view_settings.view_transform = "Standard"
    render.render.image_settings.file_format = "OPEN_EXR"
    render.render.image_settings.color_depth = "32"
    if args.position:
        position = Vector(args.position)
    else:
        corners = [obj.matrix_world @ Vector(corner) for obj in pbrt_blender.source_meshes()
                   for corner in obj.bound_box]
        eye = pbrt_scene.camera_pose(scene)["eye"]
        position = Vector(((min(c.x for c in corners) + max(c.x for c in corners)) / 2,
                           (min(c.y for c in corners) + max(c.y for c in corners)) / 2,
                           eye[2]))
    data = bpy.data.cameras.new("ProbeCamera")
    data.type = "PERSP"
    data.sensor_fit = "HORIZONTAL"
    data.angle = math.radians(90.0)
    data.clip_start = 0.01
    camera = bpy.data.objects.new("ProbeCamera", data)
    render.collection.objects.link(camera)
    render.camera = camera
    args.out_dir.mkdir(parents=True, exist_ok=True)
    faces = {}
    for name in reflection_probe.FACES:
        forward, up, right = (Vector(tuple(v)) for v in reflection_probe.face_basis(name))
        # Blender camera space: +X right, +Y up, looking down -Z.
        camera.matrix_world = Matrix((
            (right.x, up.x, -forward.x, position.x),
            (right.y, up.y, -forward.y, position.y),
            (right.z, up.z, -forward.z, position.z),
            (0.0, 0.0, 0.0, 1.0)))
        bpy.ops.render.render()
        path = args.out_dir / (name + ".exr")
        bpy.data.images["Render Result"].save_render(filepath=str(path.resolve()), scene=render)
        if not path.is_file():
            raise RuntimeError("Cycles did not write probe face " + name)
        faces[name] = sha256(path)
    receipt = {"status": "pass", "scope": "pbrt-reflection-probe-faces",
               "stage_sha256": sha256(args.stage), "scene_sha256": scene["source_sha256"],
               "position": list(position), "face_size": args.face_size,
               "samples": args.samples, "device": device, "faces": faces,
               "blender": bpy.app.version_string}
    (args.out_dir / "probe.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print("PBRT_REFLECTION_PROBE " + json.dumps({"status": "pass", "device": device}))


if __name__ == "__main__":
    main()
