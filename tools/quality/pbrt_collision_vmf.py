#!/usr/bin/env python3
"""Build a sealed Source collision VMF for a PBRT-derived USD stage.

The USD meshes remain the visible WMSH authority; this VMF owns only gameplay
collision, a player spawn and a fallback compile light:

* a six-brush shell around the envelope meshes (all meshes by default);
* one axis-aligned solid per selected mesh (`--solid-material`,
  `--solid-mesh`), clipped to the shell interior;
* `info_player_start` on the first surface straight below the PBRT reference
  camera, facing its view and pushed clear of the shell and solids by the
  player hull; that surface's mesh also gets a floor slab when it stands
  above the shell floor (for example a hall above a stairwell).

Axis-aligned boxes are coarse on purpose; convex decomposition is future work.
Run with the OpenUSD Python (`pxr`) on PYTHONPATH.
"""

import argparse
import hashlib
import json
import math
import shutil
import sys
from pathlib import Path

from pxr import Gf, Usd, UsdGeom, UsdShade

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbrt_scene  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SOURCE_UNITS_PER_METER = 39.37007874015748
MATERIAL = "DEV/DEV_MEASUREWALL01A"
WALL = 16
PLAYER_HALF_WIDTH = 16
PLAYER_HEIGHT = 72
MIN_SOLID_THICKNESS = 4
FLOOR_SLAB = 8


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def box_text(box_id, side_id, bounds):
    xmin, ymin, zmin, xmax, ymax, zmax = bounds
    if any(not math.isfinite(v) for v in bounds) or not (
            xmin < xmax and ymin < ymax and zmin < zmax):
        raise ValueError("collision brush has invalid bounds: " + str(bounds))
    a, b, c, d = (xmin, ymin, zmin), (xmax, ymin, zmin), (xmax, ymax, zmin), (xmin, ymax, zmin)
    e, f, g, h = (xmin, ymin, zmax), (xmax, ymin, zmax), (xmax, ymax, zmax), (xmin, ymax, zmax)
    planes = ((h, g, f), (a, b, c), (b, f, g), (d, h, e), (c, g, h), (a, e, f))
    result = ["\tsolid", "\t{", f'\t\t"id" "{box_id}"']
    for index, plane in enumerate(planes):
        text = " ".join("(%g %g %g)" % point for point in plane)
        result.extend(("\t\tside", "\t\t{", f'\t\t\t"id" "{side_id + index}"',
                       f'\t\t\t"plane" "{text}"', f'\t\t\t"material" "{MATERIAL}"',
                       '\t\t\t"uaxis" "[1 0 0 0] 0.25"', '\t\t\t"vaxis" "[0 -1 0 0] 0.25"',
                       '\t\t\t"rotation" "0"', '\t\t\t"lightmapscale" "16"',
                       '\t\t\t"smoothing_groups" "0"', "\t\t}"))
    return "\n".join(result + ["\t}"])


def mesh_bounds(stage, scale):
    """World AABB, triangles (Source units) and bound material for every mesh."""
    cache = UsdGeom.XformCache()
    result = {}
    for prim in stage.Traverse():
        if not prim.IsA(UsdGeom.Mesh):
            continue
        name = prim.GetName()
        if name.startswith(("LightQuad", "LightDisk")):
            continue
        points = UsdGeom.Mesh(prim).GetPointsAttr().Get() or []
        if not points:
            continue
        transform = cache.GetLocalToWorldTransform(prim)
        world = [transform.Transform(Gf.Vec3d(point)) for point in points]
        material = UsdShade.MaterialBindingAPI(prim).ComputeBoundMaterial()[0]
        if name in result:
            raise ValueError("USD mesh names must be unique: " + name)
        counts = list(UsdGeom.Mesh(prim).GetFaceVertexCountsAttr().Get() or [])
        indices = list(UsdGeom.Mesh(prim).GetFaceVertexIndicesAttr().Get() or [])
        triangles = []
        offset = 0
        for count in counts:
            for k in range(1, count - 1):
                triangles.append((indices[offset], indices[offset + k], indices[offset + k + 1]))
            offset += count
        result[name] = {
            "material": material.GetPrim().GetName() if material else None,
            "points": [[value * scale for value in point] for point in world],
            "triangles": triangles,
            "bounds": [min(p[axis] for p in world) * scale for axis in range(3)] +
                      [max(p[axis] for p in world) * scale for axis in range(3)]}
    if not result:
        raise ValueError("USD stage has no meshes")
    return result


def union(boxes):
    return [min(box[axis] for box in boxes) for axis in range(3)] + [
        max(box[axis] for box in boxes) for axis in range(3, 6)]


def overlaps(box, point_box):
    return all(box[axis] < point_box[axis + 3] and point_box[axis] < box[axis + 3]
               for axis in range(3))


def surface_below(meshes, x, y, top):
    """Highest mesh triangle under (x, y) that lies below `top`: (z, mesh)."""
    import numpy as np
    best = (None, None)
    for name, mesh in meshes.items():
        if not mesh["triangles"]:
            continue
        points = np.asarray(mesh["points"], dtype=np.float64)
        a, b, c = (points[np.asarray(mesh["triangles"])[:, k]] for k in range(3))
        denominator = (b[:, 1] - c[:, 1]) * (a[:, 0] - c[:, 0]) + \
            (c[:, 0] - b[:, 0]) * (a[:, 1] - c[:, 1])
        valid = np.abs(denominator) > 1e-9
        denominator = np.where(valid, denominator, 1.0)
        u = ((b[:, 1] - c[:, 1]) * (x - c[:, 0]) + (c[:, 0] - b[:, 0]) * (y - c[:, 1])) / denominator
        v = ((c[:, 1] - a[:, 1]) * (x - c[:, 0]) + (a[:, 0] - c[:, 0]) * (y - c[:, 1])) / denominator
        w = 1.0 - u - v
        inside = valid & (u >= 0) & (v >= 0) & (w >= 0)
        z = u * a[:, 2] + v * b[:, 2] + w * c[:, 2]
        candidates = z[inside & (z < top)]
        if candidates.size and (best[0] is None or candidates.max() > best[0]):
            best = (float(candidates.max()), name)
    return best


def choose_spawn(scene, meshes, interior, solids, scale, stage_scale):
    """Stand on the surface below the reference camera, clear of walls and solids."""
    pose = pbrt_scene.camera_pose(scene)
    eye = [value * stage_scale * scale for value in pose["eye"]]
    margin = PLAYER_HALF_WIDTH + 2
    floor, floor_mesh = surface_below(meshes, eye[0], eye[1], eye[2])
    if floor is None:
        floor, floor_mesh = interior[2], None

    def clamp(x, y):
        return (min(max(x, interior[0] + margin), interior[3] - margin),
                min(max(y, interior[1] + margin), interior[4] - margin))

    def hull(x, y):
        return (x - PLAYER_HALF_WIDTH, y - PLAYER_HALF_WIDTH, floor + 1,
                x + PLAYER_HALF_WIDTH, y + PLAYER_HALF_WIDTH, floor + PLAYER_HEIGHT)

    forward = pose["forward"]
    start = clamp(eye[0], eye[1])
    # Walk back toward the room center until the hull is clear.
    center = ((interior[0] + interior[3]) / 2, (interior[1] + interior[4]) / 2)
    for step in range(0, 101):
        t = step / 100
        x, y = clamp(start[0] + (center[0] - start[0]) * t, start[1] + (center[1] - start[1]) * t)
        if not any(overlaps(solid, hull(x, y)) for solid in solids):
            yaw = math.degrees(math.atan2(forward[1], forward[0]))
            return {"origin": [round(x), round(y), round(floor + 4)], "yaw": round(yaw, 2),
                    "floor_mesh": floor_mesh, "floor_z": round(floor, 2),
                    "camera_eye_source_units": [round(v, 2) for v in eye],
                    "moved_from_camera_units": round(math.hypot(x - eye[0], y - eye[1]), 2)}
    raise ValueError("no clear spawn between the reference camera and the room center")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--map-name", required=True)
    parser.add_argument("--envelope-mesh", action="append", default=[])
    parser.add_argument("--solid-material", action="append", default=[])
    parser.add_argument("--solid-mesh", action="append", default=[])
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()
    if args.out_dir.exists():
        parser.error("output directory already exists: " + str(args.out_dir))
    scene = pbrt_scene.parse(args.scene)
    stage = Usd.Stage.Open(str(args.stage))
    if not stage or UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z:
        raise ValueError("collision requires a readable Z-up USD stage")
    stage_scale = UsdGeom.GetStageMetersPerUnit(stage)
    scale = SOURCE_UNITS_PER_METER * stage_scale
    meshes = mesh_bounds(stage, scale)
    missing = [name for name in args.envelope_mesh + args.solid_mesh if name not in meshes]
    materials = {mesh["material"] for mesh in meshes.values()}
    missing += [name for name in args.solid_material if name not in materials]
    if missing:
        raise ValueError("collision selection names unknown meshes/materials: " +
                         ", ".join(sorted(missing)))
    envelope = union([meshes[name]["bounds"] for name in
                      (args.envelope_mesh or sorted(meshes))])
    interior = [math.floor(envelope[0]), math.floor(envelope[1]), math.floor(envelope[2]),
                math.ceil(envelope[3]), math.ceil(envelope[4]), math.ceil(envelope[5])]
    x0, y0, z0, x1, y1, z1 = interior
    boxes = [(x0 - WALL, y0 - WALL, z0 - WALL, x1 + WALL, y1 + WALL, z0),
             (x0 - WALL, y0 - WALL, z1, x1 + WALL, y1 + WALL, z1 + WALL),
             (x0 - WALL, y0 - WALL, z0, x0, y1 + WALL, z1),
             (x1, y0 - WALL, z0, x1 + WALL, y1 + WALL, z1),
             (x0, y0 - WALL, z0, x1, y0, z1),
             (x0, y1, z0, x1, y1 + WALL, z1)]
    solids = []
    skipped = []
    solid_names = sorted({name for name, mesh in meshes.items()
                          if mesh["material"] in args.solid_material} | set(args.solid_mesh))
    for name in solid_names:
        bounds = meshes[name]["bounds"]
        clipped = [max(math.floor(bounds[a]), interior[a]) for a in range(3)] + [
            min(math.ceil(bounds[a + 3]), interior[a + 3]) for a in range(3)]
        # Slivers left by clipping produce degenerate compile faces.
        if any(clipped[a + 3] - clipped[a] < MIN_SOLID_THICKNESS for a in range(3)):
            skipped.append(name)
            continue
        solids.append(tuple(clipped))
    spawn = choose_spawn(scene, meshes, interior, solids, SOURCE_UNITS_PER_METER, stage_scale)
    if spawn["floor_mesh"] and spawn["floor_z"] - FLOOR_SLAB >= interior[2] + MIN_SOLID_THICKNESS:
        # The standing surface floats above the shell floor; give it collision.
        bounds = meshes[spawn["floor_mesh"]]["bounds"]
        top = round(spawn["floor_z"])
        slab = (max(math.floor(bounds[0]), interior[0]), max(math.floor(bounds[1]), interior[1]),
                top - FLOOR_SLAB, min(math.ceil(bounds[3]), interior[3]),
                min(math.ceil(bounds[4]), interior[4]), top)
        solids.append(slab)
        spawn["floor_slab"] = list(slab)
    light = [(x0 + x1) / 2, (y0 + y1) / 2, z1 - 8]
    lines = ["versioninfo", "{", '\t"editorversion" "400"', '\t"editorbuild" "8000"',
             '\t"mapversion" "1"', '\t"formatversion" "100"', '\t"prefab" "0"', "}",
             "visgroups", "{", "}", "world", "{", '\t"id" "1"', '\t"mapversion" "1"',
             '\t"classname" "worldspawn"', '\t"skyname" "sky_day01_01"']
    for index, bounds in enumerate(boxes + solids):
        lines.append(box_text(10 + index, 1000 + index * 6, bounds))
    lines.extend(["}", "entity", "{", '\t"id" "2"', '\t"classname" "info_player_start"',
                  '\t"origin" "%d %d %d"' % tuple(spawn["origin"]),
                  '\t"angles" "0 %g 0"' % spawn["yaw"], "}",
                  "entity", "{", '\t"id" "3"', '\t"classname" "light"',
                  '\t"origin" "%g %g %g"' % tuple(light), '\t"_light" "255 255 240 300"', "}",
                  "cameras", "{", '\t"activecamera" "-1"', "}", ""])
    args.out_dir.mkdir(parents=True)
    shutil.copytree(ROOT / "quality/fixtures/vbsp-host/game", args.out_dir / "game")
    vmf = args.out_dir / (args.map_name + "_collision.vmf")
    vmf.write_text("\n".join(lines))
    receipt = {"status": "pass", "scope": "pbrt-usd-collision-vmf",
               "stage_sha256": sha256(args.stage), "scene_sha256": scene["source_sha256"],
               "vmf": vmf.name, "vmf_sha256": sha256(vmf),
               "interior_source_units": interior, "shell_brush_count": len(boxes),
               "solid_meshes": solid_names, "solid_brush_count": len(solids),
               "skipped_outside_or_thin": skipped,
               "spawn": spawn, "policy": "axis-aligned mesh bounds; no convex decomposition"}
    (args.out_dir / "collision-receipt.json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "solid_brush_count", "spawn")}))


if __name__ == "__main__":
    main()
