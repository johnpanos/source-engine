#!/usr/bin/env python3
"""SDFV: the RFC 0011 G6 signed distance volume, the SDF-traced producer's
input, in Python.

This module is the pipeline's writer and the independent reader the engine's
C++ reader (mapcontainer/sdf_volume) is checked against. The bake
(sdf_volume_bake.py) owns the values.

    python3 tools/quality/sdf_volume.py validate <file.sdfv>
    python3 tools/quality/sdf_volume.py info <file.sdfv>
    python3 tools/quality/sdf_volume.py fixtures <dir>

Model. Source units. A uniform grid of voxels over the static world, each
holding the signed distance at its centre to the nearest world surface
(negative inside closed solids), the reflectance and the emitted radiance of
that surface (diffuse light units, irradiance / pi), and the index of the
light source that emission belongs to (its light style scales it; 0xFFFF:
none). Analytic lights the producer samples with shadow rays: rectangles
(radiance, centre, two half-extent axes, emitting along -(u x v)... the
winding the bake records as `normal`), distant lights (irradiance, the
direction light travels, angular diameter) and a uniform dome (radiance).
Each light carries its light style (-1: fixed), matching the map's RTRN
sources.

Encoding (little-endian):

  header (64 bytes)
    0  u32 magic 'SDFV'   4  u32 version (1)   8  u32 header bytes (64)   12 u32 flags (0)
    16 f32 origin[3] (the first voxel's centre)  28 f32 voxel size
    32 u32 dims[3]        44 u32 lights        48 f32 max distance (clamp)
    52 u32 reserved[3] (0)
  voxels   dims.x * dims.y * dims.z records of 16 bytes, x fastest:
           f16 distance, f16 reflectance rgb, f16 emission rgb, u16 source (0xFFFF none)
  lights   records of 64 bytes:
           u32 kind (0 rect, 1 distant, 2 dome), i32 style, f32 rgb[3] (rect/dome radiance,
           distant irradiance), f32 a[3], f32 b[3], f32 c[3], f32 reserved[2]
           rect: a centre, b half-axis u, c half-axis v, emitting along normalize(b x c)
           distant: a the direction light travels (unit), b[0] angular diameter (radians)
           dome: a, b, c zero
"""

import argparse
import json
import math
import struct
import sys
from pathlib import Path

import numpy as np

MAGIC = 0x56464453  # "SDFV"
VERSION = 1
HEADER_BYTES = 64
VOXEL_BYTES = 16
LIGHT_BYTES = 64
MAX_VOXELS = 1 << 24
MAX_LIGHTS = 64
NO_SOURCE = 0xFFFF
KINDS = ("rect", "distant", "dome")


class SdfError(ValueError):
    def __init__(self, code, detail=""):
        super().__init__(code + (": " + detail if detail else ""))
        self.code = code


def build(origin, voxel, dims, distance, reflectance, emission, source, lights, max_distance):
    """SDFV bytes. distance (Z, Y, X); reflectance and emission (Z, Y, X, 3);
    source (Z, Y, X) u16; lights: [{kind, style, rgb, a, b, c}]."""
    dims = [int(v) for v in dims]
    count = dims[0] * dims[1] * dims[2]
    voxels = np.zeros((count, 8), dtype="<u2")
    voxels[:, 0] = np.asarray(distance, np.float32).reshape(-1).astype("<f2").view("<u2")
    voxels[:, 1:4] = np.asarray(reflectance, np.float32).reshape(-1, 3).astype("<f2").view("<u2")
    voxels[:, 4:7] = np.asarray(emission, np.float32).reshape(-1, 3).astype("<f2").view("<u2")
    voxels[:, 7] = np.asarray(source).reshape(-1).astype("<u2")
    header = struct.pack("<IIII3ff3IIf3I", MAGIC, VERSION, HEADER_BYTES, 0, *origin, voxel, *dims,
                         len(lights), max_distance, 0, 0, 0)
    records = b""
    for light in lights:
        records += struct.pack("<Ii3f3f3f3f2f", KINDS.index(light["kind"]), int(light["style"]),
                               *light["rgb"], *light.get("a", (0, 0, 0)),
                               *light.get("b", (0, 0, 0)), *light.get("c", (0, 0, 0)), 0, 0)
    return header + voxels.tobytes() + records


class Volume:
    def __init__(self, data):
        data = bytes(data)
        if len(data) < HEADER_BYTES:
            raise SdfError("truncated")
        values = struct.unpack_from("<IIII3ff3IIf3I", data, 0)
        magic, version, header, flags = values[0:4]
        if magic != MAGIC:
            raise SdfError("bad-magic")
        if version != VERSION or header != HEADER_BYTES or flags or any(values[13:16]):
            raise SdfError("unsupported-version")
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
        if len(data) != HEADER_BYTES + count * VOXEL_BYTES + self.light_count * LIGHT_BYTES:
            raise SdfError("size-mismatch")
        raw = np.frombuffer(data, "<u2", count * 8, HEADER_BYTES).reshape(count, 8)
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
        base = HEADER_BYTES + count * VOXEL_BYTES
        for i in range(self.light_count):
            v = struct.unpack_from("<Ii3f3f3f3f2f", data, base + i * LIGHT_BYTES)
            if v[0] >= len(KINDS) or not -1 <= v[1] <= 63 or v[14] or v[15] or \
                    not all(math.isfinite(x) for x in v[2:14]) or min(v[2:5]) < 0:
                raise SdfError("invalid-light", "light %d" % i)
            light = {"kind": KINDS[v[0]], "style": v[1], "rgb": list(v[2:5]), "a": list(v[5:8]),
                     "b": list(v[8:11]), "c": list(v[11:14])}
            if light["kind"] == "distant" and abs(np.linalg.norm(light["a"]) - 1) > 1e-3:
                raise SdfError("invalid-light", "light %d direction" % i)
            self.lights.append(light)
        used = self.source[self.source != NO_SOURCE]
        if used.size and int(used.max()) >= 64:
            raise SdfError("invalid-voxel", "source index")
        self.bytes = len(data)

    def info(self):
        return {"bytes": self.bytes, "origin": self.origin.tolist(), "voxel": self.voxel,
                "dims": self.dims, "lights": self.lights, "max_distance": self.max_distance,
                "inside_fraction": float((self.distance < 0).mean()),
                "emissive_voxels": int((self.emission.max(axis=-1) > 0).sum())}


def malformations(data):
    header = struct.unpack_from("<IIII3ff3IIf3I", data, 0)
    count = header[8] * header[9] * header[10]

    def patched(offset, fmt, *values):
        out = bytearray(data)
        struct.pack_into(fmt, out, offset, *values)
        return bytes(out)

    first_light = HEADER_BYTES + count * VOXEL_BYTES
    return [("truncated", data[:HEADER_BYTES - 2]), ("short", data[:-8]),
            ("bad-magic", patched(0, "<I", 0)), ("version", patched(4, "<I", 9)),
            ("flags", patched(12, "<I", 1)), ("voxel-size", patched(28, "<f", 0.0)),
            ("dims", patched(32, "<I", 1)), ("max-distance", patched(48, "<f", float("nan"))),
            ("reflectance", patched(HEADER_BYTES + 2, "<e", 1.5)),
            ("emission", patched(HEADER_BYTES + 8, "<e", -1.0)),
            ("distance", patched(HEADER_BYTES, "<e", 60000.0)),
            ("light-kind", patched(first_light, "<I", 9)),
            ("light-style", patched(first_light + 4, "<i", 99))]


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


def write_fixtures(out):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    data = fixture_contract()
    Volume(data)
    (out / "contract.sdfv").write_bytes(data)
    variants = []
    for name, variant in malformations(data):
        try:
            Volume(variant)
            code = None
        except SdfError as error:
            code = error.code
        variants.append({"name": name, "error": code})
    (out / "fixtures.json").write_text(json.dumps({"schema": "sdfv-fixtures/v1",
                                                   "contract": Volume(data).info(),
                                                   "malformations": variants}, indent=1) + "\n")
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
