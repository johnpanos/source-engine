#!/usr/bin/env python3
"""PRBV: the RFC 0011 probe volume (render.probe-volume.v1), in Python.

This module is the pipeline's writer and the independent reader and
reference sampler the engine's C++ implementation (mapcontainer/probe_volume)
and the GLSL sampler are checked against. It owns no policy beyond the
encoding; the bake (probe_volume_bake.py) owns probe values.

    python3 tools/quality/probe_volume.py validate <file.prbv>
    python3 tools/quality/probe_volume.py info <file.prbv>
    python3 tools/quality/probe_volume.py sample <file.prbv> --position X Y Z --normal X Y Z

Encoding (little-endian). One RGBA16F atlas holds every section of every grid:

  header (64 bytes)
    0  u32 magic 'PRBV'         4  u32 version (1)       8  u32 header bytes (64)
    12 u32 grid count (1..16)   16 u32 irradiance layers (1: total; 2: total,
                                   indirect)
    20 u32 irradiance tile (8)  24 u32 visibility tile (16)
    28 u32 atlas width          32 u32 atlas height      36 u32 flags (0)
    40 u64 atlas offset         48 u64 atlas bytes (width * height * 8)
    56 u64 reserved (0)
  grid records (96 bytes each)
    0  f32[3] origin (Source units)   12 f32[3] spacing   24 u32[3] dims
    36 u32 tiles per row              40 f32 max relocation (units)
    44 f32 max visibility distance    48 u32[2] irradiance layer 0 origin
    56 u32[2] irradiance layer 1 origin (0 when absent)
    64 u32[2] visibility origin       72 u32[2] state origin    80 16 zero bytes
  atlas texels, rows top-left first

Probe i = x + dims.x * (y + dims.y * z) owns tile (i % tilesPerRow,
i / tilesPerRow) of each section: an 8 x 8 irradiance tile (6 x 6
octahedral interior plus a one-texel border, rgb = irradiance / pi, the
diffuse light unit of the lightmap), a 16 x 16 visibility tile (14 x 14
interior plus border; r = mean and g = mean squared distance to the first hit,
both as a fraction of the grid's max visibility distance so they fit half
precision, hits beyond it counted at it), and one state texel
at (origin.x + i % (tilesPerRow * 16), origin.y + i / (tilesPerRow * 16)):
xyz relocation offset in units (|offset| <= max relocation per axis), w
active (1) or inside geometry (0).

Sampling (DDGI, Majercik et al. 2019): the eight probes around a biased
position, weighted by trilinear position, a smooth backface term and a
Chebyshev visibility test on each probe's distance moments; inactive probes
are skipped; the weights are renormalized.
"""

import argparse
import json
import math
import struct
import sys
from pathlib import Path

import numpy as np

MAGIC = 0x56425250  # "PRBV"
VERSION = 1
HEADER_BYTES = 64
GRID_BYTES = 96
MAX_GRIDS = 16
IRRADIANCE_TILE = 8
VISIBILITY_TILE = 16
MAX_ATLAS = 16384
MAX_PROBES = 1 << 20
LAYER_NAMES = ("total", "indirect")
# Sampling biases, in units of the grid's smallest spacing (the GLSL and C++
# samplers use the same constants).
NORMAL_BIAS = 0.1
CRUSH_THRESHOLD = 0.2


class ProbeVolumeError(ValueError):
    def __init__(self, code, detail=""):
        super().__init__("%s%s" % (code, (": " + detail) if detail else ""))
        self.code = code


# ------------------------------------------------------------------ octahedral

def oct_encode(direction):
    """Unit direction(s) (..., 3) -> octahedral [-1, 1]^2 (..., 2)."""
    d = np.asarray(direction, dtype=np.float64)
    d = d / np.sum(np.abs(d), axis=-1, keepdims=True)
    p = d[..., :2].copy()
    lower = d[..., 2] < 0
    folded = (1.0 - np.abs(p[..., ::-1])) * np.where(p >= 0, 1.0, -1.0)
    p[lower] = folded[lower]
    return p


def oct_decode(p):
    p = np.asarray(p, dtype=np.float64)
    z = 1.0 - np.abs(p[..., 0]) - np.abs(p[..., 1])
    xy = p.copy()
    lower = z < 0
    folded = (1.0 - np.abs(p[..., ::-1])) * np.where(p >= 0, 1.0, -1.0)
    xy[lower] = folded[lower]
    v = np.concatenate([xy, z[..., None]], axis=-1)
    return v / np.linalg.norm(v, axis=-1, keepdims=True)


def interior_directions(interior):
    """(interior, interior, 3) texel-center directions of a tile interior."""
    centers = (np.arange(interior) + 0.5) / interior * 2.0 - 1.0
    u, v = np.meshgrid(centers, centers)
    return oct_decode(np.stack([u, v], axis=-1))


def with_border(interior):
    """(n, n, c) interior -> (n + 2, n + 2, c) tile with the octahedral border
    that makes bilinear filtering continuous across tile edges."""
    n = interior.shape[0]
    tile = np.zeros((n + 2, n + 2) + interior.shape[2:], dtype=interior.dtype)
    tile[1:-1, 1:-1] = interior
    tile[0, 1:-1] = interior[0, ::-1]
    tile[-1, 1:-1] = interior[-1, ::-1]
    tile[1:-1, 0] = interior[::-1, 0]
    tile[1:-1, -1] = interior[::-1, -1]
    tile[0, 0] = interior[-1, -1]
    tile[0, -1] = interior[-1, 0]
    tile[-1, 0] = interior[0, -1]
    tile[-1, -1] = interior[0, 0]
    return tile


# ------------------------------------------------------------------ layout

def grid_layout(dims, tiles_per_row, layers, y0):
    """Section origins of one grid stacked from atlas row y0; returns
    (sections, rows used)."""
    count = int(np.prod(dims))
    tile_rows = (count + tiles_per_row - 1) // tiles_per_row
    width = tiles_per_row * VISIBILITY_TILE
    sections = {}
    y = y0
    for layer in range(layers):
        sections["irradiance%d" % layer] = (0, y)
        y += tile_rows * IRRADIANCE_TILE
    sections["visibility"] = (0, y)
    y += tile_rows * VISIBILITY_TILE
    sections["state"] = (0, y)
    y += (count + width - 1) // width
    return sections, y - y0


def probe_index(dims, x, y, z):
    return x + dims[0] * (y + dims[1] * z)


# ------------------------------------------------------------------ writer

def build(grids, layers):
    """PRBV bytes from grids: dicts with origin, spacing, dims, max_relocation,
    max_distance, irradiance (layers, N, 6, 6, 3), visibility (N, 14, 14, 2),
    offsets (N, 3), active (N,)."""
    if not 1 <= len(grids) <= MAX_GRIDS or layers not in (1, 2):
        raise ProbeVolumeError("invalid-counts")
    tiles = []
    layouts = []
    y = 0
    width = 0
    for grid in grids:
        count = int(np.prod(grid["dims"]))
        per_row = max(1, int(math.ceil(math.sqrt(count))))
        sections, rows = grid_layout(grid["dims"], per_row, layers, y)
        layouts.append((per_row, sections))
        y += rows
        width = max(width, per_row * VISIBILITY_TILE)
    height = y
    if width > MAX_ATLAS or height > MAX_ATLAS:
        raise ProbeVolumeError("atlas-too-large", "%dx%d" % (width, height))
    atlas = np.zeros((height, width, 4), dtype=np.float32)
    records = b""
    for grid, (per_row, sections) in zip(grids, layouts):
        dims = [int(v) for v in grid["dims"]]
        count = int(np.prod(dims))
        irradiance = np.asarray(grid["irradiance"], dtype=np.float32)
        visibility = np.asarray(grid["visibility"], dtype=np.float32)
        if irradiance.shape != (layers, count, IRRADIANCE_TILE - 2, IRRADIANCE_TILE - 2, 3) or \
                visibility.shape != (count, VISIBILITY_TILE - 2, VISIBILITY_TILE - 2, 2):
            raise ProbeVolumeError("invalid-probe-data")
        for i in range(count):
            tx, ty = i % per_row, i // per_row
            for layer in range(layers):
                ox, oy = sections["irradiance%d" % layer]
                tile = with_border(irradiance[layer, i])
                atlas[oy + ty * IRRADIANCE_TILE:oy + (ty + 1) * IRRADIANCE_TILE,
                      ox + tx * IRRADIANCE_TILE:ox + (tx + 1) * IRRADIANCE_TILE, :3] = tile
                atlas[oy + ty * IRRADIANCE_TILE:oy + (ty + 1) * IRRADIANCE_TILE,
                      ox + tx * IRRADIANCE_TILE:ox + (tx + 1) * IRRADIANCE_TILE, 3] = 1.0
            vx, vy = sections["visibility"]
            # Stored as fractions of the max distance (mean, mean squared).
            scale = np.array([1.0 / grid["max_distance"], 1.0 / grid["max_distance"] ** 2],
                             dtype=np.float64)
            atlas[vy + ty * VISIBILITY_TILE:vy + (ty + 1) * VISIBILITY_TILE,
                  vx + tx * VISIBILITY_TILE:vx + (tx + 1) * VISIBILITY_TILE, :2] = \
                with_border((np.asarray(visibility[i], np.float64) * scale).astype(np.float32))
            sx, sy = sections["state"]
            state_row = per_row * VISIBILITY_TILE
            atlas[sy + i // state_row, sx + i % state_row] = (
                *np.asarray(grid["offsets"][i], dtype=np.float32), float(grid["active"][i]))
        layer1 = sections.get("irradiance1", (0, 0))
        records += struct.pack(
            "<3f3f3IIff2I2I2I2I16x", *grid["origin"], *grid["spacing"], *dims, per_row,
            grid["max_relocation"], grid["max_distance"], *sections["irradiance0"], *layer1,
            *sections["visibility"], *sections["state"])
    half = atlas.astype("<f2")
    if not np.isfinite(half).all():
        raise ProbeVolumeError("non-finite-texels", "values overflow half precision")
    offset = HEADER_BYTES + GRID_BYTES * len(grids)
    offset = (offset + 15) & ~15
    header = struct.pack("<IIIIIIIIIIQQQ", MAGIC, VERSION, HEADER_BYTES, len(grids), layers,
                         IRRADIANCE_TILE, VISIBILITY_TILE, width, height, 0, offset,
                         width * height * 8, 0)
    blob = header + records
    return blob + b"\0" * (offset - len(blob)) + half.tobytes()


# ------------------------------------------------------------------ reader

class Grid:
    def __init__(self, record, atlas, layers):
        values = struct.unpack("<3f3f3IIff2I2I2I2I16x", record)
        self.origin = np.array(values[0:3], dtype=np.float64)
        self.spacing = np.array(values[3:6], dtype=np.float64)
        self.dims = [int(v) for v in values[6:9]]
        self.tiles_per_row = int(values[9])
        self.max_relocation = float(values[10])
        self.max_distance = float(values[11])
        self.irradiance_origin = [tuple(values[12:14]), tuple(values[14:16])][:layers]
        self.visibility_origin = tuple(values[16:18])
        self.state_origin = tuple(values[18:20])
        self.atlas = atlas
        self.count = int(np.prod(self.dims))

    def state(self, i):
        row = self.tiles_per_row * VISIBILITY_TILE
        x, y = self.state_origin
        return self.atlas[y + i // row, x + i % row]

    def tile_origin(self, origin, tile, i):
        return (origin[0] + (i % self.tiles_per_row) * tile,
                origin[1] + (i // self.tiles_per_row) * tile)


def bilinear(atlas, x, y):
    """Sample atlas at continuous texel coordinates (centers at i + 0.5)."""
    x -= 0.5
    y -= 0.5
    x0, y0 = int(math.floor(x)), int(math.floor(y))
    fx, fy = x - x0, y - y0
    height, width = atlas.shape[:2]

    def texel(xi, yi):
        return atlas[min(max(yi, 0), height - 1), min(max(xi, 0), width - 1)]
    top = texel(x0, y0) * (1 - fx) + texel(x0 + 1, y0) * fx
    bottom = texel(x0, y0 + 1) * (1 - fx) + texel(x0 + 1, y0 + 1) * fx
    return top * (1 - fy) + bottom * fy


def tile_sample(atlas, tile_origin, tile, direction):
    interior = tile - 2
    p = oct_encode(np.asarray(direction, dtype=np.float64))
    x = tile_origin[0] + 1 + (p[0] * 0.5 + 0.5) * interior
    y = tile_origin[1] + 1 + (p[1] * 0.5 + 0.5) * interior
    return bilinear(atlas, x, y)


class Volume:
    def __init__(self, data):
        self.data = data = bytes(data)
        if len(data) < HEADER_BYTES:
            raise ProbeVolumeError("truncated")
        (magic, version, header, grids, layers, irradiance_tile, visibility_tile, width, height,
         flags, offset, size, reserved) = struct.unpack_from("<IIIIIIIIIIQQQ", data)
        if magic != MAGIC:
            raise ProbeVolumeError("bad-magic")
        if version != VERSION or header != HEADER_BYTES or flags or reserved:
            raise ProbeVolumeError("unsupported-version")
        if not 1 <= grids <= MAX_GRIDS or layers not in (1, 2) or \
                irradiance_tile != IRRADIANCE_TILE or visibility_tile != VISIBILITY_TILE:
            raise ProbeVolumeError("invalid-counts")
        if not 1 <= width <= MAX_ATLAS or not 1 <= height <= MAX_ATLAS or \
                size != width * height * 8 or offset % 16 or \
                offset < HEADER_BYTES + GRID_BYTES * grids or offset + size != len(data):
            raise ProbeVolumeError("invalid-atlas")
        self.layers, self.width, self.height = layers, width, height
        self.atlas = np.frombuffer(data, "<f2", width * height * 4, offset).reshape(
            height, width, 4).astype(np.float32)
        self.grids = []
        for index in range(grids):
            record = data[HEADER_BYTES + index * GRID_BYTES:HEADER_BYTES + (index + 1) * GRID_BYTES]
            if record[80:96] != b"\0" * 16:
                raise ProbeVolumeError("invalid-grid", "reserved bytes")
            grid = Grid(record, self.atlas, layers)
            self.validate_grid(index, grid)
            self.grids.append(grid)

    def validate_grid(self, index, grid):
        where = "grid %d" % index
        if not (np.isfinite(grid.origin).all() and np.isfinite(grid.spacing).all() and
                (grid.spacing > 0).all()):
            raise ProbeVolumeError("invalid-grid", where + " origin/spacing")
        if min(grid.dims) < 2 or grid.count > MAX_PROBES or grid.tiles_per_row < 1:
            raise ProbeVolumeError("invalid-grid", where + " dims")
        if not (0 <= grid.max_relocation <= 0.5 * float(grid.spacing.min())) or \
                not (0 < grid.max_distance < 1e6):
            raise ProbeVolumeError("invalid-grid", where + " relocation/distance bounds")
        tile_rows = (grid.count + grid.tiles_per_row - 1) // grid.tiles_per_row
        rects = [(origin, grid.tiles_per_row * IRRADIANCE_TILE, tile_rows * IRRADIANCE_TILE)
                 for origin in grid.irradiance_origin]
        rects.append((grid.visibility_origin, grid.tiles_per_row * VISIBILITY_TILE,
                      tile_rows * VISIBILITY_TILE))
        state_row = grid.tiles_per_row * VISIBILITY_TILE
        rects.append((grid.state_origin, min(state_row, grid.count),
                      (grid.count + state_row - 1) // state_row))
        for (x, y), w, h in rects:
            if x + w > self.width or y + h > self.height:
                raise ProbeVolumeError("section-outside-atlas", where)
        for i in range(grid.count):
            state = grid.state(i)
            if state[3] not in (0.0, 1.0):
                raise ProbeVolumeError("invalid-state", "%s probe %d active flag" % (where, i))
            if np.max(np.abs(state[:3])) > grid.max_relocation * (1 + 1e-3) + 1e-3:
                raise ProbeVolumeError("relocation-out-of-bounds", "%s probe %d" % (where, i))
        for (x, y), w, h in rects[:-2]:
            block = self.atlas[y:y + h, x:x + w, :3]
            if not np.isfinite(block).all() or block.min() < 0:
                raise ProbeVolumeError("invalid-irradiance", where)
        (x, y), w, h = rects[-2]
        moments = self.atlas[y:y + h, x:x + w, :2]
        if not np.isfinite(moments).all() or moments.min() < 0:
            raise ProbeVolumeError("invalid-visibility", where)

    # -------------------------------------------------------------- sampling
    def probe_position(self, grid, index3):
        i = probe_index(grid.dims, *index3)
        return grid.origin + np.asarray(index3) * grid.spacing + grid.state(i)[:3], i

    def sample(self, position, normal, layer=0, visibility=True, grid_index=None):
        """Irradiance / pi at a surface point; None outside every grid."""
        position = np.asarray(position, dtype=np.float64)
        normal = np.asarray(normal, dtype=np.float64)
        normal = normal / np.linalg.norm(normal)
        for index, grid in enumerate(self.grids):
            if grid_index is not None and index != grid_index:
                continue
            biased = position + normal * NORMAL_BIAS * float(grid.spacing.min())
            g = (biased - grid.origin) / grid.spacing
            if (g < 0).any() or (g > np.asarray(grid.dims) - 1).any():
                continue
            return self._sample_grid(grid, position, biased, normal, g, layer, visibility)
        return None

    def _sample_grid(self, grid, position, biased, normal, g, layer, visibility):
        base = np.minimum(np.floor(g).astype(int), np.asarray(grid.dims) - 2)
        alpha = g - base
        total = np.zeros(3)
        weights = 0.0
        for corner in range(8):
            offset = np.array([(corner >> k) & 1 for k in range(3)])
            probe, i = self.probe_position(grid, base + offset)
            if grid.state(i)[3] < 0.5:
                continue
            trilinear = float(np.prod(np.where(offset, alpha, 1 - alpha)))
            to_probe = probe - position
            length = np.linalg.norm(to_probe)
            direction = to_probe / length if length > 1e-9 else normal
            weight = ((float(np.dot(direction, normal)) + 1) * 0.5) ** 2 + 0.2
            if visibility:
                probe_to_point = biased - probe
                distance = float(np.linalg.norm(probe_to_point))
                if distance > 1e-9:
                    moments = tile_sample(grid.atlas, grid.tile_origin(
                        grid.visibility_origin, VISIBILITY_TILE, i), VISIBILITY_TILE,
                        probe_to_point / distance)
                    mean = float(moments[0]) * grid.max_distance
                    mean2 = float(moments[1]) * grid.max_distance ** 2
                    if distance > mean:
                        variance = abs(mean * mean - mean2)
                        chebyshev = variance / (variance + (distance - mean) ** 2)
                        weight *= max(chebyshev ** 3, 0.0)
            weight = max(weight, 1e-6)
            if weight < CRUSH_THRESHOLD:
                weight *= weight * weight / (CRUSH_THRESHOLD * CRUSH_THRESHOLD)
            weight *= trilinear
            irradiance = tile_sample(grid.atlas, grid.tile_origin(
                grid.irradiance_origin[layer], IRRADIANCE_TILE, i), IRRADIANCE_TILE, normal)
            total += weight * irradiance[:3]
            weights += weight
        return total / weights if weights > 0 else np.zeros(3)

    def ambient_cube(self, position, layer=0, visibility=True):
        """Source's six-axis ambient cube (+X -X +Y -Y +Z -Z) at a point."""
        axes = ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1))
        return [self.sample(position, axis, layer, visibility) for axis in axes]


def read(path):
    return Volume(Path(path).read_bytes())


# ------------------------------------------------------------------ fixtures

def uniform_visibility(count, distance):
    """Moments of a probe that sees every direction hit at `distance`."""
    visibility = np.zeros((count, VISIBILITY_TILE - 2, VISIBILITY_TILE - 2, 2), np.float32)
    visibility[..., 0] = distance
    visibility[..., 1] = distance * distance
    return visibility


def fixture_leak():
    """Four probes along x (32-unit spacing) with a wall at x = 48: probes 0-1
    are lit (irradiance 1), 2-3 dark (0), and each sees the wall at its true
    distance toward it. Visibility must keep the lit probes out of the dark side."""
    dims = [4, 2, 2]
    count = int(np.prod(dims))
    spacing = 32.0
    wall = 48.0
    irradiance = np.zeros((1, count, 6, 6, 3), np.float32)
    visibility = uniform_visibility(count, 200.0)
    directions = interior_directions(VISIBILITY_TILE - 2)
    for z in range(2):
        for y in range(2):
            for x in range(4):
                i = probe_index(dims, x, y, z)
                px = x * spacing
                irradiance[0, i] = 1.0 if x <= 1 else 0.0
                toward = directions[..., 0] if x <= 1 else -directions[..., 0]
                gap = abs(wall - px)
                # Distance along each texel direction to the wall plane.
                hit = np.where(toward > 1e-3, gap / np.maximum(toward, 1e-3), 200.0)
                hit = np.minimum(hit, 200.0)
                visibility[i, ..., 0] = hit
                visibility[i, ..., 1] = hit * hit
    grid = {"origin": [0.0, 0.0, 0.0], "spacing": [spacing] * 3, "dims": dims,
            "max_relocation": 8.0, "max_distance": 200.0, "irradiance": irradiance,
            "visibility": visibility, "offsets": np.zeros((count, 3)),
            "active": np.ones(count)}
    return build([grid], 1)


def fixture_analytic():
    """3 x 3 x 3 probes, irradiance(n) = 0.3 + 0.2 n_z (layer 0) and half that
    (layer 1, indirect), one relocated probe and one inactive probe."""
    dims = [3, 3, 3]
    count = int(np.prod(dims))
    directions = interior_directions(IRRADIANCE_TILE - 2)
    tile = (0.3 + 0.2 * directions[..., 2])[..., None].repeat(3, axis=-1)
    irradiance = np.stack([np.broadcast_to(tile, (count,) + tile.shape),
                           np.broadcast_to(tile * 0.5, (count,) + tile.shape)]).astype(np.float32)
    offsets = np.zeros((count, 3))
    offsets[probe_index(dims, 1, 1, 1)] = (3.0, -2.0, 1.0)
    active = np.ones(count)
    active[probe_index(dims, 2, 2, 2)] = 0
    grid = {"origin": [-64.0, -64.0, 0.0], "spacing": [64.0, 64.0, 48.0], "dims": dims,
            "max_relocation": 16.0, "max_distance": 300.0, "irradiance": irradiance,
            "visibility": uniform_visibility(count, 300.0), "offsets": offsets,
            "active": active}
    return build([grid], 2)


def write_fixtures(out):
    """Checked-in PRBV conformance fixtures and the Python sampler's values."""
    out.mkdir(parents=True, exist_ok=True)
    files = {"leak.prbv": fixture_leak(), "analytic.prbv": fixture_analytic()}
    samples = []
    for name, data in files.items():
        (out / name).write_bytes(data)
        volume = Volume(data)
        points = ([((56.0, 16.0, 16.0), (1.0, 0.0, 0.0)), ((40.0, 16.0, 16.0), (-1.0, 0.0, 0.0)),
                   ((20.0, 10.0, 20.0), (0.0, 0.0, 1.0)), ((70.0, 20.0, 12.0), (0.0, 1.0, 0.0))]
                  if name == "leak.prbv" else
                  [((0.0, 0.0, 30.0), (0.0, 0.0, 1.0)), ((10.0, -20.0, 40.0), (0.0, 0.0, -1.0)),
                   ((30.0, 30.0, 70.0), (0.6, 0.0, 0.8)), ((-50.0, 5.0, 10.0), (1.0, 0.0, 0.0)),
                   ((60.0, 60.0, 90.0), (0.0, 0.0, 1.0))])
        for position, normal in points:
            for layer in range(volume.layers):
                for visibility in (True, False):
                    value = volume.sample(position, normal, layer, visibility)
                    samples.append({"file": name, "position": list(position),
                                    "normal": list(normal), "layer": layer,
                                    "visibility": visibility,
                                    "expected": None if value is None else
                                    [float(v) for v in value]})
    # The same samples as whitespace-separated text for the C++ suite:
    # file px py pz nx ny nz layer visibility (1/0) then r g b, or "none".
    lines = ["# prbv-samples/v1: generated by tools/quality/probe_volume.py fixture"]
    for sample in samples:
        values = sample["position"] + sample["normal"]
        lines.append(" ".join([sample["file"]] + ["%.9g" % v for v in values] +
                              [str(sample["layer"]), "1" if sample["visibility"] else "0"] +
                              (["%.9g" % v for v in sample["expected"]]
                               if sample["expected"] is not None else ["none"])))
    (out / "samples.txt").write_text("\n".join(lines) + "\n")
    (out / "samples.json").write_text(json.dumps({
        "schema": "prbv-samples/v1", "generator": "tools/quality/probe_volume.py fixture",
        "samples": samples}, indent=2) + "\n")
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("validate", "info"):
        commands.add_parser(name).add_argument("file", type=Path)
    fixture = commands.add_parser("fixture", help="write the PRBV conformance fixtures")
    fixture.add_argument("--out", type=Path,
                         default=Path(__file__).resolve().parents[2] / "quality/fixtures/gi/prbv")
    sample = commands.add_parser("sample")
    sample.add_argument("file", type=Path)
    sample.add_argument("--position", type=float, nargs=3, required=True)
    sample.add_argument("--normal", type=float, nargs=3, required=True)
    sample.add_argument("--layer", type=int, default=0)
    sample.add_argument("--no-visibility", action="store_true")
    args = parser.parse_args()
    if args.command == "fixture":
        write_fixtures(args.out)
        print("wrote PRBV fixtures to %s" % args.out)
        return 0
    try:
        volume = read(args.file)
    except ProbeVolumeError as error:
        print("PRBV invalid: %s" % error)
        return 1
    if args.command == "validate":
        print("PRBV valid: %d grid(s), %d layer(s), atlas %dx%d" % (
            len(volume.grids), volume.layers, volume.width, volume.height))
    elif args.command == "info":
        print(json.dumps({"layers": [LAYER_NAMES[i] for i in range(volume.layers)],
                          "atlas": [volume.width, volume.height],
                          "grids": [{"origin": g.origin.tolist(), "spacing": g.spacing.tolist(),
                                     "dims": g.dims, "probes": g.count,
                                     "active": int(sum(g.state(i)[3] for i in range(g.count))),
                                     "max_relocation": g.max_relocation,
                                     "max_distance": g.max_distance}
                                    for g in volume.grids]}, indent=2))
    else:
        value = volume.sample(args.position, args.normal, args.layer, not args.no_visibility)
        print(json.dumps(None if value is None else list(map(float, value))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
