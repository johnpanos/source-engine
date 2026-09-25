#!/usr/bin/env python3
"""Lightmap layout owned by the pipeline, not guessed by Blender.

    planar   flat geometry (a compiled BSP's world faces): exact by construction

Blender only bakes: the `layout` step writes `lightmap_st` into the stage and
the bake uses it unchanged (pbrt_lightmap_bake.py --layout authored).

Planar layout rules - each one exact, with no angle heuristic or search:

  charts      triangles lying in one plane - the same BSP plane when the stage
              carries `primvars:sourceEngine:plane`, else normals within
              0.01 degrees - that meet along an edge, whole or
              in part (a T-junction), form one chart - across mesh and
              material boundaries, since lightmap UVs do not depend on
              material; zero-area slivers draw nothing and are parked
  projection  each chart is projected onto its own plane, turned to its
              minimum-area bounding rectangle: no stretch, and one texel
              density (texels per metre) for every chart
  packing     each chart is an integer-texel rectangle with MARGIN texels of
              border on every side (so neighbouring charts' texels are at
              least 2 x MARGIN apart and no bilinear lookup or bake dilation
              reaches another chart), placed by a shelf packer in a fixed
              order; the density is the largest the packer fits, the probe
              band's rows (v < reserved / size) and the top texel row stay
              empty, and uncharted triangles are parked on the top-right texel

Everything is keyed on geometry, so the same triangles give the same UVs in
any order. `author` (OpenUSD Python) lays out a stage:

    python3.12 lightmap_layout.py author --stage in.usdc --out out.usdc --size 4096 \\
        [--margin 2] [--reserve-rows N] [--exclude-mesh NAME ...] --receipt layout.json
"""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lightmap_seams  # noqa: E402
import map_scene  # noqa: E402

SCHEMA = "lightmap-layout/v1"
PLANE_PRIMVAR = "sourceEngine:plane"  # a relit BSP face's plane (legacy_bsp_scene.py)
MARGIN = 2
# Below this area (square metres) a triangle covers no pixel at any distance.
DEGENERATE_AREA = 1e-10
FLAT_COS = lightmap_seams.FLAT_COS
SEARCH_STEPS = 60


def parking_uv(size):
    """Where uncharted triangles' corners go: the top-right texel centre."""
    return 1.0 - 0.5 / size


def planar_charts(positions, charted, planes=None):
    """Chart id per triangle (-1: uncharted or zero-area).

    Two triangles meeting along an edge join when they lie on one plane:
    with plane ids (a BSP face's plane, `planes` >= 0 on both) when the ids
    are equal - exact, whatever float32 rounding does to a sliver's normal -
    and otherwise when their normals agree within FLAT_COS."""
    normals, _ = lightmap_seams.triangle_normals(positions)
    area = 0.5 * np.linalg.norm(np.cross(positions[:, 1] - positions[:, 0],
                                         positions[:, 2] - positions[:, 0]), axis=1)
    live = np.asarray(charted, dtype=bool) & (area > DEGENERATE_AREA)
    parent = np.arange(len(positions))

    def root(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    edges = [(t, k, (k + 1) % 3) for t in np.flatnonzero(live) for k in range(3)]
    planes = np.full(len(positions), -1) if planes is None else np.asarray(planes)
    for t, _, _, t2, _, _, _, _, _ in lightmap_seams.line_overlaps(positions, edges, normals,
                                                                   -2.0):
        if planes[t] >= 0 and planes[t2] >= 0:
            if planes[t] != planes[t2]:
                continue
        elif np.dot(normals[t], normals[t2]) <= FLAT_COS:
            continue
        first, second = root(t), root(t2)
        if first != second:
            parent[max(first, second)] = min(first, second)
    roots = np.array([root(i) for i in range(len(positions))])
    # Provisional ids; planar_layout renumbers charts by geometry.
    labels = np.full(len(positions), -1)
    ids = {}
    for t in np.flatnonzero(live):
        labels[t] = ids.setdefault(roots[t], len(ids))
    return labels, normals


def chart_frame(points, normal):
    """(origin, e1, e2, width, height) of the minimum-area rectangle around a
    chart's corner points in its plane; width >= height."""
    axis = np.eye(3)[int(np.argmin(np.abs(normal)))]
    e1 = np.cross(normal, axis)
    e1 /= np.linalg.norm(e1)
    e2 = np.cross(normal, e1)
    flat = np.stack([points @ e1, points @ e2], axis=1)
    unique = np.unique(np.round(flat, 12), axis=0)
    angles = [0.0]
    if len(unique) >= 3:
        try:
            from scipy.spatial import ConvexHull
            hull = unique[ConvexHull(unique).vertices]
            edges = np.roll(hull, -1, axis=0) - hull
            angles = sorted({round(math.atan2(e[1], e[0]) % (math.pi / 2), 12) for e in edges})
        except Exception:  # collinear or duplicate points: axis-aligned box
            angles = [0.0]
    best = None
    for angle in angles:
        c, s = math.cos(angle), math.sin(angle)
        a = flat @ np.array([c, s])
        b = flat @ np.array([-s, c])
        w, h = a.max() - a.min(), b.max() - b.min()
        if best is None or w * h < best[0] - 1e-15:
            best = (w * h, angle, w, h)
    _, angle, w, h = best
    c, s = math.cos(angle), math.sin(angle)
    u = c * e1 + s * e2
    v = -s * e1 + c * e2
    if h > w:
        u, v, w, h = v, -u, h, w
    return u, v, w, h


def shelf_pack(rects, size, y0, y1):
    """Place (width, height) integer rects in order on shelves in
    [0, size) x [y0, y1); returns their lower-left corners or None."""
    x, y, shelf = 0, y0, 0
    placed = []
    for w, h in rects:
        if w > size:
            return None
        if x + w > size:
            x, y, shelf = 0, y + shelf, 0
        if y + h > y1:
            return None
        placed.append((x, y))
        x += w
        shelf = max(shelf, h)
    return placed


def planar_layout(positions, size, charted=None, margin=MARGIN, reserved_rows=0, planes=None):
    """Lightmap UVs (n, 3, 2) for world triangles (n, 3, 3), and a record;
    `planes` (n,) optionally names each triangle's plane (-1: unknown)."""
    positions = np.asarray(positions, dtype=np.float64)
    count = len(positions)
    charted = np.ones(count, bool) if charted is None else np.asarray(charted, bool)
    labels, normals = planar_charts(positions, charted, planes)
    uvs = np.full((count, 3, 2), parking_uv(size))
    record = {"schema": SCHEMA, "method": "planar", "size": size, "margin": margin,
              "reserved_rows": reserved_rows, "triangles": count,
              "charted_triangles": int((labels >= 0).sum()),
              "parked_triangles": int((labels < 0).sum())}
    if labels.max(initial=-1) < 0:
        return uvs, dict(record, charts=0, texels_per_metre=None)
    # Number charts by geometry alone (each chart's sorted corner points), and derive each chart's plane from its sorted unique points, so
    # every float operation below runs in an order the triangle order cannot
    # change: the same geometry gives the same bytes.
    members_of, keys = [], []
    for chart in range(labels.max() + 1):
        members = np.flatnonzero(labels == chart)
        points = np.unique(positions[members].reshape(-1, 3), axis=0)
        members_of.append(members)
        # Distinct charts have distinct point sets (neighbours share corners,
        # so the least corner alone is not unique).
        keys.append(tuple(points.ravel()))
    order = sorted(range(len(keys)), key=lambda c: keys[c])
    remap = np.full(labels.max() + 1, -1)
    remap[order] = np.arange(len(order))
    labels = np.where(labels >= 0, remap[np.maximum(labels, 0)], -1)
    members_of = [members_of[c] for c in order]
    keys = [keys[c] for c in order]
    frames, extents = [], []
    for members in members_of:
        points = np.unique(positions[members].reshape(-1, 3), axis=0)
        centred = points - points.mean(axis=0)
        normal = np.linalg.svd(centred, full_matrices=False)[2][2]
        if np.dot(normal, normals[members].sum(axis=0)) < 0:
            normal = -normal
        u, v, w, h = chart_frame(points, normal)
        frames.append((u, v, (points @ u).min(), (points @ v).min()))
        extents.append((w, h))
    record["keys"] = keys
    y0, y1 = reserved_rows, size - 1
    density, best = largest_density(extents, keys, size, margin, y0, y1)
    for chart in range(labels.max() + 1):
        u, v, umin, vmin = frames[chart]
        x, y = best[chart]
        members = np.flatnonzero(labels == chart)
        corners = positions[members]
        uvs[members, :, 0] = (x + margin + (corners @ u - umin) * density) / size
        uvs[members, :, 1] = (y + margin + (corners @ v - vmin) * density) / size
    used = sum(w * h for w, h in chart_rects(extents, density, margin))
    record.update({"charts": int(labels.max() + 1), "texels_per_metre": density,
                   "atlas_fill": used / (size * (y1 - y0)),
                   "extents_m": [list(map(float, e)) for e in extents]})
    return uvs, record


def chart_rects(extents, density, margin):
    """Integer rectangle (texels, border included) of each chart extent (metres)."""
    return [(int(math.ceil(w * density)) + 2 * margin,
             int(math.ceil(h * density)) + 2 * margin) for w, h in extents]


def fits(extents, keys, density, size, margin, y0, y1):
    """Lower-left corner of every chart's rectangle at `density`, or None."""
    sizes = chart_rects(extents, density, margin)
    sequence = sorted(range(len(sizes)), key=lambda i: (-sizes[i][1], -sizes[i][0], tuple(keys[i])))
    placed = shelf_pack([sizes[i] for i in sequence], size, y0, y1)
    return None if placed is None else dict(zip(sequence, placed))


def largest_density(extents, keys, size, margin, y0, y1):
    """(density, placement): the largest texels-per-metre the packer fits,
    found by bisection from the area bound."""
    area = math.fsum(w * h for w, h in extents)
    low, high = 0.0, math.sqrt(size * (y1 - y0) / max(area, 1e-30))
    best = None
    for _ in range(SEARCH_STEPS):
        middle = (low + high) / 2
        placement = fits(extents, keys, middle, size, margin, y0, y1)
        if placement is None:
            high = middle
        else:
            low, best = middle, placement
    if best is None:
        raise ValueError("lightmap charts do not fit a %d atlas at any density" % size)
    return low, best


# --- OpenUSD stage authoring ------------------------------------------------

def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def author(stage_path, out_path, size, margin, reserved_rows, excluded):
    """Copy a stage with `primvars:lightmap_st` (faceVarying) on every mesh."""
    from pxr import Sdf, Usd, UsdGeom
    layer = Sdf.Layer.FindOrOpen(str(stage_path))
    out_path.parent.mkdir(parents=True, exist_ok=True)
    if not layer.Export(str(out_path)):
        raise RuntimeError("could not copy the stage to " + str(out_path))
    stage = Usd.Stage.Open(str(out_path))
    cache = UsdGeom.XformCache()
    meshes, positions, charted, planes = [], [], [], []
    for prim in stage.Traverse():
        if not prim.IsA(UsdGeom.Mesh):
            continue
        mesh = UsdGeom.Mesh(prim)
        counts = np.asarray(mesh.GetFaceVertexCountsAttr().Get() or [])
        indices = np.asarray(mesh.GetFaceVertexIndicesAttr().Get() or [])
        if len(counts) and np.any(counts != 3):
            raise ValueError("layout needs triangulated meshes: %s" % prim.GetPath())
        points = np.asarray(mesh.GetPointsAttr().Get() or [], dtype=np.float64)
        world = np.asarray(cache.GetLocalToWorldTransform(prim), dtype=np.float64)
        corners = (points @ world[:3, :3] + world[3, :3])[indices].reshape(-1, 3, 3) \
            if len(indices) else np.zeros((0, 3, 3))
        name = prim.GetName()
        plane = UsdGeom.PrimvarsAPI(prim).GetPrimvar(PLANE_PRIMVAR)
        if plane and plane.HasValue():
            values = np.asarray(plane.Get(), dtype=np.int64)
            if plane.GetInterpolation() != UsdGeom.Tokens.uniform or len(values) != len(corners):
                raise ValueError("%s: %s must be uniform, one per triangle" %
                                 (prim.GetPath(), PLANE_PRIMVAR))
            planes.append(values)
        else:
            planes.append(np.full(len(corners), -1))
        meshes.append((prim, len(corners)))
        positions.append(corners)
        charted.append(np.full(len(corners), name not in excluded and
                               not name.startswith(map_scene.EMITTER_PREFIXES)))
    if not meshes:
        raise ValueError("stage has no meshes to lay out")
    planes = np.concatenate(planes)
    uvs, record = planar_layout(np.concatenate(positions), size, np.concatenate(charted),
                                margin, reserved_rows, planes)
    record["plane_ids"] = "all" if (planes >= 0).all() else \
        "none" if (planes < 0).all() else "some"
    offset = 0
    for prim, triangles in meshes:
        primvar = UsdGeom.PrimvarsAPI(prim).CreatePrimvar(
            lightmap_seams.LIGHTMAP_PRIMVAR, Sdf.ValueTypeNames.TexCoord2fArray,
            UsdGeom.Tokens.faceVarying)
        primvar.Set([tuple(map(float, uv)) for uv in uvs[offset:offset + triangles].reshape(-1, 2)])
        offset += triangles
    stage.GetRootLayer().Save()
    return np.concatenate(positions), uvs, record


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("author")
    command.add_argument("--stage", type=Path, required=True)
    command.add_argument("--out", type=Path, required=True)
    command.add_argument("--size", type=int, required=True)
    command.add_argument("--margin", type=int, default=MARGIN)
    command.add_argument("--reserve-rows", type=int, default=0)
    command.add_argument("--exclude-mesh", action="append", default=[],
                         help="mesh (prim name) that gets no lightmap space")
    command.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    if args.stage.resolve() == args.out.resolve():
        parser.error("--out must differ from --stage")
    positions, uvs, record = author(args.stage, args.out, args.size, args.margin,
                                    args.reserve_rows, set(args.exclude_mesh))
    record.pop("keys")
    # Self-check with the independent invariant code: the layout must hold
    # every chart invariant and split no flat region.
    # Float32 storage of positions and UVs moves density by ~1e-4 at most.
    invariants = lightmap_seams.chart_invariants(positions, uvs, args.size, 1.001)
    _, _, _, triangles, _ = lightmap_seams.find_seams(positions, uvs, args.size)
    normals, _ = lightmap_seams.triangle_normals(positions)
    flat = int((np.einsum("ij,ij->i", normals[triangles[:, 0]], normals[triangles[:, 1]])
                > FLAT_COS).sum()) if len(triangles) else 0
    violations = [key for key in ("out_of_bounds", "overlap_texels", "bleed_texels",
                                  "density_violation") if invariants[key]]
    if flat:
        violations.append("flat_seam_samples")
    record.update({"status": "fail" if violations else "pass", "violations": violations,
                   "invariants": invariants, "flat_seam_samples": flat,
                   "stage_sha256": sha256(args.stage), "out_sha256": sha256(args.out),
                   "excluded_meshes": sorted(args.exclude_mesh)})
    args.receipt.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("LIGHTMAP_LAYOUT " + json.dumps({k: record[k] for k in (
        "status", "charts", "texels_per_metre", "atlas_fill")}, sort_keys=True))
    if violations:
        raise SystemExit("lightmap layout violates: " + ", ".join(violations))


if __name__ == "__main__":
    main()
