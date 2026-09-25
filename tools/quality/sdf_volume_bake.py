"""Bake the RFC 0011 G6 signed distance volume (SDFV) of a map scene.

Run inside Blender (``blender -b --factory-startup --python-exit-code 9
--python tools/quality/sdf_volume_bake.py -- ...``); `pbrt_map_build.py` runs
it after the radiosity transfer. It reads the same normalized stage, scene
model and material policy as the other bakes; sdf_volume.py documents the
encoding.

Voxels. A uniform grid over the world meshes' bounds (dynamic props left out)
plus two voxels, at the declared voxel size (grown by 1.25x steps while the
grid would exceed the voxel cap). At each voxel centre the nearest world surface (a Blender
BVH) gives the distance, negative inside a closed (manifold, welded) mesh,
and that surface's attributes: the reflectance the radiosity bake calibrated
in Cycles (read from its receipt) (specular + diffuse x base colour or texture), and its emitted
radiance with the light style of its emissive-material source.

Lights. The scene's area emitters become rectangles (centre, half axes whose
cross product is the emitting side; two-sided emitters get a record per side;
a non-rectangular emitter becomes the square of its area, recorded in the
receipt), its distant lights distant records (the
direction light travels, angular diameter), and a dome the mean radiance of
the environment (solid-angle weighted). Their styles are the radiosity
transfer's: the same sources in the same order (radiosity_transfer_bake's
collect_sources).
"""

import argparse
import hashlib
import json
import math
import sys
import time
from pathlib import Path

import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

sys.path.insert(0, str(Path(__file__).resolve().parent))
import map_scene  # noqa: E402
import pbrt_blender  # noqa: E402
import radiosity_transfer_bake as rtb  # noqa: E402
import sdf_volume  # noqa: E402

SOURCE_UNITS_PER_METER = 39.37007874015748
MAX_VOXELS = 4_000_000
CLAMP_VOXELS = 32  # the distance clamp, in voxels


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def world_triangles(objects):
    """Triangles of the static world: points (T, 3, 3), uv (T, 3, 2), the
    owning object's index per triangle, and each object's closedness."""
    points, uvs, owner, closed = [], [], [], []
    for index, obj in enumerate(objects):
        tri, _, uv, is_closed = rtb.mesh_triangles(obj)
        closed.append(bool(is_closed))
        if not len(tri):
            continue
        points.append(tri)
        uvs.append(uv if uv is not None else np.zeros((len(tri), 3, 2)))
        owner.append(np.full(len(tri), index))
    return (np.concatenate(points), np.concatenate(uvs), np.concatenate(owner),
            np.asarray(closed))


def rect_records(obj, radiance, one_sided):
    """An area emitter as rectangle records (Source units). A planar
    rectangle is exact; any other shape (a disk, a mesh) becomes the square
    of its area at its centroid, facing its mean normal."""
    mesh = obj.data
    matrix = obj.matrix_world
    points = np.array([tuple(matrix @ v.co) for v in mesh.vertices], dtype=np.float64)
    mesh.calc_loop_triangles()
    tri = points[np.array([tuple(t.vertices) for t in mesh.loop_triangles])]
    cross = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    area = 0.5 * np.linalg.norm(cross, axis=1)
    normal = cross.sum(axis=0)
    normal /= np.linalg.norm(normal)
    centre = (tri.mean(axis=1) * area[:, None]).sum(axis=0) / area.sum()
    exact = False
    if len(points) == 4:
        edge_a, edge_b = points[1] - points[0], points[2] - points[1]
        if abs(np.dot(edge_a, edge_b)) <= 1e-4 * np.linalg.norm(edge_a) * np.linalg.norm(edge_b) \
                and np.allclose(points[0] + edge_b, points[3], atol=1e-5):
            u, v, exact = edge_a / 2, edge_b / 2, True
    if not exact:
        side = math.sqrt(area.sum())
        t0 = np.cross(normal, (0.0, 0.0, 1.0) if abs(normal[2]) < 0.9 else (1.0, 0.0, 0.0))
        t0 /= np.linalg.norm(t0)
        u, v = t0 * side / 2, np.cross(normal, t0) * side / 2
    if np.dot(np.cross(u, v), normal) < 0:
        v = -v
    records = [(centre, u, v)] + ([] if one_sided else [(centre, v, u)])
    return [{"kind": "rect", "rgb": radiance.tolist(),
             "a": (c * SOURCE_UNITS_PER_METER).tolist(),
             "b": (a * SOURCE_UNITS_PER_METER).tolist(),
             "c": (b * SOURCE_UNITS_PER_METER).tolist()} for c, a, b in records], exact


def dome_radiance(scene, environment):
    """The environment's mean radiance over the sphere (solid-angle
    weighted): the dome the producer lights escaping rays with."""
    radiance = np.asarray(scene["environment"].get("radiance") or (1.0, 1.0, 1.0), np.float64)
    if environment:
        image = bpy.data.images.load(str(environment.resolve()), check_existing=True)
        width, height = image.size
        pixels = np.array(image.pixels[:], dtype=np.float64).reshape(height, width,
                                                                    image.channels)[..., :3]
        latitude = (np.arange(height) + 0.5) / height * math.pi - math.pi / 2
        weight = np.cos(latitude)
        radiance = (pixels * weight[:, None, None]).sum(axis=(0, 1)) / (weight.sum() * width)
    return radiance


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--voxel", type=float, required=True, help="voxel size, meters")
    parser.add_argument("--transfer-receipt", type=Path, required=True,
                        help="the radiosity bake's rtrn-bake.json: its calibrated material "
                             "reflectance and source styles are reused")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    args = parser.parse_args(arguments)
    started = time.monotonic()
    scene = map_scene.parse(args.scene)
    args.work.mkdir(parents=True, exist_ok=True)
    transfer = json.loads(args.transfer_receipt.read_text())
    if transfer.get("scene_sha256") != scene["source_sha256"] or \
            transfer.get("stage_sha256") != sha256(args.stage):
        raise ValueError("the radiosity transfer receipt is for another scene or stage")
    calibration = {name: (value["specular"], value["diffuse"])
                   for name, value in transfer["material_reflectance"].items()}
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=False) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the normalized stage")
    meshes = pbrt_blender.source_meshes()
    pbrt_blender.rebind_materials(scene, normal_maps=False)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    props = map_scene.prop_shape_names(scene)
    world = [obj for obj in meshes if obj.name not in props]
    assignments = {shape["name"]: shape["material"] for shape in scene["shapes"]}
    materials = {assignments[obj.name]: obj.data.materials[0] for obj in world
                 if obj.data.materials}
    sources = rtb.collect_sources(scene, materials)
    if [(s["name"], s["style"]) for s in sources] != \
            [(s["name"], s["style"]) for s in transfer["sources"]]:
        raise ValueError("light sources differ from the radiosity transfer's")
    style_of = {s["name"]: s["style"] for s in sources}
    emissive_style = {s["material"].name: s["style"] for s in sources if s["kind"] == "emissive"}

    # The voxel grid: the world's bounds plus two voxels, under the cap.
    tri, uv, owner, closed = world_triangles(world)
    normals = np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0])
    normals /= np.maximum(np.linalg.norm(normals, axis=1, keepdims=True), 1e-12)
    low, high = tri.reshape(-1, 3).min(axis=0), tri.reshape(-1, 3).max(axis=0)
    voxel = args.voxel
    while True:
        lo = low - 2 * voxel
        dims = np.ceil((high + 2 * voxel - lo) / voxel).astype(int) + 1
        if int(np.prod(dims)) <= MAX_VOXELS:
            break
        voxel *= 1.25
    max_distance = CLAMP_VOXELS * voxel

    # Nearest surface per voxel centre (x fastest).
    bvh = BVHTree.FromPolygons([Vector(p) for p in tri.reshape(-1, 3)],
                               np.arange(3 * len(tri)).reshape(-1, 3).tolist(), epsilon=0.0)
    zz, yy, xx = np.meshgrid(np.arange(dims[2]), np.arange(dims[1]), np.arange(dims[0]),
                             indexing="ij")
    centres = lo + np.stack([xx, yy, zz], axis=-1).reshape(-1, 3) * voxel
    count = len(centres)
    nearest = np.zeros((count, 3))
    face = np.full(count, -1, np.int64)
    distance = np.full(count, max_distance)
    for k, p in enumerate(centres):
        location, _, index, dist = bvh.find_nearest(Vector(p), max_distance)
        if location is not None:
            nearest[k], face[k], distance[k] = location, index, dist
    traced = time.monotonic()

    hit = face >= 0
    f = face[hit]
    # Inside a closed mesh: behind the nearest face.
    behind = np.einsum("ij,ij->i", centres[hit] - nearest[hit], normals[f]) < 0
    sign = np.ones(count)
    sign[np.flatnonzero(hit)[behind & closed[owner[f]]]] = -1.0
    distance = np.minimum(distance, max_distance) * sign
    # Barycentrics of the nearest points, for the texture lookup.
    a, b, c = tri[f, 0], tri[f, 1], tri[f, 2]
    v0, v1, v2 = b - a, c - a, nearest[hit] - a
    d00, d01, d11 = (v0 * v0).sum(1), (v0 * v1).sum(1), (v1 * v1).sum(1)
    d20, d21 = (v2 * v0).sum(1), (v2 * v1).sum(1)
    den = np.where(np.abs(d00 * d11 - d01 * d01) > 1e-20, d00 * d11 - d01 * d01, 1.0)
    w1, w2 = (d11 * d20 - d01 * d21) / den, (d00 * d21 - d01 * d20) / den
    coords = ((1 - w1 - w2)[:, None] * uv[f, 0] + w1[:, None] * uv[f, 1] +
              w2[:, None] * uv[f, 2])
    albedo = rtb.Albedo(scene, calibration)
    reflectance = np.zeros((count, 3))
    emission = np.zeros((count, 3))
    source = np.full(count, sdf_volume.NO_SOURCE, np.uint16)
    hit_index = np.flatnonzero(hit)
    material_of = np.array([assignments[obj.name] for obj in world])
    for material in sorted(set(material_of)):
        members = np.isin(owner[f], np.flatnonzero(material_of == material))
        if not members.any():
            continue
        where = hit_index[members]
        reflectance[where] = albedo.evaluate(material, coords[members])
        summary = map_scene.material_summary(scene, material)
        colour = summary["emission_color"]
        blender = materials[material].name if material in materials else None
        if colour and max(colour[:3]) > 0 and blender in emissive_style:
            emission[where] = colour[:3]
            style = emissive_style[blender]
            source[where] = style if style >= 0 else sdf_volume.NO_SOURCE

    # Analytic lights, styled as the radiosity transfer's sources.
    lights, approximated = [], []
    for index, shape in enumerate(scene["emitters"]):
        name = rtb.prim_name(shape.get("source") or pbrt_blender.emitter_name(index, shape))
        obj = bpy.data.objects.get(pbrt_blender.emitter_name(index, shape))
        radiance = np.asarray(shape["emission"]["radiance"], np.float64) * \
            shape["emission"]["scale"]
        records, exact = rect_records(obj, radiance, shape["emission"].get("one_sided", False))
        if not exact:
            approximated.append(name)
        for record in records:
            record["style"] = style_of.get(name, -1)
        lights += records
    for index, light in enumerate(scene.get("distant_lights", [])):
        name = rtb.prim_name(light.get("source") or "Sun%02d" % index)
        direction = np.asarray(light["direction"], np.float64)
        lights.append({"kind": "distant", "style": style_of.get(name, -1),
                       "rgb": list(light["irradiance"]),
                       "a": (direction / np.linalg.norm(direction)).tolist(),
                       "b": (math.radians(light["angle_degrees"]), 0.0, 0.0)})
    if scene["environment"]:
        name = rtb.prim_name(scene["environment"].get("source") or "Sky")
        lights.append({"kind": "dome", "style": style_of.get(name, -1),
                       "rgb": dome_radiance(scene, args.environment).tolist()})

    shape = (dims[2], dims[1], dims[0])
    data = sdf_volume.build((lo * SOURCE_UNITS_PER_METER).tolist(),
                            voxel * SOURCE_UNITS_PER_METER, dims.tolist(),
                            (distance * SOURCE_UNITS_PER_METER).reshape(shape),
                            reflectance.reshape(shape + (3,)), emission.reshape(shape + (3,)),
                            source.reshape(shape), lights,
                            max_distance * SOURCE_UNITS_PER_METER)
    volume = sdf_volume.Volume(data)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(data)
    receipt = {"schema": "sdfv-bake/v1", "status": "pass",
               "scene_sha256": scene["source_sha256"], "stage_sha256": sha256(args.stage),
               "sdfv": args.out.name, "sdfv_sha256": hashlib.sha256(data).hexdigest(),
               "sdfv_bytes": len(data), "voxel_m": voxel, "dims": dims.tolist(),
               "lights": lights, "approximated_emitters": approximated,
               "sources": [{k: s[k] for k in ("name", "kind", "style")} for s in sources],
               "closed_meshes": int(closed.sum()), "meshes": len(world),
               "info": {k: v for k, v in volume.info().items() if k != "lights"},
               "seconds": {"nearest": traced - started, "total": time.monotonic() - started},
               "baker_sha256": sha256(__file__)}
    (args.work / "sdfv-bake.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print("SDFV_BAKE " + json.dumps({"status": "pass", "dims": dims.tolist(),
                                     "bytes": len(data), "lights": len(lights)}))


if __name__ == "__main__":
    main()
