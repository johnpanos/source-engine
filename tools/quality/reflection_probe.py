"""Reflection probe layout shared by the Blender face renderer, the KTX2
packager and `world_pbr.frag`.

A probe is rendered as six 90-degree cube faces at one point (USD/Source Z-up
world), resampled to the pipeline's equirect convention (the same one as
`pbrt_scene.environment_equirect`: row 0 is the zenith, u = 0.5 - phi / 2pi
with phi = atan2(y, x)), and reduced to a GGX-prefiltered roughness mip chain:
mip i of M holds the scene convolved with the GGX lobe of perceptual roughness
i / (M - 1) (alpha = roughness^2) under the split-sum N = V = R assumption, by
filtered importance sampling of a box-reduced source pyramid. `world_pbr.frag`
selects lod = roughness * (M - 1), so a rough surface no longer mirrors small
bright emitters as hot spots the way the former blurred box mips did.

Legacy carrier: maps built before RPRB keep the chain in a band of rows
reserved at the top of the LMAP atlas (texel row 0 up, the rows the shader
fetches first). Mip i is W_i x W_i/2 texels at x = sum of earlier widths,
y = 0. Texel (atlas_width - 1, 0) is the marker: RGBA = (mip count, mip-0
width, band rows, -1). A real lightmap texel never has negative alpha, so the
shader enables probe specular only when it finds it. New maps carry their
probes in the RPRB lump instead (`reflection_probe_set.py`), with the same mip
layout per probe.

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
MARKER_ALPHA = -1.0


def face_basis(name):
    forward, up = (np.asarray(v, dtype=np.float64) for v in FACES[name])
    right = np.cross(forward, up)
    return forward, up, right


def equirect_directions(width):
    height = width // 2
    u = (np.arange(width) + 0.5) / width
    v = (np.arange(height) + 0.5) / height
    u, v = np.meshgrid(u, v)
    phi = (0.5 - u) * 2.0 * math.pi
    elevation = (0.5 - v) * math.pi
    return np.stack((np.cos(elevation) * np.cos(phi), np.cos(elevation) * np.sin(phi),
                     np.sin(elevation)), axis=-1)


def cube_to_equirect(faces, width, channels=3):
    """faces: name -> (N, N, C) image (C >= 3; 3 channels are kept unless
    `channels` says otherwise), row 0 at the face's `up` edge; nearest texel."""
    directions = equirect_directions(width).reshape(-1, 3)
    result = np.zeros((directions.shape[0], channels), dtype=np.float64)
    best = np.full(directions.shape[0], -np.inf)
    for name, image in faces.items():
        forward, up, right = face_basis(name)
        depth = directions @ forward
        chosen = depth > best
        best = np.where(chosen, depth, best)
        size = image.shape[0]
        safe = np.maximum(depth, 1e-9)
        x = (directions @ right) / safe
        y = (directions @ up) / safe
        column = np.clip(((x + 1) / 2 * size).astype(np.int64), 0, size - 1)
        row = np.clip(((1 - y) / 2 * size).astype(np.int64), 0, size - 1)
        result[chosen] = image[row[chosen], column[chosen], :channels]
    return result.reshape(width // 2, width, channels)


def box_pyramid(equirect, minimum_width):
    levels = [equirect]
    while levels[-1].shape[1] // 2 >= minimum_width:
        previous = levels[-1]
        height, width = previous.shape[0] // 2, previous.shape[1] // 2
        levels.append(previous[:height * 2, :width * 2].reshape(
            height, 2, width, 2, previous.shape[2]).mean(axis=(1, 3)))
    return levels


def sample_equirect(image, directions):
    """Bilinear lookup (longitude wraps, latitude clamps) of unit directions."""
    height, width = image.shape[:2]
    phi = np.arctan2(directions[:, 1], directions[:, 0])
    elevation = np.arcsin(np.clip(directions[:, 2], -1.0, 1.0))
    x = (0.5 - phi / (2 * math.pi)) * width - 0.5
    y = np.clip((0.5 - elevation / math.pi) * height - 0.5, 0.0, height - 1.0)
    x0 = np.floor(x).astype(np.int64)
    y0 = np.floor(y).astype(np.int64)
    fx, fy = (x - x0)[:, None], (y - y0)[:, None]
    x1, y1 = (x0 + 1) % width, np.minimum(y0 + 1, height - 1)
    x0 %= width
    top = image[y0, x0] * (1 - fx) + image[y0, x1] * fx
    bottom = image[y1, x0] * (1 - fx) + image[y1, x1] * fx
    return top * (1 - fy) + bottom * fy


def hammersley(count):
    bits = np.arange(count, dtype=np.uint32)
    bits = ((bits << 16) | (bits >> 16)) & 0xFFFFFFFF
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1)
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2)
    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4)
    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8)
    return np.arange(count) / count, bits.astype(np.float64) / 2.0 ** 32


def ggx_prefilter(pyramid, width, roughness, samples):
    """One mip: GGX(alpha = roughness^2) convolution with N = V = R."""
    directions = equirect_directions(width).reshape(-1, 3)
    alpha = roughness * roughness
    first, second = hammersley(samples)
    phi = 2 * math.pi * first
    cos_theta = np.sqrt((1 - second) / (1 + (alpha * alpha - 1) * second))
    sin_theta = np.sqrt(1 - cos_theta * cos_theta)
    # Sample half vectors in a local frame (z = N); L = 2 (V.H) H - V with V = N.
    local_h = np.stack((sin_theta * np.cos(phi), sin_theta * np.sin(phi), cos_theta), axis=1)
    local_l = 2 * cos_theta[:, None] * local_h - np.array((0.0, 0.0, 1.0))
    n_dot_l = local_l[:, 2]
    keep = n_dot_l > 0
    local_l, n_dot_l, cos_theta = local_l[keep], n_dot_l[keep], cos_theta[keep]
    # Filtered importance sampling: read a coarser source where a sample's
    # solid angle covers several source texels (pdf(L) = D(H) / 4 with V = N).
    d = alpha * alpha / (math.pi * ((cos_theta ** 2) * (alpha * alpha - 1) + 1) ** 2)
    sample_solid_angle = 1.0 / (len(first) * d / 4.0 + 1e-12)
    base = pyramid[0]
    texel_solid_angle = 4 * math.pi / (base.shape[0] * base.shape[1])
    lods = np.clip(0.5 * np.log2(sample_solid_angle / texel_solid_angle) + 1.0, 0,
                   len(pyramid) - 1)
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
        radiance = sample_equirect(pyramid[lower], world) * (1 - blend) + \
            sample_equirect(pyramid[upper], world) * blend
        total += radiance * n_dot_l[k]
        weight += n_dot_l[k]
    return (total / weight).reshape(width // 2, width, 3)


def mip_chain(equirect, minimum_width=4, samples=256):
    """GGX-prefiltered chain; mip i has perceptual roughness i / (count - 1)."""
    pyramid = box_pyramid(equirect, minimum_width)
    count = len(pyramid)
    mips = [equirect]
    for index in range(1, count):
        mips.append(ggx_prefilter(pyramid, pyramid[index].shape[1], index / (count - 1),
                                  samples))
    return mips


def write_band(atlas, mips):
    """Write the chain and marker into the atlas's top rows (in place)."""
    rows = mips[0].shape[0]
    width = atlas.shape[1]
    # Check before writing: numpy slicing would silently truncate a mip.
    if sum(mip.shape[1] for mip in mips) >= width or rows > atlas.shape[0]:
        raise ValueError("probe mip chain does not fit the atlas")
    x = 0
    for mip in mips:
        atlas[:mip.shape[0], x:x + mip.shape[1], :3] = mip
        atlas[:mip.shape[0], x:x + mip.shape[1], 3] = 1.0
        x += mip.shape[1]
    atlas[0, width - 1] = (len(mips), mips[0].shape[1], rows, MARKER_ALPHA)
    return {"mips": len(mips), "mip0_width": mips[0].shape[1], "band_rows": rows,
            "marker_texel": [width - 1, 0]}


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
    the face's `up` edge (the layout `cube_to_equirect` reads). Cycles' depth
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


def equirect_uv(directions):
    """world_pbr_probe.glsl's equirect coordinates of unit directions."""
    directions = np.asarray(directions, dtype=np.float64)
    return np.stack((0.5 - np.arctan2(directions[:, 1], directions[:, 0]) / (2 * math.pi),
                     0.5 - np.arcsin(np.clip(directions[:, 2], -1.0, 1.0)) / math.pi), axis=1)


def sample_level(mip, uv, channels=3):
    """Bilinear fetch as the shader does: texel coordinates clamped to
    [0.5, extent - 0.5] (no longitude wrap), then linear filtering."""
    height, width = mip.shape[:2]
    extent = np.array((width, height), dtype=np.float64)
    texel = np.clip(uv * extent, 0.5, extent - 0.5) - 0.5
    x0 = np.floor(texel[:, 0]).astype(np.int64)
    y0 = np.floor(texel[:, 1]).astype(np.int64)
    fx, fy = (texel[:, 0] - x0)[:, None], (texel[:, 1] - y0)[:, None]
    x1, y1 = np.minimum(x0 + 1, width - 1), np.minimum(y0 + 1, height - 1)
    c = slice(0, channels)
    top = mip[y0, x0, c] * (1 - fx) + mip[y0, x1, c] * fx
    bottom = mip[y1, x0, c] * (1 - fx) + mip[y1, x1, c] * fx
    return top * (1 - fy) + bottom * fy


def sample_chain(mips, directions, roughness, channels=3):
    """The shader's split-sum probe fetch: lod = roughness * (M - 1),
    interpolated between the two nearest mips."""
    roughness = np.broadcast_to(np.asarray(roughness, dtype=np.float64),
                                (len(directions),))
    uv = equirect_uv(directions)
    lod = np.clip(roughness, 0.0, 1.0) * (len(mips) - 1)
    lower = np.floor(lod).astype(np.int64)
    upper = np.minimum(lower + 1, len(mips) - 1)
    result = np.zeros((len(directions), channels))
    for level in range(len(mips)):
        sample = None
        for side, weight in ((lower, 1 - (lod - lower)), (upper, lod - lower)):
            chosen = side == level
            if chosen.any():
                sample = sample_level(mips[level], uv, channels) if sample is None else sample
                result[chosen] += sample[chosen] * weight[chosen, None]
    return result
