"""Bake the RFC 0011 probe volume (PRBV) of a map scene in Cycles.

Run inside Blender (``blender -b --factory-startup --python-exit-code 9
--python tools/quality/probe_volume_bake.py -- ...``); `pbrt_map_build.py`
runs it after the lightmap bake. It reads the same normalized stage and scene
model as the lightmap bake and the Cycles references, with the same material
policy (smooth normals, no normal maps) and light paths, so a probe measures
the light the lightmap and `gi_reference.py` measure.

Irradiance. Each probe owns one receiver quad per octahedral tile texel
(6 x 6), a centimetre across, at the probe position and facing that
texel's direction. The quads are invisible to every light-transport ray, so
they neither occlude nor reflect, and each maps to one texel of a bake image. A Cycles
DIFFUSE bake without colour then gives each texel exactly the diffuse light
(irradiance / pi, the lightmap unit) arriving at the probe for that normal,
from every light type with its shadows: layer `total` (direct + indirect) and
layer `indirect` (every bounce plus direct light from the sky and glowing
surfaces that the runtime does not draw), the split the lightmap layers use.

Visibility and placement. Rays from each probe against the world meshes (a
Blender BVH; dynamic models are not world and are left out) give:
  - the distance moments of each visibility texel, cos^50-weighted over the
    rays around its direction and clamped to the grid's max distance;
  - the backface fraction: a probe that sees more than a quarter backfaces is
    inside geometry and is moved toward its nearest backface; one within a
    small clearance of a surface is pushed off it. A probe still inside after
    relocation (|offset| <= max relocation per axis) is inactive.

The grid covers the world meshes' bounds inset by a small margin (or the
bounds the manifest declares, for open scenes), with the declared spacing
rounded so the probes span the bounds exactly.
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
import cpu_budget  # noqa: E402
import fork_pool  # noqa: E402
import map_scene  # noqa: E402
import pbrt_blender  # noqa: E402
import probe_volume  # noqa: E402

SOURCE_UNITS_PER_METER = 39.37007874015748
INTERIOR = probe_volume.IRRADIANCE_TILE - 2
VIS_INTERIOR = probe_volume.VISIBILITY_TILE - 2
RAYS_PER_PROBE = 4096
VISIBILITY_SHARPNESS = 50.0
BACKFACE_LIMIT = 0.25
# Fractions of the smallest spacing.
BOUNDS_INSET = 0.05
CLEARANCE = 0.05
MAX_RELOCATION = 0.45
MAX_DISTANCE = 1.5 * math.sqrt(3.0)
# Half-size of a receiver quad. The quads are invisible to light transport, so
# their size blurs nothing but the sample position; they must be large enough
# that float32 vertex positions far from the origin keep each quad's face
# normal on its texel direction (2 um quads 5 m out were tilted by ~0.05).
QUAD_HALF_METERS = 5e-3
BAKE_WIDTH = 256
SEED = 20260924


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def fibonacci_sphere(count):
    i = np.arange(count) + 0.5
    z = 1.0 - 2.0 * i / count
    r = np.sqrt(np.maximum(0.0, 1.0 - z * z))
    phi = math.pi * (3.0 - math.sqrt(5.0)) * i
    return np.stack([r * np.cos(phi), r * np.sin(phi), z], axis=-1)


def world_bvh(objects):
    vertices, polygons = [], []
    for obj in objects:
        mesh = obj.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh()
        base = len(vertices)
        vertices.extend(obj.matrix_world @ v.co for v in mesh.vertices)
        polygons.extend([base + i for i in p.vertices] for p in mesh.polygons)
        obj.to_mesh_clear()
    return BVHTree.FromPolygons(vertices, polygons, epsilon=0.0)


def trace(bvh, origin, directions, limit):
    """(distance, backface) per direction; misses are at `limit`."""
    distances = np.full(len(directions), limit)
    backface = np.zeros(len(directions), dtype=bool)
    start = Vector(origin)
    for k, d in enumerate(directions):
        direction = Vector(d)
        _, normal, _, distance = bvh.ray_cast(start, direction, limit)
        if distance is not None:
            distances[k] = distance
            backface[k] = normal.dot(direction) > 0
    return distances, backface


def place(bvh, position, directions, spacing_min):
    """(offset, active, backface fraction) of one probe, in meters."""
    limit = MAX_DISTANCE * spacing_min * 4
    offset = np.zeros(3)
    for _ in range(3):
        distances, backface = trace(bvh, position + offset, directions, limit)
        fraction = float(backface.mean())
        if fraction > BACKFACE_LIMIT:
            nearest = np.argmin(np.where(backface, distances, np.inf))
            step = directions[nearest] * (distances[nearest] + CLEARANCE * spacing_min)
        else:
            nearest = int(np.argmin(distances))
            if distances[nearest] >= CLEARANCE * spacing_min:
                return offset, True, fraction
            step = -directions[nearest] * (CLEARANCE * spacing_min - distances[nearest])
        offset = np.clip(offset + step, -MAX_RELOCATION * spacing_min,
                         MAX_RELOCATION * spacing_min)
    distances, backface = trace(bvh, position + offset, directions, limit)
    fraction = float(backface.mean())
    return offset, fraction <= BACKFACE_LIMIT, fraction


# Probe placement and visibility read only the BVH, so forked workers trace
# contiguous runs of probes; each probe's result is computed exactly as it
# would be serially, so the output does not depend on the worker count.
_TRACE = {}


def trace_run(bounds):
    """Place and trace probes [first, last) of _TRACE's grid."""
    job = _TRACE
    first, last = bounds
    homes, rays, lobes = job["homes"], job["rays"], job["lobes"]
    positions = np.zeros((last - first, 3))
    offsets = np.zeros((last - first, 3))
    active = np.zeros(last - first)
    fractions = np.zeros(last - first)
    visibility = np.zeros((last - first, VIS_INTERIOR, VIS_INTERIOR, 2), dtype=np.float32)
    for k, home in enumerate(homes[first:last]):
        offset, is_active, fractions[k] = place(job["bvh"], home, rays[::4],
                                                job["spacing_min"])
        positions[k], offsets[k], active[k] = home + offset, offset, is_active
        distances, _ = trace(job["bvh"], positions[k], rays, job["max_distance"])
        # einsum, not BLAS: a forked worker inherits the parent's OpenMP
        # BLAS state (libopenblaso, started by `texels @ rays.T`) without its
        # threads, and a threaded gemv then waits at the barrier forever.
        visibility[k, ..., 0] = np.einsum("ij,j->i", lobes, distances).reshape(
            VIS_INTERIOR, VIS_INTERIOR)
        visibility[k, ..., 1] = np.einsum("ij,j->i", lobes, distances ** 2).reshape(
            VIS_INTERIOR, VIS_INTERIOR)
    return first, positions, offsets, active, fractions, visibility


def trace_probes(bvh, homes, rays, lobes, spacing_min, max_distance, workers):
    """(positions, offsets, active, backface fractions, visibility) per probe."""
    count = len(homes)
    _TRACE.update(bvh=bvh, homes=homes, rays=rays, lobes=lobes, spacing_min=spacing_min,
                  max_distance=max_distance)
    run = max(1, min(64, count // max(1, 4 * workers)))
    runs = [(first, min(first + run, count)) for first in range(0, count, run)]
    try:
        if workers > 1 and len(runs) > 1:
            with fork_pool.ForkPool(min(workers, len(runs))) as pool:
                results = pool.map(trace_run, runs, chunksize=1)
        else:
            results = [trace_run(bounds) for bounds in runs]
    finally:
        _TRACE.clear()
    positions = np.zeros((count, 3))
    offsets = np.zeros((count, 3))
    active = np.zeros(count)
    fractions = np.zeros(count)
    visibility = np.zeros((count, VIS_INTERIOR, VIS_INTERIOR, 2), dtype=np.float32)
    for first, *parts in results:
        last = first + len(parts[0])
        for array, part in zip((positions, offsets, active, fractions, visibility), parts):
            array[first:last] = part
    return positions, offsets, active, fractions, visibility


def receiver_mesh(positions, directions):
    """One tiny quad per (probe, texel direction), each mapped to its own bake
    texel (probe p's direction t is texel p * len(directions) + t); returns
    the object, its BakeTarget node and the image height."""
    positions = np.asarray(positions, dtype=np.float64)
    directions = np.asarray(directions, dtype=np.float64)
    return pbrt_blender.receiver_mesh(
        "ProbeReceivers", "probe", np.repeat(positions, len(directions), axis=0),
        np.tile(directions, (len(positions), 1)), QUAD_HALF_METERS, BAKE_WIDTH,
        normalize=False)


def bake(obj, target, height, passes, name, out_dir):
    image = bpy.data.images.new(name, width=BAKE_WIDTH, height=height, alpha=True,
                                float_buffer=True)
    target.image = image
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    if bpy.ops.object.bake(type="DIFFUSE", pass_filter=passes) != {"FINISHED"}:
        raise RuntimeError("Cycles could not bake the probe receivers")
    path = out_dir / (name + ".exr")
    scene = bpy.context.scene
    scene.render.image_settings.file_format = "OPEN_EXR"
    scene.render.image_settings.color_depth = "32"
    image.save_render(filepath=str(path.resolve()), scene=scene)
    pixels = np.array(image.pixels[:], dtype=np.float32).reshape(height, BAKE_WIDTH, 4)
    return pixels[..., :3], path


def bake_irradiance_layers(obj, target, height, emitters, work):
    """Both layers with the lightmap baker's runtime/static light split."""
    layers = []
    images = {}
    static_emitters = pbrt_blender.has_static_emitters(emitters)
    for name, passes in (("total", {"DIRECT", "INDIRECT"}), ("indirect", {"INDIRECT"})):
        pixels, path = bake(obj, target, height, passes, "ProbeIrradiance_" + name, work)
        images[name] = {"exr": path.name, "exr_sha256": sha256(path),
                        "pass_filter": sorted(passes)}
        if name == "indirect" and static_emitters:
            with pbrt_blender.static_emitters_only(emitters):
                static, static_path = bake(obj, target, height, {"DIRECT"},
                                          "ProbeIrradiance_static_direct", work)
            pixels = pixels + static
            images[name]["static_direct"] = {
                "exr": static_path.name, "exr_sha256": sha256(static_path),
                "pass_filter": ["DIRECT"]}
        layers.append(pixels)
    return layers, images, static_emitters


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--environment", type=Path)
    parser.add_argument("--spacing", type=float, required=True, help="probe spacing, meters")
    parser.add_argument("--bounds", type=float, nargs=6,
                        help="grid bounds in meters (min xyz, max xyz); default: the world "
                             "meshes' bounds, inset")
    parser.add_argument("--fit-limit", action="store_true",
                        help="widen the spacing just enough for the grid to fit --max-probes, "
                             "instead of failing (recorded in the receipt)")
    parser.add_argument("--max-probes", type=int, default=probe_volume.MAX_PROBES,
                        help="hard probe budget (at most the PRBV limit): every "
                             "probe is traced and baked through its own receiver quads, so a "
                             "whole retail map's million probes would take hours")
    parser.add_argument("--samples", type=int, required=True)
    parser.add_argument("--device", choices=pbrt_blender.DEVICES,
                        default=pbrt_blender.DEFAULT_DEVICE)
    parser.add_argument("--light-paths", default="gi-reference")
    parser.add_argument("--out", type=Path, required=True, help="PRBV file")
    parser.add_argument("--work", type=Path, required=True, help="bake images and receipt")
    parser.add_argument("--trace-workers", type=int, default=cpu_budget.available_cpus(),
                        help="processes placing and tracing probes (the result does not "
                             "depend on it)")
    args = parser.parse_args(arguments)
    started = time.monotonic()
    scene = map_scene.parse(args.scene)
    pbrt_blender.clear_scene()
    if bpy.ops.wm.usd_import(filepath=str(args.stage.resolve()), import_materials=True,
                             import_lights=True, import_cameras=False) != {"FINISHED"}:
        raise RuntimeError("Blender could not import the normalized stage")
    meshes = pbrt_blender.source_meshes()
    if sorted(obj.name for obj in meshes) != sorted(s["name"] for s in scene["shapes"]):
        raise ValueError("normalized stage meshes differ from the scene model")
    pbrt_blender.rebind_materials(scene, normal_maps=False)
    pbrt_blender.restore_emitters(scene)
    pbrt_blender.apply_environment(scene, args.environment)
    props = map_scene.nonstatic_shape_names(scene)
    for obj in meshes:
        if obj.name in props:
            obj.hide_render = True
    world = [obj for obj in bpy.data.objects
             if obj.type == "MESH" and obj.name not in props]
    bvh = world_bvh(world)

    if args.bounds:
        low, high = np.array(args.bounds[:3]), np.array(args.bounds[3:])
    else:
        corners = np.array([obj.matrix_world @ Vector(c) for obj in world
                            for c in obj.bound_box])
        low, high = corners.min(axis=0), corners.max(axis=0)
        inset = BOUNDS_INSET * args.spacing
        low, high = low + inset, high - inset
    if (high <= low).any():
        raise ValueError("probe bounds are empty: %s .. %s" % (low, high))
    spacing = args.spacing
    if args.fit_limit:
        spacing = probe_volume.fit_spacing(low, high, args.spacing,
                                           min(args.max_probes, probe_volume.MAX_PROBES))
        if spacing > args.spacing:
            print("probe spacing %.3g m widened to %.3g m to fit %d probes" %
                  (args.spacing, spacing, args.max_probes), flush=True)
    origin, step, dims = probe_volume.grid_for(low, high, spacing)
    count = int(np.prod(dims))
    limit = min(args.max_probes, probe_volume.MAX_PROBES)
    if limit < 1 or count > limit:
        raise ValueError("%d probes exceed the declared limit %d; spacing is %.3g m" %
                         (count, limit, spacing))
    spacing_min = float(step.min())
    max_distance = MAX_DISTANCE * float(step.max())

    rays = fibonacci_sphere(RAYS_PER_PROBE)
    texels = probe_volume.interior_directions(VIS_INTERIOR).reshape(-1, 3)
    lobes = np.maximum(texels @ rays.T, 0.0) ** VISIBILITY_SHARPNESS
    lobes /= lobes.sum(axis=1, keepdims=True)
    homes = np.zeros((count, 3))
    for z in range(dims[2]):
        for y in range(dims[1]):
            for x in range(dims[0]):
                homes[probe_volume.probe_index(dims, x, y, z)] = \
                    origin + np.array([x, y, z]) * step
    positions, offsets, active, backfaces, visibility = trace_probes(
        bvh, homes, rays, lobes, spacing_min, max_distance, args.trace_workers)
    traced = time.monotonic()

    device = pbrt_blender.configure_cycles(args.samples, args.device)
    light_paths = pbrt_blender.configure_light_paths(args.light_paths)
    render = bpy.context.scene
    # Every texel takes all its samples (adaptive sampling would stop at a 1%
    # noise estimate), from a recorded seed.
    sampling = pbrt_blender.pin_sampling(seed=SEED)
    render.render.bake.use_clear = True
    render.render.bake.margin = 0
    render.render.bake.use_pass_color = False
    directions = probe_volume.interior_directions(INTERIOR).reshape(-1, 3)
    obj, target, height = receiver_mesh(positions, directions)
    args.work.mkdir(parents=True, exist_ok=True)
    pixels, images, static_emitters = bake_irradiance_layers(
        obj, target, height, scene["emitters"], args.work)
    # Bake images are stored bottom row first; texel k is row k // width.
    layers = [layer.reshape(-1, 3)[:count * len(directions)].reshape(
        count, INTERIOR, INTERIOR, 3) for layer in pixels]
    irradiance = np.stack(layers)
    irradiance[:, active < 0.5] = 0.0
    if not np.isfinite(irradiance).all() or (irradiance < 0).any():
        raise ValueError("probe bake produced negative or non-finite irradiance")

    scale = SOURCE_UNITS_PER_METER
    grid = {"origin": list(origin * scale), "spacing": list(step * scale),
            "dims": [int(v) for v in dims], "max_relocation": MAX_RELOCATION * spacing_min * scale,
            "max_distance": max_distance * scale, "irradiance": irradiance,
            "visibility": visibility * np.array([scale, scale * scale], dtype=np.float32),
            "offsets": offsets * scale, "active": active}
    data = probe_volume.build([grid], 2)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(data)
    volume = probe_volume.Volume(data)
    receipt = {"schema": "prbv-bake/v1", "status": "pass",
               "scene_sha256": scene["source_sha256"], "stage_sha256": sha256(args.stage),
               "environment_sha256": sha256(args.environment) if args.environment else None,
               "prbv": args.out.name, "prbv_sha256": hashlib.sha256(data).hexdigest(),
               "prbv_bytes": len(data), "cycles_device": device, "samples": args.samples, "seed": SEED,
               "adaptive_sampling": False, "sampling": sampling,
               "light_paths": light_paths, "normal_maps": False,
               "layers": ["total", "indirect"], "bake_images": images,
               "separation": {"runtime_emitters": len(scene["emitters"]),
                              "runtime_distant_lights": len(scene.get("distant_lights", [])),
                              "static_emitters_in_indirect": static_emitters},
               "grid": {"origin": grid["origin"], "spacing": grid["spacing"],
                        "dims": grid["dims"], "max_relocation": grid["max_relocation"],
                        "max_distance": grid["max_distance"]},
               "requested_spacing": args.spacing, "fit_limit": args.fit_limit,
               "max_probes": args.max_probes,
               "probes": count, "active_probes": int(active.sum()),
               "relocated_probes": int((np.abs(offsets).max(axis=1) > 1e-9).sum()),
               "max_backface_fraction": float(backfaces.max()),
               "rays_per_probe": RAYS_PER_PROBE, "visibility_sharpness": VISIBILITY_SHARPNESS,
               "atlas": [volume.width, volume.height],
               "seconds": {"trace": traced - started, "total": time.monotonic() - started},
               "baker_sha256": sha256(__file__)}
    (args.work / "prbv-bake.json").write_text(json.dumps(receipt, indent=2, sort_keys=True) +
                                              "\n")
    print("PRBV_BAKE " + json.dumps({k: receipt[k] for k in (
        "status", "probes", "active_probes", "relocated_probes", "prbv_bytes")}))


if __name__ == "__main__":
    main()
