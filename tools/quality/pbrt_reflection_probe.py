"""Place a map's reflection probes and render each one's cube faces in Cycles.

Run inside Blender; `pbrt_map_build.py` drives it. The scene is the baked
lighting stage with the PBRT material policy, emitters and sky, so each probe
sees what the lightmap was baked from.

Placement (`reflection_probe_set.place`, on a BVH over the scene's meshes):
room probes cover every eye-height point above a floor, one per region a
probe's proxy box contains and sees; glossy probes serve glossy surfaces
(material roughness at or below `glossy_roughness`) that no probe is near
enough to. `--position` seeds a capture (the manifest's `position`, kept for
scenes whose default placement a wall would spoil); `--max-probes 1` with a
position is the former single probe.

Each probe writes six 90-degree faces `probe<i>/<face>.exr` as multilayer
EXRs with the Combined and Depth passes: the depth pass costs no extra render
and is what `reflection_probe_set.py pack` fits the proxy box to. Face
orientation comes from `reflection_probe.FACES`. The receipt `probes.json`
records the placement, and per face a few BVH ray distances at known texels,
by which the packer checks the depth pass's convention (planar or radial)
before trusting it.

`--gbuffer` (R50-RELIGHT) adds the Diffuse Color and Normal passes to the
same faces (no extra render): with the depth pass they are the G-buffer the
packer writes as RPRB v2's relight bands, so the engine can relight each
probe with the scene's diffuse-light change since the bake.
"""

import argparse
import hashlib
import json
import math
import os
import sys
from pathlib import Path

import bpy
import numpy as np
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cycles_device  # noqa: E402
import pbrt_blender  # noqa: E402
import map_scene  # noqa: E402
import reflection_probe  # noqa: E402
import reflection_probe_set  # noqa: E402

DEPTH_SAMPLES_PER_FACE = 64


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def scene_triangles(objects):
    """World-space triangles and their shading normals, per object."""
    depsgraph = bpy.context.evaluated_depsgraph_get()
    result = {}
    for obj in objects:
        evaluated = obj.evaluated_get(depsgraph)
        mesh = evaluated.to_mesh()
        mesh.calc_loop_triangles()
        matrix = evaluated.matrix_world
        normal_matrix = matrix.to_3x3().inverted_safe().transposed()
        points = np.array([tuple(matrix @ v.co) for v in mesh.vertices], dtype=np.float64)
        corners = mesh.corner_normals
        triangles, normals = [], []
        for triangle in mesh.loop_triangles:
            triangles.append(points[list(triangle.vertices)])
            n = Vector((0.0, 0.0, 0.0))
            for loop in triangle.loops:
                n += Vector(corners[loop].vector)
            normals.append(tuple((normal_matrix @ n).normalized()))
        evaluated.to_mesh_clear()
        result[obj.name] = (np.array(triangles).reshape(-1, 3, 3),
                            np.array(normals, dtype=np.float64).reshape(-1, 3))
    return result


class BvhCaster:
    """`reflection_probe_set` ray-cast interface over every scene triangle.
    Both sides of a surface block (closed PBRT meshes need not wind
    consistently); a miss is infinite."""

    def __init__(self, triangles):
        stacked = np.concatenate([t for t, _ in triangles.values()]) if triangles else \
            np.zeros((0, 3, 3))
        vertices = [tuple(p) for p in stacked.reshape(-1, 3)]
        polygons = [(3 * i, 3 * i + 1, 3 * i + 2) for i in range(len(stacked))]
        self.tree = BVHTree.FromPolygons(vertices, polygons, all_triangles=True)

    def __call__(self, origins, directions, max_distance):
        distance = np.full(len(origins), np.inf)
        normal = np.zeros((len(origins), 3))
        for i, (origin, direction) in enumerate(zip(origins, directions)):
            location, n, _, d = self.tree.ray_cast(Vector(origin), Vector(direction),
                                                   float(max_distance))
            if location is not None:
                distance[i] = d
                normal[i] = tuple(n)
        return distance, normal


def glossy_shapes(scene, threshold):
    """Shapes whose material reflects sharply: base or clear-coat perceptual
    roughness at or below `threshold` (a textured roughness is judged by its
    scalar)."""
    result = []
    for shape in scene["shapes"]:
        summary = map_scene.material_summary(scene, shape["material"])
        roughness = summary.get("roughness", 1.0)
        coat = summary.get("clearcoat", 0.0) > 0 and \
            summary.get("clearcoat_roughness", 1.0) <= threshold
        if roughness <= threshold or coat:
            result.append(shape["name"])
    return sorted(set(result))


def face_texel_directions(name, size, rows, columns):
    forward, up, right = (np.asarray(v, dtype=np.float64)
                          for v in reflection_probe.face_basis(name))
    x = (columns + 0.5) / size * 2 - 1
    y = 1 - (rows + 0.5) / size * 2
    vectors = forward + x[:, None] * right + y[:, None] * up
    return vectors / np.linalg.norm(vectors, axis=1, keepdims=True)


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--face-size", type=int, default=256)
    parser.add_argument("--samples", type=int, default=512)
    parser.add_argument("--device", choices=pbrt_blender.DEVICES,
                        default=pbrt_blender.DEFAULT_DEVICE)
    parser.add_argument("--position", type=float, nargs=3, action="append", default=[],
                        help="a seeded capture (USD stage units); repeatable")
    parser.add_argument("--placement", default="{}",
                        help="JSON overrides of reflection_probe_set.PLACEMENT_DEFAULTS")
    parser.add_argument("--bounds", type=float, nargs=6,
                        help="placement bounds, min xyz then max xyz (default: the meshes')")
    parser.add_argument("--light-paths", default="blender-default",
                        help="Cycles light-path policy (pbrt_blender.LIGHT_PATH_POLICIES); "
                             "probes see glossy paths, so never the diffuse-only `lightmap`")
    parser.add_argument("--seed", type=int, default=pbrt_blender.SEED)
    parser.add_argument("--denoise", action=argparse.BooleanOptionalAction, default=True,
                        help="Cycles' OIDN denoiser on the faces (off for an exact probe)")
    parser.add_argument("--gbuffer", action="store_true",
                        help="also write the Diffuse Color and Normal passes (relight bands)")
    args = parser.parse_args(arguments)
    if not os.environ.get("OCIO"):
        parser.error("OCIO is required")
    params = dict(reflection_probe_set.PLACEMENT_DEFAULTS, **json.loads(args.placement))
    scene = map_scene.parse(args.scene)
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=True) != {"FINISHED"}:
        raise RuntimeError("could not import the lighting stage")
    pbrt_blender.rebind_materials(scene)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    # Dynamic models' Cycles stand-ins can move: like the lightmap bake, the
    # probes neither reflect them nor place captures around them.
    props = map_scene.prop_shape_names(scene)
    for obj in bpy.data.objects:
        if obj.name in props:
            obj.hide_render = True

    meshes = [obj for obj in pbrt_blender.source_meshes() if obj.name not in props]
    triangles = scene_triangles(meshes)
    caster = BvhCaster(triangles)
    if args.bounds:
        bounds_min, bounds_max = np.array(args.bounds[:3]), np.array(args.bounds[3:])
    else:
        corners = np.array([tuple(obj.matrix_world @ Vector(corner)) for obj in meshes
                            for corner in obj.bound_box])
        bounds_min, bounds_max = corners.min(axis=0), corners.max(axis=0)
    glossy_names = glossy_shapes(scene, params["glossy_roughness"])
    glossy_triangles = [triangles[name] for name in glossy_names if name in triangles]
    glossy = None
    if glossy_triangles:
        tris = np.concatenate([t for t, _ in glossy_triangles])
        shading = np.concatenate([n for _, n in glossy_triangles])
        points, winding = reflection_probe_set.glossy_samples(
            tris, params["glossy_samples_per_m2"], seed=args.seed)
        # Orient each sample by its triangle's shading normal (PBRT meshes
        # need not wind consistently): nearest triangle centroid.
        centroids = tris.mean(axis=1)
        nearest = np.array([int(np.argmin(np.sum((centroids - p) ** 2, axis=1)))
                            for p in points])
        normals = shading[nearest]
        glossy = (points, normals)
    seeds = [np.asarray(position, dtype=np.float64) for position in args.position]
    probes, report = reflection_probe_set.place(caster, bounds_min, bounds_max, glossy, params,
                                                seeds=seeds)

    device = pbrt_blender.configure_cycles(args.samples, args.device)
    light_paths = pbrt_blender.configure_light_paths(args.light_paths)
    sampling = pbrt_blender.pin_sampling(seed=args.seed, denoise=args.denoise)
    render = bpy.context.scene
    render.render.resolution_x = render.render.resolution_y = args.face_size
    render.render.resolution_percentage = 100
    render.render.film_transparent = False
    render.view_settings.view_transform = "Standard"
    layer = render.view_layers[0]
    layer.use_pass_combined = True
    layer.use_pass_z = True
    layer.use_pass_diffuse_color = args.gbuffer
    layer.use_pass_normal = args.gbuffer
    settings = render.render.image_settings
    if hasattr(settings, "media_type"):
        settings.media_type = "MULTI_LAYER_IMAGE"
    settings.file_format = "OPEN_EXR_MULTILAYER"
    settings.color_depth = "32"
    settings.exr_codec = "NONE"
    data = bpy.data.cameras.new("ProbeCamera")
    data.type = "PERSP"
    data.sensor_fit = "HORIZONTAL"
    data.angle = math.radians(90.0)
    data.clip_start = 0.01
    camera = bpy.data.objects.new("ProbeCamera", data)
    render.collection.objects.link(camera)
    render.camera = camera
    args.out_dir.mkdir(parents=True, exist_ok=True)
    rng = np.random.default_rng(args.seed)
    records = []
    for index, probe in enumerate(probes):
        directory = args.out_dir / ("probe%d" % index)
        directory.mkdir(parents=True, exist_ok=True)
        position = Vector(tuple(probe["capture"]))
        faces, depth_checks = {}, {}
        for name in reflection_probe.FACES:
            forward, up, right = (Vector(tuple(v)) for v in reflection_probe.face_basis(name))
            # Blender camera space: +X right, +Y up, looking down -Z.
            camera.matrix_world = Matrix((
                (right.x, up.x, -forward.x, position.x),
                (right.y, up.y, -forward.y, position.y),
                (right.z, up.z, -forward.z, position.z),
                (0.0, 0.0, 0.0, 1.0)))
            path = directory / (name + ".exr")
            render.render.filepath = str(path.resolve())
            bpy.ops.render.render(write_still=True)
            if not path.is_file():
                raise RuntimeError("Cycles did not write probe face %d %s" % (index, name))
            faces[name] = sha256(path)
            rows = rng.integers(0, args.face_size, DEPTH_SAMPLES_PER_FACE)
            columns = rng.integers(0, args.face_size, DEPTH_SAMPLES_PER_FACE)
            directions = face_texel_directions(name, args.face_size, rows, columns)
            distance, _ = caster(np.tile(probe["capture"], (len(directions), 1)), directions,
                                 1.0e4)
            depth_checks[name] = [[int(r), int(c), float(d) if np.isfinite(d) else None]
                                  for r, c, d in zip(rows, columns, distance)]
        records.append({
            "index": index, "role": probe["role"], "global": bool(probe.get("global")),
            "capture": [float(v) for v in probe["capture"]],
            "estimated_box_min": [float(v) for v in probe["box_min"]],
            "estimated_box_max": [float(v) for v in probe["box_max"]],
            "influence_min": [float(v) for v in probe["influence_min"]],
            "influence_max": [float(v) for v in probe["influence_max"]],
            "fade": float(probe["fade"]), "covers": probe["covers"],
            "dir": directory.name, "faces": faces, "depth_checks": depth_checks})
    receipt = {"status": "pass", "scope": "pbrt-reflection-probes",
               "schema": "reflection-probe-faces/v2",
               "stage_sha256": sha256(args.stage), "scene_sha256": scene["source_sha256"],
               "face_size": args.face_size, "samples": args.samples, "device": device,
               "gbuffer": args.gbuffer,
               "sampling": sampling, "light_paths": light_paths,
               "determinism": cycles_device.determinism(device, args.denoise),
               "placement": dict(params, bounds_min=[float(v) for v in bounds_min],
                                 bounds_max=[float(v) for v in bounds_max],
                                 seeds=[[float(v) for v in s] for s in seeds],
                                 glossy_shapes=glossy_names,
                                 excluded_dynamic_models=sorted(props), **report),
               "probes": records, "blender": bpy.app.version_string}
    (args.out_dir / "probes.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) +
                                              "\n")
    print("PBRT_REFLECTION_PROBE " + json.dumps({"status": "pass", "device": device,
                                                 "probes": len(records)}))


if __name__ == "__main__":
    main()
