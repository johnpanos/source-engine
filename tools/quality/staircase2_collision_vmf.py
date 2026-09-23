#!/usr/bin/env python3
"""Build a sealed Portal collision VMF from the staircase2 USD reference stage.

The imported USD mesh remains the visible BSP2 WMSH authority. This VMF owns
only gameplay collision, a player spawn and a fallback compile light.
"""

import argparse
import hashlib
import json
import math
import shutil
from collections import defaultdict
from pathlib import Path

from pxr import Gf, Usd, UsdGeom


ROOT = Path(__file__).resolve().parents[2]
SOURCE_UNITS_PER_METER = 39.37007874015748
MATERIAL = "DEV/DEV_MEASUREWALL01A"


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def world_points(stage, name):
    prim = stage.GetPrimAtPath("/root/" + name + "/" + name)
    if not prim or not prim.IsA(UsdGeom.Mesh):
        raise ValueError("staircase2 USD is missing " + name)
    mesh = UsdGeom.Mesh(prim)
    transform = UsdGeom.XformCache().GetLocalToWorldTransform(prim)
    points = [transform.Transform(Gf.Vec3d(point)) for point in mesh.GetPointsAttr().Get()]
    return mesh, points


def box_text(box_id, side_id, bounds):
    xmin, ymin, zmin, xmax, ymax, zmax = bounds
    if any(not math.isfinite(v) for v in bounds) or not (xmin < xmax and ymin < ymax and zmin < zmax):
        raise ValueError("collision brush has invalid bounds")
    a = (xmin, ymin, zmin)
    b = (xmax, ymin, zmin)
    c = (xmax, ymax, zmin)
    d = (xmin, ymax, zmin)
    e = (xmin, ymin, zmax)
    f = (xmax, ymin, zmax)
    g = (xmax, ymax, zmax)
    h = (xmin, ymax, zmax)
    planes = ((h, g, f), (a, b, c), (b, f, g),
              (d, h, e), (c, g, h), (a, e, f))
    result = ['\tsolid', '\t{', f'\t\t"id" "{box_id}"']
    for index, plane in enumerate(planes):
        def point(value):
            return "(%g %g %g)" % value
        result.extend(('\t\tside', '\t\t{', f'\t\t\t"id" "{side_id + index}"',
                       '\t\t\t"plane" "' + ' '.join(map(point, plane)) + '"',
                       f'\t\t\t"material" "{MATERIAL}"',
                       '\t\t\t"uaxis" "[1 0 0 0] 0.25"',
                       '\t\t\t"vaxis" "[0 -1 0 0] 0.25"',
                       '\t\t\t"rotation" "0"',
                       '\t\t\t"lightmapscale" "16"',
                       '\t\t\t"smoothing_groups" "0"', '\t\t}'))
    return '\n'.join(result + ['\t}'])


def collision_boxes(stage):
    floor_mesh, floor_points = world_points(stage, "Mesh011")
    if len(floor_mesh.GetFaceVertexCountsAttr().Get()) != 32:
        raise ValueError("staircase2 floor topology changed")
    xmin = min(point[0] for point in floor_points)
    xmax = max(point[0] for point in floor_points)
    ymin = min(point[1] for point in floor_points)
    ymax = max(point[1] for point in floor_points)
    if (not (-15 < xmin < -13) or not (6 < xmax < 8) or
            not (-8 < ymin < -6) or not (2 < ymax < 4)):
        raise ValueError("staircase2 floor bounds changed")
    scale = SOURCE_UNITS_PER_METER * UsdGeom.GetStageMetersPerUnit(stage)
    x0, x1 = math.floor((xmin - 0.2) * scale), math.ceil((xmax + 0.2) * scale)
    y0, y1 = math.floor((ymin - 0.2) * scale), math.ceil((ymax + 0.2) * scale)
    top = math.ceil(11.4 * scale)
    wall = 16
    boxes = [
        (x0 - wall, y0 - wall, -wall, x1 + wall, y1 + wall, 0),
        (x0 - wall, y0 - wall, top, x1 + wall, y1 + wall, top + wall),
        (x0 - wall, y0 - wall, 0, x0, y1 + wall, top),
        (x1, y0 - wall, 0, x1 + wall, y1 + wall, top),
        (x0, y0 - wall, 0, x1, y0, top),
        (x0, y1, 0, x1, y1 + wall, top),
    ]
    stair_mesh, points = world_points(stage, "Mesh001")
    indices = list(stair_mesh.GetFaceVertexIndicesAttr().Get())
    groups = defaultdict(list)
    for start in range(0, len(indices), 3):
        corners = [points[index] for index in indices[start:start + 3]]
        normal = Gf.Cross(corners[1] - corners[0], corners[2] - corners[0]).GetNormalized()
        if (normal[2] > 0.9 and max(abs(point[2] - corners[0][2]) for point in corners) < 0.005
                and corners[0][2] > 0):
            groups[round(corners[0][2], 2)].extend(corners)
    if len(groups) != 16:
        raise ValueError("staircase2 upper-flight tread count changed")
    treads = []
    for height, corners in sorted(groups.items()):
        xlo = min(point[0] for point in corners)
        xhi = max(point[0] for point in corners)
        ylo = min(point[1] for point in corners)
        yhi = max(point[1] for point in corners)
        if not (0.5 < xhi - xlo < 0.7 and 2.8 < yhi - ylo < 3.1):
            raise ValueError("staircase2 tread dimensions changed")
        box = (math.floor(xlo * scale), math.floor(ylo * scale), 0,
               math.ceil(xhi * scale), math.ceil(yhi * scale), round(height * scale))
        boxes.append(box)
        treads.append({"height_meters": height, "bounds_source_units": box})
    return boxes, treads


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stage", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()
    if args.out_dir.exists():
        parser.error("output directory already exists: " + str(args.out_dir))
    stage = Usd.Stage.Open(str(args.stage))
    if not stage or UsdGeom.GetStageUpAxis(stage) != UsdGeom.Tokens.z:
        raise ValueError("staircase2 requires a readable Z-up USD stage")
    boxes, treads = collision_boxes(stage)
    world = ['versioninfo', '{', '\t"editorversion" "400"', '\t"editorbuild" "8000"',
             '\t"mapversion" "1"', '\t"formatversion" "100"', '\t"prefab" "0"',
             '}', 'visgroups', '{', '}', 'world', '{', '\t"id" "1"',
             '\t"mapversion" "1"', '\t"classname" "worldspawn"',
             '\t"skyname" "sky_day01_01"']
    for index, bounds in enumerate(boxes):
        world.append(box_text(10 + index, 100 + index * 6, bounds))
    world.extend(['}', 'entity', '{', '\t"id" "50"',
                  '\t"classname" "info_player_start"',
                  '\t"origin" "272 -100 4"', '\t"angles" "0 154 0"', '}',
                  'entity', '{', '\t"id" "51"', '\t"classname" "light"',
                  '\t"origin" "0 0 320"', '\t"_light" "255 255 220 400"',
                  '}', 'cameras', '{', '\t"activecamera" "-1"', '}', ''])
    args.out_dir.mkdir(parents=True)
    shutil.copytree(ROOT / 'quality/fixtures/vbsp-host/game', args.out_dir / 'game')
    vmf = args.out_dir / 'staircase2_collision.vmf'
    vmf.write_text('\n'.join(world))
    receipt = {"status": "pass", "scope": "staircase2-usd-collision-vmf",
               "stage_sha256": sha256(args.stage), "vmf_sha256": sha256(vmf),
               "brush_count": len(boxes), "upper_treads": treads,
               "spawn_source_units": [272, -100, 4]}
    (args.out_dir / 'collision-receipt.json').write_text(
        json.dumps(receipt, indent=2, sort_keys=True) + '\n')
    print(json.dumps(receipt, sort_keys=True))


if __name__ == '__main__':
    main()
