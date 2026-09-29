#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Judge native Vulkan's legacy shader ports against the shipped D3D9 bytecode.

The material pixel harness's "legacy" family draws stdshader_dx9 materials
through the native backend and reads pixels back (pixels.json); with
-vklegacycapture the backend records every pass a legacy port drew: the shader
pair, its combo indices, every constant register, the bound textures, the
geometry and the fixed-function state (capture.jsonl, source-legacy-pass/v1).
This oracle replays each pass on the CPU with the retail game's compiled
shaders (source_vcs.py reads them from the VPK; d3d9_shader_vm.py executes
them): the vertex shader per vertex, D3D9 rasterization of the sampled pixel,
the pixel shader, the alpha test, blending and the sRGB write. The native
pixel must match within the tolerance.

    python3 tools/quality/legacy_shader_oracle.py --pixels pixels.json \\
        --capture capture.jsonl [--vpk hl2_misc_dir.vpk] [--tolerance 2] [--json out]

The oracle is independent of the ports: it never reads their GLSL, only what
the material's shader DLL set and what fxc compiled.
"""

import argparse
import json
import math
import os
import struct
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import d3d9_shader_vm  # noqa: E402
import d3d9_texture  # noqa: E402
import shader_artifacts  # noqa: E402
import source_vcs  # noqa: E402

PASS_SCHEMA = "source-legacy-pass/v1"
DEFAULT_TOLERANCE = 2

# public/vtf/vtf.h texture flags the harness sets.
TEXTUREFLAGS_POINTSAMPLE = 0x1
TEXTUREFLAGS_CLAMPS = 0x4
TEXTUREFLAGS_CLAMPT = 0x8
TEXTUREFLAGS_CLAMPU = 0x02000000
# public/materialsystem/imaterial.h vertex format flags.
VERTEX_TANGENT_S = 0x10

# VkBlendFactor values the capture records.
VK_BLEND = {0: "zero", 1: "one", 2: "src_color", 3: "one_minus_src_color", 4: "dst_color",
            5: "one_minus_dst_color", 6: "src_alpha", 7: "one_minus_src_alpha", 8: "dst_alpha",
            9: "one_minus_dst_alpha", 14: "src_alpha_saturate"}
VK_CULL_NONE, VK_CULL_FRONT, VK_CULL_BACK = 0, 1, 2

# Material primitive types (materialsystem/imesh.h MaterialPrimitiveType_t).
MATERIAL_POINTS, MATERIAL_LINES, MATERIAL_TRIANGLES, MATERIAL_TRIANGLE_STRIP = 0, 1, 2, 3
MATERIAL_LINE_STRIP, MATERIAL_LINE_LOOP, MATERIAL_POLYGON, MATERIAL_QUADS = 4, 5, 6, 7


class OracleError(Exception):
    pass


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def srgb_encode(value):
    value = min(max(value, 0.0), 1.0)
    if value <= 0.0031308:
        return value * 12.92
    return 1.055 * value ** (1.0 / 2.4) - 0.055


def unorm8(value):
    """A float written to an 8-bit UNORM target: saturated, round to nearest."""
    if value != value:  # NaN
        return 0
    return int(math.floor(min(max(value, 0.0), 1.0) * 255.0 + 0.5))


# --- inputs -------------------------------------------------------------------

def load_capture(path):
    passes = []
    for number, line in enumerate(Path(path).read_text().splitlines(), 1):
        if not line.strip():
            continue
        record = json.loads(line)
        if record.get("schema") != PASS_SCHEMA:
            raise OracleError("%s:%d: not %s" % (path, number, PASS_SCHEMA))
        passes.append(record)
    return passes


class SourceMatchedShaders:
    """Bytecode compiled from this tree's stdshaders .fxc with the pinned FXC
    (shader_artifacts.py: fxc_prep.pl's plan and compiler options), for exactly
    the combination a pass selected. The shader DLL computes combo indices with
    this tree's fxctmp9 selectors, which some retail .vcs layouts do not share
    (e.g. a PIXELFOGTYPE of 0..2), so this is the default reference. Outputs are
    cached by compiler, plan and source hashes."""

    def __init__(self, cache, timeout=120):
        self.cache = Path(cache)
        self.timeout = timeout
        self.profile_path, self.profile = shader_artifacts.load_compiler_profile()
        self.compiler = shader_artifacts.ROOT / self.profile["compiler"]["path"]
        self.source_dir = shader_artifacts.ROOT / self.profile["source_directory"]
        self.plans = {}

    def _plan(self, name):
        if name not in self.plans:
            source, _, plan = shader_artifacts.generate_plan(name, self.source_dir, self.profile)
            identity, _, _ = shader_artifacts.plan_identity(name, source, plan, self.source_dir,
                                                            self.profile_path, self.profile)
            self.plans[name] = (source, plan, identity)
        return self.plans[name]

    def bytecode(self, name, static_index, dynamic_index):
        source, plan, identity = self._plan(name)
        dynamic_count = plan["dynamic_count"]
        if static_index % dynamic_count or not 0 <= dynamic_index < dynamic_count:
            raise OracleError("%s: combo %d + %d is outside its %d dynamic combos"
                              % (name, static_index, dynamic_index, dynamic_count))
        combos = shader_artifacts.valid_combos(plan, [static_index])
        values = next((v for _, d, v in combos if d == dynamic_index), None)
        if values is None:
            raise OracleError("%s: combo %d + %d was skipped by the shader's SKIP rules"
                              % (name, static_index, dynamic_index))
        output = self.cache / identity / name / ("%d-%d.o" % (static_index // dynamic_count,
                                                              dynamic_index))
        try:
            return shader_artifacts.compile_shader(self.compiler, source, plan["arguments"], values,
                                                   output, self.timeout)
        except RuntimeError as error:
            raise OracleError(str(error))


def default_fxc_cache():
    base = os.environ.get("XDG_CACHE_HOME") or os.path.join(os.path.expanduser("~"), ".cache")
    return Path(base) / "source-engine" / "fxc"


class ShaderSource:
    """Shipped bytecode by shader file name, from the game's VPK (--shaders retail;
    only valid for shaders whose retail combo layout is this tree's)."""

    def __init__(self, vpk):
        self.vpk = vpk
        self.directory = None
        self.files = {}

    def bytecode(self, name, static_index, dynamic_index):
        if name not in self.files:
            if self.directory is None:
                self.directory = source_vcs.VpkDirectory(str(self.vpk))
            self.files[name] = source_vcs.VcsFile.from_vpk(self.directory, name)
        code = self.files[name].bytecode(static_index, dynamic_index)
        if code is None:
            raise OracleError("%s: combo %d + %d was skipped by the shader's SKIP rules"
                              % (name, static_index, dynamic_index))
        return code


def make_texture(entry, srgb):
    """A d3d9_texture of a harness texture (texels as 8-bit UNORM values): 2D,
    a cube map (six faces) or a volume (depth slices)."""
    width, height, faces = entry["width"], entry["height"], entry.get("faces", 1)
    depth = entry.get("depth", 1)
    texels = entry["texels"]
    flags = entry.get("flags", 0)
    options = {
        "filter": "point" if flags & TEXTUREFLAGS_POINTSAMPLE else "bilinear",
        "address": ("clamp" if flags & TEXTUREFLAGS_CLAMPS else "wrap",
                    "clamp" if flags & TEXTUREFLAGS_CLAMPT else "wrap",
                    "clamp" if flags & TEXTUREFLAGS_CLAMPU else "wrap"),
        "srgb": srgb,
    }

    def face(index):
        start = index * width * height * 4
        rows = []
        for y in range(height):
            row = []
            for x in range(width):
                at = start + (y * width + x) * 4
                row.append(tuple(value / 255.0 for value in texels[at:at + 4]))
            rows.append(row)
        return rows

    if faces == 6:
        return d3d9_texture.TextureCube([d3d9_texture.Texture2D(face(i), **options)
                                         for i in range(6)])
    if depth > 1:
        return d3d9_texture.TextureVolume([face(z) for z in range(depth)], **options)
    return d3d9_texture.Texture2D(face(0), **options)


# The material system's standard textures by the debug name the backend records
# (CMaterialSystem::CreateDebugMaterials and friends, cmaterialsystem.cpp):
# one texel each, BGRX8888 unless noted (X reads as 255). A port that reads one
# is judged with these values. Append entries as ports need more of them.
STANDARD_TEXTURES = {
    "[white_texid]": (255, 255, 255, 255),
    "[black_texid]": (0, 0, 0, 255),
    "[grey_texid]": (128, 128, 128, 255),
    "[greyalphazero_texid]": (128, 128, 128, 0),  # RGBA8888
    "[flat_normal_texture]": (127, 127, 255, 255),
}


def normalization_cube(signed):
    """The material system's 32x32 normalization cube maps (texturemanager.cpp,
    CNormalizationCubemap / CSignedNormalizationCubemap without
    DX_TO_GL_ABSTRACTION): "normalize" in BGRX8888, "normalizesigned" in
    UVWQ8888, whose signed bytes the hardware reads as max( v / 127, -1 ).
    Faces in VTF order (right, left, back, front, up, down), which is D3D9's."""
    size = 32
    faces = []
    for face in range(6):
        rows = []
        for y in range(size):
            v = y * (2.0 / (size - 1)) - 1.0
            row = []
            for x in range(size):
                u = x * (2.0 / (size - 1)) - 1.0
                oow = 1.0 / math.sqrt(1.0 + u * u + v * v)
                ix = min(max(int(255.0 * 0.5 * (u * oow + 1.0) + 0.5), 0), 255)
                iy = min(max(int(255.0 * 0.5 * (v * oow + 1.0) + 0.5), 0), 255)
                iz = min(max(int(255.0 * 0.5 * (oow + 1.0) + 0.5), 0), 255)
                if not signed:
                    texel = [(iz, 255 - iy, 255 - ix), (255 - iz, 255 - iy, ix), (ix, iz, iy),
                             (ix, 255 - iz, 255 - iy), (ix, 255 - iy, iz),
                             (255 - ix, 255 - iy, 255 - iz)][face]
                    row.append(tuple(c / 255.0 for c in texel) + (1.0,))
                    continue
                flips = [(1, 1, 0), (0, 1, 1), (0, 0, 0), (0, 1, 1), (0, 1, 0), (1, 1, 1)][face]
                sx, sy, sz = [(255 - c if f else c) - 128
                              for c, f in zip((ix, iy, iz), flips)]
                texel = [(sz, sy, sx), (sz, sy, sx), (sx, sz, sy), (sx, sz, sy), (sx, sy, sz),
                         (sx, sy, sz)][face]
                row.append(tuple(max(c / 127.0, -1.0) for c in texel) + (0.0,))
            rows.append(row)
        faces.append(d3d9_texture.Texture2D(rows, filter="bilinear",
                                            address=("clamp", "clamp", "clamp")))
    return d3d9_texture.TextureCube(faces)


# The material system's procedural cube maps by name.
STANDARD_CUBES = {
    "normalize": lambda: normalization_cube(False),
    "normalizesigned": lambda: normalization_cube(True),
}


def solid_texture(rgba, srgb):
    return d3d9_texture.Texture2D.constant(tuple(v / 255.0 for v in rgba), filter="point",
                                           address="clamp", srgb=srgb)


def pass_samplers(record, case, report):
    """The pass's sampler set: every bound texture the harness defined."""
    textures = {name.lower(): entry for name, entry in report.get("textures", {}).items()}
    # A case's own textures (its color correction lookups' volumes).
    textures.update({name.lower(): entry for name, entry in case.get("volumes", {}).items()})
    srgb_mask = record.get("srgb_samplers", 0)
    stages = {}
    for stage_text, name in record.get("textures", {}).items():
        stage = int(stage_text)
        srgb = bool(srgb_mask & (1 << stage))
        key = name.lower()
        if key in textures:
            stages[stage] = make_texture(textures[key], srgb)
        elif key == "_rt_fullframefb" and "framebuffer" in case:
            stages[stage] = solid_texture(case["framebuffer"], srgb)
        elif key == "_rt_fullframefb1" and "framebuffer1" in case:
            stages[stage] = solid_texture(case["framebuffer1"], srgb)
        elif key in STANDARD_TEXTURES:
            stages[stage] = solid_texture(STANDARD_TEXTURES[key], srgb)
        elif key in STANDARD_CUBES:
            stages[stage] = STANDARD_CUBES[key]()
        else:
            stages[stage] = UnknownTexture(name)
    # A sampler the pass left without a texture (the capture lists only the
    # samplers the pass enabled; CShaderAPIDx8::ApplyTextureEnable sets no
    # texture on the others) reads as D3D9 reads a sampler with no texture
    # set: ( 0, 0, 0, 1 ).
    return d3d9_texture.SamplerSet(stages, default=(0.0, 0.0, 0.0, 1.0))


class UnknownTexture:
    """A texture the harness did not define: reading it fails the case."""

    def __init__(self, name):
        self.name = name

    def sample(self, *args, **kwargs):
        raise OracleError("the shader read texture %r, which the case does not define"
                          % self.name)

    def __getattr__(self, attribute):
        raise OracleError("the shader read texture %r, which the case does not define"
                          % self.name)


def register_constants(rows, bools, ints):
    consts = {"c": {i: tuple(row) for i, row in enumerate(rows)}}
    consts["b"] = {i: bool(bools & (1 << i)) for i in range(16)}
    if ints:
        consts["i"] = {i: tuple(row) for i, row in enumerate(ints)}
    return consts


def vertex_inputs(vertex, vertex_format):
    """A captured vertex as D3D9 vertex declaration inputs (vertexdecl.cpp)."""
    pos = vertex["pos"]
    color = [c / 255.0 for c in vertex["color"]]
    tangent = vertex["tangent_s"] if vertex_format & VERTEX_TANGENT_S else vertex["user_data"]
    weights = vertex.get("bone_weights", [1.0, 0.0])
    inputs = {
        "position": (pos[0], pos[1], pos[2], 1.0),
        "normal": tuple(vertex["normal"]) + (1.0,),
        "color": tuple(color),
        "blendweight": (weights[0], weights[1], 0.0, 1.0),
        "blendindices": tuple(float(i) for i in vertex.get("bone_indices", [0, 0, 0])) + (0.0,),
        "tangent": tuple(tangent) + ((1.0,) if len(tangent) == 3 else ()),
        "binormal": tuple(vertex["tangent_t"]) + (1.0,),
    }
    for index, uv in enumerate(vertex["uv"]):
        inputs["texcoord%d" % index] = (uv[0], uv[1], 0.0, 1.0)
    return inputs


# --- rasterization -------------------------------------------------------------

def triangles(primitive, indices):
    if primitive == MATERIAL_TRIANGLE_STRIP:
        for i in range(len(indices) - 2):
            a, b, c = indices[i], indices[i + 1], indices[i + 2]
            yield (b, a, c) if i & 1 else (a, b, c)
    elif primitive == MATERIAL_POLYGON:
        for i in range(1, len(indices) - 1):
            yield indices[0], indices[i], indices[i + 1]
    elif primitive == MATERIAL_QUADS:
        for i in range(0, len(indices) - 3, 4):
            yield indices[i], indices[i + 1], indices[i + 2]
            yield indices[i], indices[i + 2], indices[i + 3]
    elif primitive == MATERIAL_TRIANGLES:
        for i in range(0, len(indices) - 2, 3):
            yield indices[i], indices[i + 1], indices[i + 2]
    else:
        raise OracleError("primitive type %d is not rasterized by the oracle" % primitive)


def screen_position(clip, frame):
    """D3D9 viewport transform: pixel centers at integer coordinates."""
    x, y, z, w = clip
    if w <= 0.0:
        return None
    width, height = frame
    return ((x / w + 1.0) * 0.5 * width, (1.0 - y / w) * 0.5 * height, z / w, w)


def cover(point, tri, cull, extrapolate=False):
    """Perspective-correct barycentric weights of `point` in `tri`, or None.
    With `extrapolate`, a point outside the triangle gets the weights of the
    triangle's plane there (a helper pixel of its 2x2 quad)."""
    (x0, y0, _, w0), (x1, y1, _, w1), (x2, y2, _, w2) = tri
    area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
    if area == 0.0:
        return None
    # D3D screen space has y down: a positive area is clockwise, D3D9's front
    # face for the default cull state (native culls the same winding).
    if cull == VK_CULL_BACK and area < 0.0:
        return None
    if cull == VK_CULL_FRONT and area > 0.0:
        return None
    px, py = point
    l0 = ((x1 - px) * (y2 - py) - (x2 - px) * (y1 - py)) / area
    l1 = ((x2 - px) * (y0 - py) - (x0 - px) * (y2 - py)) / area
    l2 = 1.0 - l0 - l1
    epsilon = -1e-7
    if not extrapolate and (l0 < epsilon or l1 < epsilon or l2 < epsilon):
        return None
    p0, p1, p2 = l0 / w0, l1 / w1, l2 / w2
    total = p0 + p1 + p2
    return p0 / total, p1 / total, p2 / total


def interpolate(outputs, weights):
    """Every vertex shader output but the position, interpolated."""
    result = {}
    for semantic in outputs[0]:
        values = [o[semantic] for o in outputs]
        result[semantic] = tuple(f32(sum(w * v[k] for w, v in zip(weights, values)))
                                 for k in range(4))
    return result


# --- output merger ---------------------------------------------------------------

def blend_factor(name, src, dst):
    if name == "zero":
        return (0.0,) * 4
    if name == "one":
        return (1.0,) * 4
    if name == "src_color":
        return src
    if name == "one_minus_src_color":
        return tuple(1.0 - v for v in src)
    if name == "dst_color":
        return dst
    if name == "one_minus_dst_color":
        return tuple(1.0 - v for v in dst)
    if name == "src_alpha":
        return (src[3],) * 4
    if name == "one_minus_src_alpha":
        return (1.0 - src[3],) * 4
    if name == "dst_alpha":
        return (dst[3],) * 4
    if name == "one_minus_dst_alpha":
        return (1.0 - dst[3],) * 4
    if name == "src_alpha_saturate":
        f = min(src[3], 1.0 - dst[3])
        return (f, f, f, 1.0)
    raise OracleError("blend factor %r" % name)


def merge(record, color, dest):
    """The pass's output merged into the 8-bit destination `dest` (RGBA)."""
    srgb = record.get("srgb_write", False)
    src = [min(max(v, 0.0), 1.0) if v == v else 0.0 for v in color]
    dst = [v / 255.0 for v in dest]
    if srgb:
        dst = [d3d9_texture.srgb_to_linear(v) for v in dst[:3]] + [dst[3]]
    if record.get("blend"):
        sf = blend_factor(VK_BLEND.get(record["src_blend"]), src, dst)
        df = blend_factor(VK_BLEND.get(record["dst_blend"]), src, dst)
        out = [src[k] * sf[k] + dst[k] * df[k] for k in range(4)]
    else:
        out = src
    rgb = [srgb_encode(v) for v in out[:3]] if srgb else out[:3]
    result = list(dest)
    if record.get("color_write", True):
        result[:3] = [unorm8(v) for v in rgb]
    if record.get("alpha_write", False):
        result[3] = unorm8(out[3])
    return result


def alpha_test(record, alpha):
    ref = record.get("alpha_ref", -1.0)
    if ref < 0.0:
        return True
    alpha = min(max(alpha, 0.0), 1.0)
    return alpha > ref if record.get("alpha_greater") else alpha >= ref


# --- replay --------------------------------------------------------------------------

class PassReplay:
    """One captured pass: its vertex outputs and screen positions, once."""

    def __init__(self, record, source, frame):
        self.record = record
        self.frame = frame
        self.vs = source.bytecode(record["vertex_shader"], record["vs_static"], record["vs_dynamic"])
        self.ps = source.bytecode(record["pixel_shader"], record["ps_static"], record["ps_dynamic"])
        vs_consts = register_constants(record["vs_constants"], record.get("vs_bools", 0),
                                       record.get("vs_ints"))
        self.outputs = []
        self.screen = []
        vertex_format = record.get("vertex_format", 0)
        for vertex in record["vertices"]:
            result = d3d9_shader_vm.run_vertex(self.vs, vertex_inputs(vertex, vertex_format),
                                               vs_consts)
            self.outputs.append(result.as_pixel_inputs())
            # vs_3_0 declares its position output (o# dcl_position) by semantic.
            position = result["oPos"] if "oPos" in result else result["position0"]
            self.screen.append(screen_position(position, frame))
        self.ps_consts = register_constants(record["ps_constants"], record.get("ps_bools", 0),
                                            record.get("ps_ints"))

    @staticmethod
    def inputs_at(outputs, tri, point):
        weights = cover(point, tri, VK_CULL_NONE, extrapolate=True)
        inputs = interpolate(outputs, weights)
        inputs["vPos"] = (float(point[0]), float(point[1]), 0.0, 0.0)
        return inputs

    def derivatives(self, outputs, tri, point, samplers):
        """dsx/dsy as a 2x2 quad computes them: the difference between the value
        the instruction reads at the next pixel in x (y) and at this one. The
        neighbors run the shader over the same triangle's interpolants (helper
        pixels where it does not cover them); the n-th dsx (dsy) of this pixel
        pairs with the n-th of its neighbor. Exact for attributes interpolated
        linearly in screen space."""
        neighbors = {}

        def neighbor_values(kind):
            if kind not in neighbors:
                offset = (1, 0) if kind == "dsx" else (0, 1)
                at = (point[0] + offset[0], point[1] + offset[1])
                calls = {"dsx": [], "dsy": []}

                def record(call_kind, ins, value):
                    calls[call_kind].append(tuple(value))
                    return (0.0, 0.0, 0.0, 0.0)

                def neighbor_sampler(stage, coords, kind, extra):
                    # The neighbor's own derivatives read as zero, which can
                    # leave its later coordinates non-finite; only the values
                    # reaching its dsx/dsy matter.
                    if not all(math.isfinite(c) for c in coords):
                        return (0.0, 0.0, 0.0, 0.0)
                    return samplers(stage, coords, kind, extra)

                d3d9_shader_vm.run_pixel(self.ps, self.inputs_at(outputs, tri, at),
                                         self.ps_consts, neighbor_sampler, record)
                neighbors[kind] = calls[kind]
            return neighbors[kind]

        counts = {"dsx": 0, "dsy": 0}

        def derivative(kind, ins, value):
            values = neighbor_values(kind)
            index = counts[kind]
            counts[kind] += 1
            if index >= len(values):
                raise OracleError("%s: the neighbor pixel ran a different path" % kind)
            return tuple(f32(n - v) for n, v in zip(values[index], value))

        return derivative

    def shade(self, point, samplers):
        """The pass's color at pixel `point`, or None where nothing is drawn."""
        for a, b, c in triangles(self.record["primitive"], self.record["indices"]):
            tri = (self.screen[a], self.screen[b], self.screen[c])
            if None in tri:
                continue
            weights = cover(point, tri, self.record.get("cull", VK_CULL_NONE))
            if weights is None:
                continue
            outputs = [self.outputs[a], self.outputs[b], self.outputs[c]]
            inputs = self.inputs_at(outputs, tri, point)
            derivatives = self.derivatives(outputs, tri, point, samplers)
            result = d3d9_shader_vm.run_pixel(self.ps, inputs, self.ps_consts, samplers,
                                              derivatives)
            if result.discarded:
                return None
            color = result["oC0"]
            if not alpha_test(self.record, color[3]):
                return None
            return color
        return None


def replay_case(case, passes, source, report):
    """The oracle's RGBA at each of the case's sampled pixels."""
    frame = tuple(report["frame"])
    replays = [(PassReplay(record, source, frame), pass_samplers(record, case, report))
               for record in passes]
    expected = []
    for pixel in case["pixels"]:
        dest = list(case["clear"])
        for replay, samplers in replays:
            color = replay.shade((pixel["x"], pixel["y"]), samplers)
            if color is not None:
                dest = merge(replay.record, color, dest)
        expected.append(dest)
    return expected


def evaluate(report, passes, source, tolerance=DEFAULT_TOLERANCE):
    """Failures (strings) and per-case details."""
    if report.get("family") != "legacy":
        return ["pixels hold family %r, not legacy" % report.get("family")], []
    probe = report.get("clear_probe", {})
    failures = []
    if probe.get("pixel") != probe.get("clear"):
        failures.append("readback self-check: cleared frame read %s, expected %s"
                        % (probe.get("pixel"), probe.get("clear")))
    by_material = {}
    for record in passes:
        by_material.setdefault(record["material"].lower(), []).append(record)
    details = []
    if not report.get("cases"):
        failures.append("no cases")
    for case in report.get("cases", []):
        name = case["name"]
        case_passes = by_material.get(case["material"].lower(), [])
        detail = {"name": name, "shader": case.get("shader"), "passes": len(case_passes)}
        details.append(detail)
        if not case_passes:
            failures.append("%s: no legacy port drew the material (declined or another path)"
                            % name)
            continue
        detail["pairs"] = sorted({"%s/%s" % (p["pixel_shader"], p["vertex_shader"])
                                  for p in case_passes})
        try:
            expected = replay_case(case, case_passes, source, report)
        except (OracleError, OSError, KeyError, ValueError, NotImplementedError,
                d3d9_shader_vm.ShaderRuntimeError) as error:
            # KeyError: the shader sampled a stage the pass bound no texture to.
            failures.append("%s: oracle could not replay the pass: %s" % (name, error))
            continue
        limit = case.get("tolerance", tolerance)
        detail["pixels"] = []
        for pixel, want in zip(case["pixels"], expected):
            got = pixel["rgba"]
            # Alpha is compared only where some pass writes it.
            channels = 4 if any(p.get("alpha_write") for p in case_passes) else 3
            error = max(abs(got[k] - want[k]) for k in range(channels))
            detail["pixels"].append({"x": pixel["x"], "y": pixel["y"], "native": got,
                                     "oracle": want, "error": error})
            if error > limit:
                failures.append("%s: pixel (%d, %d) native %s, D3D9 bytecode %s (error %d > %d)"
                                % (name, pixel["x"], pixel["y"], got[:channels], want[:channels],
                                   error, limit))
    return failures, details


def compare_native_family(port_report, native_report, native_passes, tolerance=DEFAULT_TOLERANCE):
    """Failures (strings) and per-case details for a native family's pixels.

    The port run drew every case through a legacy port (a -vklegacy* switch,
    judged against the bytecode by evaluate); the native run drew the same
    cases without the switch, so the native family (shaders/skin.*, ...) drew
    them. Each native case must have been drawn by no port, else the check
    would compare the port with itself, and its color must equal the port's
    within the tolerance. Alpha is not compared: native passes do not write
    destination alpha (a frame copy derives it from depth, depth_to_alpha.frag).
    """
    failures = []
    if native_report.get("family") != "legacy":
        return ["native pixels hold family %r, not legacy" % native_report.get("family")], []
    ported = {record["material"].lower() for record in native_passes}
    port_cases = {case["name"]: case for case in port_report.get("cases", [])}
    details = []
    if not native_report.get("cases"):
        failures.append("native run: no cases")
    for case in native_report.get("cases", []):
        name = case["name"]
        detail = {"name": name}
        details.append(detail)
        if case["material"].lower() in ported:
            failures.append("%s: a legacy port drew the material in the native run" % name)
            continue
        port = port_cases.get(name)
        if port is None:
            failures.append("%s: no port run of the case" % name)
            continue
        if [(p["x"], p["y"]) for p in port["pixels"]] != [(p["x"], p["y"]) for p in case["pixels"]]:
            failures.append("%s: the runs sampled different pixels" % name)
            continue
        limit = case.get("tolerance", tolerance)
        worst = 0
        for got, want in zip(case["pixels"], port["pixels"]):
            error = max(abs(got["rgba"][k] - want["rgba"][k]) for k in range(3))
            worst = max(worst, error)
            if error > limit:
                failures.append("%s: pixel (%d, %d) native family %s, port %s (error %d > %d)"
                                % (name, got["x"], got["y"], got["rgba"][:3], want["rgba"][:3],
                                   error, limit))
        detail["max_error"] = worst
    return failures, details


def make_source(kind, fxc_cache=None, vpk=None):
    """The reference bytecode: this tree's .fxc compiled, or the retail .vcs."""
    if kind == "source":
        return SourceMatchedShaders(fxc_cache or default_fxc_cache())
    vpk = vpk or source_vcs.find_default_vpk()
    if vpk is None:
        raise OracleError("no VPK with shaders/fxc found")
    return ShaderSource(str(vpk))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--pixels", type=Path, required=True)
    parser.add_argument("--capture", type=Path, required=True)
    parser.add_argument("--shaders", choices=("source", "retail"), default="source",
                        help="reference bytecode: compiled from this tree's .fxc (default) or the "
                             "retail .vcs files")
    parser.add_argument("--fxc-cache", type=Path, default=None,
                        help="compiled-combo cache (default ~/.cache/source-engine/fxc)")
    parser.add_argument("--vpk", type=Path, default=None,
                        help="with --shaders retail, the VPK holding shaders/fxc")
    parser.add_argument("--tolerance", type=int, default=DEFAULT_TOLERANCE)
    parser.add_argument("--json", type=Path, help="write the per-case details here")
    args = parser.parse_args(argv)
    try:
        source = make_source(args.shaders, args.fxc_cache, args.vpk)
        report = json.loads(args.pixels.read_text())
        passes = load_capture(args.capture)
        failures, details = evaluate(report, passes, source, args.tolerance)
    except (OracleError, OSError, ValueError) as error:
        print("legacy shader oracle: invalid input: %s" % error, file=sys.stderr)
        return 2
    if args.json:
        args.json.write_text(json.dumps({"failures": failures, "cases": details}, indent=1) + "\n")
    print("legacy shader oracle: %s (%d cases)" % ("pass" if not failures else "fail",
                                                    len(details)))
    for failure in failures:
        print("  " + failure)
    return 0 if not failures else 1


if __name__ == "__main__":
    sys.exit(main())
