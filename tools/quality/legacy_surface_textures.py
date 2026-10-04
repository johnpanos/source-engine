"""Rasterize compiled surface blend weights and authored paint into textures.

The relight scene and its compiled PBR content consume the same result. UVs
remain in the original material's tangent frame; only a positive scale and
translation put the finite surface footprint into a texture. Layer transforms
change the lookup, not the normal vector (Source's bump-transform semantics).
"""

import math
from pathlib import Path

import numpy as np
from PIL import Image


IDENTITY = "center 0 0 scale 1 1 rotate 0 translate 0 0"


def transform(text=IDENTITY):
    tokens = text.lower().split()
    values, pos = {}, 0
    for name, count in (("center", 2), ("scale", 2), ("rotate", 1), ("translate", 2)):
        if pos >= len(tokens) or tokens[pos] != name:
            raise ValueError("invalid surface texture transform: " + text)
        pos += 1
        # Source accepts uniform scale as well as a two-component scale.
        end = pos + count
        if name == "scale" and pos + 1 < len(tokens) and tokens[pos + 1] == "rotate":
            end = pos + 1
        try:
            value = [float(v) for v in tokens[pos:end]]
        except ValueError as error:
            raise ValueError("invalid surface texture transform: " + text) from error
        if name == "scale" and len(value) == 1:
            value *= 2
        if len(value) != count or not np.isfinite(value).all():
            raise ValueError("invalid surface texture transform: " + text)
        values[name] = np.asarray(value)
        pos = end
    if pos != len(tokens):
        raise ValueError("trailing surface texture transform fields")
    angle = math.radians(values["rotate"][0])
    c, s = math.cos(angle), math.sin(angle)
    matrix = np.array(((c, -s), (s, c))) @ np.diag(values["scale"])
    return matrix, values["center"] + values["translate"] - matrix @ values["center"]


def sample(image, uv, matrix=None, offset=None):
    """Repeat/bilinear lookup in Source's top-left texture coordinates."""
    if matrix is not None:
        uv = uv @ matrix.T + offset
    height, width = image.shape[:2]
    p = uv * (width, height) - 0.5
    lo = np.floor(p).astype(np.int64)
    f = p - lo
    x, y = lo[..., 0] % width, lo[..., 1] % height
    fx, fy = f[..., 0, None], f[..., 1, None]
    return ((image[y, x] * (1 - fx) + image[y, (x + 1) % width] * fx) * (1 - fy) +
            (image[(y + 1) % height, x] * (1 - fx) +
             image[(y + 1) % height, (x + 1) % width] * fx) * fy)


def srgb_decode(value):
    return np.where(value <= 0.04045, value / 12.92, ((value + 0.055) / 1.055) ** 2.4)


def srgb_encode(value):
    value = np.clip(value, 0, 1)
    return np.where(value <= 0.0031308, 12.92 * value, 1.055 * value ** (1 / 2.4) - 0.055)


def rasterize(uv, triangles, values, lo, span, size):
    """Piecewise-linear vertex values at texel centers, on the actual triangles."""
    width, height = size
    values = np.asarray(values)
    result = np.zeros((height, width) + values.shape[1:], dtype=np.float64)
    covered = np.zeros((height, width), dtype=bool)
    pixel = (uv - lo) / span * size - 0.5
    for ids in triangles:
        p = pixel[ids]
        first = np.maximum(np.ceil(p.min(axis=0)).astype(int), 0)
        last = np.minimum(np.floor(p.max(axis=0)).astype(int) + 1, size)
        if (last <= first).any():
            continue
        basis = np.column_stack((p[1] - p[0], p[2] - p[0]))
        if abs(np.linalg.det(basis)) < 1e-12:
            raise ValueError("surface has a degenerate texture triangle")
        x, y = np.meshgrid(np.arange(first[0], last[0]), np.arange(first[1], last[1]))
        ab = (np.stack((x, y), axis=-1) - p[0]) @ np.linalg.inv(basis).T
        bary = np.stack((1 - ab[..., 0] - ab[..., 1], ab[..., 0], ab[..., 1]), axis=-1)
        inside = (bary >= -1e-8).all(axis=-1)
        region = np.s_[first[1]:last[1], first[0]:last[0]]
        result[region][inside] = np.tensordot(bary[inside], values[ids], axes=1)
        covered[region] |= inside
    return result, covered


def modulation(weight, channels):
    """WorldVertexTransition's height/width smoothstep, including zero width."""
    low = np.clip(channels[..., 1] - channels[..., 0], 0, 1)
    high = np.clip(channels[..., 1] + channels[..., 0], 0, 1)
    t = np.clip((weight - low) / np.maximum(high - low, 1e-30), 0, 1)
    return np.where(high > low, t * t * (3 - 2 * t), weight >= high)


def compose(record, uv, triangles, alpha, out, name, layers):
    """Material tile at at least every input texture's authored texel density.

    layers maps a channel to (decoded float image, native UV matrix, offset).
    The second normal/AO/base layer and modulation are optional. A footprint
    exceeding the texture limit fails explicitly; it is never downsampled.
    """
    lo, hi = uv.min(axis=0), uv.max(axis=0)
    span = hi - lo
    if (span <= 0).any() or not np.isfinite(uv).all():
        raise ValueError("invalid surface UV footprint")
    density = np.ones(2)
    for image, matrix, _offset in layers.values():
        density = np.maximum(density, np.linalg.norm(
            matrix * np.asarray((image.shape[1], image.shape[0]))[:, None], axis=0))
    size = tuple(1 << max(0, math.ceil(math.log2(max(1, v)))) for v in span * density)
    if max(size) > 16384:
        raise ValueError("surface texture footprint exceeds 16384 texels: " + name)
    width, height = size
    weights, _covered = rasterize(uv, triangles, alpha, lo, span, size)
    channels = {key: np.empty((height, width, count), dtype=np.uint8)
                for key, count in (("base", 4), ("normal", 3), ("occlusion", 1))
                if key in layers or key == "base"}
    # Bound working allocations even for large tiled surfaces.
    for row in range(0, height, 32):
        stop = min(row + 32, height)
        x, y = np.meshgrid((np.arange(width) + 0.5) / width,
                           (np.arange(row, stop) + 0.5) / height)
        coords = lo + np.stack((x, y), axis=-1) * span
        samples = {key: sample(image, coords, matrix, offset)
                   for key, (image, matrix, offset) in layers.items()}
        weight = weights[row:stop]
        if "modulation" in samples:
            weight = modulation(weight, samples["modulation"])
        weight = weight[..., None]
        for key, target in channels.items():
            first = samples.get(key)
            if first is None:
                first = np.broadcast_to((*record.get("base_color", (0.5,) * 3), 1),
                                        (stop - row, width, 4))
            second = samples.get(key + "2", first)
            value = first * (1 - weight) + second * weight
            if key == "base":
                value = value.copy()
                value[..., :3] = srgb_encode(value[..., :3])
            elif key == "normal":
                value /= np.maximum(np.linalg.norm(value, axis=-1, keepdims=True), 1e-30)
                value = value * 0.5 + 0.5
            target[row:stop] = np.clip(np.rint(value * 255), 0, 255).astype(np.uint8)
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    for key, pixels in channels.items():
        path = out / (name + "_" + key + ".png")
        Image.fromarray(pixels[..., 0] if pixels.shape[-1] == 1 else pixels).save(path)
        record[key] = {"file": str(path), "source": "compiled surface layers",
                       "size": list(size), "alpha": bool(key == "base" and
                                                         (pixels[..., 3] < 255).any()),
                       "wrap": "clamp"}
    # The transforms have already been applied to the tile.
    record["params"] = dict(record["params"])
    record["params"].pop("$basetexturetransform", None)
    return (uv - lo) / span, {"size": list(size), "uv_min": lo.tolist(),
                              "uv_span": span.tolist(), "layers": sorted(layers)}
