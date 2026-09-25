#!/usr/bin/env python3
"""Decode a Valve Texture Format (VTF 7.0-7.5) image to RGBA8 in Python.

Frame 0 (the first frame), face 0, slice 0 of one mip level, as an (H, W, 4)
uint8 array in the file's stored colour encoding (sRGB for colour textures;
the caller decides). Formats: RGBA8888, ABGR8888, RGB888, BGR888, RGB565,
I8, IA88, A8, ARGB8888, BGRA8888, BGRX8888, BGR565, BGRA4444, BGRA5551,
BGRX5551, DXT1 (+ one-bit alpha), DXT3, DXT5, UV88 and the two bluescreen
variants. Anything else (volume textures, float formats) raises ValueError.

    python3 tools/quality/vtf_decode.py <file.vtf> <out.png>
"""

import argparse
import struct
import sys
from pathlib import Path

import numpy as np

# (bytes per pixel, or block bytes for DXT) by image format id.
FORMATS = {0: ("RGBA8888", 4), 1: ("ABGR8888", 4), 2: ("RGB888", 3), 3: ("BGR888", 3),
           4: ("RGB565", 2), 5: ("I8", 1), 6: ("IA88", 2), 8: ("A8", 1),
           9: ("RGB888_BLUESCREEN", 3), 10: ("BGR888_BLUESCREEN", 3), 11: ("ARGB8888", 4),
           12: ("BGRA8888", 4), 13: ("DXT1", 8), 14: ("DXT3", 16), 15: ("DXT5", 16),
           16: ("BGRX8888", 4), 17: ("BGR565", 2), 18: ("BGRX5551", 2), 19: ("BGRA4444", 2),
           20: ("DXT1_ONEBITALPHA", 8), 21: ("BGRA5551", 2), 22: ("UV88", 2)}
DXT = {13, 14, 15, 20}
TEXTUREFLAGS_ENVMAP = 0x4000
HIGH_RES_RESOURCE = 0x30


def header(data):
    if len(data) < 64 or data[:4] != b"VTF\0":
        raise ValueError("not a VTF file")
    major, minor, size = struct.unpack_from("<3I", data, 4)
    if major != 7 or minor > 5:
        raise ValueError("unsupported VTF version %d.%d" % (major, minor))
    width, height, flags, frames, first_frame = struct.unpack_from("<HHIHH", data, 16)
    reflectivity = struct.unpack_from("<3f", data, 32)
    fmt, mips, low_fmt, low_w, low_h = struct.unpack_from("<iBiBB", data, 52)
    depth = struct.unpack_from("<H", data, 63)[0] if minor >= 2 else 1
    info = {"version": (major, minor), "header_size": size, "width": width, "height": height,
            "flags": flags, "frames": max(frames, 1), "first_frame": first_frame,
            "reflectivity": reflectivity, "format": fmt, "mips": mips,
            "low_format": low_fmt, "low_size": (low_w, low_h), "depth": max(depth, 1)}
    if minor >= 3:
        count = struct.unpack_from("<I", data, 68)[0]
        resources = {}
        for index in range(count):
            tag, offset = struct.unpack_from("<II", data, 80 + 8 * index)
            resources[tag & 0xFFFFFF] = offset
        info["resources"] = resources
    return info


def mip_bytes(fmt, width, height):
    name, size = FORMATS[fmt]
    if fmt in DXT:
        return max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * size
    return width * height * size


def decode(data, mip=0):
    info = header(data)
    if info["format"] not in FORMATS:
        raise ValueError("unsupported VTF image format %d" % info["format"])
    if info["depth"] != 1:
        raise ValueError("volume VTFs are unsupported")
    if not 0 <= mip < info["mips"]:
        raise ValueError("VTF has no mip %d" % mip)
    faces = 1
    if info["flags"] & TEXTUREFLAGS_ENVMAP:
        # 7.0-7.4 store a sphere map as a seventh face (7.5 dropped it).
        faces = 7 if info["version"][1] < 5 and info["first_frame"] != 0xFFFF else 6
    fmt = info["format"]
    if "resources" in info:
        if HIGH_RES_RESOURCE not in info["resources"]:
            raise ValueError("VTF has no high-resolution image resource")
        offset = info["resources"][HIGH_RES_RESOURCE]
    else:
        low_w, low_h = info["low_size"]
        offset = info["header_size"] + (mip_bytes(info["low_format"], low_w, low_h)
                                        if info["low_format"] in FORMATS and low_w else 0)
    # Mips are stored smallest first; each holds frames x faces images.
    for level in range(info["mips"] - 1, mip, -1):
        w, h = max(1, info["width"] >> level), max(1, info["height"] >> level)
        offset += mip_bytes(fmt, w, h) * info["frames"] * faces
    width, height = max(1, info["width"] >> mip), max(1, info["height"] >> mip)
    size = mip_bytes(fmt, width, height)
    if offset + size > len(data):
        raise ValueError("VTF image data is truncated")
    return pixels(fmt, data[offset:offset + size], width, height), info


def unpack565(values):
    r = ((values >> 11) & 31).astype(np.float64) * 255 / 31
    g = ((values >> 5) & 63).astype(np.float64) * 255 / 63
    b = (values & 31).astype(np.float64) * 255 / 31
    return r, g, b


def pixels(fmt, raw, width, height):
    name = FORMATS[fmt][0]
    if fmt in DXT:
        return dxt(fmt, raw, width, height)
    buffer = np.frombuffer(raw, np.uint8)
    out = np.full((height, width, 4), 255, np.uint8)
    if name in ("RGBA8888", "ABGR8888", "ARGB8888", "BGRA8888", "BGRX8888"):
        v = buffer.reshape(height, width, 4)
        order = {"RGBA8888": (0, 1, 2, 3), "ABGR8888": (3, 2, 1, 0),
                 "ARGB8888": (1, 2, 3, 0), "BGRA8888": (2, 1, 0, 3),
                 "BGRX8888": (2, 1, 0, None)}[name]
        for channel, index in enumerate(order):
            if index is not None:
                out[..., channel] = v[..., index]
    elif name in ("RGB888", "BGR888", "RGB888_BLUESCREEN", "BGR888_BLUESCREEN"):
        v = buffer.reshape(height, width, 3)
        out[..., :3] = v[..., ::-1] if name.startswith("BGR") else v
        if name.endswith("BLUESCREEN"):
            out[..., 3] = np.where((out[..., 0] == 0) & (out[..., 1] == 0) &
                                   (out[..., 2] == 255), 0, 255)
    elif name in ("I8", "A8"):
        v = buffer.reshape(height, width)
        if name == "I8":
            out[..., 0] = out[..., 1] = out[..., 2] = v
        else:
            out[..., :3] = 0
            out[..., 3] = v
    elif name == "IA88":
        v = buffer.reshape(height, width, 2)
        out[..., 0] = out[..., 1] = out[..., 2] = v[..., 0]
        out[..., 3] = v[..., 1]
    elif name == "UV88":
        v = buffer.reshape(height, width, 2)
        out[..., 0], out[..., 1], out[..., 2] = v[..., 0], v[..., 1], 0
    else:
        v = np.frombuffer(raw, "<u2").reshape(height, width)
        if name in ("RGB565", "BGR565"):
            r, g, b = unpack565(v)
            if name == "RGB565":
                # Source's RGB565 stores red in the low bits (as BGR565 does blue).
                r, b = b, r
            out[..., 0], out[..., 1], out[..., 2] = r, g, b
        elif name == "BGRA4444":
            out[..., 0] = ((v >> 8) & 15) * 17
            out[..., 1] = ((v >> 4) & 15) * 17
            out[..., 2] = (v & 15) * 17
            out[..., 3] = ((v >> 12) & 15) * 17
        else:  # BGRA5551, BGRX5551
            out[..., 0] = np.round(((v >> 10) & 31) * 255 / 31)
            out[..., 1] = np.round(((v >> 5) & 31) * 255 / 31)
            out[..., 2] = np.round((v & 31) * 255 / 31)
            if name == "BGRA5551":
                out[..., 3] = np.where(v & 0x8000, 255, 0)
    return out


def dxt_colour(blocks, one_bit_alpha):
    """Colour endpoints and 2-bit indices of DXT colour blocks -> (N, 16, 4)."""
    c0 = blocks[:, 0].astype(np.uint16) | (blocks[:, 1].astype(np.uint16) << 8)
    c1 = blocks[:, 2].astype(np.uint16) | (blocks[:, 3].astype(np.uint16) << 8)
    r0, g0, b0 = unpack565(c0)
    r1, g1, b1 = unpack565(c1)
    e0 = np.stack([r0, g0, b0], -1)
    e1 = np.stack([r1, g1, b1], -1)
    four = (c0 > c1)[:, None]
    palette = np.stack([e0, e1,
                        np.where(four, (2 * e0 + e1) / 3, (e0 + e1) / 2),
                        np.where(four, (e0 + 2 * e1) / 3, 0.0)], 1)
    alpha = np.full((len(blocks), 4), 255.0)
    if one_bit_alpha:
        alpha[:, 3] = np.where(four[:, 0], 255.0, 0.0)
    bits = blocks[:, 4:8].astype(np.uint32)
    word = bits[:, 0] | (bits[:, 1] << 8) | (bits[:, 2] << 16) | (bits[:, 3] << 24)
    index = (word[:, None] >> (2 * np.arange(16, dtype=np.uint32))) & 3
    rgb = np.take_along_axis(palette, index[..., None].astype(np.int64), 1)
    a = np.take_along_axis(alpha, index.astype(np.int64), 1)
    return np.concatenate([rgb, a[..., None]], -1)


def dxt(fmt, raw, width, height):
    bw, bh = max(1, (width + 3) // 4), max(1, (height + 3) // 4)
    blocks = np.frombuffer(raw, np.uint8).reshape(bw * bh, FORMATS[fmt][1])
    if fmt in (13, 20):
        texels = dxt_colour(blocks, fmt == 20)
    else:
        texels = dxt_colour(blocks[:, 8:], False)
        if fmt == 14:
            alpha_bits = blocks[:, :8]
            low = alpha_bits & 15
            high = alpha_bits >> 4
            texels[..., 3] = np.stack([low, high], -1).reshape(-1, 16) * 17
        else:
            a0 = blocks[:, 0].astype(np.float64)
            a1 = blocks[:, 1].astype(np.float64)
            steps = np.arange(8)
            eight = (a0 > a1)[:, None]
            table = np.where(
                eight,
                np.where(steps == 0, a0[:, None], np.where(steps == 1, a1[:, None],
                         ((8 - steps) * a0[:, None] + (steps - 1) * a1[:, None]) / 7)),
                np.where(steps == 0, a0[:, None], np.where(steps == 1, a1[:, None],
                         np.where(steps == 6, 0.0, np.where(steps == 7, 255.0,
                                  ((6 - steps) * a0[:, None] + (steps - 1) * a1[:, None]) /
                                  5)))))
            bits = blocks[:, 2:8].astype(np.uint64)
            word = np.zeros(len(blocks), np.uint64)
            for i in range(6):
                word |= bits[:, i] << np.uint64(8 * i)
            index = (word[:, None] >> (np.uint64(3) * np.arange(16, dtype=np.uint64))) & \
                np.uint64(7)
            texels[..., 3] = np.take_along_axis(table, index.astype(np.int64), 1)
    image = texels.reshape(bh, bw, 4, 4, 4).transpose(0, 2, 1, 3, 4).reshape(bh * 4, bw * 4, 4)
    return np.clip(np.round(image[:height, :width]), 0, 255).astype(np.uint8)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("vtf", type=Path)
    parser.add_argument("out", type=Path)
    parser.add_argument("--mip", type=int, default=0)
    args = parser.parse_args()
    from PIL import Image
    image, info = decode(args.vtf.read_bytes(), args.mip)
    Image.fromarray(image, "RGBA").save(args.out)
    print("%s %dx%d format %s" % (args.out, image.shape[1], image.shape[0],
                                  FORMATS[info["format"]][0]))


if __name__ == "__main__":
    sys.exit(main())
