#!/usr/bin/env python3
"""Lightmap layout owned by the pipeline, not guessed by Blender.

    planar   flat geometry (a compiled BSP's world faces): exact by construction
    curved   smooth-shaded curved surfaces: cut and flattened by xatlas
             (xatlas_chart.cpp), within a stretch limit this module verifies

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

Curved rules - a curved surface cannot be flattened without cuts or
stretch, so something must choose them; that is xatlas, pinned in the map
tools profile, and only for this:

  curved      a triangle is curved when it meets a non-coplanar neighbour
              along a smooth edge - one where the authored corner normals of
              both triangles agree at both ends (within SMOOTH_COS). A hard
              edge (a normal break) or a flat face is never curved, nor is a
              triangle carrying a plane number (a BSP face, even one whose
              corners vbsp snapped off its plane)
  charts      xatlas cuts the curved triangles into charts and flattens each;
              the triangles go to it in a canonical order (corners rotated to
              the least first, triangles sorted), so input order changes
              nothing
  density     each xatlas chart is scaled so its UV area equals its surface
              area, then framed and packed exactly like a planar chart
  stretch     every curved triangle's texel-per-metre scale along its two
              principal directions stays within [1 / MAX_STRETCH,
              MAX_STRETCH] of the atlas density, or the layout fails

Everything is keyed on geometry, so the same triangles give the same UVs in
any order. `author` (OpenUSD Python) lays out a stage:

    python3.12 lightmap_layout.py author --stage in.usdc --out out.usdc --size 4096 \\
        [--margin 2] [--reserve-rows N] [--exclude-mesh NAME ...] [--xatlas PATH] \\
        --receipt layout.json
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
# A triangle this close to its BSP plane's orientation lies on it (texel
# density changes by 1 - cos, 0.015%); farther, its own corners decide.
ON_PLANE_COS = math.cos(math.radians(1.0))
# vbsp gives neighbouring brushes' faces planes a hair apart (0.01 degrees on
# testchmb_a_00); plane normals this close chart as one plane. Wider than
# FLAT_COS, which (on triangle normals) joins what the seam check calls flat.
PLANE_MERGE_COS = math.cos(math.radians(0.05))
SEARCH_STEPS = 60
# Corner normals this close are one smooth normal (a smooth edge).
SMOOTH_COS = math.cos(math.radians(1.0))
# Largest texel scale along any direction of a curved triangle, relative to
# the atlas density (and smallest: 1 / MAX_STRETCH).
MAX_STRETCH = 1.5
# xatlas chart options: maxCost, normalDeviationWeight, roundnessWeight,
# straightnessWeight, normalSeamWeight, textureSeamWeight, maxChartArea,
# maxBoundaryLength, then maxIterations. Normal seams above 1000 are always
# respected: a hard edge is never inside a chart.
XATLAS_OPTIONS = (2.0, 2.0, 0.01, 6.0, 1001.0, 0.0, 0.0, 0.0)
XATLAS_ITERATIONS = 4
# xatlas works in these units: its fixed area epsilon (FLT_EPSILON, units
# squared) must sit far below DEGENERATE_AREA, and in metres it would drop
# every triangle under 0.12 square millimetres (thousands on a dense mesh).
XATLAS_UNITS_PER_METRE = 1000.0


def parking_uv(size):
    """Where uncharted triangles' corners go: the top-right texel centre."""
    return 1.0 - 0.5 / size


def planar_charts(positions, charted, planes=None, plane_normals=None):
    """Chart id per triangle (-1: uncharted or zero-area).

    Two triangles meeting along an edge join when they lie on one plane. A
    triangle is on its BSP plane (`planes` >= 0, normal `plane_normals`) when
    its own normal is within ON_PLANE_DEGREES of the plane's: vbsp snaps the
    corners of a millimetres-wide face, which can leave it tilted far off its
    nominal plane, and the renderer draws the corners. Two such triangles join
    when they share a plane or their plane normals agree within
    PLANE_MERGE_COS (vbsp gives neighbouring brushes planes a hair apart); any pair
    joins when their own normals agree within FLAT_COS - the flatness the seam
    check measures, so no seam it would call flat is ever left."""
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
    on_plane = planes >= 0
    if plane_normals is not None:
        plane_normals = np.asarray(plane_normals, dtype=np.float64)
        on_plane &= np.einsum("ij,ij->i", normals, plane_normals) > ON_PLANE_COS
    for t, _, _, t2, _, _, _, _, _ in lightmap_seams.line_overlaps(positions, edges, normals,
                                                                   -2.0):
        same_plane = on_plane[t] and on_plane[t2] and (
            planes[t] == planes[t2] or (plane_normals is not None and np.dot(
                plane_normals[t], plane_normals[t2]) > PLANE_MERGE_COS))
        if not same_plane and np.dot(normals[t], normals[t2]) <= FLAT_COS:
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


def groups(labels):
    """Members of each label 0..max, in ascending order (one stable sort, not
    a scan per label)."""
    labels = np.asarray(labels)
    count = int(labels.max(initial=-1)) + 1
    order = np.argsort(labels, kind="stable")
    bounds = np.searchsorted(labels[order], np.arange(count + 1))
    return [order[bounds[i]:bounds[i + 1]] for i in range(count)]


def curved_triangles(positions, corner_normals, live):
    """Bool per triangle: on a curved surface. Live triangles joined along
    smooth edges (both triangles' corner normals agree at both ends) form a
    surface; it is curved when any of its smooth edges bends (joins
    non-coplanar triangles), and then all of it is - flat parts included, so
    a fillet and the flat faces it rounds are charted together."""
    positions = np.asarray(positions, dtype=np.float64)
    corner_normals = np.asarray(corner_normals, dtype=np.float64)
    corner_normals = corner_normals / np.maximum(
        np.linalg.norm(corner_normals, axis=2, keepdims=True), 1e-30)
    normals, _ = lightmap_seams.triangle_normals(positions)
    curved = np.zeros(len(positions), bool)
    triangles = np.flatnonzero(live)
    if not len(triangles):
        return curved
    # Every edge as (triangle, start corner, end corner), its ends ordered so
    # the lexicographically lesser point comes first.
    t = np.repeat(triangles, 3)
    k0 = np.tile(np.arange(3), len(triangles))
    k1 = (k0 + 1) % 3
    ends = np.stack([positions[t, k0], positions[t, k1]], axis=1)
    swap = least_corner(ends) == 1
    k0, k1 = np.where(swap, k1, k0), np.where(swap, k0, k1)
    key = np.concatenate([positions[t, k0], positions[t, k1]], axis=1)
    _, group, sizes = np.unique(key, axis=0, return_inverse=True, return_counts=True)
    group = group.reshape(-1)
    order = np.argsort(group, kind="stable")
    starts = np.concatenate([[0], np.cumsum(sizes)])

    links, bends = [], []

    def join(a, b):
        """Record the smooth ones among edge-side pairs a, b (edge indices)."""
        smooth = (np.einsum("ij,ij->i", corner_normals[t[a], k0[a]],
                            corner_normals[t[b], k0[b]]) > SMOOTH_COS) & \
            (np.einsum("ij,ij->i", corner_normals[t[a], k1[a]],
                       corner_normals[t[b], k1[b]]) > SMOOTH_COS)
        links.append(np.stack([t[a][smooth], t[b][smooth]], axis=1))
        bent = np.einsum("ij,ij->i", normals[t[a]], normals[t[b]]) <= FLAT_COS
        bends.append(t[a][smooth & bent])

    pairs = np.flatnonzero(sizes == 2)
    join(order[starts[pairs]], order[starts[pairs] + 1])
    for g in np.flatnonzero(sizes > 2):
        sides = order[starts[g]:starts[g + 1]]
        for i in range(len(sides)):
            join(np.full(len(sides) - i - 1, sides[i]), sides[i + 1:])
    links = np.concatenate(links)
    bends = np.concatenate(bends)
    if not len(bends):
        return curved
    from scipy.sparse import coo_matrix
    from scipy.sparse.csgraph import connected_components
    count = len(positions)
    graph = coo_matrix((np.ones(len(links)), (links[:, 0], links[:, 1])), shape=(count, count))
    _, surface = connected_components(graph, directed=False)
    curved[np.isin(surface, np.unique(surface[bends]))] = True
    return curved & np.asarray(live, bool)


def least_corner(corners):
    """Per row, the index of its lexicographically least corner (corners:
    (n, k, d), k points of d values each); one sort, not a loop."""
    count, points, width = corners.shape
    flat = corners.reshape(-1, width)
    row = np.repeat(np.arange(count), points)
    order = np.lexsort(tuple(flat[:, c] for c in range(width - 1, -1, -1)) + (row,))
    return order[0::points] - points * np.arange(count)


def canonical_triangles(positions, normals, members):
    """Canonical order and corner rotation of the triangles `members`: each
    triangle's corners (position, normal) rotated, winding kept, so the least
    comes first; triangles sorted by those corners. Returns (rows, first):
    positions in `members`, in canonical order, and each one's first corner."""
    members = np.asarray(members)
    corners = np.concatenate([positions[members], normals[members]], axis=2)
    first = least_corner(corners)
    turn = (first[:, None] + np.arange(3)[None]) % 3
    keys = turned(corners, turn).reshape(len(members), -1)
    rows = np.lexsort((members,) + tuple(keys[:, c] for c in range(keys.shape[1] - 1, -1, -1)))
    return rows, first[rows]


def xatlas_charts(positions, corner_normals, members, tool, max_area=0.0, groups_of=None):
    """Chart id (per member, from 0) and flattened corner coordinates
    (members, 3, 2) for the curved triangles `members`, from xatlas; charts
    grow to at most `max_area` square metres (0: no limit), and members of
    different `groups_of` (per member) never share a chart."""
    import struct
    import subprocess
    import tempfile
    members = np.asarray(members)
    rows, first = canonical_triangles(positions, corner_normals, members)
    # Millimetres from the members' least corner (the same in any order).
    origin = positions[members].reshape(-1, 3).min(axis=0)
    turn = (first[:, None] + np.arange(3)[None]) % 3
    corners = np.concatenate([
        turned((positions[members[rows]] - origin) * XATLAS_UNITS_PER_METRE, turn),
        turned(corner_normals[members[rows]], turn)], axis=2).astype(np.float32).reshape(-1, 6)
    vertices, indices = np.unique(corners, axis=0, return_inverse=True)
    with tempfile.TemporaryDirectory() as work:
        source, result = Path(work) / "in.bin", Path(work) / "out.bin"
        with open(source, "wb") as out:
            out.write(b"XAC1" + struct.pack("<II", len(vertices), len(rows)))
            out.write(np.ascontiguousarray(vertices[:, :3], "<f4").tobytes())
            out.write(np.ascontiguousarray(vertices[:, 3:], "<f4").tobytes())
            out.write(np.asarray(indices, "<u4").reshape(-1).tobytes())
            group = np.zeros(len(members)) if groups_of is None else np.asarray(groups_of)
            out.write(np.asarray(group[rows], "<u4").tobytes())
            options = XATLAS_OPTIONS[:6] + (max_area,) + XATLAS_OPTIONS[7:]
            out.write(struct.pack("<8fI", *options, XATLAS_ITERATIONS))
        subprocess.run([str(tool), str(source), str(result)], check=True)
        data = result.read_bytes()
    if data[:4] != b"XAO1":
        raise RuntimeError("xatlas_chart wrote no XAO1 output")
    count, _ = struct.unpack_from("<II", data, 4)
    if count != len(rows):
        raise RuntimeError("xatlas_chart returned %d of %d triangles" % (count, len(rows)))
    charts = np.frombuffer(data, "<i4", count, 12)
    flat = np.frombuffer(data, "<f4", 6 * count, 12 + 4 * count).reshape(count, 3, 2) / \
        XATLAS_UNITS_PER_METRE
    if (charts < 0).any():
        raise RuntimeError("xatlas left %d curved triangles uncharted" % int((charts < 0).sum()))
    labels = np.empty(len(members), np.int64)
    labels[rows] = charts
    uvs = np.empty((len(members), 3, 2))
    unturned = np.empty_like(flat, dtype=np.float64)
    np.put_along_axis(unturned, turn[..., None], flat, axis=1)
    uvs[rows] = unturned
    return labels, uvs


def canonical_turn(positions):
    """Per triangle the corner order (n, 3) that starts at its least corner and
    keeps its winding: sums over corners taken in this order round the same
    whatever corner the input starts at."""
    return (least_corner(positions)[:, None] + np.arange(3)[None]) % 3


def turned(values, turn):
    return np.take_along_axis(values, turn[..., None], axis=1)


def triangle_areas(positions, turn=None):
    """Triangle areas; with `turn` (canonical_turn) independent of corner order."""
    if turn is not None:
        positions = turned(positions, turn)
    e1, e2 = positions[:, 1] - positions[:, 0], positions[:, 2] - positions[:, 0]
    if positions.shape[-1] == 2:
        return 0.5 * np.abs(e1[:, 0] * e2[:, 1] - e1[:, 1] * e2[:, 0])
    return 0.5 * np.linalg.norm(np.cross(e1, e2), axis=-1)


def isometric(positions):
    """Each triangle laid flat in its own plane without stretch: (n, 3, 2),
    measured from its least corner so corner order changes nothing."""
    turn = canonical_turn(positions)
    rotated = turned(positions, turn)
    e1 = rotated[:, 1] - rotated[:, 0]
    e2 = rotated[:, 2] - rotated[:, 0]
    x = e1 / np.linalg.norm(e1, axis=1, keepdims=True)
    normal = np.cross(e1, e2)
    y = np.cross(normal / np.linalg.norm(normal, axis=1, keepdims=True), x)
    flat = np.zeros((len(positions), 3, 2))
    for k, e in ((1, e1), (2, e2)):
        flat[:, k] = np.stack([np.einsum("ij,ij->i", e, x), np.einsum("ij,ij->i", e, y)], 1)
    result = np.empty_like(flat)
    np.put_along_axis(result, turn[..., None], flat, axis=1)
    return result


def curved_charts(positions, corner_normals, members, tool):
    """Charts of the curved triangles `members`, each within MAX_STRETCH.

    xatlas charts them; each chart is scaled so its UV area is its surface
    area (one texel density on average). A chart whose triangles stretch
    beyond [1 / MAX_STRETCH, MAX_STRETCH] is charted again with a largest
    chart area between a quarter and a sixteenth of its own (charts of one
    size band share an xatlas run, kept apart by group). A chart xatlas will
    not split below the area it was given is split into its triangles, each
    flattened exactly. Returns (chart id per member from 0, flattened corners
    in metres (members, 3, 2))."""
    members = np.asarray(members)
    turn = canonical_turn(positions[members])
    area = triangle_areas(positions[members], turn)
    labels = np.empty(len(members), np.int64)
    result = np.empty((len(members), 3, 2))
    first_labels, flat = xatlas_charts(positions, corner_normals, members, tool)
    pending = [(rows, 0.0) for rows in groups(first_labels) if len(rows)]
    next_label = 0
    while pending:
        bands = {}
        for rows, limit in pending:
            chart_area = math.fsum(area[rows])
            uv_area = math.fsum(triangle_areas(flat[rows], turn[rows]))
            scaled = flat[rows] * math.sqrt(chart_area / max(uv_area, 1e-30))
            low, high = stretch(positions[members[rows]], scaled, 1.0)
            if low.min() >= 1 / MAX_STRETCH and high.max() <= MAX_STRETCH:
                labels[rows], result[rows] = next_label, scaled
                next_label += 1
            elif len(rows) > 1 and (limit == 0.0 or chart_area < limit):
                bands.setdefault(math.floor(math.log(chart_area, 4)), []).append(rows)
            else:
                labels[rows] = next_label + np.arange(len(rows))
                result[rows] = isometric(positions[members[rows]])
                next_label += len(rows)
        pending = []
        for band in sorted(bands):
            charts = bands[band]
            limit = 4.0 ** band / 4
            rows = np.concatenate(charts)
            group = np.concatenate([np.full(len(chart), index)
                                    for index, chart in enumerate(charts)])
            sub_labels, flat[rows] = xatlas_charts(positions, corner_normals, members[rows], tool,
                                                   limit, group)
            pending += [(rows[sub], limit) for sub in groups(sub_labels) if len(sub)]
    return labels, result


def stretch(positions, uvs, density):
    """Per triangle (smallest, largest) texel scale along its principal
    directions, relative to `density` (texels per metre); uvs in texels."""
    e1 = positions[:, 1] - positions[:, 0]
    e2 = positions[:, 2] - positions[:, 0]
    x = e1 / np.linalg.norm(e1, axis=1, keepdims=True)
    normal = np.cross(e1, e2)
    y = np.cross(normal / np.linalg.norm(normal, axis=1, keepdims=True), x)
    local = np.stack([np.stack([np.einsum("ij,ij->i", e, x), np.einsum("ij,ij->i", e, y)], 1)
                      for e in (e1, e2)], 2)                       # (n, 2, 2) columns e1, e2
    texel = np.stack([uvs[:, 1] - uvs[:, 0], uvs[:, 2] - uvs[:, 0]], 2)
    jacobian = texel @ np.linalg.inv(local)
    values = np.linalg.svd(jacobian, compute_uv=False) / density
    return values[:, 1], values[:, 0]


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


def planar_layout(positions, size, charted=None, margin=MARGIN, reserved_rows=0, planes=None,
                  plane_normals=None, corner_normals=None, xatlas=None):
    """Lightmap UVs (n, 3, 2) for world triangles (n, 3, 3), and a record;
    `planes` (n,) optionally names each triangle's plane (-1: unknown) and
    `plane_normals` (n, 3) its normal. With `corner_normals` (n, 3, 3), the
    authored shading normals, curved surfaces are charted by the xatlas_chart
    executable `xatlas`."""
    positions = np.asarray(positions, dtype=np.float64)
    count = len(positions)
    charted = np.ones(count, bool) if charted is None else np.asarray(charted, bool)
    area = 0.5 * np.linalg.norm(np.cross(positions[:, 1] - positions[:, 0],
                                         positions[:, 2] - positions[:, 0]), axis=1)
    # A triangle with a plane number is a flat face by declaration (a BSP
    # face, even one vbsp snapped off its plane), never part of a curve.
    flat_faces = np.zeros(count, bool) if planes is None else np.asarray(planes) >= 0
    curved = np.zeros(count, bool) if corner_normals is None else curved_triangles(
        positions, corner_normals, charted & (area > DEGENERATE_AREA) & ~flat_faces)
    if curved.any() and xatlas is None:
        raise ValueError("%d triangles lie on curved surfaces: charting them needs xatlas"
                         % int(curved.sum()))
    labels, normals = planar_charts(positions, charted & ~curved, planes, plane_normals)
    flattened = np.zeros((count, 3, 2))
    if curved.any():
        members = np.flatnonzero(curved)
        curved_labels, flattened[members] = curved_charts(positions, corner_normals, members,
                                                          xatlas)
        labels[members] = labels.max(initial=-1) + 1 + curved_labels
    uvs = np.full((count, 3, 2), parking_uv(size))
    record = {"schema": SCHEMA, "method": "planar+xatlas" if curved.any() else "planar",
              "curved_mask": curved,
              "size": size, "margin": margin,
              "reserved_rows": reserved_rows, "triangles": count,
              "charted_triangles": int((labels >= 0).sum()),
              "curved_triangles": int(curved.sum()),
              "parked_triangles": int((labels < 0).sum())}
    if labels.max(initial=-1) < 0:
        return uvs, dict(record, charts=0, texels_per_metre=None)
    # Number charts by geometry alone (each chart's sorted corner points), and derive each chart's plane from its sorted unique points, so
    # every float operation below runs in an order the triangle order cannot
    # change: the same geometry gives the same bytes.
    members_of, keys = [], []
    for members in groups(labels):
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
    frames, extents, curved_count = [], [], 0
    for chart, members in enumerate(members_of):
        if curved[members[0]]:
            points = np.unique(np.concatenate([flattened[members].reshape(-1, 2),
                                               np.zeros((3 * len(members), 1))], axis=1),
                               axis=0)
            u, v, w, h = chart_frame(points, np.array((0.0, 0.0, 1.0)))
            curved_count += 1
        else:
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
        members = members_of[chart]
        if curved[members[0]]:
            corners = np.concatenate([flattened[members],
                                      np.zeros((len(members), 3, 1))], axis=2)
        else:
            corners = positions[members]
        uvs[members, :, 0] = (x + margin + (corners @ u - umin) * density) / size
        uvs[members, :, 1] = (y + margin + (corners @ v - vmin) * density) / size
    used = sum(w * h for w, h in chart_rects(extents, density, margin))
    if curved.any():
        low, high = stretch(positions[curved], uvs[curved] * size, density)
        record["stretch"] = [float(low.min()), float(high.max())]
        record["curved_charts"] = curved_count
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


def mesh_corner_normals(mesh, indices, world, corners):
    """A mesh's authored shading normals per triangle corner, in world space
    (n, 3, 3); triangles without authored normals get their geometric one."""
    from pxr import UsdGeom
    primvar = UsdGeom.PrimvarsAPI(mesh.GetPrim()).GetPrimvar("normals")
    if primvar and primvar.HasValue():
        values, interpolation = primvar.ComputeFlattened(), primvar.GetInterpolation()
    else:
        values, interpolation = mesh.GetNormalsAttr().Get(), mesh.GetNormalsInterpolation()
    values = np.asarray(values if values is not None else [], dtype=np.float64).reshape(-1, 3)
    count = len(corners)
    if interpolation == UsdGeom.Tokens.faceVarying and len(values) == 3 * count:
        normals = values.reshape(count, 3, 3)
    elif interpolation in (UsdGeom.Tokens.vertex, UsdGeom.Tokens.varying) and len(values) and \
            indices.max(initial=-1) < len(values):
        normals = values[indices].reshape(count, 3, 3)
    elif interpolation == UsdGeom.Tokens.uniform and len(values) == count:
        normals = np.repeat(values[:, None], 3, axis=1)
    elif interpolation == UsdGeom.Tokens.constant and len(values) == 1:
        normals = np.tile(values, (count, 3, 1))
    else:
        geometric, _ = lightmap_seams.triangle_normals(corners)
        return np.repeat(geometric[:, None], 3, axis=1)
    # Row vectors: points map by p @ M, normals by n @ inverse(M) transposed.
    normals = normals @ np.linalg.inv(world[:3, :3]).T
    return normals / np.maximum(np.linalg.norm(normals, axis=2, keepdims=True), 1e-30)


def author(stage_path, out_path, size, margin, reserved_rows, excluded, xatlas=None):
    """Copy a stage with `primvars:lightmap_st` (faceVarying) on every mesh."""
    from pxr import Sdf, Usd, UsdGeom
    layer = Sdf.Layer.FindOrOpen(str(stage_path))
    out_path.parent.mkdir(parents=True, exist_ok=True)
    if not layer.Export(str(out_path)):
        raise RuntimeError("could not copy the stage to " + str(out_path))
    stage = Usd.Stage.Open(str(out_path))
    cache = UsdGeom.XformCache()
    meshes, positions, charted, planes, plane_normals, shading = [], [], [], [], [], []
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
            # The plane's normal, authored on every corner (legacy_bsp_scene).
            normals = np.asarray(mesh.GetNormalsAttr().Get(), dtype=np.float64)
            if mesh.GetNormalsInterpolation() != UsdGeom.Tokens.faceVarying or \
                    len(normals) != 3 * len(corners):
                raise ValueError("%s: plane meshes need faceVarying normals" % prim.GetPath())
            normals = normals.reshape(-1, 3, 3)[:, 0] @ world[:3, :3]
            plane_normals.append(normals / np.linalg.norm(normals, axis=1)[:, None])
        else:
            planes.append(np.full(len(corners), -1))
            plane_normals.append(np.zeros((len(corners), 3)))
        shading.append(mesh_corner_normals(mesh, indices, world, corners))
        meshes.append((prim, len(corners)))
        positions.append(corners)
        charted.append(np.full(len(corners), name not in excluded and
                               not name.startswith(map_scene.EMITTER_PREFIXES)))
    if not meshes:
        raise ValueError("stage has no meshes to lay out")
    planes = np.concatenate(planes)
    uvs, record = planar_layout(np.concatenate(positions), size, np.concatenate(charted),
                                margin, reserved_rows, planes, np.concatenate(plane_normals),
                                np.concatenate(shading), xatlas)
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
    command.add_argument("--xatlas", type=Path,
                         help="xatlas_chart executable, for curved surfaces")
    command.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    if args.stage.resolve() == args.out.resolve():
        parser.error("--out must differ from --stage")
    positions, uvs, record = author(args.stage, args.out, args.size, args.margin,
                                    args.reserve_rows, set(args.exclude_mesh), args.xatlas)
    record.pop("keys", None)
    curved = record.pop("curved_mask")
    # Self-check with the independent invariant code: the layout must hold
    # every chart invariant and split no flat region, flat triangles must
    # take the atlas density exactly (float32 storage of positions and UVs
    # moves it by ~1e-4 at most) and curved ones stay within MAX_STRETCH.
    # A curved triangle's two principal scales each lie within
    # [1 / MAX_STRETCH, MAX_STRETCH], so its texel area density can differ
    # from another's by up to MAX_STRETCH ** 4.
    invariants = lightmap_seams.chart_invariants(
        positions, uvs, args.size, MAX_STRETCH ** 4 if curved.any() else 1.001)
    violations = []
    # Parked and zero-area triangles draw nothing and have no texel scale.
    measured = ~np.all(uvs == parking_uv(args.size), axis=(1, 2)) & \
        (triangle_areas(positions) > DEGENERATE_AREA)
    if record.get("texels_per_metre") and measured.any():
        low, high = np.ones(len(positions)), np.ones(len(positions))
        low[measured], high[measured] = stretch(positions[measured],
                                                uvs[measured] * args.size,
                                                record["texels_per_metre"])
        planar = ~curved & measured
        if planar.any() and (low[planar].min() < 1 / 1.001 or high[planar].max() > 1.001):
            violations.append("flat_density")
        if curved.any() and (low[curved].min() < 1 / MAX_STRETCH - 1e-3 or
                             high[curved].max() > MAX_STRETCH + 1e-3):
            violations.append("stretch")
        record["measured_stretch"] = [float(low[measured].min()), float(high[measured].max())]
    _, _, _, triangles, _ = lightmap_seams.find_seams(positions, uvs, args.size)
    normals, _ = lightmap_seams.triangle_normals(positions)
    # A seam across flat geometry is a layout error; inside a curved surface
    # xatlas chose the cut, and ktx2 stitches it under its seam gate.
    flat = int(((np.einsum("ij,ij->i", normals[triangles[:, 0]], normals[triangles[:, 1]])
                 > FLAT_COS) & ~curved[triangles[:, 0]] & ~curved[triangles[:, 1]]).sum()) \
        if len(triangles) else 0
    violations += [key for key in ("out_of_bounds", "overlap_texels", "bleed_texels",
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
