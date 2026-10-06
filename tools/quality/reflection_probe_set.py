"""A map's reflection probes: RPRB encoding, blending and placement.

RFC 0008 names `RPRB` as the map's reflection probes; this module owns its
encoding (the writer `build` and the independent reader `read`), the blend
the shaders evaluate per pixel (`blend_weights`, `shade`), and the automatic
placement the Blender renderer runs (`place`). `reflection_probe.py` owns
the per-probe math: the cube GGX chain, the proxy-box fit and the
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

RPRB v8 (2026-10-06) is the one version; every earlier version, including
the v7 equirect strip atlas, is refused. Little-endian:

  header, 64 bytes
    0  u32 magic "RPRB"          4  u32 version (8)
    8  u32 probe count P (1..256) 12 u32 mip count M (1..12)
   16  u32 face size S (power of two, 8..1024, S >> (M - 1) >= 4)
   20  u32 relight face size (S with relight cubes, else 0)   24  u32 0
   28  u32 record bytes (80)     32 u64 data offset (after the candidate grid)
   40  u64 data bytes (radiance BC6H, then relight cubes)
   48  u32 prefilter version (1)
   52  u32 flags (bit 0: relight cubes present)   56 u32 global probe index   60 u32 0
  P records, 80 bytes, floats in Source units
    0 capture xyz     12 fade distance (> 0)
   16 parallax box min xyz        28 parallax box max xyz
   40 influence box min xyz       52 influence box max xyz
   64 u32 rank (a permutation of 0..P-1; the global probe ranks last)
   68 u32 flags (bit 0: global)   72 u32 cube layer (= index)
   76 u32 relight layer (= index with relight cubes, else 0)
  candidate grid (aligned after the records, unchanged since v5): a 32-byte
    header (origin xyz and power-of-two cell size as float32, u32 dimension
    32, u32 words per cell ceil(P / 64), 8 zero bytes) and 32^3 cells of that
    many little-endian uint64 rank masks (x fastest), validated for coverage.
  data: each probe is a cube, layer = probe index, in the Vulkan cube
    convention (`reflection_probe.CUBE_FACES`: +X -X +Y -Y +Z -Z, face and
    (s, t) by the specification's selection table, sampled by the world
    direction directly; texel (column, row) at s = (column + 0.5) / size).
    Mip l is (S >> l)^2 per face, GGX-prefiltered at perceptual roughness
    l / (M - 1) (`reflection_probe.cube_mip_chain`). The radiance cube array
    is BC6H unsigned blocks (4 x 4 texels, 16 bytes), ordered mip-major, then
    probe, then face, each face-mip whole blocks (its size is >= 4). With
    relight cubes, two more arrays follow, RGBA16F, same order and size: the
    albedo array (RGB diffuse albedo 0..1 of what the capture saw, A the ray
    distance in Source units, 0 < d <= MAX_DISTANCE) and the normal array (RGB
    world shading normal, each component in [-1, 1], A 1). Mips are box
    filtered (normals renormalized).

Constraints: finite values under 1e6 in magnitude; box and influence min < max
per axis; the capture inside its parallax box; exactly one global probe.

The GPU form is a probe buffer plus the cube-array textures (`gpu_buffer`):
metadata and candidate masks in a storage buffer in float32/uint32 words, so
nothing is packed into half-float texels or tiled into an atlas.

Relighting (McAuley, "Rendering the World of Far Cry 4", GDC 2015: a G-buffer
cubemap relit at runtime; here with the distance too): at the lookup
direction and lod, the capture saw the point capture + direction * distance
with the band's albedo and normal. The consumer supplies the scene's diffuse
light there (irradiance / pi, the lightmap's unit) now and as baked, and the
moving occluders (axis-aligned boxes with a reflectance: a closed door, which
no bake contains). Per channel (`relight`):

  * light removed is relative: radiance * now / baked, so a surface that went
    dark reflects dark with its baked detail scaled, not the detail less a
    smooth estimate (visibility is multiplicative);
  * light added is absolute: radiance + albedo * (now - baked), as is any
    change where the baked light is below RELIGHT_FLOOR (no ratio);
  * clamped at zero.

It is the traced producers' own rule for a probe texel (indirect_sdf.h
TracedProducer::Change: removed relative, added absolute). It is exact in the
baked state (now == baked) and for a mirror reflection of a Lambertian
surface; for a rough lookup the light is taken at the lobe's centre.

An occluder on the segment from the capture to that point hides it
(`occluded_segment`): the capture now sees the occluder's face, whose
radiance is its reflectance times the light now at the entry point along the
face's outward normal. Other engines normalize a capture by the diffuse
light at the shaded point (Lazarov, "Getting More Physical in Call of Duty:
Black Ops II", SIGGRAPH 2013; Unreal Engine's reflection-capture lightmap
mixing); relighting at the point seen keeps the capture's parallax and
detail, and the occluder test lets a closed door appear in reflections
rather than only darkening them.

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
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "texture"))
import bc_codec  # noqa: E402
import reflection_probe  # noqa: E402

MAGIC = 0x42525052  # "RPRB"
# RPRB v8, the one version (2026-10-06): every earlier version is refused.
VERSION = 8
CANDIDATE_DIM = 32
CANDIDATE_CELLS = CANDIDATE_DIM ** 3
CANDIDATE_HEADER_BYTES = 32
COORDINATE_LIMIT = 65504.0  # the GPU table holds float16 hi/lo pairs
FLAG_RELIGHT = 1  # header flags: relight bands present
# Relight bands: ray distances (Source units) stay within half range; sky
# texels (Cycles' 1e10) are clamped here, where their albedo is 0.
MAX_DISTANCE = 60000.0
NORMAL_LIMIT = 1.001
HEADER_BYTES = 64
RECORD_BYTES = 80
MAX_PROBES = 256
MAX_MIPS = 12
MIN_FACE, MAX_FACE = 8, 1024
PREFILTER_VERSION = 1
FLAG_GLOBAL = 1
MAX_COORDINATE = 1.0e6
SOURCE_UNITS_PER_METER = 39.37007874015748
FACING_EDGE = 0.1
# Relighting (RPRB v2): below this baked diffuse light a removal is absolute
# (indirect_sdf.h's kRelativeFloor), and at most this many moving occluders.
RELIGHT_FLOOR = 1e-4
MAX_OCCLUDERS = 16
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

def mip_sizes(face, mips):
    return [face >> level for level in range(mips)]


def radiance_block_bytes(count, face, mips):
    """The v8 radiance cube array's BC6H bytes: mip-major, then probe, then
    the six faces, each face-mip whole 4x4 blocks."""
    return sum(count * 6 * bc_codec.block_bytes("bc6hu", size, size)
               for size in mip_sizes(face, mips))


def relight_bytes(count, face, mips):
    """Both relight cube arrays (albedo + distance, then normal): RGBA16F,
    mip-major, then probe, then face."""
    return 2 * sum(count * 6 * size * size * 8 for size in mip_sizes(face, mips))


def relight_chain(albedo, normal, distance, minimum_size=4):
    """A probe's relight cubes from its cube G-buffer (meters): box-filtered
    chains of (albedo rgb, distance) and of the unit normal. `albedo` and
    `normal` are (6, N, N, 3), `distance` (6, N, N)."""
    albedo_distance = np.concatenate((np.clip(albedo, 0.0, 1.0), distance[..., None]), axis=3)
    first = reflection_probe.cube_pyramid(albedo_distance, minimum_size)
    second = reflection_probe.cube_pyramid(normal, minimum_size)
    normals = []
    for level in second:
        length = np.linalg.norm(level, axis=3, keepdims=True)
        normals.append(np.where(length > 1e-6, level / np.maximum(length, 1e-6),
                                (0.0, 0.0, 1.0)))
    return first, normals


def ranks(probes):
    """Authored priority descending, then influence volume ascending;
    the global probe is always last. Serialized ranks own runtime ordering."""
    def volume(probe):
        return float(np.prod(np.asarray(probe["influence_max"]) -
                             np.asarray(probe["influence_min"])))
    order = sorted(range(len(probes)),
                   key=lambda i: (bool(probes[i].get("global")),
                                  -probes[i].get("priority", 0), volume(probes[i]), i))
    rank = [0] * len(probes)
    for position, index in enumerate(order):
        rank[index] = position
    return rank


def candidate_required(probes, origin, step):
    """Conservative rank masks for the serialized records, including GPU rounding.

    Axis overlap deliberately retains corner false positives. Facing and blend
    weights remain runtime inputs. Global probes are present in every cell.
    Cell order is x fastest, then y, then z. No light or probe is truncated.
    """
    z, y, x = np.indices((CANDIDATE_DIM,) * 3)
    low = np.stack((x.ravel(), y.ravel(), z.ravel()), axis=1) * step + origin
    high = low + step
    words = candidate_words(len(probes))
    masks = np.zeros((CANDIDATE_CELLS, words), dtype=np.uint64)
    for probe in probes:
        lo = probe["influence_min"] - probe["fade"]
        hi = probe["influence_max"] + probe["fade"]
        magnitude = np.maximum(np.max(np.abs(low), axis=1),
                               np.max(np.abs(high), axis=1))
        magnitude = np.maximum(magnitude, max(np.abs(lo).max(), np.abs(hi).max()))
        guard = 0.001 + magnitude * 1e-5
        reaches = np.ones(len(low), dtype=bool) if probe["global"] or probe["rank"] < 2 else np.all(
            (low <= hi + guard[:, None]) & (high >= lo - guard[:, None]), axis=1)
        masks[reaches, probe["rank"] // 64] |= np.uint64(1) << np.uint64(probe["rank"] % 64)
    return masks


def candidate_words(count):
    """uint64 rank words per cell: one per 64 probes."""
    return (count + 63) // 64


def candidate_bytes(count):
    return CANDIDATE_HEADER_BYTES + CANDIDATE_CELLS * 8 * candidate_words(count)


def candidate_grid(probes, bounds=None):
    """The CANDIDATE_DIM^3 grid over the local probes' reach. `bounds` (world
    min, max) limits it to where surfaces can be: a probe whose fitted box
    reaches past the world (a depth fit that saw the void) would otherwise
    stretch every cell over the whole map. Masks stay conservative inside the
    grid and every rank is a candidate outside it, so bounds change only the
    candidates visited, never a weight."""
    local = [p for p in probes if not p["global"]] or probes
    low = np.min([p["influence_min"] - p["fade"] for p in local], axis=0)
    high = np.max([p["influence_max"] + p["fade"] for p in local], axis=0)
    if bounds is not None:
        low = np.maximum(low, np.asarray(bounds[0], dtype=np.float64))
        high = np.minimum(high, np.asarray(bounds[1], dtype=np.float64))
        if np.any(high <= low):
            raise RprbError("InvalidCandidates", "the bounds miss every local probe")
    step = 2.0 ** math.ceil(math.log2(max(1.0, float(np.max(high - low)) /
                                       (CANDIDATE_DIM - 1))))
    origin = np.floor(low / step) * step
    return {"origin": origin.astype(np.float32), "step": step,
            "masks": candidate_required(probes, origin, step)}


def candidate_section(grid, count):
    """The grid's bytes: origin xyz, cell size (float32), dimension and
    words per cell (uint32), 8 zero bytes, then the masks (x fastest)."""
    masks = np.asarray(grid["masks"], dtype=np.uint64).reshape(CANDIDATE_CELLS, -1)
    if masks.shape[1] != candidate_words(count):
        raise RprbError("InvalidCandidates", "the masks have the wrong word count")
    return (struct.pack("<4f4I", *grid["origin"], grid["step"], CANDIDATE_DIM, masks.shape[1], 0, 0)
            + masks.astype("<u8").tobytes())


def serialized_probes(records, count):
    """The candidate grid's inputs as the runtime validator reads them."""
    serialized = []
    for index in range(count):
        f = struct.unpack_from("<16f4I", records, index * RECORD_BYTES)
        serialized.append({"influence_min": np.array(f[10:13]),
                           "influence_max": np.array(f[13:16]), "fade": f[3],
                           "rank": f[16], "global": bool(f[17] & FLAG_GLOBAL)})
    return serialized


def regrid(data, bounds):
    """RPRB bytes with the candidate grid rebuilt inside world `bounds`
    (Source units); records, header and atlas are unchanged."""
    layout = read(data)
    count = layout["count"]
    base_offset = (HEADER_BYTES + RECORD_BYTES * count + 15) & ~15
    grid = candidate_grid(serialized_probes(data[HEADER_BYTES:], count), bounds)
    section = candidate_section(grid, count)
    if base_offset + len(section) != layout["data_offset"]:
        raise RprbError("InvalidCandidates", "the grid section changed size")
    out = data[:base_offset] + section + data[base_offset + len(section):]
    read(out)
    return out


def build(probes, chains, scale=SOURCE_UNITS_PER_METER, relight=None, candidate_bounds=None,
          tool=None):
    """RPRB v8 bytes. `candidate_bounds` (min, max in Source units) limits the
    candidate grid to the playable envelope (candidate_grid). `probes`: dicts
    with capture, box_min, box_max, influence_min, influence_max (meters),
    fade (meters), global; `chains`: each probe's
    `reflection_probe.cube_mip_chain` (linear, already exposure scaled; mip
    i is (6, S >> i, S >> i, 3)). Exactly one probe is global. `relight`:
    each probe's `relight_chain` (distances in meters), mip for mip like its
    chain. `tool`: the pinned ktx (default bc_codec.default_tool())."""
    count = len(probes)
    if not 1 <= count <= MAX_PROBES or len(chains) != count:
        raise RprbError("InvalidCounts", "1..%d probes with one chain each" % MAX_PROBES)
    if sum(bool(probe.get("global")) for probe in probes) != 1:
        raise RprbError("InvalidGlobal", "exactly one probe must be global")
    mips = len(chains[0])
    face = chains[0][0].shape[1]
    for chain in chains:
        if len(chain) != mips or chain[0].shape[1] != face:
            raise RprbError("InvalidAtlas", "every probe needs the same mip count and face size")
        for level, mip in enumerate(chain):
            if mip.shape[:3] != (6, face >> level, face >> level):
                raise RprbError("InvalidAtlas", "mip %d has the wrong size" % level)
    if relight is not None:
        if len(relight) != count:
            raise RprbError("InvalidCounts", "one relight chain pair per probe")
        for first, second in relight:
            if len(first) != mips or len(second) != mips or any(
                    a.shape[:3] != b.shape[:3] or a.shape[:3] != chains[0][level].shape[:3]
                    for level, (a, b) in enumerate(zip(first, second))):
                raise RprbError("InvalidAtlas", "relight cubes must match the radiance mips")
    rank = ranks(probes)
    records = b""
    for index, probe in enumerate(probes):
        values = [np.asarray(probe[key], dtype=np.float64) * scale
                  for key in ("capture", "box_min", "box_max", "influence_min", "influence_max")]
        records += struct.pack("<3ff3f3f3f3fIIII", *values[0], probe["fade"] * scale,
                               *values[1], *values[2], *values[3], *values[4], rank[index],
                               FLAG_GLOBAL if probe.get("global") else 0, index,
                               index if relight is not None else 0)
    base_offset = (HEADER_BYTES + RECORD_BYTES * count + 15) & ~15
    # Read back the exact serialized floats used by the runtime validator.
    grid = candidate_grid(serialized_probes(records, count), candidate_bounds)
    candidate_data = candidate_section(grid, count)
    data_offset = base_offset + len(candidate_data)
    tool = tool or bc_codec.default_tool()
    if tool is None:
        raise RprbError("InvalidAtlas", "no pinned ktx tool to encode the radiance cubes")
    texels = []
    for level in range(mips):
        # All probes' faces of a mip as one tall image: faces are whole block
        # rows, so its blocks are the (probe, face) blocks in order.
        size = face >> level
        tall = np.stack([chain[level] for chain in chains]).reshape(count * 6 * size, size, 3)
        print("pack: BC6H mip %d (%d faces of %d texels)" % (level, count * 6, size),
              file=sys.stderr, flush=True)
        texels.append(bc_codec.encode_bc6h(tool, np.ascontiguousarray(tall.astype(np.float32))))
    if relight is not None:
        for which in (0, 1):
            for level in range(mips):
                size = face >> level
                cubes = [pair[which][level] for pair in relight]
                if which == 0:
                    cubes = [np.concatenate((c[..., :3], np.clip(
                        c[..., 3:4] * scale, 1.0 / 16.0, MAX_DISTANCE)), axis=3) for c in cubes]
                else:
                    cubes = [np.concatenate((c, np.ones(c.shape[:3] + (1,))), axis=3)
                             for c in cubes]
                texels.append(np.stack(cubes).astype("<f2").tobytes())
    payload = b"".join(texels)
    global_index = next(i for i, probe in enumerate(probes) if probe.get("global"))
    header = struct.pack("<IIIIIIIIQQIIII", MAGIC, VERSION, count, mips, face,
                         face if relight is not None else 0, 0, RECORD_BYTES, data_offset,
                         len(payload), PREFILTER_VERSION,
                         0 if relight is None else FLAG_RELIGHT, global_index, 0)
    data = header + records
    data += b"\0" * (base_offset - len(data)) + candidate_data + payload
    read(data)  # the writer never emits what the reader rejects
    return data


# The shared malformation corpus: (offset, struct format, value, error) edits
# of a valid two-probe payload. The C++ reader's suite applies the same edits
# (quality/fixtures/reflection/rprb/malformations.txt) and must report the
# same error for each.
MALFORMATIONS = (
    (0, "<I", 0x12345678, "BadMagic"),
    (4, "<I", 7, "UnsupportedVersion"),        # every version but 8 is refused
    (4, "<I", 1, "UnsupportedVersion"),
    (8, "<I", 0, "InvalidCounts"),
    (8, "<I", 257, "InvalidCounts"),
    (16, "<I", 48, "InvalidAtlas"),            # face not a power of two
    (28, "<I", 96, "InvalidAtlas"),            # record size
    (32, "<Q", 64 + 160 + 16, "InvalidSections"),
    (48, "<I", 2, "UnsupportedVersion"),       # prefilter version
    (52, "<I", 1, "InvalidAtlas"),             # the relight flag without its cubes
    (64 + 12, "<f", 0.0, "InvalidRecord"),     # fade
    (64 + 0, "<f", float("nan"), "InvalidRecord"),
    (64 + 0, "<f", -500.0, "InvalidRecord"),   # capture outside its box
    (64 + 28, "<f", -10.0, "InvalidRecord"),   # box max below min
    (64 + 40, "<f", 1.0e7, "InvalidRecord"),   # beyond the coordinate bound
    (64 + 64, "<I", 1, "InvalidRanks"),
    (64 + 68, "<I", 1, "InvalidGlobal"),       # two global probes
    (64 + 68, "<I", 2, "InvalidRecord"),       # unknown flag
    (64 + 72, "<I", 7, "InvalidRecord"),       # layer
    (64 + 80 + 64, "<I", 0, "InvalidRanks"),   # duplicate rank
    (56, "<I", 0, "InvalidGlobal"),            # header names a non-global probe
)

# Edits of the valid relight payload, which has two probes of face 16 and
# mips 2 (sizes 16, 8). Its data section follows the records and candidate
# grid: radiance BC6H (mip 0: 2 probes x 6 faces x 256 B, mip 1: 2 x 6 x 64
# B), then the albedo cube array (mip 0: 12 faces x 256 texels x 8 B, mip 1:
# 12 x 64 x 8), then the normal array. Offsets are computed by `relight_
# offsets` from the layout the fixture writer produces.
RELIGHT_FIXTURE_FACE = 16
RELIGHT_FIXTURE_MIPS = 2


def relight_offsets(count, face, mips, data_offset):
    """(albedo array start, normal array start, per-mip texel starts of the
    albedo array) in bytes from the payload start."""
    radiance = radiance_block_bytes(count, face, mips)
    albedo = data_offset + radiance
    normal = albedo + relight_bytes(count, face, mips) // 2
    starts, at = [], albedo
    for size in mip_sizes(face, mips):
        starts.append(at)
        at += count * 6 * size * size * 8
    return albedo, normal, starts


def relight_malformations(data_offset):
    albedo, normal, starts = relight_offsets(2, RELIGHT_FIXTURE_FACE, RELIGHT_FIXTURE_MIPS,
                                             data_offset)
    normal_starts = [normal + (start - albedo) for start in starts]
    texel = 8 * (3 * 16 + 5)  # a texel inside the first face's mip 0
    return (
        (52, "<I", 3, "UnsupportedVersion"),       # an unknown header flag
        (20, "<I", 8, "InvalidAtlas"),             # relight face differs from the face size
        (64 + 76, "<I", 1, "InvalidRecord"),       # a relight layer other than the probe's
        (64 + 80 + 76, "<I", 0, "InvalidRecord"),  # a probe without its relight layer
        (starts[0] + texel + 0, "<e", 1.5, "InvalidTexels"),    # albedo > 1
        (starts[0] + texel + 6, "<e", 0.0, "InvalidTexels"),    # distance 0
        (starts[0] + texel + 6, "<e", -2.0, "InvalidTexels"),   # negative
        (normal_starts[0] + texel + 2, "<e", 1.5, "InvalidTexels"),  # normal > 1
        (normal_starts[0] + texel + 6, "<e", 0.5, "InvalidTexels"),  # normal alpha
    )


def read(data):
    """Validate RPRB bytes; return the layout and records (Source units)."""
    if len(data) < HEADER_BYTES:
        raise RprbError("Truncated")
    (magic, version, count, mips, face, relight_face, reserved0, record_bytes, data_offset,
     data_bytes, prefilter, flags, global_index, reserved) = struct.unpack_from(
        "<IIIIIIIIQQIIII", data)
    if magic != MAGIC:
        raise RprbError("BadMagic")
    if (version != VERSION or prefilter != PREFILTER_VERSION or
            flags not in (0, FLAG_RELIGHT) or reserved):
        raise RprbError("UnsupportedVersion")
    relight = bool(flags & FLAG_RELIGHT)
    if not 1 <= count <= MAX_PROBES or not 1 <= mips <= MAX_MIPS:
        raise RprbError("InvalidCounts")
    if (face & (face - 1) or not MIN_FACE <= face <= MAX_FACE or (face >> (mips - 1)) < 4
            or relight_face != (face if relight else 0) or reserved0
            or record_bytes != RECORD_BYTES):
        raise RprbError("InvalidAtlas")
    base_offset = (HEADER_BYTES + RECORD_BYTES * count + 15) & ~15
    words = candidate_words(count)
    expected_offset = base_offset + candidate_bytes(count)
    radiance_bytes = radiance_block_bytes(count, face, mips)
    expected_bytes = radiance_bytes + (relight_bytes(count, face, mips) if relight else 0)
    if (data_offset != expected_offset or data_bytes != expected_bytes or
            len(data) != data_offset + data_bytes):
        raise RprbError("InvalidSections")
    if any(data[HEADER_BYTES + RECORD_BYTES * count:base_offset]):
        raise RprbError("InvalidSections", "nonzero padding")
    probes = []
    for index in range(count):
        fields = struct.unpack_from("<3ff3f3f3f3fIIII", data, HEADER_BYTES + index * RECORD_BYTES)
        floats = np.array(fields[:16], dtype=np.float64)
        rank, flag, layer, relight_layer = fields[16:]
        probe = {"capture": floats[0:3], "fade": float(floats[3]), "box_min": floats[4:7],
                 "box_max": floats[7:10], "influence_min": floats[10:13],
                 "influence_max": floats[13:16], "rank": rank,
                 "global": bool(flag & FLAG_GLOBAL)}
        if (not np.all(np.isfinite(floats)) or np.any(np.abs(floats) > COORDINATE_LIMIT) or
                probe["fade"] <= 0 or np.any(probe["box_min"] >= probe["box_max"]) or
                np.any(probe["influence_min"] >= probe["influence_max"]) or
                np.any(probe["capture"] < probe["box_min"]) or
                np.any(probe["capture"] > probe["box_max"])):
            raise RprbError("InvalidRecord", str(index))
        if (flag & ~FLAG_GLOBAL or layer != index or
                relight_layer != (index if relight else 0)):
            raise RprbError("InvalidRecord", "flags %d" % index)
        probes.append(probe)
    if sorted(probe["rank"] for probe in probes) != list(range(count)):
        raise RprbError("InvalidRanks")
    if (sum(probe["global"] for probe in probes) != 1 or global_index >= count or
            not probes[global_index]["global"] or probes[global_index]["rank"] != count - 1):
        raise RprbError("InvalidGlobal")
    fields = struct.unpack_from("<4f4I", data, base_offset)
    origin, step = np.array(fields[:3]), fields[3]
    if (not np.isfinite(fields[:4]).all() or np.abs(origin).max() > 4e6 or
            not 1 <= step <= 4e6 or math.frexp(step)[0] != 0.5 or
            fields[4] != CANDIDATE_DIM or fields[5] != words or any(fields[6:])):
        raise RprbError("InvalidCandidates", "invalid grid header")
    masks = np.frombuffer(data, dtype="<u8", count=CANDIDATE_CELLS * words,
                          offset=base_offset + CANDIDATE_HEADER_BYTES).reshape(CANDIDATE_CELLS, words)
    valid = np.array([(1 << max(0, min(64, count - 64 * word))) - 1
                      for word in range(words)], dtype=np.uint64)
    required = candidate_required(probes, origin, step)
    if np.any(masks & ~valid) or np.any((masks & required) != required):
        raise RprbError("InvalidCandidates", "missing required or out-of-range rank")
    candidates = {"origin": origin, "step": step, "masks": masks}
    # The radiance cubes as the C++ consumers see them: BC6H decoded to half
    # floats. chains[probe][mip] is (6, S, S, 3).
    chains = [[None] * mips for _ in range(count)]
    at = data_offset
    for level, size in enumerate(mip_sizes(face, mips)):
        length = count * 6 * bc_codec.block_bytes("bc6hu", size, size)
        light = bc_codec.decode_bc6h(data[at:at + length], size, count * 6 * size)
        light = light.astype("<f2").astype(np.float64).reshape(count, 6, size, size, 3)
        for index in range(count):
            chains[index][level] = light[index]
        at += length
    relight_chains = None
    if relight:
        arrays = []
        for which in (0, 1):
            levels = []
            for size in mip_sizes(face, mips):
                length = count * 6 * size * size * 8
                levels.append(np.frombuffer(data, dtype="<f2", count=count * 6 * size * size * 4,
                                            offset=at).astype(np.float64).reshape(
                    count, 6, size, size, 4))
                at += length
            arrays.append(levels)
        for levels in arrays:
            for array in levels:
                if not np.all(np.isfinite(array)):
                    raise RprbError("InvalidTexels")
        for levels in arrays[:1]:
            for array in levels:
                if (array[..., :3].min() < 0 or array[..., :3].max() > 1.0 or
                        array[..., 3].min() <= 0.0 or array[..., 3].max() > MAX_DISTANCE):
                    raise RprbError("InvalidTexels")
        for array in arrays[1]:
            if np.any(np.abs(array[..., :3]) > NORMAL_LIMIT) or np.any(array[..., 3] != 1.0):
                raise RprbError("InvalidTexels")
        relight_chains = [([arrays[0][level][index] for level in range(mips)],
                           [arrays[1][level][index][..., :3] for level in range(mips)])
                          for index in range(count)]
    return {"count": count, "mips": mips, "face": face, "data_offset": data_offset,
            "probes": probes, "chains": chains, "relight": relight_chains,
            "global_index": global_index, "candidates": candidates}


# The storage buffer the shaders read (mapcontainer::WriteReflectionProbeBuffer):
# little-endian 32-bit words. Header, 16 words: count, mips, face size,
# candidate words per cell, mode, relight switch (1 when probes are relit:
# `mat_reflection_relight` and relight cubes present), candidate dimension, base mip (the radiance array's first lump mip),
# candidate origin xyz (float), candidate step (float), four zeros. From word
# GPU_PROBES_WORD, one 20-float record per rank (capture.xyz, fade | box
# min.xyz, layer | box max.xyz, global | influence min.xyz, relight layer |
# influence max.xyz, 0); from GPU_MASKS_WORD the candidate masks as lo/hi
# word pairs. Radiance and relight live in cube-array textures, not here.
GPU_HEADER_WORDS = 16
GPU_PROBES_WORD = 16
GPU_RECORD_WORDS = 20
GPU_MASKS_WORD = GPU_PROBES_WORD + GPU_RECORD_WORDS * MAX_PROBES


def gpu_buffer(layout, mode=MODE_BLEND, relight=True, base_mip=0):
    """The shader's probe buffer for a read `layout`, as uint32 words."""
    count = layout["count"]
    words = candidate_words(count)
    candidates = layout["candidates"]
    out = np.zeros(GPU_MASKS_WORD + CANDIDATE_CELLS * words * 2, dtype=np.uint32)
    out[0:8] = (count, layout["mips"], layout["face"], words, mode,
                1 if relight and layout["relight"] is not None else 0, CANDIDATE_DIM, base_mip)
    out[8:12] = np.array([*candidates["origin"], candidates["step"]],
                         dtype=np.float32).view(np.uint32)
    for index, probe in enumerate(layout["probes"]):
        values = np.array([(*probe["capture"], probe["fade"]),
                           (*probe["box_min"], index),
                           (*probe["box_max"], 1.0 if probe["global"] else 0.0),
                           (*probe["influence_min"], index if layout["relight"] else 0.0),
                           (*probe["influence_max"], 0.0)], dtype=np.float32)
        at = GPU_PROBES_WORD + probe["rank"] * GPU_RECORD_WORDS
        out[at:at + GPU_RECORD_WORDS] = values.reshape(-1).view(np.uint32)
    out[GPU_MASKS_WORD:] = np.frombuffer(candidates["masks"].astype("<u8").tobytes(),
                                         dtype="<u4")
    return out


def buffer_table(buffer):
    """Decode the buffer's probe records back to (count, 5, 4) float32 in rank
    order (the shader's view)."""
    count = int(buffer[0])
    at = GPU_PROBES_WORD
    return buffer[at:at + GPU_RECORD_WORDS * count].view(np.float32).reshape(count, 5, 4)


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


def relight(radiance, albedo, now, baked):
    """RPRB v2's relight rule per channel (see the module notes): light
    removed scales the baked radiance, light added is albedo times the
    increase; clamped at zero."""
    relative = (now < baked) & (baked > RELIGHT_FLOOR)
    ratio = np.maximum(now, 0.0) / np.where(relative, baked, 1.0)
    return np.maximum(np.where(relative, radiance * ratio, radiance + albedo * (now - baked)),
                      0.0)


def occluded_segment(origin, directions, lengths, occluders):
    """The nearest occluder box on each segment origin + t * direction,
    0 <= t < length: (hit, entry t, the entered face's outward normal, the
    occluder's reflectance). A segment that starts inside a box is hidden at
    t = 0 with the normal facing back along it."""
    count = len(directions)
    hit = np.zeros(count, dtype=bool)
    best = np.asarray(lengths, dtype=np.float64).copy()
    normal = np.zeros((count, 3))
    reflectance = np.zeros(count)
    safe = np.where(directions >= 0.0, np.maximum(directions, 1e-12),
                    np.minimum(directions, -1e-12))
    for lo, hi, value in occluders:
        first = (np.asarray(lo, dtype=np.float64) - origin) / safe
        second = (np.asarray(hi, dtype=np.float64) - origin) / safe
        near_axes = np.minimum(first, second)
        near = near_axes.max(axis=1)
        far = np.maximum(first, second).min(axis=1)
        entry = np.maximum(near, 0.0)
        found = (far >= entry) & (entry < best)
        if not found.any():
            continue
        axis = near_axes.argmax(axis=1)
        face = np.zeros((count, 3))
        face[np.arange(count), axis] = -np.sign(safe[np.arange(count), axis])
        face = np.where((near < 0.0)[:, None], -directions, face)
        hit |= found
        best = np.where(found, entry, best)
        normal = np.where(found[:, None], face, normal)
        reflectance = np.where(found, value, reflectance)
    return hit, best, normal, reflectance


def probe_radiance(points, reflected, roughness, probe, chain, parallax=True, bands=None,
                   light=None, occluders=()):
    """One probe's split-sum fetch for rays leaving shaded points; with its
    relight `bands`, a `light(points, normals) -> (now, baked)` (the scene's
    diffuse light, irradiance / pi, at world points) and the moving
    `occluders` ((lo, hi, reflectance) boxes), relit (see RPRB v2)."""
    direction, local = reflected, roughness
    if parallax:
        lookup, shaded, captured, valid = reflection_probe.parallax_lookup(
            points, reflected, probe["capture"], probe["box_min"], probe["box_max"])
        local = np.where(valid, reflection_probe.distance_roughness(roughness, shaded,
                                                                    captured), roughness)
        direction = reflection_probe.corrected_direction(lookup, reflected, roughness)
    radiance = reflection_probe.sample_cube_chain(chain, direction, local)
    if bands is None or light is None:
        return radiance
    albedo = reflection_probe.sample_cube_chain(bands[0], direction, local, channels=4)
    normal = reflection_probe.sample_cube_chain(bands[1], direction, local)
    normal /= np.maximum(np.linalg.norm(normal, axis=1, keepdims=True), 1e-12)
    capture = np.asarray(probe["capture"], dtype=np.float64)
    hidden, t, face, reflectance = occluded_segment(capture, direction, albedo[:, 3],
                                                    list(occluders)[:MAX_OCCLUDERS])
    seen = capture + direction * np.where(hidden, t, albedo[:, 3])[:, None]
    now, baked = light(seen, np.where(hidden[:, None], face, normal))
    relit = relight(radiance, albedo[:, :3], now, baked)
    return np.where(hidden[:, None], np.maximum(reflectance[:, None] * now, 0.0), relit)


def shade(points, normals, reflected, roughness, layout, mode=MODE_BLEND, light=None,
          occluders=()):
    """The blended probe radiance the shader computes (world units of the
    layout, directions unit). Oracle for world_pbr_probe.glsl. `light` and
    `occluders` relight the probes that carry relight bands (None: as
    baked)."""
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
                parallax=selection != MODE_DIRECTION, bands=bands, light=light,
                occluders=occluders)
    return result


# ------------------------------------------------------------- placement

PLACEMENT_DEFAULTS = {
    "spacing_m": 0.75,         # walkable sample grid
    "eye_height_m": 1.63,      # a standing player's eye (64 Source units)
    "headroom_m": 0.2,         # clearance above the eye for a walkable sample
    "fit_rays": 2048,          # directions for a candidate's proxy-box estimate
    "max_probes": 8,
    "glossy_roughness": 0.35,  # perceptual roughness at or below which a surface is glossy
    "glossy_radius_m": 2.5,    # a glossy sample needs a capture this near that sees it
    "glossy_samples_per_m2": 16.0,
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
    # At most this many walkable samples (None: no cap). Placement fits a
    # box with fit_rays rays per sample and tests visibility between pairs,
    # so a whole retail map at 0.75 m (tens of thousands of samples) takes
    # hours; the grid's spacing widens until the samples fit. The RPRB holds
    # max_probes captures whatever the spacing.
    "max_walkable": None,
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


def room_bounded(box, fit, covered, capture, bounds_min, bounds_max, fade):
    """A placed probe's box with every open face (its fit saw sky or void
    there: reflection_probe.OPEN_EXTENT) cut back to the probe's own room.

    Horizontally the room is what the probe covers: the walkable samples it
    sees inside its box, plus the fade. Vertically it is the placement
    bounds, so walls above eye height in an open-roofed space keep their
    probe. Without this an open face reaches about a kilometre: the probe
    then wins the blend on surfaces far from its capture (on
    sp_a1_intro4_relit, 9% of the playable surface took a reflection
    captured a median 2,254 units away) and stretches the candidate grid.
    """
    box_min, box_max = (np.array(value, dtype=np.float64) for value in box)
    faces = fit.get("faces", {})
    points = np.asarray(covered, dtype=np.float64).reshape(-1, 3)
    if not len(points):
        points = np.asarray(capture, dtype=np.float64).reshape(1, 3)
    for name, axis, sign in reflection_probe.AXES:
        if not faces.get(name, {}).get("open"):
            continue
        if axis == 2:
            limit = bounds_max[2] if sign > 0 else bounds_min[2]
        else:
            limit = points[:, axis].max() + fade if sign > 0 else points[:, axis].min() - fade
        if sign > 0:
            box_max[axis] = min(box_max[axis], max(limit, capture[axis] + fade))
        else:
            box_min[axis] = max(box_min[axis], min(limit, capture[axis] - fade))
    return box_min, box_max


def visible(raycast, origins, targets):
    delta = np.asarray(targets) - np.asarray(origins)
    length = np.linalg.norm(delta, axis=1)
    safe = np.maximum(length, 1e-9)
    distance, _ = raycast(np.asarray(origins, dtype=np.float64), delta / safe[:, None], 1.0e4)
    return ~(distance < length - 1e-3)


def inside(points, box_min, box_max, margin=0.05):
    return np.all((points >= box_min - margin) & (points <= box_max + margin), axis=1)


def authored_volume(record, params):
    """Validate one authored room in stage meters before any placement/bake work."""
    if not isinstance(record, dict):
        raise ValueError("authored probe must be an object")
    keys = ("capture", "box_min", "box_max", "influence_min", "influence_max")
    result = dict(record)
    for key in keys:
        value = np.asarray(record.get(key), dtype=np.float64)
        if value.shape != (3,) or not np.isfinite(value).all() or \
                np.any(np.abs(value) * SOURCE_UNITS_PER_METER > MAX_COORDINATE):
            raise ValueError("authored probe needs finite in-range " + key)
        result[key] = value
    for lower, upper in (("box_min", "box_max"), ("influence_min", "influence_max")):
        if np.any(result[lower] >= result[upper]):
            raise ValueError("authored probe has an empty " + lower)
    if np.any(result["capture"] < result["box_min"]) or \
            np.any(result["capture"] > result["box_max"]):
        raise ValueError("authored probe capture must be inside its proxy box")
    result["fade"] = float(record.get("fade", params["fade_m"]))
    if not math.isfinite(result["fade"]) or not 0 < result["fade"] <= \
            MAX_COORDINATE / SOURCE_UNITS_PER_METER:
        raise ValueError("authored probe fade must be finite and positive")
    priority = record.get("priority", 0)
    if type(priority) is not int or not -32768 <= priority <= 32767:
        raise ValueError("authored probe priority must be a signed 16-bit integer")
    if type(record.get("global", False)) is not bool:
        raise ValueError("authored probe global must be boolean")
    result.update(priority=priority, role="authored", seeded=True,
                  global_probe=bool(record.get("global", False)))
    return result


def select_coverage(covers, glossy_views, uncovered, unserved, eligible, preferred, capacity,
                    walkable_limit, glossy_limit):
    """Choose captures jointly for both coverage obligations, without truncation.

    Each iteration strictly removes uncovered samples. A finite candidate set
    bounds the search; exhaustion reports failure rather than looping or
    weakening the limits. Ties are deterministic in candidate order.
    """
    selected = []
    while True:
        room_need = max(0, int(uncovered.sum()) - walkable_limit)
        glossy_need = max(0, int(unserved.sum()) - glossy_limit)
        if not room_need and not glossy_need:
            return selected, "coverage_pass"
        room_gain = (covers & uncovered).sum(axis=1)
        glossy_gain = (glossy_views & unserved).sum(axis=1)
        score = (np.minimum(room_gain, room_need) / max(1, room_need) +
                 np.minimum(glossy_gain, glossy_need) / max(1, glossy_need))
        score[selected] = 0
        # A capture with a bad proxy fit cannot enter the shipped resource.
        # Coverage is useful only when the resulting parallax correction is
        # valid, so an ineligible candidate must never be selected merely to
        # satisfy a coverage count.
        score[~eligible] = 0
        best_score = score.max()
        if best_score <= 0:
            return selected, "no_progress"
        if len(selected) >= capacity:
            return selected, "max_probes"
        choices = np.nonzero(score == best_score)[0]
        best_fit = choices[preferred[choices]]
        best = int(best_fit[0] if len(best_fit) else choices[0])
        selected.append(best)
        uncovered &= ~covers[best]
        unserved &= ~glossy_views[best]


def place(raycast, bounds_min, bounds_max, glossy=None, params=None, seeds=(), volumes=(),
          coverage_rules=None):
    """Probe captures, proxy boxes and influence volumes (meters).

    Authored probes are retained. Automatic captures jointly cover walkable
    samples (eye height over floors, within the estimated box and visible)
    and nearby visible glossy surfaces. Each insertion removes uncovered
    samples until both profile coverage thresholds pass. Capacity exhaustion
    and an empty useful candidate set remain explicit preflight failures.
    The room probe covering the most walkable samples is global (a box open
    to the sky can be the largest while covering a window bay). `glossy`: (points, normals) arrays or None. `seeds`:
    captures placed first, as room probes (a manifest's chosen positions).
    Returns (probes, report); the proxy boxes here are estimates that the
    capture's depth faces replace.
    """
    params = dict(PLACEMENT_DEFAULTS, **(params or {}))
    authored = [authored_volume(record, params) for record in volumes]
    if type(params["max_probes"]) is not int or not 1 <= params["max_probes"] <= MAX_PROBES:
        raise ValueError("max_probes must be between 1 and %d" % MAX_PROBES)
    if len(authored) + len(seeds) > params["max_probes"]:
        raise ValueError("authored probes exceed the requested probe capacity")
    if sum(probe["global_probe"] for probe in authored) > 1:
        raise ValueError("at most one authored global probe is allowed")
    bounds_min = np.asarray(bounds_min, dtype=np.float64)
    bounds_max = np.asarray(bounds_max, dtype=np.float64)
    walkable = walkable_samples(raycast, bounds_min, bounds_max, params)
    cap = params["max_walkable"]
    while cap and len(walkable) > cap:
        # Samples lie on a 2D grid of columns: their count falls as spacing^2.
        params["spacing_m"] *= max(math.sqrt(len(walkable) / cap), 1.05)
        walkable = walkable_samples(raycast, bounds_min, bounds_max, params)
    if not len(walkable):
        raise ValueError("no walkable sample under the bounds: nowhere to place a probe")
    directions = fibonacci_directions(params["fit_rays"])
    # A native-scene caster may fan the independent candidate fits out over
    # workers.  Each fit still casts the same directions in the same order;
    # the generic/oracle path remains the serial definition.
    estimates = raycast.estimate_boxes(walkable, directions) \
        if hasattr(raycast, "estimate_boxes") else \
        [estimate_box(raycast, candidate, directions) for candidate in walkable]
    boxes = [estimate[:2] for estimate in estimates]
    rules = coverage_rules or {}
    residual_limit = rules.get("max_reflection_probe_residual")
    # The percentile is a useful quality ranking, but it is deliberately
    # conservative at doorways and cannot be the admission test. The profile
    # owns the shipped mean-residual limit; when it is present this is the
    # hard admission test that coverage may not bypass.
    eligible = np.array([
        estimate[2]["clearance"] >= params["min_clearance_m"] and
        (residual_limit is None or
         estimate[2]["mean_relative_residual"] <= residual_limit)
        for estimate in estimates])
    preferred = np.array([
        estimate[2]["p90_relative_residual"] <= params["max_candidate_residual"]
        for estimate in estimates])
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
    for probe in authored:
        within = np.nonzero(influence_weight(walkable, probe) > 0)[0]
        seen = np.zeros(len(walkable), dtype=bool)
        seen[within] = visible(raycast, np.tile(probe["capture"], (len(within), 1)),
                               walkable[within])
        probe["covers"] = int((seen & uncovered).sum())
        probes.append(probe)
        uncovered &= ~seen
    for seed in seeds:
        seed = np.asarray(seed, dtype=np.float64)
        box_min, box_max, fit = estimate_box(raycast, seed, directions)
        within = np.nonzero(inside(walkable, box_min, box_max, params["fade_m"]))[0]
        seen = np.zeros(len(walkable), dtype=bool)
        seen[within] = visible(raycast, np.tile(seed, (len(within), 1)), walkable[within])
        box_min, box_max = room_bounded((box_min, box_max), fit, walkable[seen], seed,
                                        bounds_min, bounds_max, params["fade_m"])
        probes.append({"capture": seed.copy(), "box_min": box_min, "box_max": box_max,
                       "influence_min": box_min - margin, "influence_max": box_max + margin,
                       "fade": params["fade_m"], "role": "room", "seeded": True,
                       "covers": int((seen & uncovered).sum())})
        uncovered &= ~seen
    candidate_views = np.zeros((len(walkable), 0), dtype=bool)
    servable = served = np.zeros(0, dtype=bool)
    report = {}
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
        report.update(glossy_samples=int(len(points)), glossy_servable=int(servable.sum()))
    unserved = servable & ~served
    initial_uncovered = uncovered.copy()
    chosen, stop = select_coverage(
        covers, candidate_views, uncovered, unserved, eligible, preferred,
        params["max_probes"] - len(probes),
        math.floor(len(walkable) * rules.get("max_uncovered_walkable_fraction", 0)),
        math.floor(int(servable.sum()) * rules.get("max_unserved_glossy_fraction", 0)))
    for index in chosen:
        box_min, box_max = room_bounded(boxes[index], estimates[index][2], walkable[covers[index]],
                                        walkable[index], bounds_min, bounds_max, params["fade_m"])
        influence_min, influence_max = box_min - margin, box_max + margin
        # A joint capture also serves visible glossy points within the same
        # near-field radius; include those in its authored influence.
        if candidate_views[index].any():
            visible_points = points[candidate_views[index]]
            influence_min = np.minimum(influence_min, visible_points.min(axis=0) - margin)
            influence_max = np.maximum(influence_max, visible_points.max(axis=0) + margin)
        probes.append({"capture": walkable[index].copy(), "box_min": box_min,
                       "box_max": box_max, "influence_min": influence_min,
                       "influence_max": influence_max, "fade": params["fade_m"],
                       "role": "joint", "covers": int((covers[index] & initial_uncovered).sum())})
        initial_uncovered &= ~covers[index]
    report.update(walkable_samples=int(len(walkable)), walkable_spacing_m=params["spacing_m"],
                  uncovered_walkable=int(uncovered.sum()), room_stop=stop)
    if len(servable):
        report.update(unserved_glossy=int(unserved.sum()), glossy_stop=stop)
    rooms = [probe for probe in probes if probe["role"] in ("room", "authored", "joint")]
    chosen_global = next((probe for probe in authored if probe["global_probe"]), None)
    if chosen_global is None:
        chosen_global = max(rooms, key=lambda probe: probe["covers"])
    chosen_global["global"] = True
    report["probes"] = len(probes)
    return probes, report


def glossy_samples(triangles, density, seed=0, return_indices=False):
    """Area-uniform points and normals on triangles ((N, 3, 3) arrays).

    With ``return_indices``, also return each point's source-triangle index.
    A renderer that owns authored shading normals can then select the exact
    triangle's normal without a quadratic nearest-centroid reconstruction.
    """
    triangles = np.asarray(triangles, dtype=np.float64).reshape(-1, 3, 3)
    edges = np.cross(triangles[:, 1] - triangles[:, 0], triangles[:, 2] - triangles[:, 0])
    area = np.linalg.norm(edges, axis=1) / 2
    keep = area > 1e-9
    source_indices = np.flatnonzero(keep)
    triangles, edges, area = triangles[keep], edges[keep], area[keep]
    if not len(area):
        empty = np.zeros((0, 3))
        return (empty, empty, np.zeros(0, dtype=np.int64)) if return_indices else (empty, empty)
    count = max(1, int(round(area.sum() * density)))
    rng = np.random.default_rng(seed)
    chosen = rng.choice(len(area), size=count, p=area / area.sum())
    u, v = rng.random(count), rng.random(count)
    flip = u + v > 1
    u[flip], v[flip] = 1 - u[flip], 1 - v[flip]
    t = triangles[chosen]
    points = t[:, 0] + u[:, None] * (t[:, 1] - t[:, 0]) + v[:, None] * (t[:, 2] - t[:, 0])
    normals = edges[chosen] / (2 * area[chosen, None])
    return (points, normals, source_indices[chosen]) if return_indices else (points, normals)


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


def _pack_probe(job):
    """One probe of `pack`: verify and read its faces, fit its proxy box and
    prefilter its cube chain. Pure in its arguments, so the probes run in
    parallel processes and the result does not depend on the schedule."""
    import gi_reference
    import hashlib
    probes_dir, record, gbuffer, face, gain, prefilter_samples, margin = job
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
    probe = {"capture": capture, "box_min": box_min, "box_max": box_max,
             "influence_min": influence_min, "influence_max": influence_max,
             "fade": record["fade"], "role": record["role"],
             "priority": record.get("priority", 0), "name": record.get("name")}
    chain = [mip * gain for mip in reflection_probe.cube_mip_chain(
        reflection_probe.faces_to_cube(colors, face), samples=prefilter_samples)]
    bands = None
    if gbuffer:
        distance = reflection_probe.faces_to_cube(
            {name: np.minimum(depth, MAX_DISTANCE / SOURCE_UNITS_PER_METER)[..., None]
             for name, depth in distances.items()}, face, channels=1)[..., 0]
        bands = relight_chain(reflection_probe.faces_to_cube(albedos, face),
                              reflection_probe.faces_to_cube(normals, face), distance)
    report = dict(report, index=record["index"], role=record["role"],
                  capture=list(capture), priority=record.get("priority", 0),
                  name=record.get("name"), box_min=list(box_min), box_max=list(box_max),
                  depth_convention=convention, depth_check_median_error=medians[convention])
    return probe, chain, bands, report, convention


def pack(probes_dir, face, gain, prefilter_samples=256, max_mean_relative_residual=None,
         candidate_bounds_m=None, tool=None, workers=None):
    """(RPRB bytes, receipt) from `pbrt_reflection_probe.py`'s output. The
    probes are prefiltered in `workers` processes (default: the CPUs, at most
    one per probe); the bytes do not depend on it."""
    import concurrent.futures
    import hashlib
    receipt = json.loads((probes_dir / "probes.json").read_text())
    if receipt.get("status") != "pass" or receipt.get("schema") != "reflection-probe-faces/v2":
        raise ValueError("probe faces receipt is missing or not a v2 pass")
    margin = receipt["placement"]["influence_margin_m"]
    # Faces rendered with the Diffuse Color and Normal passes carry the
    # relight G-buffer (RPRB v2).
    gbuffer = bool(receipt.get("gbuffer"))
    total = len(receipt["probes"])
    jobs = [(probes_dir, record, gbuffer, face, gain, prefilter_samples, margin)
            for record in receipt["probes"]]
    workers = max(1, min(workers or os.cpu_count() or 1, total))
    print("pack: %d probes on %d worker%s" % (total, workers, "" if workers == 1 else "s"),
          file=sys.stderr, flush=True)
    results = []
    if workers == 1:
        for index, job in enumerate(jobs):
            # Progress: the pipeline stops a step that prints nothing for 10
            # minutes, and prefiltering a large set takes longer than that.
            print("pack: probe %d/%d (%s)" % (index + 1, total, job[1]["dir"]), file=sys.stderr,
                  flush=True)
            results.append(_pack_probe(job))
    else:
        with concurrent.futures.ProcessPoolExecutor(max_workers=workers) as pool:
            for index, result in enumerate(pool.map(_pack_probe, jobs)):
                print("pack: probe %d/%d (%s)" % (index + 1, total, jobs[index][1]["dir"]),
                      file=sys.stderr, flush=True)
                results.append(result)
    probes = [result[0] for result in results]
    chains = [result[1] for result in results]
    relight = [result[2] for result in results]
    reports = [result[3] for result in results]
    conventions = {result[4] for result in results}
    residuals = [report["mean_relative_residual"] for report in reports]
    if max_mean_relative_residual is not None and max(residuals) > max_mean_relative_residual:
        raise ValueError("reflection probe proxy residual %.6g exceeds the runtime limit %g" %
                         (max(residuals), max_mean_relative_residual))
    # Placement's global probe: the room probe covering the most walkable
    # space.
    global_index = next(i for i, record in enumerate(receipt["probes"]) if record["global"])
    probes[global_index]["global"] = True
    bounds = None
    if candidate_bounds_m is not None:
        bounds = (np.asarray(candidate_bounds_m[0], dtype=np.float64) * SOURCE_UNITS_PER_METER,
                  np.asarray(candidate_bounds_m[1], dtype=np.float64) * SOURCE_UNITS_PER_METER)
    print("pack: encoding %d probes (BC6H cube array)" % total, file=sys.stderr, flush=True)
    data = build(probes, chains, tool=tool, relight=relight if gbuffer else None,
                 candidate_bounds=bounds)
    print("pack: encoded %d bytes" % len(data), file=sys.stderr, flush=True)
    masks = read(data)["candidates"]["masks"]
    candidate_counts = [sum(int(mask).bit_count() for mask in row)
                        for row in masks.reshape(CANDIDATE_CELLS, -1)]
    return data, {"status": "pass", "schema": "rprb-pack/v1", "probes": len(probes),
                  "face": face, "mips": len(chains[0]), "preview_gain": gain,
                  "relight": gbuffer,
                  "candidate_grid": {"dimensions": [CANDIDATE_DIM] * 3,
                                     "bytes": candidate_bytes(len(probes)),
                                     "mean_candidates": float(np.mean(candidate_counts)),
                                     "max_candidates": max(candidate_counts)},
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
        relight = [([np.concatenate((level[..., :3], level[..., 3:4] * scale), axis=3)
                     for level in first], second) for first, second in layout["relight"]]
    return dict(layout, probes=probes, relight=relight)


FIXTURE_DIR = Path(__file__).resolve().parents[2] / "quality/fixtures/reflection/rprb"


def fixture_directions(face=16):
    """(6 * face * face, 3) unit directions of a cube's texels, in the face
    and row-major order the cube stores them."""
    return np.concatenate([reflection_probe.cube_face_directions(name, face).reshape(-1, 3)
                           for name in reflection_probe.CUBE_FACES])


def fixture_layout(face=16, minimum_size=8):
    """The shared two-probe fixture (meters): a 6 x 4 x 3 m striped room, a
    small probe at x = 1.5 and the global one at x = 4.5, face 16, 2 mips (`face` and
    `minimum_size` give the longer chains of the base-mip fixture)."""
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
        directions = fixture_directions(face)
        distance, _ = scene(np.tile(probe["capture"], (len(directions), 1)), directions, 1e4)
        hits = probe["capture"] + distance[:, None] * directions
        phase = np.floor(hits[:, 0] / 0.5) + 2 * np.floor(hits[:, 1] / 0.5) + \
            3 * np.floor(hits[:, 2] / 0.5)
        radiance = np.stack((0.2 + 0.8 * (phase % 2), 0.2 + 0.8 * ((phase // 2) % 2),
                             0.2 + 0.8 * ((phase // 4) % 2)), axis=1).reshape(6, face, face, 3)
        chains.append(reflection_probe.cube_mip_chain(radiance, minimum_size=minimum_size,
                                                      samples=16))
    return probes, chains


def fixture_relight(probes):
    """Relight bands for the shared fixture: the room's surfaces seen from
    each capture (distance and the inward normal), with an albedo of stripes
    that differ from the radiance's."""
    room = ((0.0, 0.0, 0.0), (6.0, 4.0, 3.0))
    scene = BoxScene(rooms=[room])
    bands = []
    for probe in probes:
        directions = fixture_directions()
        distance, normal = scene(np.tile(probe["capture"], (len(directions), 1)), directions,
                                 1e4)
        hits = probe["capture"] + distance[:, None] * directions
        stripe = np.floor(hits[:, 0] / 0.75) + np.floor(hits[:, 2] / 0.75)
        albedo = np.stack((0.3 + 0.5 * (stripe % 2), np.full(len(hits), 0.6),
                           0.8 - 0.4 * (stripe % 2)), axis=1)
        # The scene's normal faces the ray; the room's surfaces face inward.
        normal = -np.sign(np.sum(normal * directions, axis=1))[:, None] * normal
        bands.append(relight_chain(albedo.reshape(6, 16, 16, 3), normal.reshape(6, 16, 16, 3),
                                   distance.reshape(6, 16, 16), minimum_size=8))
    return bands


def fixture_light(points, normals):
    """The analytic diffuse light (now, baked) the relight samples use
    (Source units): the baked light is positive, and now differs from it by
    a change that removes light in some places and adds it in others, so
    both halves of `relight` are exercised. The C++ suite and
    reflection_probes_check.comp implement it too."""
    points = np.asarray(points, dtype=np.float64)
    normals = np.asarray(normals, dtype=np.float64)
    baked = np.stack((0.3 + 0.001 * points[:, 0], 0.25 + 0.1 * np.abs(normals[:, 2]),
                      0.2 + 0.0005 * points[:, 2]), axis=1)
    change = np.stack((0.002 * points[:, 0] + 0.5 * np.maximum(normals[:, 2], 0.0) - 0.45,
                       0.3 - 0.003 * points[:, 1], 0.25 * normals[:, 0] - 0.1), axis=1)
    return baked + change, baked


# The fixture's moving occluder (meters): a door-sized slab across the room
# between the two captures, which hides part of each capture's view.
FIXTURE_OCCLUDER = ((2.8, 1.2, 0.0), (3.2, 2.8, 2.2), 0.4)


def fixture_occluders(scale=SOURCE_UNITS_PER_METER):
    lo, hi, reflectance = FIXTURE_OCCLUDER
    return [(np.asarray(lo) * scale, np.asarray(hi) * scale, reflectance)]


def capacity_fixture(relight=False, count=64):
    """Only records 62/63 influence the original room: catches a 16-probe clamp.

    All earlier records have equally sized influences in distant rooms, so
    the active local rank is also 62. Distinct bands exercise tiled addressing.
    """
    original, chains = fixture_layout()
    probes = []
    for index in range(count - 2):
        offset = (np.array((100.0 + index * 10.0, 0.0, 0.0)) if count <= 64 else
                  np.array((100.0 + (index % 16) * 10.0, (index // 16) * 10.0, 0.0)))
        probes.append(dict(original[0], priority=1, **{key: original[0][key] + offset for key in
            ("capture", "box_min", "box_max", "influence_min", "influence_max")}))
    probes += original
    colors = [[mip * (0.1 + index / count) for mip in chains[0]]
              for index in range(count - 2)] + chains
    bands = fixture_relight(original) if relight else None
    return build(probes, colors, relight=([bands[0]] * (count - 2) + bands)
                 if bands else None)


def write_fixture(out):
    """The C++ reader's inputs: valid.rprb, its GPU buffer (mode 1),
    the malformation corpus and shading samples of the reference blend."""
    probes, chains = fixture_layout()
    data = build(probes, chains)
    out.mkdir(parents=True, exist_ok=True)
    (out / "valid.rprb").write_bytes(data)
    # One probe (the global one): a cube array of one cube, which the
    # consumers pad to two so its view is an array view.
    (out / "single.rprb").write_bytes(build(probes[1:], chains[1:]))
    # Five mips (faces 64 to 4), so a texture setting can drop the top one.
    long_probes, long_chains = fixture_layout(64, 4)
    (out / "mips5.rprb").write_bytes(build(long_probes, long_chains))
    (out / "capacity64.rprb").write_bytes(capacity_fixture())
    (out / "capacity64-relight.rprb").write_bytes(capacity_fixture(relight=True))
    layout = read(data)
    (out / "gpu-mode1.u32").write_bytes(gpu_buffer(layout, MODE_BLEND).tobytes())
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
    # v2: the same probes with relight bands, relit by fixture_light with the
    # fixture's occluder.
    relit = build(probes, chains, relight=fixture_relight(probes))
    (out / "valid-relight.rprb").write_bytes(relit)
    relit_layout = read(relit)
    (out / "gpu-relight-mode1.u32").write_bytes(gpu_buffer(relit_layout, MODE_BLEND).tobytes())
    lines = ["# offset format value error (reflection_probe_set.relight_malformations)"]
    for offset, fmt, value, code in relight_malformations(relit_layout["data_offset"]):
        lines.append("%d %s %r %s" % (offset, fmt[1:], value, code))
    (out / "relight-malformations.txt").write_text("\n".join(lines) + "\n")
    rows = ["# mode px py pz nx ny nz rx ry rz roughness -> r g b (Source units), relit by "
            "fixture_light with FIXTURE_OCCLUDER"]
    for mode in (MODE_BLEND, MODE_NEAREST, MODE_DIRECTION):
        count = 48
        points = np.column_stack((rng.uniform(0.2, 5.8, count), rng.uniform(0.2, 3.8, count),
                                  rng.choice((0.0, 1.2), count))) * scale
        normals = np.where(points[:, 2:3] == 0, (0.0, 0.0, 1.0), (1.0, 0.0, 0.0))
        reflected = rng.normal(size=(count, 3))
        reflected /= np.linalg.norm(reflected, axis=1, keepdims=True)
        roughness = rng.choice((0.0, 0.2, 0.5, 1.0), count)
        radiance = shade(points, normals, reflected, roughness, relit_layout, mode,
                         light=fixture_light, occluders=fixture_occluders())
        for i in range(count):
            rows.append(" ".join("%.9g" % v for v in (mode, *points[i], *normals[i],
                                                       *reflected[i], roughness[i],
                                                       *radiance[i])))
    (out / "relight-samples.txt").write_text("\n".join(rows) + "\n")


def world_bounds(bsp2):
    """Model 0's bounds (the world brushes) from a BSP2 map's carried lump."""
    import legacy_bsp
    raw = legacy_bsp.decompress_lzma_lump(bsp2.legacy_lump(legacy_bsp.LUMP_MODELS), "model lump")
    if len(raw) < legacy_bsp.MODEL_BYTES:
        raise RprbError("InvalidCandidates", "the map has no world model")
    f = struct.unpack_from("<6f", raw, 0)
    return np.array(f[:3]), np.array(f[3:])


def candidates_per_cell(data):
    masks = read(data)["candidates"]["masks"].reshape(CANDIDATE_CELLS, -1)
    return [sum(int(word).bit_count() for word in row) for row in masks]


def regrid_map(path, out):
    import bsp2_reader
    data = Path(path).read_bytes()
    kind, bsp2 = bsp2_reader.open_any(data)
    if kind != "bsp2":
        raise RprbError("InvalidCandidates", "regrid reads BSP2 maps")
    entry = bsp2.by_id.get(bsp2_reader.fourcc("RPRB"))
    if entry is None:
        raise RprbError("InvalidCandidates", "the map has no RPRB lump")
    old = bsp2.lump(entry)
    bounds = world_bounds(bsp2)
    added = False
    new = regrid(old, bounds)
    lumps = [(e["fourcc"], e["version"], e["flags"], e["alignment"],
              new if e is entry else bsp2.lump(e)) for e in bsp2.entries]
    Path(out).write_bytes(bsp2_reader.write_bsp2(bsp2.revision, lumps))
    changed_outside = 0
    if not added:
        base = (HEADER_BYTES + RECORD_BYTES * read(old)["count"] + 15) & ~15
        end = read(old)["data_offset"]
        changed_outside = sum(a != b for a, b in zip(old[:base] + old[end:], new[:base] + new[end:]))
        if len(old) != len(new) or changed_outside:
            raise RprbError("InvalidCandidates", "regrid changed bytes outside the grid")
    count = read(old)["count"]
    before = [count] * CANDIDATE_CELLS if added else candidates_per_cell(old)
    after = candidates_per_cell(new)
    grid = read(new)["candidates"]
    return {"world_bounds": [bounds[0].tolist(), bounds[1].tolist()],
            "step": float(grid["step"]), "origin": [float(v) for v in grid["origin"]],
            "candidates_per_cell": {"before_max": max(before), "after_max": max(after),
                                    "before_mean": round(sum(before) / len(before), 2),
                                    "after_mean": round(sum(after) / len(after), 2)},
            "changed_bytes_outside_grid": changed_outside, "grid_added": added}


def main():
    import argparse
    import hashlib
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("pack", help="fit, prefilter and encode the rendered probes")
    command.add_argument("--probes-dir", type=Path, required=True)
    command.add_argument("--ktx-tool", type=Path,
                         help="the pinned ktx that encodes the BC6H radiance bands "
                              "(default: the map toolchain's)")
    command.add_argument("--workers", type=int,
                         help="prefilter processes (default: the CPUs, at most one per probe)")
    command.add_argument("--face", type=int, default=256,
                         help="cube face size of every probe's mip 0")
    command.add_argument("--preview-gain", type=float, default=1.0,
                        help="the lightmap's exposure gain (the LMAP writer's)")
    command.add_argument("--max-mean-relative-residual", type=float,
                         help="reject depth-fitted proxy boxes above this runtime limit")
    command.add_argument("--candidate-bounds-m", type=float, nargs=6,
                         metavar=("XMIN", "YMIN", "ZMIN", "XMAX", "YMAX", "ZMAX"),
                         help="the playable envelope (meters) the candidate grid covers")
    command.add_argument("--out", type=Path, required=True)
    command = commands.add_parser(
        "regrid", help="rebuild a BSP2 map's candidate grid inside its world bounds")
    command.add_argument("--map", type=Path, required=True)
    command.add_argument("--out", type=Path, required=True)
    command = commands.add_parser("info", help="validate RPRB bytes and print their records")
    command.add_argument("rprb", type=Path)
    command = commands.add_parser("candidate-fixture", help="write a candidate invariant fixture")
    command.add_argument("--out", type=Path, default=FIXTURE_DIR / "candidates64.rprb")
    command.add_argument("--count", type=int, default=64, choices=(64, 256))
    command = commands.add_parser("fixture", help="write the C++ reader's shared fixtures")
    command.add_argument("--out", type=Path, default=FIXTURE_DIR)
    args = parser.parse_args()
    if args.command == "candidate-fixture":
        layout = read(capacity_fixture(count=args.count))
        data = build(layout["probes"], layout["chains"], scale=1)
        read(data)
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_bytes(data)
        return
    if args.command == "regrid":
        report = regrid_map(args.map, args.out)
        print(json.dumps(report, indent=1))
        return
    if args.command == "fixture":
        write_fixture(args.out)
        return
    if args.command == "info":
        layout = read(args.rprb.read_bytes())
        print(json.dumps({"count": layout["count"], "mips": layout["mips"],
                          "face": layout["face"], "global_index": layout["global_index"],
                          "probes": [{key: (value.tolist() if hasattr(value, "tolist")
                                            else value) for key, value in probe.items()}
                                     for probe in layout["probes"]]}, indent=2))
        return
    data, receipt = pack(args.probes_dir, args.face, args.preview_gain,
                         max_mean_relative_residual=args.max_mean_relative_residual,
                         candidate_bounds_m=(args.candidate_bounds_m[:3], args.candidate_bounds_m[3:])
                         if args.candidate_bounds_m else None, tool=args.ktx_tool,
                         workers=args.workers)
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
