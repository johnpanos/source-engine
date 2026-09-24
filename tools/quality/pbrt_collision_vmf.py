#!/usr/bin/env python3
"""Build a sealed Source collision VMF for a PBRT-derived USD stage.

The USD meshes remain the visible WMSH authority; this VMF owns only gameplay
collision, a player spawn and a fallback compile light:

* a six-brush shell around the envelope meshes (all meshes by default);
* one convex 18-DOP brush (axes plus the six edge diagonals) per connected
  component of each selected mesh (`--solid-material`, `--solid-mesh`),
  clipped to the shell interior, so separate stair treads or cushions become
  separate brushes;
* `info_player_start` on the first surface straight below the PBRT reference
  camera, facing its view and pushed clear of the shell and solids by the
  player hull; when that surface stands above the shell floor (a hall over a
  stairwell) its upward-facing triangles are extruded into prism brushes, so
  openings in the floor stay open.

The solids and floor prisms form one nodraw `func_detail`: they block the player but
not vis, since the render mesh is not their hull, and the engine walks their
leaves, so the world mesh inside them stays visible.

A k-DOP is exact for box-like parts and conservative for curved ones.
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
import map_scene  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
SOURCE_UNITS_PER_METER = 39.37007874015748
MATERIAL = "DEV/DEV_MEASUREWALL01A"
# Collision-only solids: the WMSH mesh is the visible world, so they need no
# faces (and chopped detail faces can be zero-area slivers).
NODRAW_MATERIAL = "TOOLS/TOOLSNODRAW"
WALL = 16
PLAYER_HALF_WIDTH = 16
PLAYER_HEIGHT = 72
# Thinner solids are skipped. vbsp2 handles thin brushes; earlier "sliver"
# compile failures were its first-three-points winding test (now Newell).
MIN_SOLID_THICKNESS = 1
FLOOR_SLAB = 8
MAX_COMPONENTS = 512


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


KDOP_DIRECTIONS = [(1, 0, 0), (0, 1, 0), (0, 0, 1), (1, 1, 0), (1, -1, 0), (1, 0, 1),
                   (1, 0, -1), (0, 1, 1), (0, 1, -1)]


def box_planes(bounds):
    xmin, ymin, zmin, xmax, ymax, zmax = bounds
    if any(not math.isfinite(v) for v in bounds) or not (
            xmin < xmax and ymin < ymax and zmin < zmax):
        raise ValueError("collision brush has invalid bounds: " + str(bounds))
    return [((1, 0, 0), xmax), ((-1, 0, 0), -xmin), ((0, 1, 0), ymax),
            ((0, -1, 0), -ymin), ((0, 0, 1), zmax), ((0, 0, -1), -zmin)]


def kdop_planes(points):
    import numpy as np
    planes = []
    for direction in KDOP_DIRECTIONS:
        normal = np.asarray(direction, dtype=np.float64)
        normal /= np.linalg.norm(normal)
        extent = points @ normal
        planes += [(tuple(normal), float(extent.max())), (tuple(-normal), float(-extent.min()))]
    return planes


def convex_brush(planes, minimum_radius):
    """Trim a half-space set (n.x <= d) to the planes that bound faces.

    Returns (planes, vertices), or None when the solid is thinner than
    2 * minimum_radius; vbsp rejects slivers and faceless sides.
    """
    import numpy as np
    from scipy.optimize import linprog
    from scipy.spatial import HalfspaceIntersection
    normals = np.array([plane[0] for plane in planes], dtype=np.float64)
    offsets = np.array([plane[1] for plane in planes], dtype=np.float64)
    norms = np.linalg.norm(normals, axis=1)
    normals, offsets = normals / norms[:, None], offsets / norms
    # Chebyshev center: the deepest interior point and its inscribed radius.
    program = linprog(np.r_[np.zeros(3), -1.0], A_ub=np.c_[normals, np.ones(len(planes))],
                      b_ub=offsets, bounds=[(None, None)] * 3 + [(0, None)])
    if not program.success or program.x[3] < minimum_radius:
        return None
    hull = HalfspaceIntersection(np.c_[normals, -offsets], program.x[:3])
    vertices = hull.intersections
    kept = []
    for normal, offset in zip(normals, offsets):
        on_plane = vertices[np.abs(vertices @ normal - offset) < 1e-3]
        if len(on_plane) >= 3 and np.linalg.matrix_rank(on_plane - on_plane[0], tol=1e-3) >= 2:
            if not any(np.allclose(normal, other) for other, _ in kept):
                kept.append((normal, float(offset)))
    return kept, vertices


def brush_text(brush_id, side_id, planes, vertices, material=MATERIAL):
    """VMF solid; vbsp's plane normal is cross(p0 - p1, p2 - p1), outward."""
    import numpy as np
    result = ["\tsolid", "\t{", f'\t\t"id" "{brush_id}"']
    for index, (normal, offset) in enumerate(planes):
        on_plane = vertices[np.abs(vertices @ normal - offset) < 1e-3]
        center = on_plane.mean(axis=0)
        center += normal * (offset - center @ normal)
        helper = np.array((0.0, 0.0, 1.0)) if abs(normal[2]) < 0.9 else np.array((1.0, 0.0, 0.0))
        u = np.cross(helper, normal)
        u /= np.linalg.norm(u)
        v = np.cross(normal, u)
        points = (center + 64 * u, center, center + 64 * v)
        text = " ".join("(%.4f %.4f %.4f)" % tuple(point) for point in points)
        result.extend(("\t\tside", "\t\t{", f'\t\t\t"id" "{side_id + index}"',
                       f'\t\t\t"plane" "{text}"', f'\t\t\t"material" "{material}"',
                       '\t\t\t"uaxis" "[1 0 0 0] 0.25"', '\t\t\t"vaxis" "[0 -1 0 0] 0.25"',
                       '\t\t\t"rotation" "0"', '\t\t\t"lightmapscale" "16"',
                       '\t\t\t"smoothing_groups" "0"', "\t\t}"))
    return "\n".join(result + ["\t}"])


def components(points, triangles, weld=0.05):
    """Triangle groups connected through welded vertex positions."""
    import numpy as np
    keys = np.round(np.asarray(points) / weld).astype(np.int64)
    _, welded = np.unique(keys, axis=0, return_inverse=True)
    welded = welded.ravel()
    parent = list(range(int(welded.max()) + 1))

    def find(item):
        while parent[item] != item:
            parent[item] = parent[parent[item]]
            item = parent[item]
        return item
    for triangle in triangles:
        roots = [find(int(welded[index])) for index in triangle]
        for root in roots[1:]:
            parent[root] = roots[0]
    groups = {}
    for triangle in triangles:
        groups.setdefault(find(int(welded[triangle[0]])), []).append(triangle)
    return list(groups.values())


def floor_prisms(points, triangles, thickness):
    """One prism brush per upward-facing triangle, extruded downward."""
    import numpy as np
    points = np.asarray(points, dtype=np.float64)
    prisms = []
    for triangle in triangles:
        a, b, c = points[list(triangle)]
        normal = np.cross(b - a, c - a)
        if np.linalg.norm(normal) < 1e-6:
            continue
        normal /= np.linalg.norm(normal)
        if normal[2] < 0:
            normal = -normal
        if normal[2] < 0.7:
            continue
        planes = [(tuple(normal), float(normal @ a)),
                  ((0.0, 0.0, -1.0), -(min(a[2], b[2], c[2]) - thickness))]
        centroid = (a + b + c) / 3
        for p, q in ((a, b), (b, c), (c, a)):
            side = np.cross(q - p, (0.0, 0.0, 1.0))
            if np.linalg.norm(side) < 1e-6:
                break
            side /= np.linalg.norm(side)
            if side @ (centroid - p) > 0:
                side = -side
            planes.append((tuple(side), float(side @ p)))
        else:
            prisms.append(planes)
    return prisms


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


def walkable_tops(solids, clearance=PLAYER_HEIGHT, minimum=PLAYER_HALF_WIDTH, step=4.0):
    """Horizontal brush tops a player can stand on, for the drop test.

    The hull (32 units) may be wider than a stair tread, so search positions
    whose center lies on the top face and whose 72-unit column is clear;
    overhanging the open side of a step is how players stand on stairs.
    """
    import numpy as np
    boxes = [aabb(vertices) for _, vertices in solids]
    tops = []
    for index, (planes, vertices) in enumerate(solids):
        for normal, offset in planes:
            if normal[2] < 0.999:
                continue
            face = vertices[np.abs(vertices @ normal - offset) < 1e-3]
            low, high = face.min(axis=0), face.max(axis=0)
            if (high[0] - low[0]) < minimum or (high[1] - low[1]) < minimum:
                continue
            z = float(offset)
            center = ((low[0] + high[0]) / 2, (low[1] + high[1]) / 2)
            candidates = sorted(
                ((x, y) for x in np.arange(low[0] + 1, high[0], step)
                 for y in np.arange(low[1] + 1, high[1], step)),
                key=lambda point: (point[0] - center[0]) ** 2 + (point[1] - center[1]) ** 2)
            for x, y in candidates[:400]:
                column = (x - PLAYER_HALF_WIDTH, y - PLAYER_HALF_WIDTH, z + 0.5,
                          x + PLAYER_HALF_WIDTH, y + PLAYER_HALF_WIDTH, z + clearance)
                if not any(other != index and overlaps(box, column)
                           for other, box in enumerate(boxes)):
                    tops.append({"x": round(float(x), 2), "y": round(float(y), 2),
                                 "z": round(z, 2), "brush": index})
                    break
    return tops


def aabb(vertices):
    return [float(v) for v in vertices.min(axis=0)] + [float(v) for v in vertices.max(axis=0)]


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
    pose = map_scene.camera_pose(scene)
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
    scene = map_scene.parse(args.scene)
    stage = Usd.Stage.Open(str(args.stage))
    if not stage or UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z:
        raise ValueError("collision requires a readable Z-up USD stage")
    stage_scale = UsdGeom.GetStageMetersPerUnit(stage)
    scale = SOURCE_UNITS_PER_METER * stage_scale
    meshes = mesh_bounds(stage, scale)
    # Dynamic-model stand-ins are entities, not world: no shell, no solid.
    prop_shapes = map_scene.prop_shape_names(scene)
    meshes = {name: mesh for name, mesh in meshes.items() if name not in prop_shapes}
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
    # An open world (a floor under the sky) has no enclosing geometry; its
    # shell still needs a player's headroom, or the top and bottom brushes
    # coincide.
    headroom_raised = interior[5] < interior[2] + 2 * PLAYER_HEIGHT
    interior[5] = max(interior[5], interior[2] + 2 * PLAYER_HEIGHT)
    x0, y0, z0, x1, y1, z1 = interior
    shell = [(x0 - WALL, y0 - WALL, z0 - WALL, x1 + WALL, y1 + WALL, z0),
             (x0 - WALL, y0 - WALL, z1, x1 + WALL, y1 + WALL, z1 + WALL),
             (x0 - WALL, y0 - WALL, z0, x0, y1 + WALL, z1),
             (x1, y0 - WALL, z0, x1 + WALL, y1 + WALL, z1),
             (x0, y0 - WALL, z0, x1, y0, z1),
             (x0, y1, z0, x1, y1 + WALL, z1)]
    brushes = [convex_brush(box_planes(bounds), 0.5) for bounds in shell]
    clip = box_planes(interior)
    solids = []
    skipped = []
    solid_names = sorted({name for name, mesh in meshes.items()
                          if mesh["material"] in args.solid_material} | set(args.solid_mesh))
    import numpy as np
    for name in solid_names:
        mesh = meshes[name]
        points = np.asarray(mesh["points"], dtype=np.float64)
        groups = components(points, mesh["triangles"])
        if len(groups) > MAX_COMPONENTS:
            groups = [mesh["triangles"]]
        made = 0
        for group in groups:
            used = points[sorted({index for triangle in group for index in triangle})]
            brush = convex_brush(kdop_planes(used) + clip, MIN_SOLID_THICKNESS / 2)
            if brush:
                solids.append(brush)
                made += 1
        if not made:
            skipped.append(name)
    spawn = choose_spawn(scene, meshes, interior,
                         [aabb(vertices) for _, vertices in solids],
                         SOURCE_UNITS_PER_METER, stage_scale)
    if spawn["floor_mesh"] and spawn["floor_z"] - FLOOR_SLAB >= interior[2] + MIN_SOLID_THICKNESS:
        # The standing surface floats above the shell floor; give it real collision.
        floor = meshes[spawn["floor_mesh"]]
        prisms = [convex_brush(planes + clip, 0.25)
                  for planes in floor_prisms(floor["points"], floor["triangles"], FLOOR_SLAB)]
        prisms = [prism for prism in prisms if prism]
        solids += prisms
        spawn["floor_prisms"] = len(prisms)
    elif spawn["floor_mesh"] and spawn["floor_z"] > interior[2]:
        # Too thin for a slab (the shell mesh reaches below the walkable floor,
        # e.g. wall thickness): raise the shell floor to the standing surface.
        interior[2] = z0 = math.floor(spawn["floor_z"])
        brushes[0] = convex_brush(box_planes((x0 - WALL, y0 - WALL, z0 - WALL,
                                              x1 + WALL, y1 + WALL, z0)), 0.5)
        for side in range(2, 6):
            bounds = list(shell[side])
            bounds[2] = z0
            brushes[side] = convex_brush(box_planes(bounds), 0.5)
        spawn["shell_floor_raised_to"] = z0
    tops = walkable_tops(solids)
    light = [(x0 + x1) / 2, (y0 + y1) / 2, z1 - 8]
    lines = ["versioninfo", "{", '\t"editorversion" "400"', '\t"editorbuild" "8000"',
             '\t"mapversion" "1"', '\t"formatversion" "100"', '\t"prefab" "0"', "}",
             "visgroups", "{", "}", "world", "{", '\t"id" "1"', '\t"mapversion" "1"',
             '\t"classname" "worldspawn"', '\t"skyname" "sky_day01_01"']
    side = 1000
    for index, (planes, vertices) in enumerate(brushes):
        lines.append(brush_text(10 + index, side, planes, vertices))
        side += len(planes)
    lines.append("}")
    # Furniture and floor solids are func_detail: they collide like world
    # brushes but do not split vis clusters or make the view see them as
    # opaque (the render mesh is not their hull), and the engine walks their
    # leaves, so WMSH meshlets inside them stay visible through them.
    if solids:
        lines.extend(["entity", "{", '\t"id" "4"', '\t"classname" "func_detail"'])
        for index, (planes, vertices) in enumerate(solids):
            lines.append(brush_text(10 + len(brushes) + index, side, planes, vertices,
                                    NODRAW_MATERIAL))
            side += len(planes)
        lines.append("}")
    lines.extend(["entity", "{", '\t"id" "2"', '\t"classname" "info_player_start"',
                  '\t"origin" "%d %d %d"' % tuple(spawn["origin"]),
                  '\t"angles" "0 %g 0"' % spawn["yaw"], "}",
                  "entity", "{", '\t"id" "3"', '\t"classname" "light"',
                  '\t"origin" "%g %g %g"' % tuple(light), '\t"_light" "255 255 240 300"', "}"])
    placed = []
    for index, prop in enumerate(map_scene.props(scene)):
        origin = [value * SOURCE_UNITS_PER_METER for value in prop["origin_m"]]
        model = map_scene.prop_model_path(args.map_name, prop)
        lines.extend(["entity", "{", '\t"id" "%d"' % (100 + index),
                      # _override: placed as authored even when the model carries
                      # physics prop data (a plain prop_dynamic deletes itself).
                      '\t"classname" "prop_dynamic_override"',
                      '\t"targetname" "%s"' % prop["name"],
                      '\t"model" "%s"' % model, '\t"origin" "%.3f %.3f %.3f"' % tuple(origin),
                      '\t"angles" "0 0 0"', '\t"solid" "0"', '\t"DisableShadows" "1"', "}"])
        placed.append({"name": prop["name"], "model": model, "source_model": prop["model"],
                       "origin_source_units": origin})
    lines.extend(["cameras", "{", '\t"activecamera" "-1"', "}", ""])
    args.out_dir.mkdir(parents=True)
    shutil.copytree(ROOT / "quality/fixtures/vbsp-host/game", args.out_dir / "game")
    vmf = args.out_dir / (args.map_name + "_collision.vmf")
    vmf.write_text("\n".join(lines))
    receipt = {"status": "pass", "scope": "pbrt-usd-collision-vmf",
               "stage_sha256": sha256(args.stage), "scene_sha256": scene["source_sha256"],
               "vmf": vmf.name, "vmf_sha256": sha256(vmf),
               "interior_source_units": interior, "shell_brush_count": len(brushes),
               "shell_raised_to_headroom": headroom_raised,
               "solid_meshes": solid_names, "solid_brush_count": len(solids),
               "skipped_outside_or_thin": skipped,
               "spawn": spawn, "walkable_tops": tops, "dynamic_models": placed,
               "policy": "18-DOP per connected component; floor triangles extruded; "
                         "solids are func_detail"}
    (args.out_dir / "collision-receipt.json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: receipt[k] for k in ("status", "solid_brush_count", "spawn")}))


if __name__ == "__main__":
    main()
