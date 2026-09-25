#!/usr/bin/env python3
"""SDFV: the RFC 0011 G6 signed distance volume, the SDF-traced producer's
input, in Python.

This module is the pipeline's writer and the independent reader the engine's
C++ reader (mapcontainer/sdf_volume) is checked against. The bake
(sdf_volume_bake.py) owns the values.

    python3 tools/quality/sdf_volume.py validate <file.sdfv>
    python3 tools/quality/sdf_volume.py info <file.sdfv>
    python3 tools/quality/sdf_volume.py fixtures <dir>   (contract.sdfv, contract.tris, ...)

Model. Source units. A uniform grid of voxels over the static world, each
holding the signed distance at its centre to the nearest world surface
(negative inside closed solids), the reflectance and the emitted radiance of
that surface (diffuse light units, irradiance / pi), and the index of the
light source that emission belongs to (its light style scales it; 0xFFFF:
none). Analytic lights the producer samples with shadow rays: rectangles
(radiance, centre, two half-extent axes, emitting along -(u x v)... the
winding the bake records as `normal`), distant lights (irradiance, the
direction light travels, angular diameter), a uniform dome (radiance),
spheres (radiance, centre, radius) and spots (a one-sided disk: radiance,
centre, emitting normal, radius, and vrad's cone: full inside the inner cone,
((cos - outer) / (inner - outer)) ** exponent between the cones, nothing
outside the outer one; exponent 0 is linear). Each light carries its light
style (-1: fixed), matching the map's RTRN sources.

Version 2 adds the light cells: a coarse grid over the world whose every cell
lists the lights that may reach points in it (the bake culls by range and,
for a compiled map, by its visibility). A point outside the grid uses the
nearest cell. A version-1 volume has no cells: every light reaches every
point.

Encoding (little-endian):

  header (64 bytes; 96 in version 2)
    0  u32 magic 'SDFV'   4  u32 version (1 or 2)   8  u32 header bytes (64 or 96)
    12 u32 flags (0)
    16 f32 origin[3] (the first voxel's centre)  28 f32 voxel size
    32 u32 dims[3]        44 u32 lights        48 f32 max distance (clamp)
    52 u32 reserved[3] (0)
    version 2:
    64 f32 cell origin[3] (the grid's low corner)  76 f32 cell size
    80 u32 cell dims[3]   92 u32 cell entries
  voxels   dims.x * dims.y * dims.z records of 16 bytes, x fastest:
           f16 distance, f16 reflectance rgb, f16 emission rgb, u16 source (0xFFFF none)
  lights   records of 64 bytes:
           u32 kind (0 rect, 1 distant, 2 dome, 3 sphere, 4 spot; 3 and 4 in version 2),
           i32 style, f32 rgb[3] (radiance; distant: irradiance), f32 a[3], f32 b[3],
           f32 c[3], f32 d[2] (zero but for a spot's exponent)
           rect: a centre, b half-axis u, c half-axis v, emitting along normalize(b x c)
           distant: a the direction light travels (unit), b[0] angular diameter (radians)
           dome: a, b, c zero
           sphere: a centre, b[0] radius (> 0)
           spot: a centre, b emitting normal (unit), c radius (> 0), cos inner, cos outer
                 (-1 <= outer <= inner <= 1), d[0] exponent (>= 0)
  cells    (version 2) u32 first entry per cell, x fastest, then the entry count;
           u16 light index per entry, zero-padded to 4 bytes
"""

import argparse
import json
import math
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import radiosity_transfer  # noqa: E402  (its byte-diff recipe for the corpus)

MAGIC = 0x56464453  # "SDFV"
VERSION = 2
HEADER_BYTES = 96
V1_HEADER_BYTES = 64
VOXEL_BYTES = 16
LIGHT_BYTES = 64
MAX_VOXELS = 1 << 24
MAX_LIGHTS = 4096
MAX_CELLS = 1 << 22
NO_SOURCE = 0xFFFF
KINDS = ("rect", "distant", "dome", "sphere", "spot")
V1_KINDS = 3


def check_light(light, index):
    """A light record's kind-specific invariants (SdfError otherwise)."""
    kind, b, c, d = light["kind"], light["b"], light["c"], light["d"]
    ok = True
    if kind == "distant":
        ok = abs(np.linalg.norm(light["a"]) - 1) <= 1e-3
    elif kind == "sphere":
        ok = b[0] > 0 and not any(b[1:]) and not any(c) and not any(d)
    elif kind == "spot":
        ok = abs(np.linalg.norm(b) - 1) <= 1e-3 and c[0] > 0 and \
            -1.0 <= c[2] <= c[1] <= 1.0 and d[0] >= 0 and d[1] == 0
    else:
        ok = not any(d)
    if not ok:
        raise SdfError("invalid-light", "light %d %s" % (index, kind))


def spot_multiplier(cos_angle, inner, outer, exponent):
    """vrad's spot cone at `cos_angle` from the axis (one-sided disk's
    cosine not included)."""
    if cos_angle >= inner:
        return 1.0
    if cos_angle <= outer:
        return 0.0
    t = (cos_angle - outer) / (inner - outer)
    return t ** exponent if exponent not in (0.0, 1.0) else t


class SdfError(ValueError):
    def __init__(self, code, detail=""):
        super().__init__(code + (": " + detail if detail else ""))
        self.code = code


def build(origin, voxel, dims, distance, reflectance, emission, source, lights, max_distance,
          cells=None):
    """SDFV bytes. distance (Z, Y, X); reflectance and emission (Z, Y, X, 3);
    source (Z, Y, X) u16; lights: [{kind, style, rgb, a, b, c, d}]; cells:
    {origin, size, dims, lists: [[light index, ...] per cell, x fastest]}
    (default: one cell over the voxels listing every light)."""
    dims = [int(v) for v in dims]
    count = dims[0] * dims[1] * dims[2]
    voxels = np.zeros((count, 8), dtype="<u2")
    voxels[:, 0] = np.asarray(distance, np.float32).reshape(-1).astype("<f2").view("<u2")
    voxels[:, 1:4] = np.asarray(reflectance, np.float32).reshape(-1, 3).astype("<f2").view("<u2")
    voxels[:, 4:7] = np.asarray(emission, np.float32).reshape(-1, 3).astype("<f2").view("<u2")
    voxels[:, 7] = np.asarray(source).reshape(-1).astype("<u2")
    if cells is None:
        low = np.asarray(origin, np.float64) - voxel / 2
        extent = (np.asarray(dims) - 1) * voxel + voxel
        cells = {"origin": low.tolist(), "size": float(extent.max()), "dims": [1, 1, 1],
                 "lists": [list(range(len(lights)))]}
    lists = cells["lists"]
    if len(lists) != int(np.prod(cells["dims"])):
        raise SdfError("invalid-cells", "one list per cell")
    entries = sum(len(entry) for entry in lists)
    header = struct.pack("<IIII3ff3IIf3I3ff3II", MAGIC, VERSION, HEADER_BYTES, 0, *origin, voxel,
                         *dims, len(lights), max_distance, 0, 0, 0, *cells["origin"],
                         cells["size"], *cells["dims"], entries)
    records = b""
    for light in lights:
        records += struct.pack("<Ii3f3f3f3f2f", KINDS.index(light["kind"]), int(light["style"]),
                               *light["rgb"], *light.get("a", (0, 0, 0)),
                               *light.get("b", (0, 0, 0)), *light.get("c", (0, 0, 0)),
                               *light.get("d", (0, 0)))
    offsets = np.concatenate([[0], np.cumsum([len(entry) for entry in lists])]).astype("<u4")
    indices = np.asarray([i for entry in lists for i in entry], "<u2")
    padding = b"\0\0" if entries % 2 else b""
    return (header + voxels.tobytes() + records + offsets.tobytes() + indices.tobytes() +
            padding)


class Volume:
    def __init__(self, data):
        data = bytes(data)
        if len(data) < HEADER_BYTES:
            raise SdfError("truncated")
        values = struct.unpack_from("<IIII3ff3IIf3I", data, 0)
        magic, version, header, flags = values[0:4]
        if magic != MAGIC:
            raise SdfError("bad-magic")
        if (version, header) not in ((1, V1_HEADER_BYTES), (2, HEADER_BYTES)) or flags or \
                any(values[13:16]) or len(data) < header:
            raise SdfError("unsupported-version")
        self.version = version
        self.origin = np.array(values[4:7], np.float64)
        self.voxel = float(values[7])
        self.dims = [int(v) for v in values[8:11]]
        self.light_count = int(values[11])
        self.max_distance = float(values[12])
        count = self.dims[0] * self.dims[1] * self.dims[2]
        if not np.isfinite(self.origin).all() or not (self.voxel > 0 and math.isfinite(self.voxel)) \
                or min(self.dims) < 2 or count > MAX_VOXELS or self.light_count > MAX_LIGHTS or \
                not (self.max_distance > 0 and math.isfinite(self.max_distance)):
            raise SdfError("invalid-grid")
        lights_end = header + count * VOXEL_BYTES + self.light_count * LIGHT_BYTES
        if version == 1:
            cell_count = entries = 0
            if len(data) != lights_end:
                raise SdfError("size-mismatch")
        else:
            cell = struct.unpack_from("<3ff3II", data, 64)
            self.cell_origin = np.array(cell[0:3], np.float64)
            self.cell_size = float(cell[3])
            self.cell_dims = [int(v) for v in cell[4:7]]
            entries = int(cell[7])
            cell_count = self.cell_dims[0] * self.cell_dims[1] * self.cell_dims[2]
            if not np.isfinite(self.cell_origin).all() or \
                    not (self.cell_size > 0 and math.isfinite(self.cell_size)) or \
                    min(self.cell_dims) < 1 or cell_count > MAX_CELLS:
                raise SdfError("invalid-cells", "grid")
            size = lights_end + 4 * (cell_count + 1) + 2 * entries + 2 * (entries % 2)
            if len(data) != size:
                raise SdfError("size-mismatch")
        raw = np.frombuffer(data, "<u2", count * 8, header).reshape(count, 8)
        with np.errstate(invalid="ignore", over="ignore"):
            halves = raw[:, :7].view("<f2").astype(np.float32)
        if not np.isfinite(halves).all():
            raise SdfError("invalid-voxel", "non-finite")
        if (halves[:, 1:4] < 0).any() or (halves[:, 1:4] > 1).any() or (halves[:, 4:7] < 0).any():
            raise SdfError("invalid-voxel", "reflectance or emission out of range")
        if (np.abs(halves[:, 0]) > self.max_distance * (1 + 1e-2) + 1).any():
            raise SdfError("invalid-voxel", "distance beyond the clamp")
        shape = (self.dims[2], self.dims[1], self.dims[0])
        self.distance = halves[:, 0].reshape(shape)
        self.reflectance = halves[:, 1:4].reshape(shape + (3,))
        self.emission = halves[:, 4:7].reshape(shape + (3,))
        self.source = raw[:, 7].reshape(shape)
        self.lights = []
        base = header + count * VOXEL_BYTES
        for i in range(self.light_count):
            v = struct.unpack_from("<Ii3f3f3f3f2f", data, base + i * LIGHT_BYTES)
            kinds = V1_KINDS if version == 1 else len(KINDS)
            if v[0] >= kinds or not -1 <= v[1] <= 63 or \
                    not all(math.isfinite(x) for x in v[2:16]) or min(v[2:5]) < 0:
                raise SdfError("invalid-light", "light %d" % i)
            light = {"kind": KINDS[v[0]], "style": v[1], "rgb": list(v[2:5]), "a": list(v[5:8]),
                     "b": list(v[8:11]), "c": list(v[11:14]), "d": list(v[14:16])}
            check_light(light, i)
            self.lights.append(light)
        if version == 1:
            # Every light everywhere: one cell over the voxels.
            self.cell_origin = self.origin - self.voxel / 2
            self.cell_size = float(max(self.dims)) * self.voxel
            self.cell_dims = [1, 1, 1]
            self.cell_offsets = np.array([0, self.light_count], np.int64)
            self.cell_entries = np.arange(self.light_count, dtype=np.int64)
        else:
            at = lights_end
            self.cell_offsets = np.frombuffer(data, "<u4", cell_count + 1, at).astype(np.int64)
            self.cell_entries = np.frombuffer(data, "<u2", entries,
                                              at + 4 * (cell_count + 1)).astype(np.int64)
            padding = data[at + 4 * (cell_count + 1) + 2 * entries:]
            if self.cell_offsets[0] != 0 or self.cell_offsets[-1] != entries or \
                    (np.diff(self.cell_offsets) < 0).any() or any(padding):
                raise SdfError("invalid-cells", "offsets")
            if entries and int(self.cell_entries.max()) >= self.light_count:
                raise SdfError("invalid-cells", "light index")
        used = self.source[self.source != NO_SOURCE]
        if used.size and int(used.max()) >= 64:
            raise SdfError("invalid-voxel", "source index")
        self.bytes = len(data)

    def cell_lights(self, point):
        """The light indices listed for the cell holding `point` (the nearest
        cell outside the grid)."""
        index = np.floor((np.asarray(point, np.float64) - self.cell_origin) / self.cell_size)
        index = np.clip(index, 0, np.asarray(self.cell_dims) - 1).astype(int)
        cell = index[0] + self.cell_dims[0] * (index[1] + self.cell_dims[1] * index[2])
        return self.cell_entries[self.cell_offsets[cell]:self.cell_offsets[cell + 1]].tolist()

    def info(self):
        lengths = np.diff(self.cell_offsets)
        return {"bytes": self.bytes, "version": self.version, "origin": self.origin.tolist(),
                "voxel": self.voxel, "dims": self.dims, "lights": self.lights,
                "max_distance": self.max_distance,
                "cells": {"origin": self.cell_origin.tolist(), "size": self.cell_size,
                          "dims": self.cell_dims, "entries": int(len(self.cell_entries)),
                          "max_lights": int(lengths.max()) if len(lengths) else 0,
                          "mean_lights": float(lengths.mean()) if len(lengths) else 0.0},
                "inside_fraction": float((self.distance < 0).mean()),
                "emissive_voxels": int((self.emission.max(axis=-1) > 0).sum())}


def malformations(data):
    header = struct.unpack_from("<IIII3ff3IIf3I", data, 0)
    count = header[8] * header[9] * header[10]

    def patched(offset, fmt, *values):
        out = bytearray(data)
        struct.pack_into(fmt, out, offset, *values)
        return bytes(out)

    def edited(*edits):
        out = bytearray(data)
        for offset, fmt, *values in edits:
            struct.pack_into(fmt, out, offset, *values)
        return bytes(out)

    first_light = HEADER_BYTES + count * VOXEL_BYTES
    lights = header[11]
    cells = first_light + lights * LIGHT_BYTES
    return [("truncated", data[:HEADER_BYTES - 2]), ("short", data[:-8]),
            ("bad-magic", patched(0, "<I", 0)), ("version", patched(4, "<I", 9)),
            ("flags", patched(12, "<I", 1)), ("voxel-size", patched(28, "<f", 0.0)),
            ("dims", patched(32, "<I", 1)), ("max-distance", patched(48, "<f", float("nan"))),
            ("reflectance", patched(HEADER_BYTES + 2, "<e", 1.5)),
            ("emission", patched(HEADER_BYTES + 8, "<e", -1.0)),
            ("distance", patched(HEADER_BYTES, "<e", 60000.0)),
            ("light-kind", patched(first_light, "<I", 9)),
            ("light-style", patched(first_light + 4, "<i", 99)),
            # Version 2: a reserved word set on a dome, a sphere of radius 0,
            # a spot whose inner cone is outside its outer one.
            ("light-reserved", patched(first_light + 56, "<f", 1.0)),
            ("sphere-radius", patched(first_light, "<I", 3)),
            ("spot-cone", edited((first_light, "<I", 4), (first_light + 32, "<3f", 0.0, 0.0, 1.0),
                                 (first_light + 44, "<3f", 1.0, 0.2, 0.9))),
            ("cell-size", patched(76, "<f", 0.0)),
            ("cell-dims", patched(80, "<I", 0)),
            ("cell-offsets", patched(cells, "<I", 1)),
            ("cell-index", patched(cells + 8, "<H", 999))]


def fixture_contract():
    """The GPU producer suite's scene, matching the contract PRBV
    (probe_volume.fixture_contract: probes on a 32-unit grid from the origin,
    4 x 2 x 2, a wall at x = 48): room A (-16..48 in x, -16..48 in y and z) is
    a furnace, every wall emitting 0.3 with reflectance 0.6, so its light
    converges to 0.75 (indirect 0.45); room B (48..112 in x) beyond the wall
    is black and unlit. Voxels of 8 units."""
    voxel = 8.0
    lo = np.array([-16.0 - 2 * voxel, -16.0 - 2 * voxel, -16.0 - 2 * voxel])
    hi = np.array([112.0 + 2 * voxel, 48.0 + 2 * voxel, 48.0 + 2 * voxel])
    dims = np.round((hi - lo) / voxel).astype(int) + 1
    zs, ys, xs = np.meshgrid(*(lo[k] + np.arange(dims[k]) * voxel for k in (2, 1, 0)),
                             indexing="ij")
    points = np.stack([xs, ys, zs], axis=-1)

    def box_distance(p, a, b):
        centre, half = (a + b) / 2, (b - a) / 2
        q = np.abs(p - centre) - half
        outside = np.linalg.norm(np.maximum(q, 0), axis=-1)
        inside = np.minimum(q.max(axis=-1), 0)
        return outside + inside

    room_a = (np.array([-16.0, -16.0, -16.0]), np.array([48.0, 48.0, 48.0]))
    room_b = (np.array([48.0, -16.0, -16.0]), np.array([112.0, 48.0, 48.0]))
    # Solid is everything outside both rooms: distance = -(distance to the
    # union of the room interiors).
    inside_rooms = np.minimum(box_distance(points, *room_a), box_distance(points, *room_b))
    # The wall between them: a slab 48 +- 1.
    wall = box_distance(points, np.array([47.0, -16.0, -16.0]), np.array([49.0, 48.0, 48.0]))
    distance = np.minimum(-inside_rooms, wall)
    max_distance = 64.0
    distance = np.clip(distance, -max_distance, max_distance)
    in_a = points[..., 0] < 48.0
    reflectance = np.where(in_a[..., None], 0.6, 0.0) * np.ones(3)
    emission = np.where(in_a[..., None], 0.3, 0.0) * np.ones(3)
    source = np.where(in_a, 0, NO_SOURCE)
    # A dark dome: the rooms are closed, so it lights nothing (it gives the
    # light table a record for the malformation corpus).
    return build(lo.tolist(), voxel, dims.tolist(), distance, reflectance, emission, source,
                 [{"kind": "dome", "style": -1, "rgb": (0.0, 0.0, 0.0)}], max_distance)


def fixture_lights():
    """Every light kind and a 2 x 2 x 1 cell grid, over a small empty volume
    (the readers' positive fixture): a rect, a distant light, a dome, a
    sphere and a spot; the global lights are in every cell, the sphere in the
    cells with x < 32, the spot in cell 3 only."""
    voxel = 16.0
    dims = [5, 5, 3]
    shape = (dims[2], dims[1], dims[0])
    lights = [
        {"kind": "rect", "style": 32, "rgb": (1.0, 1.0, 1.0), "a": (16.0, 16.0, 30.0),
         "b": (4.0, 0.0, 0.0), "c": (0.0, -4.0, 0.0)},
        {"kind": "distant", "style": -1, "rgb": (2.0, 2.0, 2.0), "a": (0.0, 0.0, -1.0),
         "b": (0.01, 0.0, 0.0)},
        {"kind": "dome", "style": -1, "rgb": (0.1, 0.1, 0.1)},
        {"kind": "sphere", "style": -1, "rgb": (5.0, 4.0, 3.0), "a": (8.0, 8.0, 16.0),
         "b": (2.0, 0.0, 0.0)},
        {"kind": "spot", "style": 33, "rgb": (9.0, 9.0, 9.0), "a": (48.0, 48.0, 30.0),
         "b": (0.0, 0.0, -1.0), "c": (2.0, 0.9659, 0.8660), "d": (2.0, 0.0)}]
    cells = {"origin": [-8.0, -8.0, -8.0], "size": 40.0, "dims": [2, 2, 1],
             "lists": [[0, 1, 2, 3], [0, 1, 2], [0, 1, 2, 3], [0, 1, 2, 4]]}
    return build([0.0, 0.0, 0.0], voxel, dims, np.full(shape, 32.0), np.zeros(shape + (3,)),
                 np.zeros(shape + (3,)), np.full(shape, NO_SOURCE), lights, 64.0, cells)


CONTRACT_ROOMS = (((-16.0, -16.0, -16.0), (47.0, 48.0, 48.0)),
                  ((49.0, -16.0, -16.0), (112.0, 48.0, 48.0)))
TRIS_MAGIC = 0x53495254  # "TRIS"


def contract_geometry():
    """The contract scene as triangles (the ray-query producer's world): the
    two rooms' boxes, whose facing walls at x 47 and 49 are the wall slab's
    faces. Returns (positions (V, 3), indices (T * 3,))."""
    positions, indices = [], []
    for lo, hi in CONTRACT_ROOMS:
        base = len(positions)
        positions += [(x, y, z) for z in (lo[2], hi[2]) for y in (lo[1], hi[1])
                      for x in (lo[0], hi[0])]
        # Corner k has bits x=1, y=2, z=4; one quad (two triangles) per face.
        for quad in ((0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2),
                     (1, 3, 7, 5)):
            a, b, c, d = (base + k for k in quad)
            indices += [a, b, c, a, c, d]
    return np.asarray(positions, np.float32), np.asarray(indices, np.uint32)


def write_tris(positions, indices):
    """TRIS bytes (test fixtures): u32 magic, u32 vertices, u32 indices, then
    f32 xyz per vertex and u32 per index, little-endian."""
    return (struct.pack("<III", TRIS_MAGIC, len(positions), len(indices)) +
            np.asarray(positions, "<f4").tobytes() + np.asarray(indices, "<u4").tobytes())


def write_fixtures(out):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    data = fixture_contract()
    Volume(data)
    (out / "contract.sdfv").write_bytes(data)
    lights = fixture_lights()
    Volume(lights)
    (out / "lights.sdfv").write_bytes(lights)
    (out / "contract.tris").write_bytes(write_tris(*contract_geometry()))
    variants = []
    for name, variant in malformations(data):
        try:
            Volume(variant)
            code = None
        except SdfError as error:
            code = error.code
        variants.append({"name": name, "error": code, **radiosity_transfer.recipe(data, variant)})
    (out / "fixtures.json").write_text(json.dumps({"schema": "sdfv-fixtures/v2",
                                                   "contract": Volume(data).info(),
                                                   "lights": Volume(lights).info(),
                                                   "malformations": variants}, indent=1) + "\n")
    # The same corpus as plain text for the C++ reader's test:
    #   name error length [offset:hex ...]
    (out / "malformations.txt").write_text("".join(
        "%s %s %d%s\n" % (v["name"], v["error"], v["length"],
                          "".join(" %d:%s" % (o, h) for o, h in v["edits"]))
        for v in variants))
    if any(v["error"] is None for v in variants):
        raise SdfError("fixture", "a malformation was accepted")
    return len(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("validate", "info"):
        commands.add_parser(name).add_argument("sdfv", type=Path)
    commands.add_parser("fixtures").add_argument("out", type=Path)
    args = parser.parse_args()
    if args.command == "fixtures":
        print("wrote %d bytes" % write_fixtures(args.out))
        return 0
    try:
        volume = Volume(args.sdfv.read_bytes())
    except SdfError as error:
        print(json.dumps({"status": "invalid", "error": error.code, "detail": str(error)}))
        return 1
    print(json.dumps({"status": "valid", **(volume.info() if args.command == "info" else {})},
                     indent=2 if args.command == "info" else None))
    return 0


if __name__ == "__main__":
    sys.exit(main())
