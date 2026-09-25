#!/usr/bin/env python3
"""Find, measure and stitch lightmap chart seams.

A surface that continues across a lightmap chart boundary is shaded from two
different sets of texels on either side of the boundary. Each side is baked,
denoised and gutter-filled on its own, so without stitching the light jumps
along the boundary: a visible line (on a relit Portal wall, one along every
triangle edge). This tool owns that invariant:

    extract  (OpenUSD Python) every world-space edge where two coplanar,
             front-facing triangles meet with different lightmap UVs -
             chart boundaries, including T-junctions where one edge meets
             several - sampled twice per texel into paired UV samples
             (`lightmap-seams/v1` .npz plus a JSON summary)
    measure  the luminance discontinuity of an atlas at every sample pair
    stitch   the texel values closest to the input (least squares) whose
             bilinear lookups agree on both sides of every seam sample, for
             one or more atlas-sized images that share the lightmap UVs
             (total, layers, directional page, sun visibility)

The runtime samples LMAP bilinearly at mip 0 (world_pbr.frag), so agreeing
bilinear lookups along the boundary make the light continuous across it.
Stitching changes only texels that a seam lookup touches; gutter texels (no
chart coverage) are nearly free, covered texels are held near their values.

    python3.12 lightmap_seams.py extract --stage lighting.usdc --size 4096 --out seams.npz
    python3 lightmap_seams.py measure --seams seams.npz --image atlas.exr
    python3 lightmap_seams.py stitch --seams seams.npz --coverage uv-coverage.exr \\
        --image atlas.exr=atlas-stitched.exr [--image ...] --receipt stitch.json

Image arrays follow imageio: row 0 is the top of the atlas (lightmap v = 1).
"""

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import numpy as np

SCHEMA = "lightmap-seams/v1"
LIGHTMAP_PRIMVAR = "lightmap_st"
# Triangles meet across a seam only when coplanar: across a crease the
# irradiance differs physically (it depends on the normal).
COPLANAR_COS = math.cos(math.radians(2.0))
# One flat region: a seam between such triangles of one mesh is a charting error.
FLAT_COS = math.cos(math.radians(0.01))
QUANTUM = 1e-5            # world position key (stage metres)
LINE_QUANTUM = 1e-4       # line direction and offset key
MIN_UV_AREA = 1e-12       # triangles with no chart (excluded materials)
SAMPLES_PER_TEXEL = 2
# Boundary pairs whose UVs agree this closely (texels) are one chart.
CONTINUOUS_TEXELS = 1e-3
# Stitch weights: seam agreement against staying at the baked value. Gutter
# texels are only ever seen through seam lookups, so they are nearly free.
COVERED_WEIGHT = 0.05
GUTTER_WEIGHT = 1e-4
# Gate (`measure`): discontinuities are relative to the local light, with a
# floor so near-black regions (where no step is visible) do not dominate.
FLOOR_FRACTION = 0.02


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


# --- extraction ------------------------------------------------------------

def stage_triangles(stage_path):
    """World triangles with lightmap UVs: (positions (n,3,3), uvs (n,3,2), meshes)."""
    from pxr import Usd, UsdGeom
    stage = Usd.Stage.Open(str(stage_path))
    cache = UsdGeom.XformCache()
    positions, uvs, names = [], [], []
    for prim in stage.Traverse():
        if not prim.IsA(UsdGeom.Mesh):
            continue
        mesh = UsdGeom.Mesh(prim)
        primvar = UsdGeom.PrimvarsAPI(prim).GetPrimvar(LIGHTMAP_PRIMVAR)
        if not primvar or not primvar.HasValue():
            continue
        points = np.asarray(mesh.GetPointsAttr().Get(), dtype=np.float64)
        counts = np.asarray(mesh.GetFaceVertexCountsAttr().Get())
        indices = np.asarray(mesh.GetFaceVertexIndicesAttr().Get())
        if len(counts) == 0:
            continue
        if np.any(counts != 3):
            raise ValueError("lighting stage mesh is not triangulated: %s" % prim.GetPath())
        world = np.asarray(cache.GetLocalToWorldTransform(prim), dtype=np.float64)
        points = points @ world[:3, :3] + world[3, :3]
        values = np.asarray(primvar.Get(), dtype=np.float64)
        interpolation = primvar.GetInterpolation()
        if primvar.IsIndexed():
            values = values[np.asarray(primvar.GetIndices())]
        if interpolation == UsdGeom.Tokens.faceVarying:
            corner_uv = values
        elif interpolation in (UsdGeom.Tokens.vertex, UsdGeom.Tokens.varying):
            corner_uv = values[indices]
        else:
            raise ValueError("unsupported %s interpolation %s at %s" %
                             (LIGHTMAP_PRIMVAR, interpolation, prim.GetPath()))
        positions.append(points[indices].reshape(-1, 3, 3))
        uvs.append(corner_uv.reshape(-1, 3, 2))
        names += [str(prim.GetPath())] * len(counts)
    if not positions:
        raise ValueError("lighting stage has no meshes with " + LIGHTMAP_PRIMVAR)
    return np.concatenate(positions), np.concatenate(uvs), names


def uv_area(uv):
    e1, e2 = uv[:, 1] - uv[:, 0], uv[:, 2] - uv[:, 0]
    return 0.5 * np.abs(e1[:, 0] * e2[:, 1] - e1[:, 1] * e2[:, 0])


def triangle_normals(positions):
    normals = np.cross(positions[:, 1] - positions[:, 0], positions[:, 2] - positions[:, 0])
    lengths = np.linalg.norm(normals, axis=1)
    return normals / np.maximum(lengths, 1e-30)[:, None], lengths > 1e-12


def line_overlaps(positions, edges, normals, coplanar_cos=None):
    """Pairs of triangle edges that overlap along one world line, from
    coplanar triangles on opposite sides of it: shared edges and T-junctions
    alike. `edges` are (t, a, b) corner pairs. Returns
    [(t, a, b, t2, a2, b2, direction, lo, hi)] with the overlap [lo, hi]
    along `direction`."""
    limit = COPLANAR_COS if coplanar_cos is None else coplanar_cos
    lines = {}
    for t, a, b in edges:
        pa, pb = positions[t, a], positions[t, b]
        direction = pb - pa
        length = np.linalg.norm(direction)
        if length < QUANTUM:
            continue
        direction = direction / length
        for component in direction:
            if abs(component) > 1e-9:
                if component < 0:
                    direction = -direction
                break
        offset = pa - np.dot(pa, direction) * direction
        key = (tuple(np.round(direction / LINE_QUANTUM).astype(np.int64)),
               tuple(np.round(offset / LINE_QUANTUM).astype(np.int64)))
        ta, tb = np.dot(pa, direction), np.dot(pb, direction)
        lines.setdefault(key, []).append((min(ta, tb), max(ta, tb), int(t), a, b, direction))
    pairs = []
    for key in sorted(lines):
        items = sorted(lines[key], key=lambda item: (item[0], item[1], item[2], item[3]))
        for index, (s0, s1, t, a, b, direction) in enumerate(items):
            for u0, u1, t2, a2, b2, _ in items[index + 1:]:
                if u0 >= s1 - QUANTUM:
                    break
                lo, hi = max(s0, u0), min(s1, u1)
                if hi - lo < QUANTUM or t2 == t:
                    continue
                if np.dot(normals[t], normals[t2]) < limit:
                    continue
                # Opposite sides of the shared line within the plane.
                side = np.cross(direction, positions[t].mean(axis=0) - positions[t, a])
                side2 = np.cross(direction, positions[t2].mean(axis=0) - positions[t2, a2])
                if np.dot(side, normals[t]) * np.dot(side2, normals[t]) >= 0:
                    continue
                pairs.append((t, a, b, t2, a2, b2, direction, lo, hi))
    return pairs


def boundary_pairs(positions, uvs, size):
    """Coplanar boundary edges that meet across a shared line.

    A triangle edge is interior when another triangle has the same edge with
    the same lightmap UVs at both ends. Every other (boundary) edge is paired
    with each overlapping boundary edge of a coplanar triangle on the other
    side of the same world line - exact matches and T-junctions alike.
    Returns ([(t, a, b, t2, a2, b2, direction, lo, hi, continuous)], stats):
    `continuous` pairs agree in lightmap UV along the whole overlap, so the
    two triangles are one chart (a T-junction inside it), not a seam.
    """
    normals, solid = triangle_normals(positions)
    charted = (uv_area(uvs) > MIN_UV_AREA) & solid
    keys = np.round(positions / QUANTUM).astype(np.int64)
    uv_keys = np.round(uvs * size * 64).astype(np.int64)

    def edge_key(t, a, b):
        ends = sorted([(tuple(keys[t, a]), tuple(uv_keys[t, a])),
                       (tuple(keys[t, b]), tuple(uv_keys[t, b]))])
        return tuple(ends)

    uses = {}
    for t in np.flatnonzero(charted):
        for k in range(3):
            uses.setdefault(edge_key(t, k, (k + 1) % 3), []).append(t)
    edges = [(t, k, (k + 1) % 3) for t in np.flatnonzero(charted) for k in range(3)
             if len(uses[edge_key(t, k, (k + 1) % 3)]) == 1]
    boundary_edges = sum(np.linalg.norm(positions[t, b] - positions[t, a]) >= QUANTUM
                         for t, a, b in edges)
    boundary_length = float(sum(np.linalg.norm(positions[t, b] - positions[t, a])
                                for t, a, b in edges))
    pairs = []
    gap = CONTINUOUS_TEXELS / size
    for t, a, b, t2, a2, b2, direction, lo, hi in line_overlaps(positions, edges, normals):
        continuous = all(np.abs(edge_uv(uvs, positions, t, a, b, direction, s) -
                                edge_uv(uvs, positions, t2, a2, b2, direction, s)
                                ).max() < gap for s in (lo, hi))
        pairs.append((t, a, b, t2, a2, b2, direction, lo, hi, continuous))
    stats = {"triangles": int(len(positions)), "charted_triangles": int(charted.sum()),
             "boundary_edges": boundary_edges, "boundary_length_m": boundary_length}
    return pairs, stats


def find_seams(positions, uvs, size):
    """Paired lightmap UV samples along every coplanar chart boundary.

    Returns (uv_a, uv_b, world_length, triangles, summary): sample i looks
    up side A at uv_a[i] and side B at uv_b[i]; world_length[i] is the
    stretch of seam it stands for and triangles[i] the two triangles.
    """
    pairs, summary = boundary_pairs(positions, uvs, size)
    uv_a, uv_b, weights, sides = [], [], [], []
    seam_edges = junctions = 0
    paired_length = 0.0
    for t, a, b, t2, a2, b2, direction, lo, hi, continuous in pairs:
        paired_length += 2 * (hi - lo)
        if continuous:
            junctions += 1
            continue
        seam_edges += 1
        texels = max(np.linalg.norm(edge_uv(uvs, positions, t, a, b, direction, hi) -
                                    edge_uv(uvs, positions, t, a, b, direction, lo)),
                     np.linalg.norm(edge_uv(uvs, positions, t2, a2, b2, direction, hi) -
                                    edge_uv(uvs, positions, t2, a2, b2, direction, lo)))
        count = max(2, int(math.ceil(texels * size * SAMPLES_PER_TEXEL)))
        for k in range(count):
            s = lo + (hi - lo) * (k + 0.5) / count
            uv_a.append(edge_uv(uvs, positions, t, a, b, direction, s))
            uv_b.append(edge_uv(uvs, positions, t2, a2, b2, direction, s))
            weights.append((hi - lo) / count)
            sides.append((t, t2))
    summary.update({"seam_edges": seam_edges, "continuous_junctions": junctions,
                    "seam_length_m": float(np.sum(weights)) * 2,
                    # Chart boundary with no coplanar partner: open borders and creases.
                    "unpaired_boundary_length_m": max(summary["boundary_length_m"] -
                                                      paired_length, 0.0),
                    "samples": len(uv_a)})
    shape = (-1, 2)
    return (np.asarray(uv_a, dtype=np.float64).reshape(shape),
            np.asarray(uv_b, dtype=np.float64).reshape(shape),
            np.asarray(weights, dtype=np.float64),
            np.asarray(sides, dtype=np.int64).reshape(shape), summary)


def edge_uv(uvs, positions, t, a, b, direction, s):
    """Lightmap UV of triangle t's edge (a, b) at line parameter s."""
    sa, sb = np.dot(positions[t, a], direction), np.dot(positions[t, b], direction)
    f = (s - sa) / (sb - sa)
    return uvs[t, a] + f * (uvs[t, b] - uvs[t, a])


# --- chart invariants --------------------------------------------------------

def charts(positions, uvs, size=4096):
    """Chart id per triangle: triangles joined by an edge equal in world
    position and lightmap UV at both ends, or by a UV-continuous T-junction,
    belong to one chart; uncharted (degenerate-UV) triangles get -1."""
    count = len(positions)
    _, solid = triangle_normals(positions)
    charted = (uv_area(uvs) > MIN_UV_AREA) & solid
    parent = np.arange(count)

    def root(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    def join(i, j):
        first, second = root(i), root(j)
        if first != second:
            parent[max(first, second)] = min(first, second)

    keys = np.round(positions / QUANTUM).astype(np.int64)
    uv_keys = np.round(uvs * size * 64).astype(np.int64)
    owner = {}
    for t in np.flatnonzero(charted):
        for k in range(3):
            a, b = k, (k + 1) % 3
            edge = tuple(sorted([(tuple(keys[t, a]), tuple(uv_keys[t, a])),
                                 (tuple(keys[t, b]), tuple(uv_keys[t, b]))]))
            if edge in owner:
                join(owner[edge], t)
            else:
                owner[edge] = t
    for t, _, _, t2, _, _, _, _, _, continuous in boundary_pairs(positions, uvs, size)[0]:
        if continuous:
            join(t, t2)
    labels = np.array([root(i) for i in range(count)])
    ids = {value: index for index, value in enumerate(sorted(set(labels[charted])))}
    return np.array([ids[labels[i]] if charted[i] else -1 for i in range(count)])


def texel_corners(uvs, height, width):
    """Triangle corners in texel coordinates (x right, y down from row 0)."""
    return np.stack([uvs[..., 0] * width - 0.5, (1.0 - uvs[..., 1]) * height - 0.5], axis=-1)


def triangle_centres(tri, height, width):
    """Texel centres strictly inside a texel-space triangle: (ys, xs)."""
    x0, y0 = np.maximum(np.ceil(tri.min(axis=0)).astype(int), 0)
    x1, y1 = np.minimum(np.floor(tri.max(axis=0)).astype(int), [width - 1, height - 1])
    if x1 < x0 or y1 < y0:
        return np.zeros(0, int), np.zeros(0, int)
    ys, xs = np.mgrid[y0:y1 + 1, x0:x1 + 1]
    a, b, c = tri
    det = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])
    if abs(det) < 1e-18:
        return np.zeros(0, int), np.zeros(0, int)
    w1 = ((xs - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (ys - a[1])) / det
    w2 = ((b[0] - a[0]) * (ys - a[1]) - (xs - a[0]) * (b[1] - a[1])) / det
    inside = (w1 > 1e-9) & (w2 > 1e-9) & (1 - w1 - w2 > 1e-9)
    return ys[inside], xs[inside]


def triangle_footprint(tri, height, width):
    """Texels a bilinear lookup anywhere inside a texel-space triangle can
    read: those whose 2 x 2 box around the centre meets it (exact
    separating-axis test of triangle against box). Returns (ys, xs)."""
    x0, y0 = np.maximum(np.floor(tri.min(axis=0) - 1).astype(int), 0)
    x1, y1 = np.minimum(np.ceil(tri.max(axis=0) + 1).astype(int), [width - 1, height - 1])
    if x1 < x0 or y1 < y0:
        return np.zeros(0, int), np.zeros(0, int)
    ys, xs = np.mgrid[y0:y1 + 1, x0:x1 + 1]
    hit = ((np.abs(xs - np.clip(xs, tri[:, 0].min(), tri[:, 0].max())) < 1) &
           (np.abs(ys - np.clip(ys, tri[:, 1].min(), tri[:, 1].max())) < 1))
    centre = np.stack([xs, ys], axis=-1).astype(np.float64)
    for k in range(3):
        a, b, c = tri[k], tri[(k + 1) % 3], tri[(k + 2) % 3]
        normal = np.array([b[1] - a[1], a[0] - b[0]])
        if np.dot(normal, c - a) > 0:
            normal = -normal
        # The box of half-size 1 projects to a radius |nx| + |ny| on the normal.
        hit &= (centre - a) @ normal < np.abs(normal).sum()
    return ys[hit], xs[hit]


def rasterize(uvs, labels, height, width):
    """Texel-centre coverage: (count (H, W) of charted triangles containing
    each texel centre, chart (H, W) of the last one, or -1)."""
    count = np.zeros((height, width), dtype=np.int32)
    chart = np.full((height, width), -1, dtype=np.int64)
    corners = texel_corners(uvs, height, width)
    for t in np.flatnonzero(labels >= 0):
        ys, xs = triangle_centres(corners[t], height, width)
        count[ys, xs] += 1
        chart[ys, xs] = labels[t]
    return count, chart


def footprint(uvs, labels, chart_id, height, width):
    """Texels a bilinear lookup anywhere inside chart `chart_id` can read."""
    reads = np.zeros((height, width), dtype=bool)
    corners = texel_corners(uvs, height, width)
    for t in np.flatnonzero(labels == chart_id):
        ys, xs = triangle_footprint(corners[t], height, width)
        reads[ys, xs] = True
    return reads


def chart_invariants(positions, uvs, size, max_density_spread=1.5):
    """Violations of the lightmap chart invariants as a dict of counts; all
    zero means the atlas is valid:

      out_of_bounds      charted UV corners outside [0, 1]
      overlap_texels     texel centres inside triangles of two charts
      bleed_texels       texels that bilinear lookups inside two different
                         charts both read (one chart's light leaks into the other)
      density_spread     max/min texels per square metre, reported, and
                         `density_violation` when above the bound
    """
    labels = charts(positions, uvs, size)
    charted = labels >= 0
    result = {"charts": int(labels.max() + 1) if charted.any() else 0,
              "charted_triangles": int(charted.sum())}
    corner_uv = uvs[charted]
    result["out_of_bounds"] = int(((corner_uv < 0) | (corner_uv > 1)).any(axis=-1).sum())
    # One pass over triangles. Overlap: a texel centre inside triangles of two
    # charts (two triangles of one chart share only edges, which the strict
    # inside test never counts twice). Bleed: a texel that lookups inside two
    # charts both read cannot hold both charts' light.
    corners = texel_corners(uvs, size, size)
    covered_by = np.full((size, size), -1, dtype=np.int64)
    read_by = np.full((size, size), -1, dtype=np.int64)
    overlap = np.zeros((size, size), dtype=bool)
    bleed = np.zeros((size, size), dtype=bool)
    for t in np.flatnonzero(charted):
        chart_id = labels[t]
        ys, xs = triangle_centres(corners[t], size, size)
        previous = covered_by[ys, xs]
        overlap[ys[(previous >= 0) & (previous != chart_id)],
                xs[(previous >= 0) & (previous != chart_id)]] = True
        covered_by[ys, xs] = chart_id
        ys, xs = triangle_footprint(corners[t], size, size)
        previous = read_by[ys, xs]
        clash = (previous >= 0) & (previous != chart_id)
        bleed[ys[clash], xs[clash]] = True
        read_by[ys[previous < 0], xs[previous < 0]] = chart_id
    result["overlap_texels"] = int(overlap.sum())
    result["bleed_texels"] = int(bleed.sum())
    area_world = 0.5 * np.linalg.norm(np.cross(positions[:, 1] - positions[:, 0],
                                               positions[:, 2] - positions[:, 0]), axis=1)
    usable = charted & (area_world > 1e-12)
    if usable.any():
        density = uv_area(uvs)[usable] * size * size / area_world[usable]
        result["density_spread"] = float(density.max() / density.min())
    else:
        result["density_spread"] = 1.0
    result["density_violation"] = int(result["density_spread"] > max_density_spread)
    return result


# --- lookups, measurement and stitching -------------------------------------

def bilinear_taps(uv, height, width):
    """Texel indices (n,4) and weights (n,4) of a clamped bilinear lookup."""
    x = uv[:, 0] * width - 0.5
    y = (1.0 - uv[:, 1]) * height - 0.5
    x0, y0 = np.floor(x), np.floor(y)
    fx, fy = x - x0, y - y0
    x0, y0 = x0.astype(np.int64), y0.astype(np.int64)
    xs = np.clip(np.stack([x0, x0 + 1, x0, x0 + 1], axis=1), 0, width - 1)
    ys = np.clip(np.stack([y0, y0, y0 + 1, y0 + 1], axis=1), 0, height - 1)
    weights = np.stack([(1 - fx) * (1 - fy), fx * (1 - fy), (1 - fx) * fy, fx * fy], axis=1)
    return ys * width + xs, weights


def lookup(image, taps, weights):
    flat = image.reshape(-1, image.shape[2])
    return np.einsum("nk,nkc->nc", weights, flat[taps])


def luminance(rgb):
    return rgb[..., :3] @ np.array([0.2126, 0.7152, 0.0722])


def measure(image, seams, coverage=None, reference=None, absolute=False):
    """Seam discontinuity statistics of an (H, W, C) image.

    Light pages are measured relative to the light at the seam: their own
    luminance, or `reference`'s (a separated layer against the total, whose
    sum is what is seen), with a floor of FLOOR_FRACTION of the reference's
    median so a step in near-black light, which is not visible, does not
    dominate. `absolute` pages (the directional gradient, the sun
    visibility, whose values already scale the light) are measured as the
    largest per-channel difference.
    """
    height, width = image.shape[:2]
    taps_a, w_a = bilinear_taps(seams["uv_a"], height, width)
    taps_b, w_b = bilinear_taps(seams["uv_b"], height, width)
    if len(taps_a) == 0:
        raise ValueError("no seam samples to measure")
    a = lookup(image.astype(np.float64), taps_a, w_a)
    b = lookup(image.astype(np.float64), taps_b, w_b)
    channels = min(3, image.shape[2])
    result = {"samples": int(len(a)), "mode": "absolute" if absolute else "relative"}
    if absolute:
        difference = np.abs(a[:, :channels] - b[:, :channels]).max(axis=1)
    else:
        def level_of(values):
            return values[:, :channels].mean(axis=1) if channels < 3 else luminance(values)
        base = image if reference is None else reference
        ra = level_of(lookup(base.astype(np.float64), taps_a, w_a))
        rb = level_of(lookup(base.astype(np.float64), taps_b, w_b))
        covered = coverage if coverage is not None else np.ones(image.shape[:2], bool)
        base_channels = min(3, base.shape[2])
        values = base[covered][:, :base_channels].astype(np.float64)
        level = float(np.median(np.abs(values.mean(axis=1)))) if len(values) else 0.0
        floor = max(level * FLOOR_FRACTION, 1e-6)
        difference = np.abs(level_of(a) - level_of(b)) / np.maximum(
            np.abs(ra + rb) / 2, floor)
        result.update({"median_level": level, "floor": floor,
                       "reference": "own" if reference is None else "given"})
    result.update({"mean": float(difference.mean()),
                   "p95": float(np.percentile(difference, 95)),
                   "p99": float(np.percentile(difference, 99)),
                   "max": float(difference.max())})
    if coverage is not None:
        hit = coverage.reshape(-1)[taps_a[np.arange(len(taps_a)), np.argmax(w_a, axis=1)]]
        result["side_a_on_coverage"] = float(hit.mean())
    return result


def buried_texels(raw_total, coverage):
    """Covered texels the raw (undenoised) total bake left exactly black: a
    surface no light reaches, such as floor under a box resting on it or a
    face flush against another. Where a seam meets one, the black side is
    the hidden one, so stitching moves it and leaves the visible side."""
    return coverage & (raw_total[..., :3].max(axis=2) <= 0)


def stitch_system(seams, coverage, free=None):
    """Factorized normal equations shared by every image of one atlas.

    Covered texels are held near their values; gutter texels and `free`
    ones (buried_texels) are nearly free to move."""
    from scipy.sparse import coo_matrix, diags
    from scipy.sparse.linalg import splu
    height, width = coverage.shape
    if len(seams["uv_a"]) == 0:
        raise ValueError("no seam samples to stitch")
    taps_a, w_a = bilinear_taps(seams["uv_a"], height, width)
    taps_b, w_b = bilinear_taps(seams["uv_b"], height, width)
    used = np.unique(np.concatenate([taps_a.ravel(), taps_b.ravel()]))
    column = {texel: i for i, texel in enumerate(used)}
    lookup_column = np.vectorize(column.__getitem__)
    rows = np.repeat(np.arange(len(taps_a)), 8)
    cols = lookup_column(np.concatenate([taps_a, taps_b], axis=1)).ravel()
    sample_weight = np.sqrt(seams["weight"] / seams["weight"].mean())
    vals = (np.concatenate([w_a, -w_b], axis=1) * sample_weight[:, None]).ravel()
    matrix = coo_matrix((vals, (rows, cols)), shape=(len(taps_a), len(used))).tocsr()
    held = coverage if free is None else coverage & ~free
    prior = np.where(held.reshape(-1)[used], COVERED_WEIGHT, GUTTER_WEIGHT) ** 2
    normal = (matrix.T @ matrix + diags(prior)).tocsc()
    return {"used": used, "prior": prior, "factor": splu(normal)}


def stitch(image, system):
    """Stitched copy of an (H, W, C) image; alpha is left as it is."""
    flat = image.reshape(-1, image.shape[2]).astype(np.float64)
    result = flat.copy()
    for channel in range(min(3, image.shape[2])):
        values = flat[system["used"], channel]
        result[system["used"], channel] = system["factor"].solve(system["prior"] * values)
    return result.reshape(image.shape).astype(image.dtype)


def load_seams(path):
    data = np.load(path)
    if str(data["schema"]) != SCHEMA:
        raise ValueError("seam file schema must be " + SCHEMA)
    return {"uv_a": data["uv_a"], "uv_b": data["uv_b"], "weight": data["weight"],
            "size": int(data["size"])}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    extract = commands.add_parser("extract")
    extract.add_argument("--stage", type=Path, required=True)
    extract.add_argument("--size", type=int, required=True, help="atlas width in texels")
    extract.add_argument("--out", type=Path, required=True)
    extract.add_argument("--check", action="store_true",
                         help="fail on a chart invariant violation or a seam splitting a "
                              "flat region of one mesh")
    measure_parser = commands.add_parser("measure")
    measure_parser.add_argument("--seams", type=Path, required=True)
    measure_parser.add_argument("--image", type=Path, required=True)
    measure_parser.add_argument("--coverage", type=Path)
    stitch_parser = commands.add_parser("stitch")
    stitch_parser.add_argument("--seams", type=Path, required=True)
    stitch_parser.add_argument("--coverage", type=Path, required=True)
    stitch_parser.add_argument("--image", action="append", required=True,
                               help="IN=OUT atlas-sized EXR to stitch")
    stitch_parser.add_argument("--receipt", type=Path, required=True)
    stitch_parser.add_argument("--max-p99", type=float,
                               help="fail when a stitched image's seam p99 exceeds this")
    args = parser.parse_args()
    if args.command == "extract":
        positions, uvs, names = stage_triangles(args.stage)
        uv_a, uv_b, weight, triangles, summary = find_seams(positions, uvs, args.size)
        meshes = sorted(set(names))
        mesh_index = np.array([meshes.index(name) for name in names], dtype=np.int64)
        args.out.parent.mkdir(parents=True, exist_ok=True)
        with open(args.out, "wb") as handle:
            np.savez_compressed(handle, schema=SCHEMA, uv_a=uv_a, uv_b=uv_b, weight=weight,
                                size=args.size, triangles=triangles,
                                centroids=positions.mean(axis=1)[triangles.ravel()].reshape(
                                    -1, 2, 3) if len(triangles) else np.zeros((0, 2, 3)),
                                meshes=np.array(meshes),
                                mesh=mesh_index[triangles] if len(triangles) else
                                np.zeros((0, 2), np.int64))
        # A coplanar connected region of one mesh must chart as one island:
        # a seam between two triangles of the same mesh is a charting error.
        # A curved mesh cannot be one chart, so only a flat split counts.
        normals, _ = triangle_normals(positions)
        same_mesh = mesh_index[triangles[:, 0]] == mesh_index[triangles[:, 1]] \
            if len(triangles) else np.zeros(0, bool)
        flat = (np.einsum("ij,ij->i", normals[triangles[:, 0]], normals[triangles[:, 1]])
                > FLAT_COS) if len(triangles) else np.zeros(0, bool)
        invariants = chart_invariants(positions, uvs, args.size)
        invariants["flat_in_mesh_seam_length_m"] = float(weight[same_mesh & flat].sum())
        invariants["curved_in_mesh_seam_length_m"] = float(weight[same_mesh & ~flat].sum())
        invariants["flat_in_mesh_seam_meshes"] = sorted(
            {meshes[mesh_index[t]] for t in triangles[same_mesh & flat, 0]})[:20]
        violations = [key for key in ("out_of_bounds", "overlap_texels", "bleed_texels",
                                      "density_violation") if invariants[key]]
        if invariants["flat_in_mesh_seam_length_m"] > 0:
            violations.append("flat_in_mesh_seam_length_m")
        summary = dict(summary, schema=SCHEMA, stage_sha256=sha256(args.stage), size=args.size,
                       charts=invariants, status="fail" if violations else "pass",
                       violations=violations)
        args.out.with_suffix(".json").write_text(json.dumps(summary, indent=2,
                                                            sort_keys=True) + "\n")
        print("LIGHTMAP_SEAMS " + json.dumps(summary, sort_keys=True))
        if args.check and violations:
            raise SystemExit("lightmap chart invariants violated: " + ", ".join(violations))
        return
    import imageio.v3 as iio
    seams = load_seams(args.seams)
    if args.command == "measure":
        image = iio.imread(args.image)
        coverage = iio.imread(args.coverage)[..., :3].min(axis=2) > 0.5 if args.coverage else None
        print(json.dumps(measure(image, seams, coverage), indent=2, sort_keys=True))
        return
    import lightmap_denoise
    coverage = iio.imread(args.coverage)[..., :3].min(axis=2) > 0.5
    if coverage.shape[1] != seams["size"]:
        raise ValueError("coverage width differs from the seam extraction size")
    if len(seams["uv_a"]) == 0:
        raise ValueError("no seam samples: nothing proves the atlas continuous")
    system = stitch_system(seams, coverage)
    records = {}
    for item in args.image:
        source, _, target = item.partition("=")
        image = iio.imread(source)
        if image.shape[:2] != coverage.shape:
            raise ValueError("%s is not atlas-sized" % source)
        stitched = stitch(image, system)
        before, after = measure(image, seams, coverage), measure(stitched, seams, coverage)
        Path(target).parent.mkdir(parents=True, exist_ok=True)
        lightmap_denoise.write_linear_exr(Path(target), np.ascontiguousarray(
            stitched.astype(np.float32)))
        changed = np.abs(stitched.astype(np.float64) - image)[..., :3].max(axis=2)
        records[Path(source).name] = {
            "source_sha256": sha256(source), "stitched_sha256": sha256(target),
            "before": before, "after": after,
            "changed_covered_texels": int((changed[coverage] > 0).sum()),
            "max_covered_change": float(changed[coverage].max())}
        if args.max_p99 is not None and after["p99"] > args.max_p99:
            raise SystemExit("%s: stitched seam p99 %.4g exceeds %.4g" %
                             (source, after["p99"], args.max_p99))
    receipt = {"schema": SCHEMA + "-stitch", "status": "pass", "seams_sha256": sha256(args.seams),
               "coverage_sha256": sha256(args.coverage), "samples": int(len(seams["uv_a"])),
               "stitched_texels": int(len(system["used"])),
               "weights": {"covered": COVERED_WEIGHT, "gutter": GUTTER_WEIGHT},
               "images": records}
    args.receipt.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n")
    print("LIGHTMAP_STITCH " + json.dumps({name: {"before_p99": r["before"]["p99"],
                                                   "after_p99": r["after"]["p99"]}
                                            for name, r in records.items()}, sort_keys=True))


if __name__ == "__main__":
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    main()
