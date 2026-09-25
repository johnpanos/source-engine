"""A map's reflection probes: RPRB encoding, blending and placement.

RFC 0008 names `RPRB` as the map's reflection probes; this module owns its
encoding (the writer `build` and the independent reader `read`), the blend
the shaders evaluate per pixel (`blend_weights`, `shade`), and the automatic
placement the Blender renderer runs (`place`). `reflection_probe.py` owns
the per-probe math: the equirect GGX chain, the proxy-box fit and the
parallax-corrected lookup. `public/mapcontainer/reflection_probes.h` must
accept exactly what `build` writes and reject what `read` rejects.

Why (design record RFC/0007-progress.md, R50-PARALLAX): one direction-only
probe per map reflects the room correctly only at its capture point
(3kliksphilip, "Advanced Reflections in CS:GO... and for Source 2?", 2019:
Source 1 cubemaps do not move with the viewer, and Source 1 switches to the
nearest cubemap instead of blending). Lagarde and Zanuttini ("Local
Image-based Lighting With Parallax-corrected Cubemap", SIGGRAPH 2012) give
each cubemap a proxy box and an influence volume, and blend the cubemaps
whose influence contains the point: 0% at an influence boundary, full weight
inside, a smaller volume taking precedence over a larger one that contains
it, normalized. Frostbite (Lagarde and de Rousiers, SIGGRAPH 2014 notes,
Listing F.1) fades at the influence boundary with a smoothstep.

RPRB v1, little-endian:

  header, 64 bytes
    0  u32 magic "RPRB"          4  u32 version (1)
    8  u32 probe count P (1..16) 12 u32 mip count M (1..12)
   16  u32 mip-0 width W0 (power of two, 8..2048, W0 >> (M - 1) >= 4)
   20  u32 atlas width (2 W0)    24 u32 atlas height (P W0 / 2)
   28  u32 record bytes (80)     32 u64 atlas offset (64 + 80 P, 16-aligned)
   40  u64 atlas bytes (width * height * 8)
   48  u32 prefilter version (1: `reflection_probe.mip_chain`)
   52  u32 flags (0)             56 u32 global probe index   60 u32 0
  P records, 80 bytes, floats in Source units
    0 capture xyz     12 fade distance (> 0)
   16 parallax box min xyz        28 parallax box max xyz
   40 influence box min xyz       52 influence box max xyz
   64 u32 rank (a permutation of 0..P-1; the global probe ranks last)
   68 u32 flags (bit 0: global)   72 u32 band row (index * W0 / 2)   76 u32 0
  atlas: RGBA16F texels, rows top first. Probe i's chain occupies rows
    [i W0/2, (i + 1) W0/2): mip l is W_l x W_l/2 at x = 2 W0 (1 - 2^-l), the
    legacy band's layout; alpha 1 inside a mip, texels outside mips all 0.

Constraints: finite values under 1e6 in magnitude; box and influence min < max
per axis; the capture inside its parallax box; exactly one global probe.

RPRB v2 (R50-RELIGHT) is v1 plus relight bands, header flag bit 0: each
probe also carries a G-buffer of what its capture sees, in two more bands of
the same mip layout after all radiance bands. Probe i's albedo band starts at
row (P + 2 i) W0/2: RGB the diffuse albedo (Cycles' Diffuse Color pass, 0..1)
and alpha the ray distance from the capture point (Source units, 0 < d <=
MAX_DISTANCE); its normal band follows at + W0/2: RGB the world-space shading
normal (Cycles' Normal pass, each component in [-1, 1]), alpha 1. Mips are
box-filtered (normals renormalized). The atlas is then 3 P W0/2 rows tall and
the record's last u32 is the albedo band's row (0 in v1). A v1 payload never
has the flag.

Relighting (McAuley, "Rendering the World of Far Cry 4", GDC 2015: a G-buffer
cubemap relit at runtime; here with the distance too, so the relit point can
be shadowed and lit by point lights): at the lookup direction and lod, the
point the capture saw is capture + direction * distance, and the probe's
radiance gains albedo * change(point, normal), where `change` is the scene's
diffuse-light change since the bake in the lightmap's unit (irradiance / pi:
the producer's change volume and the unbaked lights, world_pbr.frag),
clamped at zero. It is exact in the baked state (change 0) and for a mirror
reflection of a Lambertian surface; for a rough lookup the change is taken at
the lobe's centre.

Blending (world_pbr_probe.glsl mirrors `blend_weights`): visit probes by rank
(influence volume ascending, the global probe last). A probe's weight is 1
inside its influence box, falling to 0 at `fade` outside it (smoothstep),
times a facing term that drops a probe whose capture point lies behind the
shaded surface (smoothstep of the cosine over [-FACING_EDGE, FACING_EDGE]),
so a room's probe does not reflect onto the far side of its wall. The global
probe's weight is 1. Each takes weight * the weight still unassigned. The two
largest shares are sampled, each less the third largest share, renormalized:
continuous wherever the ranking of shares changes (a cap of four in Lagarde
2012 pops when a fifth volume overlaps).
"""

import json
import math
import os
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import reflection_probe  # noqa: E402

MAGIC = 0x42525052  # "RPRB"
VERSION = 1
RELIGHT_VERSION = 2
FLAG_RELIGHT = 1  # header flags (v2 only): relight bands present
# Relight bands: ray distances (Source units) stay within half range; sky
# texels (Cycles' 1e10) are clamped here, where their albedo is 0.
MAX_DISTANCE = 60000.0
NORMAL_LIMIT = 1.001
HEADER_BYTES = 64
RECORD_BYTES = 80
MAX_PROBES = 16
MAX_MIPS = 12
MIN_WIDTH, MAX_WIDTH = 8, 2048
PREFILTER_VERSION = 1
FLAG_GLOBAL = 1
MAX_COORDINATE = 1.0e6
SOURCE_UNITS_PER_METER = 39.37007874015748
FACING_EDGE = 0.1
# The GPU form (`gpu_texture`, mapcontainer::WriteReflectionProbeTexture):
# one RGBA16F texture, 2 W0 wide. Row 0: texel 0 = (count, mips, W0,
# GPU_MARKER), texel 1 = (mode, 0, 0, 0). Row 1 + rank: the probe of that
# rank as five vec4 - capture.xyz, fade | box min.xyz, band row | box
# max.xyz, global | influence min.xyz, 0 | influence max.xyz, 0 - each
# stored as two texels, hi = half(v) and lo = half(v - hi), so positions
# keep ~0.01-unit precision. Rows 1 + count on: the RPRB atlas.
# GPU_MARKER tells this texture from an LMAP page (whose texels are never
# negative) wherever a shader may receive either.
GPU_MARKER = -3.0
TABLE_VEC4 = 5
# Modes (`mat_reflection_probes`): 0 off, 1 blended and parallax-corrected,
# 2 the nearest capture alone (Source 1's switch, the blend check's
# negative control), 3 blended but direction-only (the parallax check's).
# MODE_WEIGHTS (+4) shows the selection's weights instead of radiance: each
# probe's WEIGHT_PALETTE colour by rank, so a capture of a mirror floor maps
# the blend (reflection_runtime.py walk).
MODE_OFF, MODE_BLEND, MODE_NEAREST, MODE_DIRECTION = 0, 1, 2, 3
MODE_WEIGHTS = 4
MODES = (0, 1, 2, 3, 5, 6, 7)
WEIGHT_PALETTE = ((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, 1.0, 0.0), (1.0, 1.0, 0.0),
                  (1.0, 0.0, 1.0), (0.0, 1.0, 1.0))


class RprbError(ValueError):
    """A rejected payload; `code` is the C++ ReflectionProbesError name."""

    def __init__(self, code, detail=""):
        super().__init__(code + (": " + detail if detail else ""))
        self.code = code


# ------------------------------------------------------------- encoding

def atlas_layout(count, width, relight=False):
    return 2 * width, count * (width // 2) * (3 if relight else 1)


def relight_rows(count, width, index):
    """The first rows of probe `index`'s albedo and normal bands (v2)."""
    band = width // 2
    return (count + 2 * index) * band, (count + 2 * index + 1) * band


def relight_chain(albedo, normal, distance, minimum_width=4):
    """A probe's relight bands from its equirect G-buffer (meters): box-
    filtered chains of (albedo rgb, distance) and of the unit normal."""
    albedo_distance = np.concatenate((np.clip(albedo, 0.0, 1.0), distance[..., None]), axis=2)
    first = reflection_probe.box_pyramid(albedo_distance, minimum_width)
    second = reflection_probe.box_pyramid(normal, minimum_width)
    normals = []
    for level in second:
        length = np.linalg.norm(level, axis=2, keepdims=True)
        normals.append(np.where(length > 1e-6, level / np.maximum(length, 1e-6),
                                (0.0, 0.0, 1.0)))
    return first, normals


def ranks(probes):
    """Rank order: influence volume ascending (a small volume inside a
    larger one is consulted first), the global probe last."""
    def volume(probe):
        return float(np.prod(np.asarray(probe["influence_max"]) -
                             np.asarray(probe["influence_min"])))
    order = sorted(range(len(probes)),
                   key=lambda i: (bool(probes[i].get("global")), volume(probes[i]), i))
    rank = [0] * len(probes)
    for position, index in enumerate(order):
        rank[index] = position
    return rank


def build(probes, chains, scale=SOURCE_UNITS_PER_METER, relight=None):
    """RPRB bytes. `probes`: dicts with capture, box_min, box_max,
    influence_min, influence_max (meters), fade (meters), global; `chains`:
    each probe's `reflection_probe.mip_chain` (linear, already exposure
    scaled). Exactly one probe is global. `relight` (v2): each probe's
    `relight_chain` (distances in meters), mip for mip like its chain."""
    count = len(probes)
    if not 1 <= count <= MAX_PROBES or len(chains) != count:
        raise RprbError("InvalidCounts", "1..%d probes with one chain each" % MAX_PROBES)
    if sum(bool(probe.get("global")) for probe in probes) != 1:
        raise RprbError("InvalidGlobal", "exactly one probe must be global")
    mips = len(chains[0])
    width = chains[0][0].shape[1]
    for chain in chains:
        if len(chain) != mips or chain[0].shape[1] != width:
            raise RprbError("InvalidAtlas", "every probe needs the same mip count and width")
        for level, mip in enumerate(chain):
            if mip.shape[:2] != (max(width >> level, 1) // 2, width >> level):
                raise RprbError("InvalidAtlas", "mip %d has the wrong size" % level)
    if relight is not None:
        if len(relight) != count:
            raise RprbError("InvalidCounts", "one relight chain pair per probe")
        for first, second in relight:
            if len(first) != mips or len(second) != mips or any(
                    a.shape[:2] != b.shape[:2] or a.shape[:2] != chains[0][level].shape[:2]
                    for level, (a, b) in enumerate(zip(first, second))):
                raise RprbError("InvalidAtlas", "relight bands must match the radiance mips")
    atlas_width, atlas_height = atlas_layout(count, width, relight is not None)
    atlas = np.zeros((atlas_height, atlas_width, 4), dtype=np.float64)
    band = width // 2

    def place(top, mips_of_band, alpha=None):
        x = 0
        for mip in mips_of_band:
            rows, columns = mip.shape[:2]
            atlas[top:top + rows, x:x + columns, :mip.shape[2]] = mip
            if alpha is not None:
                atlas[top:top + rows, x:x + columns, 3] = alpha
            x += columns

    for index, chain in enumerate(chains):
        place(index * band, [mip[..., :3] for mip in chain], 1.0)
    if relight is not None:
        for index, (first, second) in enumerate(relight):
            albedo_row, normal_row = relight_rows(count, width, index)
            place(albedo_row, [np.concatenate(
                (level[..., :3], np.clip(level[..., 3:4] * scale, 1.0 / 16.0, MAX_DISTANCE)),
                axis=2) for level in first])
            place(normal_row, second, 1.0)
    rank = ranks(probes)
    records = b""
    for index, probe in enumerate(probes):
        values = [np.asarray(probe[key], dtype=np.float64) * scale
                  for key in ("capture", "box_min", "box_max", "influence_min", "influence_max")]
        records += struct.pack("<3ff3f3f3f3fIIII", *values[0], probe["fade"] * scale,
                               *values[1], *values[2], *values[3], *values[4], rank[index],
                               FLAG_GLOBAL if probe.get("global") else 0, index * band,
                               relight_rows(count, width, index)[0] if relight is not None
                               else 0)
    atlas_offset = (HEADER_BYTES + RECORD_BYTES * count + 15) & ~15
    texels = atlas.astype("<f2")
    global_index = next(i for i, probe in enumerate(probes) if probe.get("global"))
    header = struct.pack("<IIIIIIIIQQIIII", MAGIC,
                         VERSION if relight is None else RELIGHT_VERSION, count, mips, width,
                         atlas_width, atlas_height, RECORD_BYTES, atlas_offset, texels.nbytes,
                         PREFILTER_VERSION, 0 if relight is None else FLAG_RELIGHT,
                         global_index, 0)
    data = header + records
    data += b"\0" * (atlas_offset - len(data)) + texels.tobytes()
    read(data)  # the writer never emits what the reader rejects
    return data


# The shared malformation corpus: (offset, struct format, value, error) edits
# of a valid two-probe payload. The C++ reader's suite applies the same edits
# (quality/fixtures/reflection/rprb/malformations.txt) and must report the
# same error for each.
MALFORMATIONS = (
    (0, "<I", 0x12345678, "BadMagic"),
    (4, "<I", 3, "UnsupportedVersion"),
    (4, "<I", 2, "UnsupportedVersion"),        # v2 without the relight flag
    (8, "<I", 0, "InvalidCounts"),
    (8, "<I", 17, "InvalidCounts"),
    (16, "<I", 48, "InvalidAtlas"),            # width not a power of two
    (28, "<I", 96, "InvalidAtlas"),            # record size
    (32, "<Q", 64 + 160 + 16, "InvalidSections"),
    (48, "<I", 2, "UnsupportedVersion"),       # prefilter version
    (52, "<I", 1, "UnsupportedVersion"),       # flags
    (64 + 12, "<f", 0.0, "InvalidRecord"),     # fade
    (64 + 0, "<f", float("nan"), "InvalidRecord"),
    (64 + 0, "<f", -500.0, "InvalidRecord"),   # capture outside its box
    (64 + 28, "<f", -10.0, "InvalidRecord"),   # box max below min
    (64 + 40, "<f", 1.0e7, "InvalidRecord"),   # beyond the coordinate bound
    (64 + 64, "<I", 1, "InvalidRanks"),
    (64 + 68, "<I", 1, "InvalidGlobal"),       # two global probes
    (64 + 68, "<I", 2, "InvalidRecord"),       # unknown flag
    (64 + 72, "<I", 7, "InvalidRecord"),       # band row
    (64 + 80 + 64, "<I", 0, "InvalidRanks"),   # duplicate rank
    (56, "<I", 0, "InvalidGlobal"),            # header names a non-global probe
)

# Edits of the valid relight (v2) payload, which has two probes of width 32
# (rows: radiance 0..31, probe 0 albedo 32..47, normal 48..63, probe 1 albedo
# 64..79, normal 80..95; atlas offset 224, rows of 64 texels of 8 bytes).
RELIGHT_ATLAS = 224
RELIGHT_ROW = 64 * 8
RELIGHT_MALFORMATIONS = (
    (4, "<I", 1, "UnsupportedVersion"),        # the flag in a v1 header
    (52, "<I", 3, "UnsupportedVersion"),       # an unknown header flag
    (24, "<I", 32, "InvalidAtlas"),            # the v1 height with the flag
    (64 + 76, "<I", 48, "InvalidRecord"),      # albedo band row
    (64 + 80 + 76, "<I", 0, "InvalidRecord"),  # a v2 probe without its bands
    (RELIGHT_ATLAS + 40 * RELIGHT_ROW + 8 * 3 + 0, "<e", 1.5, "InvalidTexels"),   # albedo > 1
    (RELIGHT_ATLAS + 40 * RELIGHT_ROW + 8 * 3 + 6, "<e", 0.0, "InvalidTexels"),   # distance 0
    (RELIGHT_ATLAS + 40 * RELIGHT_ROW + 8 * 3 + 6, "<e", -2.0, "InvalidTexels"),  # negative
    (RELIGHT_ATLAS + 56 * RELIGHT_ROW + 8 * 3 + 2, "<e", 1.5, "InvalidTexels"),   # normal > 1
    (RELIGHT_ATLAS + 56 * RELIGHT_ROW + 8 * 3 + 6, "<e", 0.5, "InvalidTexels"),   # normal alpha
    (RELIGHT_ATLAS + 40 * RELIGHT_ROW + 8 * 62 + 0, "<e", 0.5, "InvalidTexels"),  # outside a mip
)


def read(data):
    """Validate RPRB bytes; return the layout and records (Source units)."""
    if len(data) < HEADER_BYTES:
        raise RprbError("Truncated")
    (magic, version, count, mips, width, atlas_width, atlas_height, record_bytes, atlas_offset,
     atlas_bytes, prefilter, flags, global_index, reserved) = struct.unpack_from(
        "<IIIIIIIIQQIIII", data)
    if magic != MAGIC:
        raise RprbError("BadMagic")
    # v1 has no flags; v2 is exactly the relight flag.
    if (version not in (VERSION, RELIGHT_VERSION) or prefilter != PREFILTER_VERSION or
            flags != (FLAG_RELIGHT if version == RELIGHT_VERSION else 0) or reserved):
        raise RprbError("UnsupportedVersion")
    relight = version == RELIGHT_VERSION
    if not 1 <= count <= MAX_PROBES or not 1 <= mips <= MAX_MIPS:
        raise RprbError("InvalidCounts")
    if (width & (width - 1) or not MIN_WIDTH <= width <= MAX_WIDTH or (width >> (mips - 1)) < 4
            or (atlas_width, atlas_height) != atlas_layout(count, width, relight)
            or record_bytes != RECORD_BYTES):
        raise RprbError("InvalidAtlas")
    expected_offset = (HEADER_BYTES + RECORD_BYTES * count + 15) & ~15
    if (atlas_offset != expected_offset or atlas_bytes != atlas_width * atlas_height * 8 or
            len(data) != atlas_offset + atlas_bytes):
        raise RprbError("InvalidSections")
    if any(data[HEADER_BYTES + RECORD_BYTES * count:atlas_offset]):
        raise RprbError("InvalidSections", "nonzero padding")
    probes = []
    for index in range(count):
        fields = struct.unpack_from("<3ff3f3f3f3fIIII", data, HEADER_BYTES + index * RECORD_BYTES)
        floats = np.array(fields[:16], dtype=np.float64)
        rank, flag, band_row, relight_row = fields[16:]
        probe = {"capture": floats[0:3], "fade": float(floats[3]), "box_min": floats[4:7],
                 "box_max": floats[7:10], "influence_min": floats[10:13],
                 "influence_max": floats[13:16], "rank": rank,
                 "global": bool(flag & FLAG_GLOBAL)}
        if (not np.all(np.isfinite(floats)) or np.any(np.abs(floats) > MAX_COORDINATE) or
                probe["fade"] <= 0 or np.any(probe["box_min"] >= probe["box_max"]) or
                np.any(probe["influence_min"] >= probe["influence_max"]) or
                np.any(probe["capture"] < probe["box_min"]) or
                np.any(probe["capture"] > probe["box_max"])):
            raise RprbError("InvalidRecord", str(index))
        if (flag & ~FLAG_GLOBAL or band_row != index * (width // 2) or
                relight_row != (relight_rows(count, width, index)[0] if relight else 0)):
            raise RprbError("InvalidRecord", "flags %d" % index)
        probes.append(probe)
    if sorted(probe["rank"] for probe in probes) != list(range(count)):
        raise RprbError("InvalidRanks")
    if (sum(probe["global"] for probe in probes) != 1 or global_index >= count or
            not probes[global_index]["global"] or probes[global_index]["rank"] != count - 1):
        raise RprbError("InvalidGlobal")
    atlas = np.frombuffer(data, dtype="<f2", count=atlas_width * atlas_height * 4,
                          offset=atlas_offset).reshape(atlas_height, atlas_width, 4)
    atlas = atlas.astype(np.float64)
    band = width // 2
    # Radiance and albedo rows hold non-negative colour; normal rows signed.
    normal_rows = np.zeros(atlas_height, dtype=bool)
    if relight:
        for index in range(count):
            row = relight_rows(count, width, index)[1]
            normal_rows[row:row + band] = True
    if not np.all(np.isfinite(atlas)) or atlas[~normal_rows, :3].min() < 0:
        raise RprbError("InvalidTexels")
    inside = np.zeros((atlas_height, atlas_width), dtype=bool)
    for top in range(0, atlas_height, band):
        for level in range(mips):
            x = 2 * width - (2 * width >> level)
            rows, columns = (width >> level) // 2, width >> level
            inside[top:top + rows, x:x + columns] = True
    if np.any(atlas[~inside] != 0.0):
        raise RprbError("InvalidTexels")
    albedo_rows = np.zeros(atlas_height, dtype=bool)
    albedo_rows[count * band:] = ~normal_rows[count * band:]
    radiance = inside.copy()
    radiance[count * band:] = False
    albedo = inside & albedo_rows[:, None]
    normal = inside & normal_rows[:, None]
    if (np.any(atlas[radiance, 3] != 1.0) or np.any(atlas[normal, 3] != 1.0) or
            np.any(atlas[albedo, :3] > 1.0) or np.any(atlas[albedo, 3] <= 0.0) or
            np.any(atlas[albedo, 3] > MAX_DISTANCE) or
            np.any(np.abs(atlas[normal, :3]) > NORMAL_LIMIT)):
        raise RprbError("InvalidTexels")

    def chain_at(top, channels):
        chain = []
        for level in range(mips):
            x = 2 * width - (2 * width >> level)
            rows, columns = (width >> level) // 2, width >> level
            chain.append(atlas[top:top + rows, x:x + columns, :channels])
        return chain

    chains = [chain_at(index * band, 3) for index in range(count)]
    relight_chains = None
    if relight:
        relight_chains = [(chain_at(relight_rows(count, width, index)[0], 4),
                           chain_at(relight_rows(count, width, index)[1], 3))
                          for index in range(count)]
    return {"count": count, "mips": mips, "width": width, "atlas_width": atlas_width,
            "atlas_height": atlas_height, "atlas_offset": atlas_offset, "probes": probes,
            "chains": chains, "relight": relight_chains, "global_index": global_index}


def gpu_texture(layout, mode=MODE_BLEND, relight=True):
    """The RGBA16F texture the shaders read (see GPU_MARKER), as float16.
    Texel (1, 0).y is 1 when the probes are relit (`mat_reflection_relight`
    and relight bands present); table field 3's w is the albedo band's row
    (0 without bands)."""
    count, width = layout["count"], layout["width"]
    bands = layout.get("relight")
    rows = 1 + count + layout["atlas_height"]
    texture = np.zeros((rows, 2 * width, 4), dtype=np.float16)
    texture[0, 0] = (count, layout["mips"], width, GPU_MARKER)
    texture[0, 1] = (mode, 1.0 if relight and bands else 0.0, 0, 0)
    for index, probe in enumerate(layout["probes"]):
        values = np.array([(*probe["capture"], probe["fade"]),
                           (*probe["box_min"], index * (width // 2)),
                           (*probe["box_max"], 1.0 if probe["global"] else 0.0),
                           (*probe["influence_min"],
                            relight_rows(count, width, index)[0] if bands else 0.0),
                           (*probe["influence_max"], 0.0)], dtype=np.float32)
        hi = values.astype(np.float16)
        lo = (values - hi.astype(np.float32)).astype(np.float16)
        row = texture[1 + probe["rank"]]
        row[0:2 * TABLE_VEC4:2] = hi
        row[1:2 * TABLE_VEC4:2] = lo
    def place(top, chain, alpha):
        x = 0
        for mip in chain:
            height, columns = mip.shape[:2]
            texture[top:top + height, x:x + columns, :mip.shape[2]] = mip
            if alpha is not None:
                texture[top:top + height, x:x + columns, 3] = alpha
            x += columns

    for index, chain in enumerate(layout["chains"]):
        place(1 + count + index * (width // 2), chain, 1.0)
    for index, (first, second) in enumerate(bands or ()):
        albedo_row, normal_row = relight_rows(count, width, index)
        place(1 + count + albedo_row, first, None)
        place(1 + count + normal_row, second, 1.0)
    return texture


def texture_table(texture):
    """Decode the GPU table rows back to float32 vec4s (the shader's view)."""
    count = int(texture[0, 0, 0])
    table = texture[1:1 + count, :2 * TABLE_VEC4].astype(np.float32)
    return table[:, 0::2] + table[:, 1::2]


# ------------------------------------------------------------- blending

def smoothstep(edge0, edge1, x):
    t = np.clip((x - edge0) / (edge1 - edge0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def influence_weight(points, probe):
    """1 inside the influence box, smoothstep to 0 at `fade` outside it."""
    outside = np.maximum(np.maximum(probe["influence_min"] - points,
                                    points - probe["influence_max"]), 0.0)
    return 1.0 - smoothstep(0.0, probe["fade"], np.linalg.norm(outside, axis=1))


def facing_weight(points, normals, capture):
    """0 when the capture point is behind the surface, 1 in front of it."""
    toward = np.asarray(capture) - points
    cosine = np.sum(normals * toward, axis=1) / np.maximum(np.linalg.norm(toward, axis=1), 1e-9)
    return smoothstep(-FACING_EDGE, FACING_EDGE, cosine)


def blend_weights(points, normals, probes, mode=MODE_BLEND):
    """(N, P) sample weights for shaded points with geometric normals.

    At most two probes per point have nonzero weight; each point's weights
    sum to 1. MODE_NEAREST gives the probe with the nearest capture all of it.
    """
    points = np.asarray(points, dtype=np.float64)
    count = len(probes)
    if mode == MODE_NEAREST:
        distance = np.stack([np.linalg.norm(points - probe["capture"], axis=1)
                             for probe in probes], axis=1)
        weights = np.zeros((len(points), count))
        weights[np.arange(len(points)), np.argmin(distance, axis=1)] = 1.0
        return weights
    order = sorted(range(count), key=lambda i: probes[i]["rank"])
    remaining = np.ones(len(points))
    shares = np.zeros((len(points), count))
    for index in order:
        probe = probes[index]
        if probe["global"]:
            weight = np.ones(len(points))
        else:
            weight = influence_weight(points, probe) * facing_weight(points, normals,
                                                                     probe["capture"])
        shares[:, index] = weight * remaining
        remaining = remaining - shares[:, index]
    ranked = np.sort(shares, axis=1)[:, ::-1]
    third = ranked[:, 2] if count > 2 else np.zeros(len(points))
    top = np.argsort(-shares, axis=1, kind="stable")[:, :2]
    weights = np.zeros_like(shares)
    rows = np.arange(len(points))
    for column in range(min(2, count)):
        weights[rows, top[:, column]] = shares[rows, top[:, column]] - third
    total = weights.sum(axis=1, keepdims=True)
    # Three equal shares leave nothing after the subtraction: split evenly.
    tie = total[:, 0] <= 1e-12
    if tie.any():
        weights[tie] = 0.0
        for column in range(min(2, count)):
            weights[rows[tie], top[tie, column]] = 1.0
        total = weights.sum(axis=1, keepdims=True)
    return weights / total


def probe_radiance(points, reflected, roughness, probe, chain, parallax=True, bands=None,
                   change=None):
    """One probe's split-sum fetch for rays leaving shaded points; with its
    relight `bands` and a `change(points, normals)` (the scene's diffuse-light
    change, irradiance / pi, at world points), relit (see RPRB v2)."""
    direction, local = reflected, roughness
    if parallax:
        lookup, shaded, captured, valid = reflection_probe.parallax_lookup(
            points, reflected, probe["capture"], probe["box_min"], probe["box_max"])
        local = np.where(valid, reflection_probe.distance_roughness(roughness, shaded,
                                                                    captured), roughness)
        direction = reflection_probe.corrected_direction(lookup, reflected, roughness)
    radiance = reflection_probe.sample_chain(chain, direction, local)
    if bands is None or change is None:
        return radiance
    albedo = reflection_probe.sample_chain(bands[0], direction, local, channels=4)
    normal = reflection_probe.sample_chain(bands[1], direction, local)
    normal /= np.maximum(np.linalg.norm(normal, axis=1, keepdims=True), 1e-12)
    seen = np.asarray(probe["capture"]) + direction * albedo[:, 3:4]
    return np.maximum(radiance + albedo[:, :3] * change(seen, normal), 0.0)


def shade(points, normals, reflected, roughness, layout, mode=MODE_BLEND, change=None):
    """The blended probe radiance the shader computes (world units of the
    layout, directions unit). Oracle for world_pbr_probe.glsl. `change`
    relights the probes that carry relight bands (None: as baked)."""
    points = np.asarray(points, dtype=np.float64)
    reflected = np.asarray(reflected, dtype=np.float64)
    roughness = np.broadcast_to(np.asarray(roughness, dtype=np.float64), (len(points),))
    result = np.zeros((len(points), 3))
    if mode == MODE_OFF:
        return result
    selection = mode & 3
    weights = blend_weights(points, normals, layout["probes"], selection)
    for index, (probe, chain) in enumerate(zip(layout["probes"], layout["chains"])):
        used = weights[:, index] > 0
        if not used.any():
            continue
        if mode & MODE_WEIGHTS:
            colour = np.asarray(WEIGHT_PALETTE[probe["rank"] % len(WEIGHT_PALETTE)])
            result[used] += weights[used, index, None] * colour
        else:
            bands = layout["relight"][index] if layout.get("relight") else None
            result[used] += weights[used, index, None] * probe_radiance(
                points[used], reflected[used], roughness[used], probe, chain,
                parallax=selection != MODE_DIRECTION, bands=bands, change=change)
    return result


# ------------------------------------------------------------- placement

PLACEMENT_DEFAULTS = {
    "spacing_m": 0.75,         # walkable sample grid
    "eye_height_m": 1.63,      # a standing player's eye (64 Source units)
    "headroom_m": 0.2,         # clearance above the eye for a walkable sample
    "fit_rays": 2048,          # directions for a candidate's proxy-box estimate
    "max_probes": 8,
    "min_cover": 2,            # walkable samples a new room probe must cover
    "glossy_roughness": 0.35,  # perceptual roughness at or below which a surface is glossy
    "glossy_radius_m": 2.5,    # a glossy sample needs a capture this near that sees it
    "glossy_samples_per_m2": 16.0,
    "glossy_min_area_m2": 0.5,  # glossy area a new glossy probe must serve
    "influence_margin_m": 0.15,
    # A capture this close to a surface sees mostly that surface (or, at a
    # doorway, the next room through it): it is not a room's probe.
    "min_clearance_m": 0.5,
    # A candidate whose own box fits its surroundings worse than this (90th
    # percentile relative residual: a box through a doorway spanning two
    # rooms scores 0.4-0.6, an empty room's own box 0, a furnished room
    # 0.1-0.3) is not placed while a better one covers the same samples. The
    # mean is diluted by the many texels that do fit.
    "max_candidate_residual": 0.35,
    "fade_m": 0.5,
}


def fibonacci_directions(count):
    index = np.arange(count) + 0.5
    z = 1 - 2 * index / count
    radius = np.sqrt(np.maximum(0.0, 1 - z * z))
    phi = math.pi * (3 - math.sqrt(5)) * index
    return np.stack((radius * np.cos(phi), radius * np.sin(phi), z), axis=1)


def walkable_samples(raycast, bounds_min, bounds_max, params):
    """Eye-height points above every upward-facing surface a vertical ray
    finds under the bounds' top, with headroom, inside the bounds."""
    spacing, eye = params["spacing_m"], params["eye_height_m"]
    xs = np.arange(bounds_min[0] + spacing / 2, bounds_max[0], spacing)
    ys = np.arange(bounds_min[1] + spacing / 2, bounds_max[1], spacing)
    columns = np.array([(x, y) for x in xs for y in ys], dtype=np.float64).reshape(-1, 2)
    origins = np.column_stack((columns, np.full(len(columns), bounds_max[2] + 0.01)))
    down = np.tile((0.0, 0.0, -1.0), (len(origins), 1))
    samples = []
    active = np.ones(len(origins), dtype=bool)
    for _ in range(32):
        if not active.any():
            break
        distance, normal = raycast(origins[active], down[active], 1.0e4)
        indices = np.nonzero(active)[0]
        hit = np.isfinite(distance)
        active[indices[~hit]] = False
        indices, distance, normal = indices[hit], distance[hit], normal[hit]
        points = origins[indices] - np.column_stack((np.zeros((len(indices), 2)), distance))
        # Horizontal and reached from above: a floor. Winding is not trusted
        # (a slab's underside, reached after stepping through it, fails the
        # headroom test instead).
        floor = np.abs(normal[:, 2]) > 0.7
        candidates = points[floor] + (0.0, 0.0, eye)
        if len(candidates):
            up = np.tile((0.0, 0.0, 1.0), (len(candidates), 1))
            clearance, _ = raycast(points[floor] + (0.0, 0.0, 1e-3), up, 1.0e4)
            keep = ((clearance >= eye + params["headroom_m"]) &
                    (candidates[:, 2] <= bounds_max[2]) & (candidates[:, 2] >= bounds_min[2]))
            samples.extend(candidates[keep])
        origins[indices] = points - (0.0, 0.0, 1e-3)
        active[indices[points[:, 2] < bounds_min[2]]] = False
    return np.array(samples, dtype=np.float64).reshape(-1, 3)


def estimate_box(raycast, capture, directions):
    """(box min, box max, fit report with `clearance`, the nearest hit)."""
    distance, _ = raycast(np.tile(capture, (len(directions), 1)), directions, 1.0e4)
    distance = np.where(np.isfinite(distance), distance, np.inf)
    weights = np.full(len(directions), 4 * math.pi / len(directions))
    box_min, box_max, report = reflection_probe.fit_parallax_box(capture, directions, distance,
                                                                 weights)
    report["clearance"] = float(distance.min())
    return box_min, box_max, report


def visible(raycast, origins, targets):
    delta = np.asarray(targets) - np.asarray(origins)
    length = np.linalg.norm(delta, axis=1)
    safe = np.maximum(length, 1e-9)
    distance, _ = raycast(np.asarray(origins, dtype=np.float64), delta / safe[:, None], 1.0e4)
    return ~(distance < length - 1e-3)


def inside(points, box_min, box_max, margin=0.05):
    return np.all((points >= box_min - margin) & (points <= box_max + margin), axis=1)


def place(raycast, bounds_min, bounds_max, glossy=None, params=None, seeds=()):
    """Probe captures, proxy boxes and influence volumes (meters).

    Room probes: greedily cover every walkable sample (eye height above each
    floor) by a probe whose estimated proxy box, grown by the blend's fade,
    contains it and which sees it; each new probe is moved to the candidate
    nearest the middle of what it newly covers. Glossy probes: every glossy
    surface sample (roughness at or below `glossy_roughness`: mirrors,
    polished floors, glass) that some walkable point sees needs a capture
    within `glossy_radius_m` in front of it that sees it; uncovered ones get
    a probe at the walkable sample covering most of them, whose influence is
    those samples' bounds, while one serves at least `glossy_min_area_m2`.
    The room probe covering the most walkable samples is global (a box open
    to the sky can be the largest while covering a window bay). `glossy`: (points, normals) arrays or None. `seeds`:
    captures placed first, as room probes (a manifest's chosen positions).
    Returns (probes, report); the proxy boxes here are estimates that the
    capture's depth faces replace.
    """
    params = dict(PLACEMENT_DEFAULTS, **(params or {}))
    bounds_min = np.asarray(bounds_min, dtype=np.float64)
    bounds_max = np.asarray(bounds_max, dtype=np.float64)
    walkable = walkable_samples(raycast, bounds_min, bounds_max, params)
    if not len(walkable):
        raise ValueError("no walkable sample under the bounds: nowhere to place a probe")
    directions = fibonacci_directions(params["fit_rays"])
    estimates = [estimate_box(raycast, candidate, directions) for candidate in walkable]
    boxes = [estimate[:2] for estimate in estimates]
    eligible = np.array([estimate[2]["clearance"] >= params["min_clearance_m"] and
                         estimate[2]["p90_relative_residual"] <=
                         params["max_candidate_residual"] for estimate in estimates])
    covers = []
    for index, (box_min, box_max) in enumerate(boxes):
        within = inside(walkable, box_min, box_max, params["fade_m"])
        seen = np.zeros(len(walkable), dtype=bool)
        indices = np.nonzero(within)[0]
        seen[indices] = visible(raycast, np.tile(walkable[index], (len(indices), 1)),
                                walkable[indices])
        covers.append(seen)
    covers = np.array(covers)
    probes = []
    uncovered = np.ones(len(walkable), dtype=bool)
    margin = params["influence_margin_m"]
    for seed in seeds:
        seed = np.asarray(seed, dtype=np.float64)
        box_min, box_max = estimate_box(raycast, seed, directions)[:2]
        within = np.nonzero(inside(walkable, box_min, box_max, params["fade_m"]))[0]
        seen = np.zeros(len(walkable), dtype=bool)
        seen[within] = visible(raycast, np.tile(seed, (len(within), 1)), walkable[within])
        probes.append({"capture": seed.copy(), "box_min": box_min, "box_max": box_max,
                       "influence_min": box_min - margin, "influence_max": box_max + margin,
                       "fade": params["fade_m"], "role": "room", "seeded": True,
                       "covers": int((seen & uncovered).sum())})
        uncovered &= ~seen
    room_stop = "covered"
    while uncovered.any():
        gain = (covers & uncovered).sum(axis=1)
        # Good captures first; any capture only for samples none of them sees.
        if (gain * eligible).max() > 0:
            gain = gain * eligible
        best = int(np.argmax(gain))
        if gain[best] < min(params["min_cover"], uncovered.sum()):
            room_stop = "min_cover"
            break
        if len(probes) >= params["max_probes"]:
            room_stop = "max_probes"
            break
        newly = covers[best] & uncovered
        # Recentre: of the candidates covering most of what `best` does, the
        # one nearest the middle of it.
        enough = (covers & newly).sum(axis=1) >= 0.9 * newly.sum()
        if (enough & eligible).any():
            enough &= eligible
        middle = walkable[newly].mean(axis=0)
        choices = np.nonzero(enough)[0]
        chosen = int(choices[np.argmin(np.linalg.norm(walkable[choices] - middle, axis=1))])
        newly = covers[chosen] & uncovered
        box_min, box_max = boxes[chosen]
        probes.append({"capture": walkable[chosen].copy(), "box_min": box_min,
                       "box_max": box_max, "influence_min": box_min - margin,
                       "influence_max": box_max + margin, "fade": params["fade_m"],
                       "role": "room", "covers": int(newly.sum())})
        uncovered &= ~covers[chosen]
    report = {"walkable_samples": int(len(walkable)),
              "uncovered_walkable": int(uncovered.sum()), "room_stop": room_stop}
    if glossy is not None and len(glossy[0]):
        points, normals = (np.asarray(value, dtype=np.float64) for value in glossy)
        radius = params["glossy_radius_m"]

        def seen_by(capture):
            toward = capture - points
            near = (np.linalg.norm(toward, axis=1) <= radius) & \
                (np.sum(toward * normals, axis=1) > 0)
            result = np.zeros(len(points), dtype=bool)
            indices = np.nonzero(near)[0]
            # Offset off the surface so the visibility ray does not hit it.
            result[indices] = visible(raycast, points[indices] + normals[indices] * 1e-3,
                                      np.tile(capture, (len(indices), 1)))
            return result
        served = np.zeros(len(points), dtype=bool)
        for probe in probes:
            served |= seen_by(probe["capture"])
        candidate_views = np.array([seen_by(candidate) for candidate in walkable])
        # Samples no eye-height point sees (under furniture, behind objects)
        # cannot be served by placement; they are reported, not chased.
        servable = candidate_views.any(axis=0)
        minimum = max(1, int(math.ceil(params["glossy_min_area_m2"] *
                                       params["glossy_samples_per_m2"])))
        glossy_stop = "served"
        while not (served | ~servable).all():
            gain = (candidate_views & ~served).sum(axis=1)
            if (gain * eligible).max() >= minimum:
                gain = gain * eligible
            best = int(np.argmax(gain))
            if gain[best] < minimum:
                glossy_stop = "min_area"
                break
            if len(probes) >= params["max_probes"]:
                glossy_stop = "max_probes"
                break
            newly = candidate_views[best] & ~served
            box_min, box_max = boxes[best]
            probes.append({"capture": walkable[best].copy(), "box_min": box_min,
                           "box_max": box_max,
                           "influence_min": points[newly].min(axis=0) - margin,
                           "influence_max": points[newly].max(axis=0) + margin,
                           "fade": params["fade_m"], "role": "glossy",
                           "covers": int(newly.sum())})
            served |= candidate_views[best]
        report.update(glossy_samples=int(len(points)), glossy_servable=int(servable.sum()),
                      unserved_glossy=int((servable & ~served).sum()), glossy_stop=glossy_stop)
    rooms = [probe for probe in probes if probe["role"] == "room"]
    max(rooms, key=lambda probe: probe["covers"])["global"] = True
    report["probes"] = len(probes)
    return probes, report


def glossy_samples(triangles, density, seed=0):
    """Area-uniform points and normals on triangles ((N, 3, 3) arrays)."""
    triangles = np.asarray(triangles, dtype=np.float64).reshape(-1, 3, 3)
    edges = np.cross(triangles[:, 1] - triangles[:, 0], triangles[:, 2] - triangles[:, 0])
    area = np.linalg.norm(edges, axis=1) / 2
    keep = area > 1e-9
    triangles, edges, area = triangles[keep], edges[keep], area[keep]
    if not len(area):
        return np.zeros((0, 3)), np.zeros((0, 3))
    count = max(1, int(round(area.sum() * density)))
    rng = np.random.default_rng(seed)
    chosen = rng.choice(len(area), size=count, p=area / area.sum())
    u, v = rng.random(count), rng.random(count)
    flip = u + v > 1
    u[flip], v[flip] = 1 - u[flip], 1 - v[flip]
    t = triangles[chosen]
    points = t[:, 0] + u[:, None] * (t[:, 1] - t[:, 0]) + v[:, None] * (t[:, 2] - t[:, 0])
    normals = edges[chosen] / (2 * area[chosen, None])
    return points, normals


# ------------------------------------------------------------- test scenes

class BoxScene:
    """Analytic ray caster over axis-aligned boxes, for tests and oracles:
    `rooms` are hollow (their inner faces face in), `solids` are filled
    (outer faces face out). Rays leaving every room hit nothing (sky)."""

    def __init__(self, rooms=(), solids=(), openings=()):
        self.rooms = [(np.asarray(lo, float), np.asarray(hi, float)) for lo, hi in rooms]
        self.solids = [(np.asarray(lo, float), np.asarray(hi, float)) for lo, hi in solids]
        # Openings: boxes punched through room walls (doorways, windows).
        self.openings = [(np.asarray(lo, float), np.asarray(hi, float)) for lo, hi in openings]

    def __call__(self, origins, directions, max_distance):
        origins = np.asarray(origins, dtype=np.float64)
        directions = np.asarray(directions, dtype=np.float64)
        best = np.full(len(origins), np.inf)
        normal = np.zeros((len(origins), 3))
        safe = np.where(np.abs(directions) < 1e-12, np.copysign(1e-12, directions), directions)
        for boxes, interior in ((self.rooms, True), (self.solids, False)):
            for lo, hi in boxes:
                for axis in range(3):
                    for plane, facing in ((lo[axis], 1.0 if interior else -1.0),
                                          (hi[axis], -1.0 if interior else 1.0)):
                        t = (plane - origins[:, axis]) / safe[:, axis]
                        point = origins + t[:, None] * directions
                        others = [a for a in range(3) if a != axis]
                        on_face = np.all([(point[:, a] >= lo[a] - 1e-9) &
                                          (point[:, a] <= hi[a] + 1e-9) for a in others], axis=0)
                        # Only the side the face faces is hit.
                        from_front = directions[:, axis] * facing < 0
                        blocked = np.zeros(len(origins), dtype=bool)
                        if interior:
                            for olo, ohi in self.openings:
                                blocked |= np.all((point > olo) & (point < ohi), axis=1)
                        valid = (t > 1e-9) & (t < best) & (t <= max_distance) & on_face & \
                            from_front & ~blocked
                        best = np.where(valid, t, best)
                        n = np.zeros(3)
                        n[axis] = facing
                        normal[valid] = n
        return best, normal


# ------------------------------------------------------------- packing

DEPTH_TOLERANCE = 0.01


def face_ray_scale(size):
    """|v| per texel of a cube face: planar depth times it is ray distance."""
    t = (np.arange(size) + 0.5) / size * 2 - 1
    x, y = np.meshgrid(t, -t)
    return np.sqrt(1 + x * x + y * y)


def depth_convention(depths, checks):
    """Which convention ('planar' or 'radial') makes the depth pass agree with
    the renderer's BVH distances at the recorded texels; raises if neither
    does (median relative error above DEPTH_TOLERANCE)."""
    errors = {"planar": [], "radial": []}
    for name, samples in checks.items():
        depth = depths[name]
        scale = face_ray_scale(depth.shape[0])
        for row, column, expected in samples:
            value = float(depth[row, column])
            if expected is None or value >= reflection_probe.SKY_DISTANCE:
                continue
            errors["planar"].append(abs(value * scale[row, column] - expected) / expected)
            errors["radial"].append(abs(value - expected) / expected)
    medians = {key: float(np.median(value)) if value else math.inf
               for key, value in errors.items()}
    best = min(medians, key=medians.get)
    if medians[best] > DEPTH_TOLERANCE:
        raise ValueError("the depth pass matches no convention: %s" % medians)
    return best, medians


def pack(probes_dir, width, gain, prefilter_samples=256):
    """(RPRB bytes, receipt) from `pbrt_reflection_probe.py`'s output."""
    import gi_reference
    import hashlib
    receipt = json.loads((probes_dir / "probes.json").read_text())
    if receipt.get("status") != "pass" or receipt.get("schema") != "reflection-probe-faces/v2":
        raise ValueError("probe faces receipt is missing or not a v2 pass")
    margin = receipt["placement"]["influence_margin_m"]
    # Faces rendered with the Diffuse Color and Normal passes carry the
    # relight G-buffer (RPRB v2).
    gbuffer = bool(receipt.get("gbuffer"))
    probes, chains, reports, conventions, relight = [], [], [], set(), []
    for record in receipt["probes"]:
        directory = probes_dir / record["dir"]
        colors, depths, albedos, normals = {}, {}, {}, {}
        for name in reflection_probe.FACES:
            path = directory / (name + ".exr")
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            if record["faces"].get(name) != digest:
                raise ValueError("probe face differs from its receipt: %s/%s" %
                                 (record["dir"], name))
            passes = gi_reference.render_passes(path)
            colors[name] = passes["Combined"][..., :3].astype(np.float64)
            depths[name] = passes["Depth"][..., 0].astype(np.float64)
            if gbuffer:
                albedos[name] = passes["DiffCol"][..., :3].astype(np.float64)
                normals[name] = passes["Normal"][..., :3].astype(np.float64)
        convention, medians = depth_convention(depths, record["depth_checks"])
        conventions.add(convention)
        distances = {name: depth * face_ray_scale(depth.shape[0]) if convention == "planar"
                     else depth for name, depth in depths.items()}
        capture = np.asarray(record["capture"], dtype=np.float64)
        directions, values, weights = reflection_probe.face_samples(distances)
        box_min, box_max, report = reflection_probe.fit_parallax_box(capture, directions,
                                                                     values, weights)
        if record["role"] == "room":
            influence_min, influence_max = box_min - margin, box_max + margin
        else:
            influence_min = np.asarray(record["influence_min"], dtype=np.float64)
            influence_max = np.asarray(record["influence_max"], dtype=np.float64)
        probes.append({"capture": capture, "box_min": box_min, "box_max": box_max,
                       "influence_min": influence_min, "influence_max": influence_max,
                       "fade": record["fade"], "role": record["role"]})
        chains.append([mip * gain for mip in reflection_probe.mip_chain(
            reflection_probe.cube_to_equirect(colors, width), samples=prefilter_samples)])
        if gbuffer:
            distance = reflection_probe.cube_to_equirect(
                {name: np.minimum(depth, MAX_DISTANCE / SOURCE_UNITS_PER_METER)[..., None]
                 for name, depth in distances.items()}, width, channels=1)[..., 0]
            relight.append(relight_chain(reflection_probe.cube_to_equirect(albedos, width),
                                         reflection_probe.cube_to_equirect(normals, width),
                                         distance))
        reports.append(dict(report, index=record["index"], role=record["role"],
                            capture=list(capture), box_min=list(box_min),
                            box_max=list(box_max), depth_convention=convention,
                            depth_check_median_error=medians[convention]))
    # Placement's global probe: the room probe covering the most walkable
    # space.
    global_index = next(i for i, record in enumerate(receipt["probes"]) if record["global"])
    probes[global_index]["global"] = True
    data = build(probes, chains, relight=relight if gbuffer else None)
    residuals = [r["mean_relative_residual"] for r in reports]
    return data, {"status": "pass", "schema": "rprb-pack/v1", "probes": len(probes),
                  "width": width, "mips": len(chains[0]), "preview_gain": gain,
                  "relight": gbuffer,
                  "prefilter_version": PREFILTER_VERSION, "global_index": global_index,
                  "faces_receipt_sha256": hashlib.sha256(
                      (probes_dir / "probes.json").read_bytes()).hexdigest(),
                  "max_mean_relative_residual": max(residuals),
                  "depth_conventions": sorted(conventions),
                  "placement": receipt["placement"], "fits": reports}


def layout_meters(layout):
    """A read layout in meters (the tools' stage units)."""
    scale = 1.0 / SOURCE_UNITS_PER_METER
    probes = []
    for probe in layout["probes"]:
        probes.append(dict(probe, **{key: probe[key] * scale for key in
                                     ("capture", "box_min", "box_max", "influence_min",
                                      "influence_max")}, fade=probe["fade"] * scale))
    relight = None
    if layout.get("relight"):
        relight = [([np.concatenate((level[..., :3], level[..., 3:4] * scale), axis=2)
                     for level in first], second) for first, second in layout["relight"]]
    return dict(layout, probes=probes, relight=relight)


FIXTURE_DIR = Path(__file__).resolve().parents[2] / "quality/fixtures/reflection/rprb"


def fixture_layout():
    """The shared two-probe fixture (meters): a 6 x 4 x 3 m striped room, a
    small probe at x = 1.5 and the global one at x = 4.5, width 32."""
    room = ((0.0, 0.0, 0.0), (6.0, 4.0, 3.0))
    scene = BoxScene(rooms=[room])
    probes = [
        {"capture": np.array((1.5, 2.0, 1.6)), "box_min": np.array(room[0]),
         "box_max": np.array(room[1]), "influence_min": np.array((-0.15, -0.15, -0.15)),
         "influence_max": np.array((3.0, 4.15, 3.15)), "fade": 0.5},
        {"capture": np.array((4.5, 2.0, 1.6)), "box_min": np.array(room[0]),
         "box_max": np.array(room[1]), "influence_min": np.array((-0.15, -0.15, -0.15)),
         "influence_max": np.array((6.15, 4.15, 3.15)), "fade": 0.5, "global": True}]
    chains = []
    for probe in probes:
        directions = reflection_probe.equirect_directions(32).reshape(-1, 3)
        distance, _ = scene(np.tile(probe["capture"], (len(directions), 1)), directions, 1e4)
        hits = probe["capture"] + distance[:, None] * directions
        phase = np.floor(hits[:, 0] / 0.5) + 2 * np.floor(hits[:, 1] / 0.5) + \
            3 * np.floor(hits[:, 2] / 0.5)
        radiance = np.stack((0.2 + 0.8 * (phase % 2), 0.2 + 0.8 * ((phase // 2) % 2),
                             0.2 + 0.8 * ((phase // 4) % 2)), axis=1).reshape(16, 32, 3)
        chains.append(reflection_probe.mip_chain(radiance, samples=16))
    return probes, chains


def fixture_relight(probes):
    """Relight bands for the shared fixture: the room's surfaces seen from
    each capture (distance and the inward normal), with an albedo of stripes
    that differ from the radiance's."""
    room = ((0.0, 0.0, 0.0), (6.0, 4.0, 3.0))
    scene = BoxScene(rooms=[room])
    bands = []
    for probe in probes:
        directions = reflection_probe.equirect_directions(32).reshape(-1, 3)
        distance, normal = scene(np.tile(probe["capture"], (len(directions), 1)), directions,
                                 1e4)
        hits = probe["capture"] + distance[:, None] * directions
        stripe = np.floor(hits[:, 0] / 0.75) + np.floor(hits[:, 2] / 0.75)
        albedo = np.stack((0.3 + 0.5 * (stripe % 2), np.full(len(hits), 0.6),
                           0.8 - 0.4 * (stripe % 2)), axis=1)
        # The scene's normal faces the ray; the room's surfaces face inward.
        normal = -np.sign(np.sum(normal * directions, axis=1))[:, None] * normal
        bands.append(relight_chain(albedo.reshape(16, 32, 3), normal.reshape(16, 32, 3),
                                   distance.reshape(16, 32)))
    return bands


def fixture_change(points, normals):
    """The analytic diffuse-light change the relight samples use (Source
    units): the C++ suite and reflection_probes_check.comp implement it too."""
    points = np.asarray(points, dtype=np.float64)
    normals = np.asarray(normals, dtype=np.float64)
    return np.stack((0.002 * points[:, 0] + 0.5 * np.maximum(normals[:, 2], 0.0),
                     0.3 - 0.003 * points[:, 1], 0.25 * normals[:, 0] - 0.1), axis=1)


def write_fixture(out):
    """The C++ reader's inputs: valid.rprb, its GPU texture (mode 1),
    the malformation corpus and shading samples of the reference blend."""
    probes, chains = fixture_layout()
    data = build(probes, chains)
    out.mkdir(parents=True, exist_ok=True)
    (out / "valid.rprb").write_bytes(data)
    layout = read(data)
    (out / "gpu-mode1.rgba16f").write_bytes(gpu_texture(layout, MODE_BLEND).tobytes())
    lines = ["# offset format value error (reflection_probe_set.MALFORMATIONS)"]
    for offset, fmt, value, code in MALFORMATIONS:
        lines.append("%d %s %r %s" % (offset, fmt[1:], value, code))
    (out / "malformations.txt").write_text("\n".join(lines) + "\n")
    rng = np.random.default_rng(7)
    rows = ["# mode px py pz nx ny nz rx ry rz roughness -> r g b (Source units)"]
    scale = SOURCE_UNITS_PER_METER
    for mode in (MODE_BLEND, MODE_NEAREST, MODE_DIRECTION, MODE_BLEND | MODE_WEIGHTS):
        count = 48
        points = np.column_stack((rng.uniform(0.2, 5.8, count), rng.uniform(0.2, 3.8, count),
                                  rng.choice((0.0, 1.2), count))) * scale
        normals = np.where(points[:, 2:3] == 0, (0.0, 0.0, 1.0), (1.0, 0.0, 0.0))
        reflected = rng.normal(size=(count, 3))
        reflected /= np.linalg.norm(reflected, axis=1, keepdims=True)
        roughness = rng.choice((0.0, 0.2, 0.5, 1.0), count)
        radiance = shade(points, normals, reflected, roughness, layout, mode)
        for i in range(count):
            rows.append(" ".join("%.9g" % v for v in (mode, *points[i], *normals[i],
                                                       *reflected[i], roughness[i],
                                                       *radiance[i])))
    (out / "samples.txt").write_text("\n".join(rows) + "\n")
    # v2: the same probes with relight bands, relit by fixture_change.
    relit = build(probes, chains, relight=fixture_relight(probes))
    (out / "valid-relight.rprb").write_bytes(relit)
    relit_layout = read(relit)
    (out / "gpu-relight-mode1.rgba16f").write_bytes(
        gpu_texture(relit_layout, MODE_BLEND).tobytes())
    lines = ["# offset format value error (reflection_probe_set.RELIGHT_MALFORMATIONS)"]
    for offset, fmt, value, code in RELIGHT_MALFORMATIONS:
        lines.append("%d %s %r %s" % (offset, fmt[1:], value, code))
    (out / "relight-malformations.txt").write_text("\n".join(lines) + "\n")
    rows = ["# mode px py pz nx ny nz rx ry rz roughness -> r g b (Source units), relit by "
            "fixture_change"]
    for mode in (MODE_BLEND, MODE_NEAREST, MODE_DIRECTION):
        count = 48
        points = np.column_stack((rng.uniform(0.2, 5.8, count), rng.uniform(0.2, 3.8, count),
                                  rng.choice((0.0, 1.2), count))) * scale
        normals = np.where(points[:, 2:3] == 0, (0.0, 0.0, 1.0), (1.0, 0.0, 0.0))
        reflected = rng.normal(size=(count, 3))
        reflected /= np.linalg.norm(reflected, axis=1, keepdims=True)
        roughness = rng.choice((0.0, 0.2, 0.5, 1.0), count)
        radiance = shade(points, normals, reflected, roughness, relit_layout, mode,
                         change=fixture_change)
        for i in range(count):
            rows.append(" ".join("%.9g" % v for v in (mode, *points[i], *normals[i],
                                                       *reflected[i], roughness[i],
                                                       *radiance[i])))
    (out / "relight-samples.txt").write_text("\n".join(rows) + "\n")


def main():
    import argparse
    import hashlib
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("pack", help="fit, prefilter and encode the rendered probes")
    command.add_argument("--probes-dir", type=Path, required=True)
    command.add_argument("--width", type=int, default=512,
                         help="equirect width of every probe's mip 0")
    command.add_argument("--preview-gain", type=float, default=1.0,
                         help="the lightmap's exposure gain (the LMAP writer's)")
    command.add_argument("--out", type=Path, required=True)
    command = commands.add_parser("info", help="validate RPRB bytes and print their records")
    command.add_argument("rprb", type=Path)
    command = commands.add_parser("fixture", help="write the C++ reader's shared fixtures")
    command.add_argument("--out", type=Path, default=FIXTURE_DIR)
    args = parser.parse_args()
    if args.command == "fixture":
        write_fixture(args.out)
        return
    if args.command == "info":
        layout = read(args.rprb.read_bytes())
        print(json.dumps({"count": layout["count"], "mips": layout["mips"],
                          "width": layout["width"], "global_index": layout["global_index"],
                          "probes": [{key: (value.tolist() if hasattr(value, "tolist")
                                            else value) for key, value in probe.items()}
                                     for probe in layout["probes"]]}, indent=2))
        return
    data, receipt = pack(args.probes_dir, args.width, args.preview_gain)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.out.with_name(args.out.name + ".tmp")
    temporary.write_bytes(data)
    os.replace(temporary, args.out)
    receipt["rprb_sha256"] = hashlib.sha256(data).hexdigest()
    args.out.with_name(args.out.name + ".json").write_text(
        json.dumps(receipt, indent=2, sort_keys=True, default=float) + "\n")
    print(json.dumps({"status": "pass", "probes": receipt["probes"],
                      "max_mean_relative_residual": receipt["max_mean_relative_residual"]}))


if __name__ == "__main__":
    main()
