#!/usr/bin/env python3
"""RTRN: the RFC 0011 radiosity transfer, the precomputed radiosity producer's
input, in Python.

This module is the pipeline's writer and the independent reader the engine's
C++ reader (mapcontainer/radiosity_transfer) is checked against. It owns the
encoding and the fixtures; the bake (radiosity_transfer_bake.py) owns the
values and the runtime solver (public/render/indirect_radiosity.h) their use.

    python3 tools/quality/radiosity_transfer.py validate <file.rtrn> [--prbv <file.prbv>]
    python3 tools/quality/radiosity_transfer.py info <file.rtrn>
    python3 tools/quality/radiosity_transfer.py fuzz <file.rtrn> [--prbv F] [--iterations N]
    python3 tools/quality/radiosity_transfer.py fixtures <dir>

Model. Units are the lightmap's diffuse light (irradiance / pi) and Source
units. The scene's light comes from sources s: lights, the sky and emissive
surfaces, each with a light style whose runtime scalar sigma_s is 1 in the
bake. The static world is split into patches p (position, normal, area,
diffuse albedo rho_p). With
  D[p, s]   the direct light at p from s alone, at its baked intensity,
  F[p, q]   the form factor from p to q: the fraction of p's cosine-weighted
            hemisphere that first sees q (visibility included; a row sums to
            at most 1, the rest leaves the world),
the light arriving at every patch solves
  H_p = sum_s sigma_s D[p, s] + sum_q F[p, q] rho_q H_q
and H_p minus its first term is p's indirect light. A probe i (the paired
PRBV's probes, in PRBV order) gathers the patches' reflected light: its
indirect light for a unit normal n is
  I_i(n) = sum_q rho_q H_q sum_k G[i, q, k] Y_k(n)
with Y_k the nine real spherical harmonics of order <= 2 (Y00, Y1-1, Y10,
Y11, Y2-2, Y2-1, Y20, Y21, Y22 in the usual normalization) and G already
convolved with the cosine lobe. Its direct light from source s is the
octahedral 6 x 6 texel grid P[s, i, t], in the PRBV interior texel order, so
the probe's total is sum_s sigma_s P[s, i, t] + I_i(n_t).

Encoding (little-endian). A 64-byte header, then nine sections in this
order, each starting at the next multiple of 16 bytes; the size is exactly
the end of the last section:

  header
    0  u32 magic 'RTRN'          4  u32 version (1)        8  u32 header bytes (64)
    12 u32 flags (0)             16 u32 sources (1..64)    20 u32 patches (1..2^22)
    24 u32 probes (the PRBV's)   28 u32 probe texels (36)  32 u32 transfer links
    36 u32 injection links       40 u32 gather links       44 u32 SH coefficients (9)
    48 u64 PRBV topology hash    56 u64 reserved (0)
  sources         64 bytes each: u32 kind (0 light, 1 sky, 2 emissive surface),
                  i32 light style (-1 fixed, 0..63), u32 flags (0), u32 reserved (0),
                  char name[48] (UTF-8, NUL-terminated and -padded, unique, non-empty)
  patches         48 bytes each: f32 position[3], f32 normal[3] (unit), f32 area
                  (units^2, > 0), f32 albedo[3] (0..1), u32 reserved[2] (0)
  transfer rows   (patches + 1) u32: row p is links [row[p], row[p + 1])
  transfer links  8 bytes each: u32 source patch, f32 form factor (>= 0)
  injection rows  (sources + 1) u32
  injection links 16 bytes each: u32 patch, f32 direct light rgb (>= 0)
  gather rows     (probes + 1) u32
  gather links    40 bytes each: u32 patch, f32 sh[9]
  probe direct    sources x probes x 36 x f32 rgb (>= 0)

Within a row the patch indices strictly increase. The gather's first
coefficient is bounded like the transfer: sum_q G[i, q, 0] Y00 <= 1, the
fraction of the probe's sphere that sees patches. The topology hash is
FNV-1a 64 over the PRBV header and grid records followed by every grid's
probe state texels in probe order: an RTRN pairs with exactly the volume it
was baked against.
"""

import argparse
import json
import math
import random
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import probe_volume  # noqa: E402

MAGIC = 0x4E525452  # "RTRN"
VERSION = 1
HEADER_BYTES = 64
SOURCE_BYTES = 64
PATCH_BYTES = 48
TRANSFER_LINK_BYTES = 8
INJECTION_LINK_BYTES = 16
GATHER_LINK_BYTES = 40
PROBE_TEXELS = 36
SH_COEFFICIENTS = 9
MAX_SOURCES = 64
MAX_PATCHES = 1 << 22
MAX_LINKS = 1 << 28
MAX_STYLE = 63
NAME_BYTES = 48
SOURCE_KINDS = ("light", "sky", "emissive")
# Tolerances of the normalization checks.
ROW_SUM_LIMIT = 1.0 + 1e-3
NORMAL_TOLERANCE = 1e-3
Y00 = 0.5 / math.sqrt(math.pi)
FNV_OFFSET = 0xCBF29CE484222325
FNV_PRIME = 0x100000001B3


class TransferError(ValueError):
    def __init__(self, code, detail=""):
        super().__init__(code + (": " + detail if detail else ""))
        self.code = code


def align16(value):
    return (value + 15) & ~15


def fnv1a64(data, state=FNV_OFFSET):
    for byte in data:
        state = ((state ^ byte) * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return state


def prbv_topology_hash(data):
    """The PRBV topology hash (see the module docstring)."""
    volume = probe_volume.Volume(bytes(data))
    grids = len(volume.grids)
    state = fnv1a64(data[:probe_volume.HEADER_BYTES + probe_volume.GRID_BYTES * grids])
    offset = struct.unpack_from("<Q", data, 40)[0]
    width = volume.width
    for grid in volume.grids:
        row = grid.tiles_per_row * probe_volume.VISIBILITY_TILE
        x0, y0 = grid.state_origin
        for i in range(grid.count):
            texel = offset + ((y0 + i // row) * width + x0 + i % row) * 8
            state = fnv1a64(data[texel:texel + 8], state)
    return state


def prbv_probe_positions(data):
    """(probes, 3) positions (Source units, relocated) and active flags, in
    the PRBV's probe order over all grids."""
    volume = probe_volume.Volume(bytes(data))
    positions, active = [], []
    for grid in volume.grids:
        dims = grid.dims
        for i in range(grid.count):
            x = i % dims[0]
            y = (i // dims[0]) % dims[1]
            z = i // (dims[0] * dims[1])
            state = grid.state(i)
            positions.append(grid.origin + np.array([x, y, z]) * grid.spacing + state[:3])
            active.append(state[3] >= 0.5)
    return np.array(positions), np.array(active)


def section_sizes(sources, patches, probes, transfer, injection, gather):
    return [sources * SOURCE_BYTES, patches * PATCH_BYTES, (patches + 1) * 4,
            transfer * TRANSFER_LINK_BYTES, (sources + 1) * 4, injection * INJECTION_LINK_BYTES,
            (probes + 1) * 4, gather * GATHER_LINK_BYTES, sources * probes * PROBE_TEXELS * 12]


def layout(sizes):
    """Section offsets and the total size."""
    offsets, cursor = [], HEADER_BYTES
    for size in sizes:
        cursor = align16(cursor)
        offsets.append(cursor)
        cursor += size
    return offsets, cursor


def rows_from_lists(lists):
    rows = np.zeros(len(lists) + 1, dtype="<u4")
    rows[1:] = np.cumsum([len(entries) for entries in lists])
    return rows


def build(sources, patches, transfer, injection, gather, probe_direct, prbv_hash):
    """RTRN bytes.

    sources: [{name, kind, style}]; patches: dict of position (N, 3),
    normal (N, 3), area (N,), albedo (N, 3); transfer: per patch a list of
    (patch, factor); injection: per source a list of (patch, rgb); gather: per
    probe a list of (patch, sh[9]); probe_direct: (sources, probes, 36, 3)."""
    position = np.asarray(patches["position"], dtype="<f4")
    normal = np.asarray(patches["normal"], dtype="<f4")
    area = np.asarray(patches["area"], dtype="<f4")
    albedo = np.asarray(patches["albedo"], dtype="<f4")
    count = len(position)
    probes = len(gather)
    direct = np.asarray(probe_direct, dtype="<f4")
    if direct.shape != (len(sources), probes, PROBE_TEXELS, 3) or len(transfer) != count or \
            len(injection) != len(sources):
        raise TransferError("invalid-counts", "section shapes disagree")
    links = [sum(len(r) for r in rows) for rows in (transfer, injection, gather)]
    sizes = section_sizes(len(sources), count, probes, *links)
    offsets, total = layout(sizes)
    blob = bytearray(total)
    struct.pack_into("<IIIIIIIIIIIIQQ", blob, 0, MAGIC, VERSION, HEADER_BYTES, 0, len(sources),
                     count, probes, PROBE_TEXELS, links[0], links[1], links[2], SH_COEFFICIENTS,
                     prbv_hash, 0)
    for index, source in enumerate(sources):
        name = source["name"].encode("utf-8")
        if not name or len(name) >= NAME_BYTES:
            raise TransferError("invalid-source", "name length: " + source["name"])
        struct.pack_into("<IiII48s", blob, offsets[0] + index * SOURCE_BYTES,
                         SOURCE_KINDS.index(source["kind"]), int(source["style"]), 0, 0, name)
    records = np.zeros((count, 12), dtype="<f4")
    records[:, 0:3] = position
    records[:, 3:6] = normal
    records[:, 6] = area
    records[:, 7:10] = albedo
    blob[offsets[1]:offsets[1] + sizes[1]] = records.tobytes()

    def put_rows(section, lists, link_bytes, fmt):
        blob[offsets[section]:offsets[section] + sizes[section]] = \
            rows_from_lists(lists).tobytes()
        cursor = offsets[section + 1]
        for entries in lists:
            for patch, values in sorted(entries, key=lambda entry: entry[0]):
                values = [values] if np.isscalar(values) else list(values)
                struct.pack_into(fmt, blob, cursor, int(patch), *[float(v) for v in values])
                cursor += link_bytes

    put_rows(2, transfer, TRANSFER_LINK_BYTES, "<If")
    put_rows(4, injection, INJECTION_LINK_BYTES, "<I3f")
    put_rows(6, gather, GATHER_LINK_BYTES, "<I9f")
    blob[offsets[8]:offsets[8] + sizes[8]] = direct.tobytes()
    return bytes(blob)


# ------------------------------------------------------------------ reader

class Transfer:
    """A fully validated RTRN; raises TransferError otherwise."""

    def __init__(self, data, prbv=None):
        data = bytes(data)
        if len(data) < HEADER_BYTES:
            raise TransferError("truncated")
        (magic, version, header_bytes, flags, sources, patches, probes, texels, transfer,
         injection, gather, sh, prbv_hash, reserved) = struct.unpack_from(
             "<IIIIIIIIIIIIQQ", data, 0)
        if magic != MAGIC:
            raise TransferError("bad-magic")
        if version != VERSION or header_bytes != HEADER_BYTES or flags or reserved:
            raise TransferError("unsupported-version")
        if not 1 <= sources <= MAX_SOURCES or not 1 <= patches <= MAX_PATCHES or \
                probes > probe_volume.MAX_PROBES or texels != PROBE_TEXELS or \
                sh != SH_COEFFICIENTS or max(transfer, injection, gather) > MAX_LINKS:
            raise TransferError("invalid-counts")
        sizes = section_sizes(sources, patches, probes, transfer, injection, gather)
        offsets, total = layout(sizes)
        if total != len(data):
            raise TransferError("size-mismatch" if total > len(data) else "size-mismatch",
                                "%d bytes, layout needs %d" % (len(data), total))
        self.source_count, self.patch_count, self.probe_count = sources, patches, probes
        self.prbv_hash = prbv_hash
        self.bytes = len(data)
        self.sources = []
        names = set()
        for index in range(sources):
            kind, style, source_flags, source_reserved, raw = struct.unpack_from(
                "<IiII48s", data, offsets[0] + index * SOURCE_BYTES)
            end = raw.find(b"\0")
            if kind >= len(SOURCE_KINDS) or not -1 <= style <= MAX_STYLE or source_flags or \
                    source_reserved or end <= 0 or raw[end:] != b"\0" * (NAME_BYTES - end):
                raise TransferError("invalid-source", "source %d" % index)
            try:
                name = raw[:end].decode("utf-8")
            except UnicodeDecodeError:
                raise TransferError("invalid-source", "source %d name" % index) from None
            if name in names:
                raise TransferError("invalid-source", "duplicate name " + name)
            names.add(name)
            self.sources.append({"name": name, "kind": SOURCE_KINDS[kind], "style": style})
        records = np.frombuffer(data, "<f4", patches * 12, offsets[1]).reshape(patches, 12)
        if records[:, 10:12].view("<u4").any():
            raise TransferError("invalid-patch", "reserved words")
        self.position = records[:, 0:3].astype(np.float64)
        self.normal = records[:, 3:6].astype(np.float64)
        self.area = records[:, 6].astype(np.float64)
        self.albedo = records[:, 7:10].astype(np.float64)
        if not np.isfinite(records[:, :10]).all() or (self.area <= 0).any() or \
                (self.albedo < 0).any() or (self.albedo > 1).any() or \
                (np.abs(np.linalg.norm(self.normal, axis=1) - 1) > NORMAL_TOLERANCE).any():
            raise TransferError("invalid-patch")
        self.transfer = self._rows(data, offsets, 2, patches, transfer, "<If", patches)
        self.injection = self._rows(data, offsets, 4, sources, injection, "<I3f", patches)
        self.gather = self._rows(data, offsets, 6, probes, gather, "<I9f", patches)
        for p, row in enumerate(self.transfer):
            factors = row[1][:, 0] if len(row[0]) else np.zeros(0)
            if (factors < 0).any():
                raise TransferError("invalid-weight", "transfer row %d" % p)
            if float(factors.astype(np.float64).sum()) > ROW_SUM_LIMIT:
                raise TransferError("transfer-not-normalized", "row %d" % p)
        for s, row in enumerate(self.injection):
            if (row[1] < 0).any():
                raise TransferError("invalid-weight", "injection source %d" % s)
        for i, row in enumerate(self.gather):
            if len(row[0]) and float(row[1][:, 0].astype(np.float64).sum()) * Y00 > ROW_SUM_LIMIT:
                raise TransferError("gather-not-normalized", "probe %d" % i)
        with np.errstate(invalid="ignore", over="ignore"):
            self.probe_direct = np.frombuffer(
                data, "<f4", sources * probes * PROBE_TEXELS * 3, offsets[8]).reshape(
                    sources, probes, PROBE_TEXELS, 3).astype(np.float64)
        if not np.isfinite(self.probe_direct).all() or (self.probe_direct < 0).any():
            raise TransferError("invalid-weight", "probe direct light")
        if prbv is not None:
            volume = probe_volume.Volume(bytes(prbv))
            if sum(grid.count for grid in volume.grids) != probes:
                raise TransferError("topology-mismatch", "probe count")
            if prbv_topology_hash(prbv) != prbv_hash:
                raise TransferError("topology-mismatch", "PRBV topology hash")

    @staticmethod
    def _rows(data, offsets, section, count, links, fmt, patches):
        rows = np.frombuffer(data, "<u4", count + 1, offsets[section]).astype(np.int64)
        if rows[0] != 0 or rows[-1] != links or (np.diff(rows) < 0).any():
            raise TransferError("invalid-rows", "section %d" % section)
        width = struct.calcsize(fmt) // 4
        raw = np.frombuffer(data, "<u4", links * width, offsets[section + 1]).reshape(links, width)
        index = raw[:, 0].astype(np.int64)
        values = raw[:, 1:].view("<f4").astype(np.float64)
        if (index >= patches).any():
            raise TransferError("index-out-of-bounds", "section %d" % section)
        if not np.isfinite(values).all():
            raise TransferError("invalid-weight", "section %d non-finite" % section)
        result = []
        for r in range(count):
            a, b = rows[r], rows[r + 1]
            if (np.diff(index[a:b]) <= 0).any():
                raise TransferError("index-out-of-bounds", "section %d row %d order" % (section, r))
            result.append((index[a:b], values[a:b]))
        return result

    # ------------------------------------------------------------ reference
    def solve(self, scalars=None, iterations=200, bounces=None):
        """H per patch (patches, 3): Jacobi iterations from the direct light;
        `bounces` limits them (the one-bounce sensitivity reference)."""
        sigma = np.ones(self.source_count) if scalars is None else np.asarray(scalars, float)
        direct = np.zeros((self.patch_count, 3))
        for s, (index, values) in enumerate(self.injection):
            direct[index] += sigma[s] * values
        light = direct.copy()
        for _ in range(iterations if bounces is None else bounces):
            reflected = self.albedo * light
            light = direct.copy()
            for p, (index, values) in enumerate(self.transfer):
                if len(index):
                    light[p] += values[:, 0] @ reflected[index]
        return light, direct

    def probe_light(self, light, scalars=None):
        """(total, indirect) per probe and interior texel: (probes, 36, 3)."""
        sigma = np.ones(self.source_count) if scalars is None else np.asarray(scalars, float)
        directions = probe_volume.interior_directions(probe_volume.IRRADIANCE_TILE - 2).reshape(
            -1, 3)
        basis = sh_basis(directions)
        reflected = self.albedo * light
        indirect = np.zeros((self.probe_count, PROBE_TEXELS, 3))
        for i, (index, values) in enumerate(self.gather):
            if len(index):
                coefficients = values.T @ reflected[index]  # (9, 3)
                indirect[i] = basis @ coefficients
        total = indirect + np.einsum("s,sitc->itc", sigma, self.probe_direct)
        return total, indirect

    def info(self):
        rows = [np.array([row[1][:, 0].sum() if len(row[0]) else 0.0 for row in self.transfer])]
        return {"bytes": self.bytes, "sources": self.sources, "patches": self.patch_count,
                "probes": self.probe_count,
                "links": {"transfer": int(sum(len(r[0]) for r in self.transfer)),
                          "injection": int(sum(len(r[0]) for r in self.injection)),
                          "gather": int(sum(len(r[0]) for r in self.gather))},
                "transfer_row_sum": {"min": float(rows[0].min()), "max": float(rows[0].max()),
                                     "mean": float(rows[0].mean())},
                "patch_area_units2": {"min": float(self.area.min()),
                                      "max": float(self.area.max())},
                "prbv_hash": "%016x" % self.prbv_hash}


def sh_basis(directions):
    """(N, 9) real SH of order <= 2 at unit directions (x, y, z)."""
    d = np.asarray(directions, dtype=np.float64)
    x, y, z = d[..., 0], d[..., 1], d[..., 2]
    return np.stack([
        np.full_like(x, 0.282094791773878),
        0.488602511902920 * y, 0.488602511902920 * z, 0.488602511902920 * x,
        1.092548430592079 * x * y, 1.092548430592079 * y * z,
        0.315391565252520 * (3 * z * z - 1),
        1.092548430592079 * x * z, 0.546274215296040 * (x * x - y * y)], axis=-1)


# Cosine-lobe convolution per band, divided by pi: radiance SH to diffuse
# light (irradiance / pi) SH.
BAND_SCALE = np.array([1.0, 2 / 3, 2 / 3, 2 / 3, 0.25, 0.25, 0.25, 0.25, 0.25])


def read(path, prbv=None):
    return Transfer(Path(path).read_bytes(), prbv=Path(prbv).read_bytes() if prbv else None)


# ------------------------------------------------------------------ malformation

def malformations(data):
    """(name, bytes) variants the reader must reject: the corpus both readers
    (this one and the C++ reader's test) are held to."""
    header = struct.unpack_from("<IIIIIIIIIIIIQQ", data, 0)
    sources, patches, probes = header[4], header[5], header[6]
    offsets, _ = layout(section_sizes(sources, patches, probes, header[8], header[9], header[10]))

    def patched(offset, fmt, *values):
        out = bytearray(data)
        struct.pack_into(fmt, out, offset, *values)
        return bytes(out)

    first_transfer = offsets[3]
    return [
        ("truncated-header", data[:HEADER_BYTES - 1]),
        ("truncated-body", data[:-4]),
        ("trailing-bytes", data + b"\0" * 16),
        ("bad-magic", patched(0, "<I", 0x4E525453)),
        ("version", patched(4, "<I", 2)),
        ("flags", patched(12, "<I", 1)),
        ("zero-sources", patched(16, "<I", 0)),
        ("too-many-sources", patched(16, "<I", MAX_SOURCES + 1)),
        ("probe-texels", patched(28, "<I", 16)),
        ("sh-coefficients", patched(44, "<I", 4)),
        ("source-kind", patched(offsets[0], "<I", 7)),
        ("source-style", patched(offsets[0] + 4, "<i", 64)),
        ("source-name-empty", patched(offsets[0] + 16, "<B", 0)),
        ("patch-nan", patched(offsets[1], "<f", float("nan"))),
        ("patch-normal", patched(offsets[1] + 12, "<3f", 2.0, 0.0, 0.0)),
        ("patch-area", patched(offsets[1] + 24, "<f", 0.0)),
        ("patch-albedo", patched(offsets[1] + 28, "<f", 1.5)),
        ("rows-start", patched(offsets[2], "<I", 1)),
        ("rows-end", patched(offsets[2] + patches * 4, "<I", header[8] + 1)),
        ("link-index", patched(first_transfer, "<I", patches)),
        ("link-negative", patched(first_transfer + 4, "<f", -0.25)),
        ("link-nan", patched(first_transfer + 4, "<f", float("inf"))),
        ("transfer-sum", patched(first_transfer + 4, "<f", 1.5)),
        ("injection-negative", patched(offsets[5] + 4, "<f", -1.0)),
        ("gather-index", patched(offsets[7], "<I", 0xFFFFFFFF)),
        ("gather-sum", patched(offsets[7] + 4, "<f", 10.0)),
        ("probe-direct-negative", patched(offsets[8], "<f", -0.5)),
    ]


def fuzz(data, prbv=None, iterations=2000, seed=20260925):
    """Random byte mutations: the reader either rejects a variant with a
    TransferError or accepts it and its every index is in range. Returns
    counts; raises on any other exception."""
    rng = random.Random(seed)
    counts = {"rejected": 0, "accepted": 0}
    for _ in range(iterations):
        out = bytearray(data)
        for _ in range(rng.randint(1, 4)):
            position = rng.randrange(len(out))
            out[position] = rng.randrange(256)
        if rng.random() < 0.1:
            out = out[:rng.randrange(len(out))]
        try:
            transfer = Transfer(bytes(out), prbv)
        except TransferError:
            counts["rejected"] += 1
            continue
        for rows in (transfer.transfer, transfer.injection, transfer.gather):
            for index, _ in rows:
                assert (index < transfer.patch_count).all()
        counts["accepted"] += 1
    return counts


# ------------------------------------------------------------------ fixtures

def fixture_contract(prbv):
    """The producer suite's transfer, paired with the contract PRBV
    (probe_volume.fixture_contract: 16 probes, lit ones at x <= 1 behind a
    wall at x = 48). The lit side is a uniform enclosure of 24 patches with
    albedo 0.6: every form factor is area-proportional and each row sums to 1,
    so it is the furnace: one source ("Emitters", style 32) gives every patch
    direct light 0.3, and the converged light is 0.3 / (1 - 0.6) = 0.75 with
    indirect 0.45, exactly the contract seed's lit probes. Those probes gather
    every patch isotropically (indirect 0.45); the dark probes gather nothing.
    The direct light is split between two sources: "Emitters" (style 32)
    gives every patch 0.2 and "Lamp" (style 33) four patches 0.6 more, so the
    mean is 0.3 while switching the lamp changes the patches unevenly. A lit
    probe's direct light is 0.2 + 0.1 from them, isotropic."""
    positions, active = prbv_probe_positions(prbv)
    count = 24
    albedo = 0.6
    normals = np.repeat(np.array([[1, 0, 0], [-1, 0, 0], [0, 1, 0], [0, -1, 0], [0, 0, 1],
                                  [0, 0, -1]], dtype=np.float64), 4, axis=0)
    centers = 16.0 - normals * 16.0
    patches = {"position": centers, "normal": normals, "area": np.full(count, 256.0),
               "albedo": np.full((count, 3), albedo)}
    share = 1.0 / count
    transfer = [[(q, share) for q in range(count)] for _ in range(count)]
    injection = [[(p, (0.2, 0.2, 0.2)) for p in range(count)],
                 [(p, (0.6, 0.6, 0.6)) for p in range(4)]]
    gather = []
    probe_direct = np.zeros((2, len(positions), PROBE_TEXELS, 3))
    for i, position in enumerate(positions):
        if position[0] <= 32.0 + 1e-3 and active[i]:
            weight = share / Y00  # isotropic: sum_q G0 Y00 = 1
            gather.append([(q, [weight] + [0.0] * 8) for q in range(count)])
            probe_direct[0, i] = 0.2
            probe_direct[1, i] = 0.1
        else:
            gather.append([])
    return build([{"name": "Emitters", "kind": "emissive", "style": 32},
                  {"name": "Lamp", "kind": "light", "style": 33}],
                 patches, transfer, injection, gather, probe_direct, prbv_topology_hash(prbv))


def recipe(original, variant):
    """A malformation as edits of the original: a new length and the byte
    runs that differ (the C++ reader's test replays them)."""
    edits, i = [], 0
    common = min(len(original), len(variant))
    while i < common:
        if original[i] != variant[i]:
            j = i
            while j < common and original[j] != variant[j]:
                j += 1
            edits.append([i, variant[i:j].hex()])
            i = j
        else:
            i += 1
    return {"length": len(variant), "edits": edits}


# Solver reference states: scalars per source (Emitters, Lamp).
REFERENCE_STATES = {"baked": [1.0, 1.0], "lamp-off": [1.0, 0.0], "half": [0.5, 0.5],
                    "emitters-off": [0.0, 1.0]}


def write_fixtures(out):
    out = Path(out)
    out.mkdir(parents=True, exist_ok=True)
    prbv = (out.parent / "prbv" / "contract.prbv").read_bytes()
    data = fixture_contract(prbv)
    (out / "contract.rtrn").write_bytes(data)
    transfer = Transfer(data, prbv)
    lit = [i for i, row in enumerate(transfer.gather) if len(row[0])]
    states = {}
    for name, scalars in REFERENCE_STATES.items():
        light, direct = transfer.solve(scalars)
        total, indirect = transfer.probe_light(light, scalars)
        states[name] = {"scalars": scalars, "patch_light": [float(v) for v in light[:, 0]],
                        "lit_probe_total": float(total[lit, :, 0].mean()),
                        "lit_probe_indirect": float(indirect[lit, :, 0].mean())}
    one_bounce = transfer.solve(bounces=1)[0]
    variants = []
    for name, variant in malformations(data):
        try:
            Transfer(variant, prbv)
            code = None
        except TransferError as error:
            code = error.code
        variants.append({"name": name, "error": code, **recipe(data, variant)})
    summary = {"schema": "rtrn-fixtures/v1", "contract": {
        "file": "contract.rtrn", "prbv": "../prbv/contract.prbv", "bytes": len(data),
        "lit_probes": lit, "states": states,
        "one_bounce_patch_light": float(one_bounce[:, 0].mean()),
        "malformations": variants}}
    (out / "fixtures.json").write_text(json.dumps(summary, indent=1) + "\n")
    # The same corpus as plain text for the C++ tests:
    #   malformations.txt  name error length [offset:hex ...]
    #   states.txt         name scalar0 scalar1 lit_probe_total lit_probe_indirect patch0 ...
    (out / "malformations.txt").write_text("".join(
        "%s %s %d%s\n" % (v["name"], v["error"], v["length"],
                          "".join(" %d:%s" % (o, h) for o, h in v["edits"]))
        for v in variants))
    (out / "states.txt").write_text("".join(
        "%s %s %.9g %.9g %s\n" % (name, " ".join("%g" % x for x in state["scalars"]),
                                  state["lit_probe_total"], state["lit_probe_indirect"],
                                  " ".join("%.9g" % x for x in state["patch_light"]))
        for name, state in states.items()))
    if any(variant["error"] is None for variant in variants):
        raise TransferError("fixture", "a malformation was accepted")
    return {"bytes": len(data), "states": {k: {"lit_probe_total": v["lit_probe_total"],
                                               "lit_probe_indirect": v["lit_probe_indirect"]}
                                           for k, v in states.items()},
            "malformations": len(variants)}


# ------------------------------------------------------------------ CLI

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("validate", "info", "fuzz"):
        command = commands.add_parser(name)
        command.add_argument("rtrn", type=Path)
        command.add_argument("--prbv", type=Path)
        if name == "fuzz":
            command.add_argument("--iterations", type=int, default=2000)
    fixtures = commands.add_parser("fixtures")
    fixtures.add_argument("out", type=Path)
    args = parser.parse_args()
    if args.command == "fixtures":
        print(json.dumps(write_fixtures(args.out), indent=2))
        return 0
    data = args.rtrn.read_bytes()
    prbv = args.prbv.read_bytes() if args.prbv else None
    try:
        if args.command == "fuzz":
            rejected = [name for name, variant in malformations(data)
                        if not _rejects(variant, prbv)]
            counts = fuzz(data, prbv, args.iterations)
            print(json.dumps({"malformations_accepted": rejected, **counts}))
            return 1 if rejected else 0
        transfer = Transfer(data, prbv)
    except TransferError as error:
        print(json.dumps({"status": "invalid", "error": error.code, "detail": str(error)}))
        return 1
    print(json.dumps({"status": "valid", **transfer.info()} if args.command == "info"
                     else {"status": "valid"}, indent=2 if args.command == "info" else None))
    return 0


def _rejects(variant, prbv):
    try:
        Transfer(variant, prbv)
    except TransferError:
        return True
    return False


if __name__ == "__main__":
    sys.exit(main())
