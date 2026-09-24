"""Reflection probe layout shared by the Blender face renderer, the KTX2
packager and `world_pbr.frag`.

A probe is rendered as six 90-degree cube faces at one point (USD/Source Z-up
world), resampled to the pipeline's equirect convention (the same one as
`pbrt_scene.environment_equirect`: row 0 is the zenith, u = 0.5 - phi / 2pi
with phi = atan2(y, x)), and reduced to a roughness mip chain by repeated 2x
box filtering plus a small blur ("blurry mips"; not a GGX convolution).

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


def mip_chain(equirect, minimum_width=4):
    from scipy.ndimage import uniform_filter
    mips = [equirect]
    while mips[-1].shape[1] // 2 >= minimum_width:
        previous = mips[-1]
        height, width = previous.shape[0] // 2, previous.shape[1] // 2
        reduced = previous[:height * 2, :width * 2].reshape(height, 2, width, 2, 3).mean(axis=(1, 3))
        # Wrap horizontally (longitude) and clamp at the poles.
        mips.append(uniform_filter(reduced, size=(3, 3, 1), mode=("nearest", "wrap", "nearest")))
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
