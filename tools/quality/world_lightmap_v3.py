#!/usr/bin/env python3
"""LMAP v3: the world lightmap as block-compressed pages (RFC 0008 F3/F4).

The one lightmap version (2026-10-05). Every layer is directional: a page of
irradiance (diffuse light, irradiance / pi) and, at the same texels, the
world-space luminance gradient beta of E(n) = a + g . n relative to it
(lightmap_directional.py). public/mapcontainer/world_lightmap.h mirrors this.

Little-endian, 64-byte header:

   0 u32 magic "LMP3"            4 u32 version (3)
   8 u32 page width              12 u32 page height (the two pages share it)
  16 u32 layer count (1..3: total, [direct,] indirect, as v2's roles)
  20 u32 flags (bit 0: layer 0's gradient alpha is the sun's visibility)
  24 u32 irradiance VkFormat (143, BC6H_UFLOAT_BLOCK)
  28 u32 gradient VkFormat (145, BC7_UNORM_BLOCK)
  32 u64 irradiance bytes per layer   40 u64 gradient bytes per layer
  48 u64 data offset (64)             56 u64 0

then per layer: its irradiance blocks, then its gradient blocks (4x4 blocks,
row-major, partial blocks padded; one mip level). The gradient's RGB is
beta * 0.5 + 0.5 with beta clamped to [-1, 1] (|beta| <= 1 keeps the
irradiance non-negative; the receipt reports the clamped share), its alpha the
sun's [0, 1] visibility on layer 0 with the flag, else 1.

Unlike v1/v2 there is no probe band or marker texel: reflection probes are the
RPRB lump and the sun is the map's light environment.
"""

import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "texture"))
import bc_codec  # noqa: E402

MAGIC = 0x33504D4C  # "LMP3"
VERSION = 3
HEADER_BYTES = 64
FLAG_SUN = 1
MAX_LAYERS = 3
MAX_DIMENSION = 16384
IRRADIANCE_VKFORMAT = bc_codec.VK_FORMAT["bc6hu"]
GRADIENT_VKFORMAT = bc_codec.VK_FORMAT["bc7"]
ROLES = {1: ["total"], 2: ["total", "indirect"], 3: ["total", "direct", "indirect"]}


class LightmapError(Exception):
    def __init__(self, code, detail=""):
        super().__init__("%s%s" % (code, ": " + detail if detail else ""))
        self.code = code


def gradient_texels(beta, sun=None):
    """uint8 RGBA of the gradient page: beta biased into [0, 1], alpha the sun."""
    beta = np.asarray(beta, np.float64)
    clamped = float(np.mean(np.abs(beta) > 1.0))
    rgba = np.empty(beta.shape[:2] + (4,), np.uint8)
    rgba[..., :3] = np.round((np.clip(beta, -1.0, 1.0) * 0.5 + 0.5) * 255.0)
    rgba[..., 3] = 255 if sun is None else np.round(np.clip(sun, 0.0, 1.0) * 255.0)
    return rgba, clamped


def build(tool, layers, sun=None):
    """LMAP v3 bytes and a quality report.

    `layers`: [(irradiance (h, w, 3) float, beta (h, w, 3) float), ...] in role
    order; `sun`: layer 0's visibility (h, w) in [0, 1], or None."""
    if not 1 <= len(layers) <= MAX_LAYERS:
        raise LightmapError("InvalidLayerCount")
    height, width = np.asarray(layers[0][0]).shape[:2]
    if not (1 <= width <= MAX_DIMENSION and 1 <= height <= MAX_DIMENSION):
        raise LightmapError("InvalidDescriptor", "page %dx%d" % (width, height))
    irradiance_bytes = bc_codec.block_bytes("bc6hu", width, height)
    gradient_bytes = bc_codec.block_bytes("bc7", width, height)
    header = struct.pack("<8I3Q8x", MAGIC, VERSION, width, height, len(layers),
                         FLAG_SUN if sun is not None else 0, IRRADIANCE_VKFORMAT,
                         GRADIENT_VKFORMAT, irradiance_bytes, gradient_bytes, HEADER_BYTES)
    body, report = bytearray(), {"layers": []}
    for index, (irradiance, beta) in enumerate(layers):
        irradiance = np.asarray(irradiance, np.float32)
        beta = np.asarray(beta, np.float32)
        if irradiance.shape != (height, width, 3) or beta.shape != (height, width, 3):
            raise LightmapError("InvalidDescriptor", "layer %d has different pages" % index)
        texels, clamped = gradient_texels(beta, sun if index == 0 else None)
        flat_blocks = bc_codec.encode_bc6h(tool, irradiance)
        gradient_blocks = bc_codec.encode_bc7(tool, texels)
        body += flat_blocks + gradient_blocks
        decoded = bc_codec.decode_bc7(gradient_blocks, width, height)
        beta_back = decoded[..., :3].astype(np.float32) / 255.0 * 2.0 - 1.0
        entry = {"role": ROLES[len(layers)][index],
                 "irradiance": bc_codec.hdr_error(
                     irradiance, bc_codec.decode_bc6h(flat_blocks, width, height)),
                 "beta_clamped_share": clamped,
                 "beta_abs_error_mean": float(np.abs(beta_back - np.clip(beta, -1, 1)).mean()),
                 "beta_abs_error_max": float(np.abs(beta_back - np.clip(beta, -1, 1)).max())}
        if index == 0 and sun is not None:
            sun_back = decoded[..., 3].astype(np.float32) / 255.0
            entry["sun_abs_error_mean"] = float(np.abs(sun_back - np.clip(sun, 0, 1)).mean())
            entry["sun_abs_error_max"] = float(np.abs(sun_back - np.clip(sun, 0, 1)).max())
        report["layers"].append(entry)
    data = header + bytes(body)
    read(data)  # the writer never emits what the reader rejects
    report.update(width=width, height=height, bytes=len(data),
                  rgba16f_bytes=len(layers) * width * height * 2 * 8)
    return data, report


def read(data):
    """Validate LMAP v3 bytes; the layout (offsets into `data`)."""
    if len(data) < HEADER_BYTES:
        raise LightmapError("Truncated")
    (magic, version, width, height, count, flags, flat_format, gradient_format, flat_bytes,
     gradient_bytes, offset) = struct.unpack_from("<8I3Q", data)
    if magic != MAGIC:
        raise LightmapError("BadIdentifier")
    if version != VERSION or flags & ~FLAG_SUN or any(data[56:64]):
        raise LightmapError("UnsupportedVersion")
    if (flat_format, gradient_format) != (IRRADIANCE_VKFORMAT, GRADIENT_VKFORMAT):
        raise LightmapError("UnsupportedFormat")
    if not 1 <= count <= MAX_LAYERS:
        raise LightmapError("InvalidLayerCount")
    if not (1 <= width <= MAX_DIMENSION and 1 <= height <= MAX_DIMENSION) or \
            flat_bytes != bc_codec.block_bytes("bc6hu", width, height) or \
            gradient_bytes != bc_codec.block_bytes("bc7", width, height) or offset != HEADER_BYTES:
        raise LightmapError("InvalidDescriptor")
    if len(data) != offset + count * (flat_bytes + gradient_bytes):
        raise LightmapError("InvalidLevelIndex", "size %d" % len(data))
    layers = []
    for index in range(count):
        start = offset + index * (flat_bytes + gradient_bytes)
        layers.append({"role": ROLES[count][index], "irradiance_offset": start,
                       "gradient_offset": start + flat_bytes})
    return {"width": width, "height": height, "sun": bool(flags & FLAG_SUN),
            "irradiance_bytes": flat_bytes, "gradient_bytes": gradient_bytes, "layers": layers}


def decode_layer(data, layout, index):
    """(irradiance, beta, sun or None) of one layer, decoded on the CPU."""
    layer = layout["layers"][index]
    w, h = layout["width"], layout["height"]
    flat = bc_codec.decode_bc6h(
        data[layer["irradiance_offset"]:layer["irradiance_offset"] + layout["irradiance_bytes"]],
        w, h)
    gradient = bc_codec.decode_bc7(
        data[layer["gradient_offset"]:layer["gradient_offset"] + layout["gradient_bytes"]], w, h)
    beta = gradient[..., :3].astype(np.float32) / 255.0 * 2.0 - 1.0
    sun = gradient[..., 3].astype(np.float32) / 255.0 if index == 0 and layout["sun"] else None
    return flat, beta, sun
