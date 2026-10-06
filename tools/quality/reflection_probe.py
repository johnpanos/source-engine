"""Reflection probe layout shared by the Blender face renderer, the KTX2
packager and `world_pbr.frag`.

A probe is rendered as six 90-degree cube faces at one point (USD/Source Z-up
world), resampled to a cube in the Vulkan convention (`CUBE_FACES`, sampled by
the world direction), and reduced to a GGX-prefiltered roughness mip chain:
mip i of M holds the scene convolved with the GGX lobe of perceptual roughness
i / (M - 1) (alpha = roughness^2) under the split-sum N = V = R assumption, by
filtered importance sampling of a box-reduced source pyramid. The core's
surface program selects lod = roughness * (M - 1) of the RPRB cube array, so a
rough surface no longer mirrors small bright emitters as hot spots the way the
former blurred box mips did. (Before RPRB v8 the chain was an equirect strip
atlas; that form and the LMAP-band carrier are gone.)

Parallax correction (the per-probe math `world_pbr_probe.glsl` mirrors):

  * `fit_parallax_box` turns the capture's per-texel hit distances (Cycles'
    depth pass of the six faces, sky excluded) into an axis-aligned proxy box.
    Each of the six box planes is the farthest strong planar peak of that
    face's solid-angle-weighted distance histogram; a plain percentile is
    pulled through doorways and is biased by the floor's continuous run of
    distances up to the wall. A face that mostly sees sky stays open.
  * `parallax_lookup` intersects the reflected ray from the shaded point with
    the box and samples toward the hit from the capture point (Lagarde and
    Zanuttini, "Local Image-based Lighting With Parallax-corrected Cubemap",
    SIGGRAPH 2012, the box-proxy method).
  * `distance_roughness` sharpens the lookup when the hit is closer to the
    shaded point than to the capture point, and `corrected_direction` eases
    the corrected direction back toward the plain one as roughness grows
    (Lagarde and de Rousiers, "Moving Frostbite to Physically Based
    Rendering 3.0", SIGGRAPH 2014 course notes, Listing 25 and Listing F.1).

Why: a direction-only probe assumes an infinitely distant scene, so the
reflection does not move as the viewer does and is skewed everywhere except at
the capture point (3kliksphilip, "Advanced Reflections in CS:GO... and for
Source 2?", 2019). The design record is RFC/0007-progress.md, R50-PARALLAX.
"""

import math

import numpy as np

# name: (forward, up) in world space. Blender cameras look down -Z with +Y up.
FACES = {
    "px": ((1, 0, 0), (0, 0, 1)), "nx": ((-1, 0, 0), (0, 0, 1)),
    "py": ((0, 1, 0), (0, 0, 1)), "ny": ((0, -1, 0), (0, 0, 1)),
    "pz": ((0, 0, 1), (0, -1, 0)), "nz": ((0, 0, -1), (0, 1, 0)),
}


def face_basis(name):
    forward, up = (np.asarray(v, dtype=np.float64) for v in FACES[name])
    right = np.cross(forward, up)
    return forward, up, right


def hammersley(count):
    bits = np.arange(count, dtype=np.uint32)
    bits = ((bits << 16) | (bits >> 16)) & 0xFFFFFFFF
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1)
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2)
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4)
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8)
    return np.arange(count) / count, bits.astype(np.float64) / 2.0 ** 32


# ------------------------------------------------------------ cube layout
#
# RPRB v8 stores each probe as a cube (layer `rank`-independent: layer = probe
# index) in the Vulkan cube convention, sampled by world direction with no
# remap: layer faces are +X -X +Y -Y +Z -Z, and a direction's face and
# (s, t) are the Vulkan specification's cube map face selection table
# (a texel (column, row) sits at s = (column + 0.5) / size, t = (row + 0.5) /
# size). World space is the bake's (Z up); nothing else is converted.

CUBE_FACES = ("px", "nx", "py", "ny", "pz", "nz")


def cube_vectors(face, sc, tc):
    """Unnormalized directions of cube face index array `face` at tangent-plane
    coordinates (sc, tc) in [-1, 1] (the inverse of `cube_select`)."""
    one = np.ones_like(sc)
    x = np.select([face == 0, face == 1, face == 2, face == 3, face == 4],
                  [one, -one, sc, sc, sc], -sc)
    y = np.select([face == 0, face == 1, face == 2, face == 3, face == 4],
                  [-tc, -tc, one, -one, -tc], -tc)
    z = np.select([face == 0, face == 1, face == 2, face == 3, face == 4],
                  [-sc, sc, tc, -tc, one], -one)
    return np.stack((x, y, z), axis=-1)


def cube_face_directions(face, size):
    """(size, size, 3) unit directions of a cube face's texel centres."""
    s = (np.arange(size) + 0.5) / size * 2.0 - 1.0
    sc, tc = np.meshgrid(s, s)  # sc varies along columns, tc along rows
    d = cube_vectors(np.full(sc.shape, CUBE_FACES.index(face)), sc, tc)
    return d / np.linalg.norm(d, axis=-1, keepdims=True)


def cube_select(directions):
    """(face index, s, t) of directions per the Vulkan face selection table;
    s and t are in [0, 1]."""
    d = np.asarray(directions, dtype=np.float64)
    a = np.abs(d)
    x_major = (a[..., 0] >= a[..., 1]) & (a[..., 0] >= a[..., 2])
    y_major = ~x_major & (a[..., 1] >= a[..., 2])
    face = np.where(x_major, np.where(d[..., 0] >= 0, 0, 1),
                    np.where(y_major, np.where(d[..., 1] >= 0, 2, 3),
                             np.where(d[..., 2] >= 0, 4, 5)))
    ma = np.where(x_major, a[..., 0], np.where(y_major, a[..., 1], a[..., 2]))
    ma = np.maximum(ma, 1e-30)
    x, y, z = d[..., 0], d[..., 1], d[..., 2]
    sc = np.select([face == 0, face == 1, face == 2, face == 3, face == 4],
                   [-z, z, x, x, x], -x)
    tc = np.select([face == 0, face == 1, face == 2, face == 3, face == 4],
                   [-y, -y, z, -z, -y], -y)
    return face, (sc / ma + 1.0) / 2.0, (tc / ma + 1.0) / 2.0


def faces_to_cube(faces, size, channels=3):
    """The bake's face images (`FACES`, name -> (N, N, C), row 0 at the face's
    up edge) resampled, nearest texel, to a (6, size, size, channels) cube."""
    result = np.zeros((6, size, size, channels), dtype=np.float64)
    for index, name in enumerate(CUBE_FACES):
        directions = cube_face_directions(name, size).reshape(-1, 3)
        best = np.full(directions.shape[0], -np.inf)
        out = np.zeros((directions.shape[0], channels))
        for source, image in faces.items():
            forward, up, right = face_basis(source)
            depth = directions @ forward
            chosen = depth > best
            best = np.where(chosen, depth, best)
            n = image.shape[0]
            safe = np.maximum(depth, 1e-9)
            column = np.clip((((directions @ right) / safe + 1) / 2 * n).astype(np.int64), 0, n - 1)
            row = np.clip(((1 - (directions @ up) / safe) / 2 * n).astype(np.int64), 0, n - 1)
            out[chosen] = image[row[chosen], column[chosen], :channels]
        result[index] = out.reshape(size, size, channels)
    return result


def cube_pyramid(cube, minimum_size):
    """Box-filtered levels (2x2 per face) down to `minimum_size`."""
    levels = [cube]
    while levels[-1].shape[1] // 2 >= minimum_size:
        previous = levels[-1]
        size = previous.shape[1] // 2
        levels.append(previous.reshape(6, size, 2, size, 2, previous.shape[3]).mean(axis=(2, 4)))
    return levels


def sample_cube(cube, directions):
    """Bilinear, seamless lookup of unit directions in a (6, N, N, C) cube:
    taps beyond a face's edge re-select the face by direction."""
    n = cube.shape[1]
    d = np.asarray(directions, dtype=np.float64).reshape(-1, 3)
    face, s, t = cube_select(d)
    x = s * n - 0.5
    y = t * n - 0.5
    x0 = np.floor(x).astype(np.int64)
    y0 = np.floor(y).astype(np.int64)
    fx, fy = (x - x0)[:, None], (y - y0)[:, None]
    result = np.zeros((d.shape[0], cube.shape[3]))
    for dx, dy, weight in ((0, 0, (1 - fx) * (1 - fy)), (1, 0, fx * (1 - fy)),
                           (0, 1, (1 - fx) * fy), (1, 1, fx * fy)):
        cx, cy = x0 + dx, y0 + dy
        inside = (cx >= 0) & (cx < n) & (cy >= 0) & (cy < n)
        tap_face = face.copy()
        tcx, tcy = np.clip(cx, 0, n - 1), np.clip(cy, 0, n - 1)
        outside = ~inside
        if outside.any():
            # The tap's direction on the tangent plane of its own face.
            sc = ((cx[outside] + 0.5) / n) * 2 - 1
            tc = ((cy[outside] + 0.5) / n) * 2 - 1
            vec = cube_vectors(face[outside], sc, tc)
            nface, ns, nt = cube_select(vec)
            tap_face[outside] = nface
            tcx[outside] = np.clip((ns * n).astype(np.int64), 0, n - 1)
            tcy[outside] = np.clip((nt * n).astype(np.int64), 0, n - 1)
        result += cube[tap_face, tcy, tcx] * weight
    return result


def ggx_prefilter_cube(pyramid, size, roughness, samples):
    """One cube mip of `size`: the GGX convolution of `ggx_prefilter` over a
    cube pyramid. Returns (6, size, size, 3)."""
    out = np.zeros((6, size, size, 3))
    alpha = roughness * roughness
    first, second = hammersley(samples)
    phi = 2 * math.pi * first
    cos_theta = np.sqrt((1 - second) / (1 + (alpha * alpha - 1) * second))
    sin_theta = np.sqrt(1 - cos_theta * cos_theta)
    local_h = np.stack((sin_theta * np.cos(phi), sin_theta * np.sin(phi), cos_theta), axis=1)
    local_l = 2 * cos_theta[:, None] * local_h - np.array((0.0, 0.0, 1.0))
    n_dot_l = local_l[:, 2]
    keep = n_dot_l > 0
    local_l, n_dot_l, cos_theta = local_l[keep], n_dot_l[keep], cos_theta[keep]
    d = alpha * alpha / (math.pi * ((cos_theta ** 2) * (alpha * alpha - 1) + 1) ** 2)
    sample_solid_angle = 1.0 / (len(first) * d / 4.0 + 1e-12)
    base = pyramid[0]
    texel_solid_angle = 4 * math.pi / (6 * base.shape[1] * base.shape[2])
    lods = np.clip(0.5 * np.log2(sample_solid_angle / texel_solid_angle) + 1.0, 0,
                   len(pyramid) - 1)
    for index, name in enumerate(CUBE_FACES):
        directions = cube_face_directions(name, size).reshape(-1, 3)
        up = np.where(np.abs(directions[:, 2:3]) < 0.999, np.array((0.0, 0.0, 1.0)),
                      np.array((1.0, 0.0, 0.0)))
        tangent = np.cross(up, directions)
        tangent /= np.linalg.norm(tangent, axis=1, keepdims=True)
        bitangent = np.cross(directions, tangent)
        total = np.zeros((directions.shape[0], 3))
        weight = 0.0
        for k in range(len(local_l)):
            world = (local_l[k, 0] * tangent + local_l[k, 1] * bitangent +
                     local_l[k, 2] * directions)
            level = lods[k]
            lower = int(math.floor(level))
            upper = min(lower + 1, len(pyramid) - 1)
            blend = level - lower
            radiance = sample_cube(pyramid[lower], world) * (1 - blend)
            if blend > 0:
                radiance += sample_cube(pyramid[upper], world) * blend
            total += radiance * n_dot_l[k]
            weight += n_dot_l[k]
        out[index] = (total / weight).reshape(size, size, 3)
    return out


def cube_mip_chain(cube, minimum_size=4, samples=256):
    """GGX-prefiltered cube chain; mip i has perceptual roughness
    i / (count - 1). Level 0 is `cube` unchanged."""
    pyramid = cube_pyramid(cube, minimum_size)
    count = len(pyramid)
    mips = [cube]
    for index in range(1, count):
        mips.append(ggx_prefilter_cube(pyramid, pyramid[index].shape[1], index / (count - 1),
                                       samples))
    return mips


# ------------------------------------------------------- parallax correction

# Cycles writes the background's depth as 1e10; anything this far is sky.
SKY_DISTANCE = 1.0e9
# A box face whose texels mostly see sky stays open at this extent (meters):
# the lookup along it is then direction-only, which is right for sky.
OPEN_EXTENT = 1000.0
# Planar peaks at least this fraction of a face's strongest peak are box
# planes; the farthest such peak wins (farther than furniture in front of a
# wall, nearer than a room seen through a doorway).
PLANE_FRACTION = 0.25
# Relative width of the log-distance histogram bins.
PLANE_BIN = 0.01
# A face is open when more than this fraction of its solid angle is sky.
OPEN_FRACTION = 0.5
AXES = (("px", 0, 1.0), ("nx", 0, -1.0), ("py", 1, 1.0), ("ny", 1, -1.0),
        ("pz", 2, 1.0), ("nz", 2, -1.0))


def face_samples(depths):
    """(directions, distances, solid-angle weights) of six depth faces.

    `depths`: name -> (N, N) ray distances from the capture point, row 0 at
    the face's `up` edge (the layout `faces_to_cube` reads). Cycles' depth
    pass is planar (camera z); `pbrt_reflection_probe.py` converts it.
    """
    directions, distances, weights = [], [], []
    for name, depth in depths.items():
        forward, up, right = face_basis(name)
        size = depth.shape[0]
        t = (np.arange(size) + 0.5) / size * 2 - 1
        x, y = np.meshgrid(t, -t)
        vectors = forward + x[..., None] * right + y[..., None] * up
        length = np.linalg.norm(vectors, axis=2)
        directions.append((vectors / length[..., None]).reshape(-1, 3))
        distances.append(np.asarray(depth, dtype=np.float64).reshape(-1))
        # Solid angle of a cube-face texel: (2/N)^2 / |v|^3.
        weights.append(((2.0 / size) ** 2 / length ** 3).reshape(-1))
    return np.concatenate(directions), np.concatenate(distances), np.concatenate(weights)


def planar_peak(distances, weights, bound=None):
    """The farthest strong planar peak of a weighted distance sample, not
    beyond `bound` when one is given (the nearest strong peak when every
    one is beyond it)."""
    logs = np.log(distances)
    step = math.log1p(PLANE_BIN)
    bins = np.floor(logs / step).astype(np.int64)
    # One empty bin either side, so the window below keeps the bins' indices.
    low = bins.min() - 1
    mass = np.bincount(bins - low, weights=weights, minlength=int(bins.max() - low) + 2)
    # A plane exactly on a bin edge splits between two bins: judge peaks on
    # the three-bin window.
    window = np.convolve(mass, np.ones(3), mode="same")
    peaks = [i for i in range(len(window))
             if window[i] > 0 and window[i] >= window[max(i - 1, 0)] and
             window[i] >= window[min(i + 1, len(window) - 1)]]
    strongest = max(window[i] for i in peaks)
    strong = [i for i in peaks if window[i] >= PLANE_FRACTION * strongest]
    if bound is not None:
        limit = math.floor(math.log(bound) / step) - low + 1
        within = [i for i in strong if i <= limit]
        strong = within or [min(strong)]
    chosen = max(strong)
    # The plane's weighted median: exact for a flat wall, whose texels share
    # one distance, whatever floor or ceiling texels share its bins.
    near = np.abs(bins - low - chosen) <= 1
    values, mass = distances[near], weights[near]
    order = np.argsort(values)
    cumulative = np.cumsum(mass[order])
    return float(values[order][np.searchsorted(cumulative, 0.5 * cumulative[-1])])


def fit_parallax_box(capture, directions, distances, weights):
    """Axis-aligned proxy box (min, max) around `capture` and its fit report.

    Per box face: the texels whose direction's dominant axis is that face's
    (the cube face itself for a cube capture); their plane distances along
    the axis; the farthest strong planar peak. Sky texels (distance >=
    SKY_DISTANCE) never place a plane; a face that is mostly sky is open.

    A wall face's plane is also bounded by how far the ceiling runs along its
    axis without a break from the zenith (`horizontal_bound`): a room's
    ceiling reaches its walls whatever furniture stands on the floor, while
    the next room's ceiling seen under a doorway's lintel is a separate run.
    Without the bound, a capture near a doorway fits a box through it. An
    open ceiling (sky) leaves the walls unbounded.
    """
    capture = np.asarray(capture, dtype=np.float64)
    directions = np.asarray(directions, dtype=np.float64)
    distances = np.asarray(distances, dtype=np.float64)
    weights = np.asarray(weights, dtype=np.float64)
    sky = ~np.isfinite(distances) | (distances >= SKY_DISTANCE)
    dominant = np.argmax(np.abs(directions), axis=1)
    extent = np.zeros(6)
    faces = {}
    hits_all = capture + np.where(sky, 0.0, distances)[:, None] * directions
    # Vertical faces first: the floor and ceiling bound the walls.
    order = [4, 5, 0, 1, 2, 3]
    for index in order:
        name, axis, sign = AXES[index]
        selected = (dominant == axis) & (np.sign(directions[:, axis]) == sign)
        total = weights[selected].sum()
        hits = selected & ~sky
        open_fraction = 1.0 - weights[hits].sum() / total if total > 0 else 1.0
        if open_fraction > OPEN_FRACTION or not hits.any():
            extent[index] = OPEN_EXTENT
            faces[name] = {"open": True, "sky_fraction": open_fraction}
            continue
        plane = distances[hits] * np.abs(directions[hits, axis])
        bound = None
        if axis != 2:
            bound = horizontal_bound(capture, hits_all, sky, directions, axis, sign, faces)
        extent[index] = max(planar_peak(np.maximum(plane, 1e-6), weights[hits], bound), 1e-3)
        faces[name] = {"open": False, "sky_fraction": open_fraction,
                       "extent": extent[index], "floor_bound": bound}
        if axis == 2:
            faces[name]["plane_z"] = capture[2] + sign * extent[index]
    box_min = capture - np.array((extent[1], extent[3], extent[5]))
    box_max = capture + np.array((extent[0], extent[2], extent[4]))
    report = fit_residual(capture, box_min, box_max, directions, distances, weights)
    report["faces"] = faces
    return box_min, box_max, report


FLOOR_TOLERANCE = 0.02
REACH_AZIMUTH_BINS = 90


def horizontal_bound(capture, hits, sky, directions, axis, sign, faces):
    """How far along axis `sign` the ceiling reaches without a break, or
    None when the ceiling is open or unknown.

    Per azimuth bin, walking down from the zenith, the ceiling hits before
    the first texel that hits anything else: a room's ceiling runs to its
    walls whatever stands on the floor, while the next room's ceiling seen
    under a doorway's lintel is a separate run and does not count."""
    face = faces.get("pz", {"open": True})
    if face.get("open") or "plane_z" not in face:
        return None
    tolerance = max(FLOOR_TOLERANCE, 0.01 * face["extent"])
    ceiling = ~sky & (np.abs(hits[:, 2] - face["plane_z"]) <= tolerance)
    along = sign * (hits[:, axis] - capture[axis])
    azimuth = np.arctan2(directions[:, 1], directions[:, 0])
    bins = np.floor((azimuth + math.pi) / (2 * math.pi) * REACH_AZIMUTH_BINS).astype(np.int64)
    bins %= REACH_AZIMUTH_BINS
    order = np.lexsort((-directions[:, 2], bins))
    reach = None
    boundaries = np.searchsorted(bins[order], np.arange(REACH_AZIMUTH_BINS + 1))
    for b in range(REACH_AZIMUTH_BINS):
        run = order[boundaries[b]:boundaries[b + 1]]
        if not len(run):
            continue
        breaks = np.nonzero(~ceiling[run])[0]
        run = run[:breaks[0]] if len(breaks) else run
        if len(run):
            value = float(along[run].max())
            reach = value if reach is None else max(reach, value)
    if reach is None or reach <= 0:
        return None
    return reach * (1 + PLANE_BIN) + FLOOR_TOLERANCE


def fit_residual(capture, box_min, box_max, directions, distances, weights):
    """How far the scene lies from the box, seen from the capture point.

    Per hit texel, |hit distance - box distance| / box distance along its
    ray: 0 for a room the box matches, large for a box that cuts through or
    floats off the geometry. Reported solid-angle weighted (mean and 90th
    percentile) over hit texels; sky is reported, never scored.
    """
    capture = np.asarray(capture, dtype=np.float64)
    sky = ~np.isfinite(distances) | (distances >= SKY_DISTANCE)
    hits = ~sky
    origins = np.broadcast_to(capture, directions.shape)
    box_distance, _ = box_exit(origins, directions, np.asarray(box_min), np.asarray(box_max))
    relative = np.abs(distances[hits] - box_distance[hits]) / np.maximum(box_distance[hits],
                                                                        1e-9)
    w = weights[hits]
    order = np.argsort(relative)
    cumulative = np.cumsum(w[order]) / max(w.sum(), 1e-12)
    p90 = float(relative[order][min(np.searchsorted(cumulative, 0.9), len(order) - 1)]) \
        if len(order) else 0.0
    return {"mean_relative_residual": float(np.average(relative, weights=w)) if len(w) else 0.0,
            "p90_relative_residual": p90,
            "sky_fraction": float(weights[sky].sum() / weights.sum())}


def box_exit(positions, directions, box_min, box_max):
    """Distance along unit `directions` from `positions` to where each ray
    leaves the box, and whether it does so ahead of the position (a point
    outside the box whose ray misses it, or leaves behind it, is invalid)."""
    positions = np.asarray(positions, dtype=np.float64)
    directions = np.asarray(directions, dtype=np.float64)
    safe = np.where(np.abs(directions) < 1e-12, np.copysign(1e-12, directions), directions)
    first = (box_max - positions) / safe
    second = (box_min - positions) / safe
    far = np.minimum.reduce(np.maximum(first, second), axis=1)
    near = np.maximum.reduce(np.minimum(first, second), axis=1)
    return far, (far > 0) & (near <= far)


def parallax_lookup(positions, directions, capture, box_min, box_max):
    """Box-projected lookup direction for rays from shaded points.

    Returns (unit lookup directions, distance hit-to-shaded point, distance
    hit-to-capture point, valid). Invalid rays keep their own direction."""
    capture = np.asarray(capture, dtype=np.float64)
    far, valid = box_exit(positions, directions, box_min, box_max)
    hits = np.asarray(positions) + far[:, None] * directions
    local = hits - capture
    capture_distance = np.linalg.norm(local, axis=1)
    lookup = np.where(valid[:, None], local / np.maximum(capture_distance, 1e-12)[:, None],
                      directions)
    return lookup, np.where(valid, far, 0.0), np.where(valid, capture_distance, 0.0), valid


def distance_roughness(roughness, shaded_distance, capture_distance):
    """Frostbite 2014 Listing 25 on perceptual roughness: the lobe's footprint
    on the proxy seen from the capture point, clamped so a mirror stays a
    mirror and eased back to `roughness` as it grows."""
    roughness = np.asarray(roughness, dtype=np.float64)
    ratio = np.where(capture_distance > 0, shaded_distance / np.maximum(capture_distance, 1e-12),
                     1.0)
    sharpened = np.clip(ratio * roughness, 0.0, roughness)
    return sharpened + (roughness - sharpened) * roughness


def corrected_direction(lookup, reflected, roughness):
    """Frostbite 2014 Listing F.1 line 21: ease the parallax-corrected lookup
    back toward the reflected direction as roughness grows (the chain was
    prefiltered from the capture point, so a wide lobe's correction is wrong
    anyway). Both inputs are unit vectors here; F.1 lerps an unnormalized
    `localR`, which weights it by its length."""
    roughness = np.asarray(roughness, dtype=np.float64).reshape(-1, 1)
    mixed = lookup + (reflected - lookup) * roughness
    return mixed / np.maximum(np.linalg.norm(mixed, axis=1, keepdims=True), 1e-12)


def sample_cube_chain(mips, directions, roughness, channels=3):
    """The shader's split-sum probe fetch on a cube chain (mips of
    (6, N, N, C)): lod = roughness * (M - 1), trilinear between the two
    nearest mips, each bilinear and seamless."""
    roughness = np.broadcast_to(np.asarray(roughness, dtype=np.float64), (len(directions),))
    lod = np.clip(roughness, 0.0, 1.0) * (len(mips) - 1)
    lower = np.floor(lod).astype(np.int64)
    upper = np.minimum(lower + 1, len(mips) - 1)
    result = np.zeros((len(directions), channels))
    for level in range(len(mips)):
        sample = None
        for side, weight in ((lower, 1 - (lod - lower)), (upper, lod - lower)):
            chosen = (side == level) & (weight > 0)
            if chosen.any():
                if sample is None:
                    sample = sample_cube(mips[level], directions)[:, :channels]
                result[chosen] += sample[chosen] * weight[chosen, None]
    return result

