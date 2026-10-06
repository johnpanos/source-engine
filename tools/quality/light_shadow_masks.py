#!/usr/bin/env python3
"""LSMK v1: baked shadow masks of static lights (RFC 0016 runtime direct light).

Source 2's split: a static light's shadow from static geometry never
changes, so the bake stores it per lightmap texel and the runtime samples it
in place of a shadow map. Each texel holds the visibility of its four
dominant lights (the most unshadowed diffuse light) and their ids; a light
that is not among a texel's four keeps its runtime shadow there.

Lights are baked in groups whose reaches (where the diffuse light falls to
REACH_CUTOFF lightmap units, the runtime's reach) are disjoint, so a group
names at most one light at any texel and a light's id is its group's.
public/mapcontainer/light_shadow_masks.h mirrors this encoding.

Little-endian, 64-byte header:

   0 u32 magic "LSMK"            4 u32 version (1)
   8 u32 page width              12 u32 page height (the LMAP page's)
  16 u32 light count N           20 u32 page VkFormat (91, R16G16B16A16_UNORM)
  24 u64 page bytes (width * height * 8)      32 u64 data offset (64 + 16 N)
  40 24 zero bytes

then N records {f32 x, y, z: the light's origin in Source units, u32 id
1..255}, then the page's texels, top row first like LMAP's pages: channel k
of a texel is id << 8 | round(visibility * 255), id 0 for none (exact, so not
block-compressed: a texel's ids change order between neighbours). A
runtime light takes the record within MATCH_UNITS of its origin (the
tolerance render.pass.lights' MergeMapLights matches lamps by): a relit
map's lights reach the core from its entities, not its worldlights lump.

    python3 tools/quality/light_shadow_masks.py info MASKS.lsmk
"""

import argparse
import json
import struct
import sys
from pathlib import Path

import numpy as np


MAGIC = 0x4B4D534C  # "LSMK"
VERSION = 1
HEADER_BYTES = 64
CHANNELS = 4
MAX_IDS = 255
MAX_LIGHTS = 4096
MAX_DIMENSION = 16384
MATCH_UNITS = 0.1
# The runtime's reach: where a light's diffuse light falls to this many
# lightmap units (render/pass/lights/map_lights.cpp kReachCutoff).
REACH_CUTOFF = 1e-3
PAGE_VKFORMAT = 91  # VK_FORMAT_R16G16B16A16_UNORM


class MaskError(Exception):
    pass


def group_lights(lights):
    """[id 1..] for lights [{origin, radius}] (Source units): greedy colouring,
    largest reach first, so lights of one id have disjoint reaches. A light
    without a reach (unbounded) gets None and keeps its runtime shadow."""
    order = sorted((index for index, light in enumerate(lights) if light["radius"] > 0),
                   key=lambda index: (-lights[index]["radius"], index))
    ids = [None] * len(lights)
    members = []  # per id: [(origin, radius)]
    for index in order:
        centre = np.asarray(lights[index]["origin"], np.float64)
        radius = lights[index]["radius"]
        for group, placed in enumerate(members):
            if all(np.linalg.norm(centre - other) >= radius + other_radius
                   for other, other_radius in placed):
                break
        else:
            if len(members) == MAX_IDS:
                continue
            members.append([])
            group = len(members) - 1
        members[group].append((centre, radius))
        ids[index] = group + 1
    return ids


class TopFour:
    """Running per-texel selection of the four dominant groups."""

    def __init__(self, height, width):
        self.light = np.zeros((height, width, CHANNELS), np.float64)
        self.visibility = np.ones((height, width, CHANNELS), np.float64)
        self.ids = np.zeros((height, width, CHANNELS), np.uint8)

    def add(self, group_id, unshadowed, visibility):
        """A group's unshadowed diffuse light and visibility, (h, w) each."""
        light = np.concatenate([self.light, unshadowed[..., None]], axis=2)
        vis = np.concatenate([self.visibility, visibility[..., None]], axis=2)
        ids = np.concatenate([self.ids, np.full(unshadowed.shape + (1,), group_id, np.uint8)],
                             axis=2)
        ids[..., CHANNELS][unshadowed <= 0] = 0
        order = np.argsort(-light, axis=2, kind="stable")[..., :CHANNELS]
        self.light = np.take_along_axis(light, order, axis=2)
        self.visibility = np.take_along_axis(vis, order, axis=2)
        self.ids = np.take_along_axis(ids, order, axis=2)
        self.ids[self.light <= 0] = 0
        self.visibility[self.ids == 0] = 1.0


def encode_page(visibility, ids):
    """uint16 (h, w, 4) texels: id << 8 | visibility in 1/255 steps."""
    vis = np.round(np.clip(visibility, 0.0, 1.0) * 255.0).astype(np.uint16)
    return (np.asarray(ids, np.uint16) << 8) | np.where(np.asarray(ids) > 0, vis, 255)


def build(records, visibility, ids):
    """LSMK bytes and a report. `records`: [(origin (3,), id)]; `visibility`
    (h, w, 4) float and `ids` (h, w, 4) uint8, top row first."""
    visibility = np.asarray(visibility, np.float64)
    ids = np.asarray(ids)
    height, width = visibility.shape[:2]
    if visibility.shape != (height, width, CHANNELS) or ids.shape != visibility.shape or \
            not np.isfinite(visibility).all():
        raise MaskError("visibility and ids must be (h, w, 4), finite")
    if not (1 <= width <= MAX_DIMENSION and 1 <= height <= MAX_DIMENSION):
        raise MaskError("page %dx%d" % (width, height))
    if not 1 <= len(records) <= MAX_LIGHTS or \
            any(not 1 <= light_id <= MAX_IDS for _, light_id in records):
        raise MaskError("1..%d lights with ids 1..%d" % (MAX_LIGHTS, MAX_IDS))
    page = encode_page(visibility, ids).astype("<u2").tobytes()
    offset = HEADER_BYTES + 16 * len(records)
    header = struct.pack("<6I2Q24x", MAGIC, VERSION, width, height, len(records),
                         PAGE_VKFORMAT, len(page), offset)
    table = b"".join(struct.pack("<3fI", *(float(v) for v in origin), light_id)
                     for origin, light_id in records)
    data = header + table + page
    read(data)  # the writer never emits what the reader rejects
    used = ids > 0
    return data, {"width": width, "height": height, "bytes": len(data),
                  "lights": len(records),
                  "texels_by_count": [int(v) for v in
                                      np.bincount(used.sum(axis=2).ravel(), minlength=5)]}


def read(data):
    if len(data) < HEADER_BYTES:
        raise MaskError("Truncated")
    magic, version, width, height, count, vkformat, page_bytes, offset = \
        struct.unpack_from("<6I2Q", data)
    if magic != MAGIC:
        raise MaskError("BadIdentifier")
    if version != VERSION or any(data[40:64]):
        raise MaskError("UnsupportedVersion")
    if vkformat != PAGE_VKFORMAT:
        raise MaskError("UnsupportedFormat")
    if not (1 <= width <= MAX_DIMENSION and 1 <= height <= MAX_DIMENSION) or \
            not 1 <= count <= MAX_LIGHTS or offset != HEADER_BYTES + 16 * count or \
            page_bytes != width * height * 8 or len(data) != offset + page_bytes:
        raise MaskError("InvalidDescriptor")
    records = [struct.unpack_from("<3fI", data, HEADER_BYTES + 16 * i) for i in range(count)]
    if any(not 1 <= r[3] <= MAX_IDS or not np.isfinite(r[:3]).all() for r in records):
        raise MaskError("InvalidDescriptor")
    return {"width": width, "height": height,
            "lights": [{"origin": list(r[:3]), "id": r[3]} for r in records],
            "page_offset": offset, "page_bytes": page_bytes}


def decode(data):
    """(layout, visibility float (h, w, 4), ids uint8 (h, w, 4))."""
    layout = read(data)
    w, h = layout["width"], layout["height"]
    page = np.frombuffer(data, "<u2", w * h * 4, layout["page_offset"]).reshape(h, w, 4)
    return layout, (page & 255) / 255.0, (page >> 8).astype(np.uint8)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    info = sub.add_parser("info")
    info.add_argument("path", type=Path)
    args = parser.parse_args()
    layout = read(args.path.read_bytes())
    print(json.dumps(layout, indent=2))


if __name__ == "__main__":
    main()
