"""Randomized variants of the lightmap charting invariants, run inside Blender.

    blender -b --factory-startup --python-exit-code 9 \\
        --python tools/quality/tests/blender_lightmap_charting.py -- --seeds 8

Builds triangle soups (every triangle owning its corners, as usd_scene and
the BSP relight write them) and charts them with the bake's own
`pbrt_lightmap_bake.pack_lightmap_uvs`. Invariants, per seed:

  * charts: no escaped UVs, no overlapping texels, no texel read by two
    charts' lookups, one texel density (lightmap_seams.chart_invariants)
  * connectivity: a flat connected region is one chart across object (and
    so material) boundaries and T-junctions: no seam joins two coplanar
    triangles; only curved surfaces may be cut into charts
  * source meshes untouched: positions, per-corner custom normals and
    material UVs bit-identical, and no charting copies left behind

Negative control: the same soups charted without the weld must show flat
seams, inside an object and between the panels, or the connectivity
invariant proves nothing. Prints one
`LIGHTMAP_CHARTING <json>` line; exits nonzero on any violation.
"""

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
import numpy as np
from mathutils import Matrix

HERE = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HERE))
import lightmap_seams  # noqa: E402
import pbrt_lightmap_bake  # noqa: E402

SIZE = 256
MARGIN_TEXELS = 4


def soup_object(name, triangles, rng, normal_jitter=0.05):
    """A mesh whose triangles share no vertices, with custom corner normals."""
    points = [tuple(p) for tri in triangles for p in tri]
    faces = [(3 * i, 3 * i + 1, 3 * i + 2) for i in range(len(triangles))]
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(points, [], faces)
    mesh.update()
    st = mesh.uv_layers.new(name="st")
    for loop in mesh.loops:
        st.data[loop.index].uv = (points[loop.vertex_index][0] * 0.37,
                                  points[loop.vertex_index][1] * 0.53)
    normals = []
    for polygon in mesh.polygons:
        n = np.array(polygon.normal) + rng.normal(0, normal_jitter, 3)
        n /= np.linalg.norm(n)
        normals += [tuple(n)] * polygon.loop_total
    mesh.normals_split_custom_set(normals)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def grid(rng, columns, rows, width, height):
    xs = np.concatenate([[0], np.cumsum(rng.uniform(0.5, 1.5, columns))])
    ys = np.concatenate([[0], np.cumsum(rng.uniform(0.5, 1.5, rows))])
    xs *= width / xs[-1]
    ys *= height / ys[-1]
    tris = []
    for i in range(columns):
        for j in range(rows):
            a, b = (xs[i], ys[j], 0), (xs[i + 1], ys[j], 0)
            c, d = (xs[i + 1], ys[j + 1], 0), (xs[i], ys[j + 1], 0)
            tris += [(a, b, c), (a, c, d)] if rng.random() < 0.5 else [(a, b, d), (b, c, d)]
    return np.array(tris, dtype=np.float64)


def t_junction_wall(rng, width, height):
    """Left half coarse, right half twice as fine: the halves meet in T-junctions."""
    left = grid(rng, 1, 2, width / 2, height)
    right = grid(rng, 2, 4, width / 2, height) + [width / 2, 0, 0]
    return np.concatenate([left, right])


def cylinder(rng, segments=12, rings=3, radius=0.6, height=1.5):
    """A curved open tube: charts may cut it, only along curved edges."""
    angles = np.linspace(0, 2 * np.pi, segments + 1)
    zs = np.linspace(0, height, rings + 1)
    tris = []
    for i in range(segments):
        for j in range(rings):
            p = [(radius * np.cos(angles[i + a]), radius * np.sin(angles[i + a]), zs[j + b])
                 for a, b in ((0, 0), (1, 0), (1, 1), (0, 1))]
            tris += [(p[0], p[1], p[2]), (p[0], p[2], p[3])]
    return np.array(tris, dtype=np.float64)


def transform(tris, rotation, translation):
    return tris @ rotation.T + translation


def rotation_about(axis, angle):
    return np.array(Matrix.Rotation(angle, 3, axis))


def random_rotation(rng):
    q, r = np.linalg.qr(rng.standard_normal((3, 3)))
    q *= np.sign(np.diag(r))
    if np.linalg.det(q) < 0:
        q[:, 0] = -q[:, 0]
    return q


def build_scene(seed):
    """A random set of soups: a wall, an L corner, a box, two coplanar
    neighbouring objects and a T-junction wall, randomly placed."""
    rng = np.random.default_rng(seed)
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    objects = []
    r = random_rotation(rng)
    objects.append(soup_object("Wall", transform(
        grid(rng, rng.integers(2, 5), rng.integers(1, 4), 3.0, 2.0), r, rng.uniform(-5, 5, 3)),
        rng))
    floor = grid(rng, 3, 2, 2.0, 1.5)
    wall = transform(grid(rng, 3, 2, 2.0, 1.5), rotation_about("X", math.pi / 2), [0, 0, 0])
    objects.append(soup_object("Corner", transform(np.concatenate([floor, wall]),
                                                   random_rotation(rng), [8, 0, 0]), rng))
    faces = []
    for axis, angle in (("X", 0), ("X", math.pi / 2), ("X", math.pi), ("X", -math.pi / 2),
                        ("Y", math.pi / 2), ("Y", -math.pi / 2)):
        square = grid(rng, 2, 2, 1.0, 1.0) - [0.5, 0.5, 0]
        faces.append(transform(square + [0, 0, 0.5], rotation_about(axis, angle), [0, 0, 0]))
    objects.append(soup_object("Box", transform(np.concatenate(faces), random_rotation(rng),
                                                [0, 8, 0]), rng))
    r = random_rotation(rng)
    left = grid(rng, 2, 2, 1.5, 1.5)
    objects.append(soup_object("PanelA", transform(left, r, [0, -8, 0]), rng))
    objects.append(soup_object("PanelB", transform(left + [1.5, 0, 0], r, [0, -8, 0]), rng))
    # A third coplanar panel, twice as finely divided, meets the other two in
    # T-junctions along its whole edge.
    fine = grid(rng, 4, 4, 1.5, 3.0) + [0, -3.0 + 1.5, 0]
    objects.append(soup_object("PanelC", transform(fine - [0, 1.5, 0], r, [0, -8, 0]), rng))
    objects.append(soup_object("TWall", transform(t_junction_wall(rng, 2.0, 1.0),
                                                  random_rotation(rng), [-8, 0, 0]), rng))
    objects.append(soup_object("Cylinder", transform(cylinder(rng), random_rotation(rng),
                                                     [-8, 8, 0]), rng))
    for obj in objects:
        obj.data.uv_layers.active = obj.data.uv_layers.new(name="lightmap_st")
    return objects


def corners(objects):
    """(positions, lightmap uvs, object index, normals, material uvs) per triangle corner."""
    positions, uvs, owners, normals, st = [], [], [], [], []
    for index, obj in enumerate(objects):
        mesh = obj.data
        mesh.calc_loop_triangles()
        world = np.array(obj.matrix_world)
        corner_normals = mesh.corner_normals
        for tri in mesh.loop_triangles:
            positions.append([world[:3, :3] @ np.array(mesh.vertices[v].co) + world[:3, 3]
                              for v in tri.vertices])
            uvs.append([tuple(mesh.uv_layers["lightmap_st"].data[l].uv) for l in tri.loops])
            normals.append([tuple(corner_normals[l].vector) for l in tri.loops])
            st.append([tuple(mesh.uv_layers["st"].data[l].uv) for l in tri.loops])
            owners.append(index)
    return (np.array(positions), np.array(uvs, dtype=np.float64), np.array(owners),
            np.array(normals), np.array(st))


def chart(objects, weld=True):
    pbrt_lightmap_bake.pack_lightmap_uvs(objects, MARGIN_TEXELS / SIZE, weld=weld)


def check(seed):
    objects = build_scene(seed)
    before = corners(objects)
    chart(objects)
    positions, uvs, owners, normals, st = corners(objects)
    problems = []
    # The source meshes are never modified: bit-identical corners.
    for label, old, new in (("positions", before[0], positions), ("normals", before[3], normals),
                            ("material UVs", before[4], st)):
        if old.shape != new.shape or not np.array_equal(old, new):
            problems.append("charting changed corner %s" % label)
    leftovers = [obj.name for obj in bpy.data.objects
                 if obj.name.startswith(pbrt_lightmap_bake.CHART_PREFIX)]
    if leftovers:
        problems.append("charting copies left in the scene: %s" % leftovers)
    invariants = lightmap_seams.chart_invariants(positions, uvs, SIZE)
    for key in ("out_of_bounds", "overlap_texels", "bleed_texels", "density_violation"):
        if invariants[key]:
            problems.append("%s = %s" % (key, invariants[key]))
    flat, curved, between = flat_seams(positions, uvs, owners)
    if flat:
        problems.append("%d seam samples across flat regions (a flat region split)" % flat)
    cylinder = [obj.name for obj in objects].index("Cylinder")
    # Negative control: without the weld every soup triangle charts alone,
    # inside objects and across the coplanar panels.
    objects = build_scene(seed)
    chart(objects, weld=False)
    positions, uvs, owners, _, _ = corners(objects)
    unwelded_flat, _, unwelded_between = flat_seams(positions, uvs, owners)
    if unwelded_flat == 0 or unwelded_between == 0:
        problems.append("negative control: unwelded soups showed no flat seams")
    return {"seed": seed, "charts": invariants["charts"],
            "density_spread": invariants["density_spread"], "curved_seam_samples": curved,
            "unwelded_flat_seam_samples": unwelded_flat,
            "unwelded_seam_samples_between_objects": unwelded_between, "problems": problems}


def flat_seams(positions, uvs, owners):
    """(flat, curved, between-object) seam sample counts."""
    _, _, _, triangles, _ = lightmap_seams.find_seams(positions, uvs, SIZE)
    if not len(triangles):
        return 0, 0, 0
    normals, _ = lightmap_seams.triangle_normals(positions)
    flat = np.einsum("ij,ij->i", normals[triangles[:, 0]], normals[triangles[:, 1]]) > \
        lightmap_seams.FLAT_COS
    between = owners[triangles[:, 0]] != owners[triangles[:, 1]]
    return int(flat.sum()), int((~flat).sum()), int(between.sum())


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--seeds", type=int, default=8)
    args = parser.parse_args(arguments)
    results = [check(seed) for seed in range(args.seeds)]
    failed = [r for r in results if r["problems"]]
    print("LIGHTMAP_CHARTING " + json.dumps({"status": "fail" if failed else "pass",
                                             "results": results}, sort_keys=True))
    if failed:
        sys.exit(1)


main()
