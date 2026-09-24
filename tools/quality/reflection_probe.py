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

The chain lives in a band of rows reserved at the top of the LMAP atlas (texel
row 0 up, the rows the shader fetches first). Mip i is W_i x W_i/2 texels at
x = sum of earlier widths, y = 0. Texel (atlas_width - 1, 0) is the marker:
RGBA = (mip count, mip-0 width, band rows, -1). A real lightmap texel never has
negative alpha, so the shader enables probe specular only when it finds it.
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


def cube_to_equirect(faces, width):
    """faces: name -> (N, N, 3) linear image, row 0 at the face's `up` edge."""
    directions = equirect_directions(width).reshape(-1, 3)
    result = np.zeros((directions.shape[0], 3), dtype=np.float64)
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
        result[chosen] = image[row[chosen], column[chosen], :3]
    return result.reshape(width // 2, width, 3)


def box_pyramid(equirect, minimum_width):
    levels = [equirect]
    while levels[-1].shape[1] // 2 >= minimum_width:
        previous = levels[-1]
        height, width = previous.shape[0] // 2, previous.shape[1] // 2
        levels.append(previous[:height * 2, :width * 2].reshape(
            height, 2, width, 2, 3).mean(axis=(1, 3)))
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
