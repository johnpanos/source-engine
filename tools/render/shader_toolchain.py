#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The pinned shader compiler (RFC 0016 K0, check shader.toolchain-pin).

    python3 tools/render/shader_toolchain.py build [--jobs N] [--fetch-only]
    python3 tools/render/shader_toolchain.py identity
    python3 tools/render/shader_toolchain.py check [--out DIR] [--seed-fault FAULT]
    python3 tools/render/shader_toolchain.py sensitivity [--out DIR]
    python3 tools/render/shader_toolchain.py build-cross [--jobs N]

quality/toolchain/shader-compiler.json pins the compiler: shaderc's glslc with
its glslang, SPIRV-Tools and SPIRV-Headers, each a source archive with its
sha256, and the identity `glslc --version` reports. This module is the one
owner of that pin:

  build     fetches the pinned archives into dependencies/shader-toolchain
            (gitignored), verifies them and builds bin/glslc and
            bin/glslangValidator. Stages are skipped when manifest.json records
            the same inputs.
            It then builds the pinned SPIRV-Cross (cross_compiler in the
            pin, RFC 0016 K4) into bin/spirv-cross, with its own stamp.
  build-cross  only the SPIRV-Cross stage.
  identity  prints the compilers the tools resolve and whether each is the
            pin.
  check     the gate: the resolved compiler must report exactly the pinned
            identity. The generated headers (GENERATED_NAMES; RFC 0016 K4
            deleted their committed copies) are written as the build writes
            them (shader_artifacts.generate_headers) and must agree with the
            regenerators' independent --check writers and, row by row, with
            the compiler. The few modules still committed (EMBEDDED) must
            rebuild byte-identically. An inventory pass fails on any committed
            SPIR-V array outside EMBEDDED, and on a committed copy of a
            generated header. Prints one `CONFORMANCE <checks> <failures>`
            record (checks-v1).

The compiler resolves, in order, from $SHADER_TOOLCHAIN_GLSLC, the pinned build
(dependencies/shader-toolchain/bin/glslc) and PATH; whichever it is, its
identity must equal the pin. The regenerators import glslc() and
glslang_validator() from here instead of calling a compiler from PATH.

--seed-fault runs the check against a seeded defect:
  foreign-compiler  a wrapper around the real compiler that reports another
                    version must fail toolchain.identity;
  flip-byte         the generated headers and copies of every committed module
                    file, each with one byte of one SPIR-V module changed,
                    must fail their checks.
`sensitivity` runs an unseeded control and both faults and requires each
fault to be rejected for exactly its seeded defects (checks-v1).
"""

import argparse
import concurrent.futures
import functools
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
PIN = ROOT / "quality" / "toolchain" / "shader-compiler.json"
ENV_GLSLC = "SHADER_TOOLCHAIN_GLSLC"
ENV_GLSLANG = "SHADER_TOOLCHAIN_GLSLANG"
ENV_SPIRV_CROSS = "SHADER_TOOLCHAIN_SPIRV_CROSS"
CROSS_MANIFEST = "manifest-spirv-cross.json"
# Bump when the SPIRV-Cross build recipe below changes.
CROSS_RECIPE = 1
SCHEMA = "shader-compiler-pin/v1"
# Bump when the build recipe below changes, to force a rebuild.
RECIPE = 1
SPIRV_MAGIC = 0x07230203

sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

SHADERS = "materialsystem/shaderapivulkan/shaders"
DEVICE = "unittests/rendertest/core/device"
SKINNING = "render/pass/skinning"
LINES = "render/pass/lines"
FAMILIES = "render/material/families"
SKINNING_TESTS = "unittests/rendertest/core/skinning"
LIGHTS = "render/pass/lights"
SHADOWS = "render/pass/shadows"
DEBUG = "render/pass/debug"
OUTPUT = "render/pass/output"
VOLUMETRIC = "render/pass/volumetric"
LAB = "render/lab"
SSR = "render/pass/ssr"
LIGHTS_TESTS = "unittests/rendertest/core/pass/lights"
SHADOWS_TESTS = "unittests/rendertest/core/pass/shadows"

# The regenerators: the independent writers of the backend's generated
# headers (material_spv.h, material_spv_index.h, legacy_spv.h). The build
# writes those headers from the artifacts (tools/render/shader_artifacts.py
# headers); the checks run each regenerator with --check --compare-dir against
# the build's copies.
REGENERATORS = (
    ("material", SHADERS + "/regen_material_spv.py",
     ("material_spv.h", "material_spv_index.h")),
    ("legacy", SHADERS + "/regen_legacy_spv.py", ("legacy_spv.h",)),
)

DEVICE_OPTIONS = ("--target-env=vulkan1.1", "-O")

# Committed SPIR-V: (file, array, GLSL source, glslc options). Only the
# render.device.v2 suite's small test modules stay committed; each array must
# equal what the pinned compiler builds.
EMBEDDED = (
    (DEVICE + "/test_shaders.h", "kFullScreenVertex", DEVICE + "/shaders/tri.vert",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kTopHalfVertex", DEVICE + "/shaders/tophalf.vert",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kColorFragment", DEVICE + "/shaders/color.frag",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kConstantFragment", DEVICE + "/shaders/constant.frag",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kSpecializedFragment", DEVICE + "/shaders/specialized.frag",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kDoubleCompute", DEVICE + "/shaders/double.comp",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kSampledFragment", DEVICE + "/shaders/sampled.frag",
     DEVICE_OPTIONS),
    (DEVICE + "/test_shaders.h", "kPositionVertex", DEVICE + "/shaders/position.vert",
     DEVICE_OPTIONS),
)

# Generated headers written by glslc rows (RFC 0016 K4: none is committed; the
# build writes them into <build>/render/shaders/generated/spv/ and consumers
# include "spv/<name>"). name: (namespace, or None for the backend's
# file-static arrays, purpose, ((array, GLSL source, glslc options), ...)).
GENERATED = {
    "demo_triangle_spv.h": (None,
        "the native Vulkan bring-up demo pipelines and the dynamic-mesh material "
        "catalog (demo_dyn.vert with demo_triangle.frag, demo_greenify.frag and "
        "demo_constcolor.frag)", (
        ("g_demoTriangleVertSpv", SHADERS + "/demo_triangle.vert", ()),
        ("g_demoTriangleFragSpv", SHADERS + "/demo_triangle.frag", ()),
        ("g_demoTexQuadVertSpv", SHADERS + "/demo_texquad.vert", ()),
        ("g_demoTexQuadFragSpv", SHADERS + "/demo_texquad.frag", ()),
        ("g_demoIndexedUboVertSpv", SHADERS + "/demo_indexed_ubo.vert", ()),
        ("g_demoIndexedUboFragSpv", SHADERS + "/demo_indexed_ubo.frag", ()),
        ("g_demoDepthVertSpv", SHADERS + "/demo_depth.vert", ()),
        ("g_demoGreenifyFragSpv", SHADERS + "/demo_greenify.frag", ()),
        ("g_demoConstColorFragSpv", SHADERS + "/demo_constcolor.frag", ()),
        ("g_demoDynVertSpv", SHADERS + "/demo_dyn.vert", ()))),
    "skin_spv.h": ("render::pass::skinning::spirv", "the skinning compute pass (RFC 0016 K6)", (
        ("kSkinCompute", SKINNING + "/skin.comp", DEVICE_OPTIONS),)),
    "families_spv.h": ("render::material::spirv",
        "the material families' programs (RFC 0016 K4, render.material)", (
        ("kSurfaceFlatVertex", FAMILIES + "/surface_flat.vert", DEVICE_OPTIONS),
        ("kSurfaceWorldVertex", FAMILIES + "/surface_world.vert", DEVICE_OPTIONS),
        ("kSurfaceModelVertex", FAMILIES + "/surface_model.vert", DEVICE_OPTIONS),
        ("kSurfaceFragment", FAMILIES + "/surface.frag", DEVICE_OPTIONS))),
    "lines_spv.h": ("render::pass::lines::spirv",
        "the lines pass: wireframe, grid and overlays (RFC 0016, Hammer viewports)", (
        ("kLinesVertex", LINES + "/lines.vert", DEVICE_OPTIONS),
        ("kLinesFragment", LINES + "/lines.frag", DEVICE_OPTIONS))),
    "debug_spv.h": ("render::pass::debug::spirv",
        "render.pass.debug: the not-applicable hatch (RFC 0014)", (
        ("kFullscreenVertex", DEBUG + "/fullscreen.vert", DEVICE_OPTIONS),
        ("kHatchFragment", DEBUG + "/hatch.frag", DEVICE_OPTIONS),
        ("kTintFragment", DEBUG + "/tint.frag", DEVICE_OPTIONS))),
    "cluster_assign_spv.h": ("render::pass::lights::spirv",
        "the clustered light assignment pass (RFC 0016 K7)", (
        ("kClusterAssignCompute", LIGHTS + "/cluster_assign.comp", DEVICE_OPTIONS),)),
    "shadow_spv.h": ("render::pass::shadows::spirv", "the shadow passes (RFC 0016 K7)", (
        ("kShadowDepthVertex", SHADOWS + "/shadow_depth.vert", DEVICE_OPTIONS),
        ("kShadowReceiverVertex", SHADOWS + "/shadow_receiver.vert", DEVICE_OPTIONS),
        ("kShadowReceiverFragment", SHADOWS + "/shadow_receiver.frag", DEVICE_OPTIONS))),
    "volumetric_spv.h": ("render::pass::volumetric::spirv",
        "the volumetric fog pass: inject and composite (RFC 0016, the "
        "participating-media term)", (
        ("kVolumetricInjectCompute", VOLUMETRIC + "/volumetric_inject.comp", DEVICE_OPTIONS),
        ("kVolumetricCompositeVertex", VOLUMETRIC + "/volumetric_composite.vert",
         DEVICE_OPTIONS),
        ("kVolumetricCompositeFragment", VOLUMETRIC + "/volumetric_composite.frag",
         DEVICE_OPTIONS))),
    "skin_defects_spv.h": ("rendertest::skinning::spirv",
        "the skinning suites' seeded kernels (render.skinning sensitivity)", (
        ("kSkinBoneIndexError", SKINNING + "/skin.comp",
         DEVICE_OPTIONS + ("-DSEEDED_BONE_INDEX_ERROR",)),
        ("kSkinFlexWeightError", SKINNING + "/skin.comp",
         DEVICE_OPTIONS + ("-DSEEDED_FLEX_WEIGHT_ERROR",)))),
    "cluster_defects_spv.h": ("rendertest::lights::spirv",
        "the cluster suite's seeded kernels (render.lights.clusters.gpu)", (
        ("kClusterSliceOffByOne", LIGHTS + "/cluster_assign.comp",
         DEVICE_OPTIONS + ("-DSEEDED_SLICE_OFF_BY_ONE",)),
        ("kClusterConeIgnored", LIGHTS + "/cluster_assign.comp",
         DEVICE_OPTIONS + ("-DSEEDED_CONE_IGNORED",)),
        ("kClusterUncountedOverflow", LIGHTS + "/cluster_assign.comp",
         DEVICE_OPTIONS + ("-DSEEDED_UNCOUNTED_OVERFLOW",)))),
    "output_spv.h": ("render::pass::output::spirv",
        "the output pass: exposure, tone map and output encoding (RFC 0016, render.output.v1)", (
        ("kOutputVertex", OUTPUT + "/output.vert", DEVICE_OPTIONS),
        ("kOutputFragment", OUTPUT + "/output.frag", DEVICE_OPTIONS))),
    "ssr_spv.h": ("render::pass::ssr::spirv",
        "render.pass.ssr: the pyramids and the trace (RFC 0016 K11, render.ssr.v1)", (
        ("kSsrPyramidCompute", SSR + "/ssr_pyramid.comp", DEVICE_OPTIONS),
        ("kSsrTraceCompute", SSR + "/ssr_trace.comp", DEVICE_OPTIONS))),
    "ssr_variants_spv.h": ("render::lab::spirv",
        "render_lab's ssr suite's trace variants: diagnostics, and the seeded defects "
        "(render.lab.ssr, RFC 0016 K11)", (
        ("kSsrTraceDiagnostics", SSR + "/ssr_trace.comp",
         DEVICE_OPTIONS + ("-DSSR_DIAGNOSTICS",)),
        ("kSsrTraceThicknessIgnored", SSR + "/ssr_trace.comp",
         DEVICE_OPTIONS + ("-DSSR_DIAGNOSTICS", "-DSEEDED_SSR_THICKNESS_IGNORED")),
        ("kSsrTraceNoEdgeFade", SSR + "/ssr_trace.comp",
         DEVICE_OPTIONS + ("-DSSR_DIAGNOSTICS", "-DSEEDED_SSR_NO_EDGE_FADE")),
        ("kSsrTraceWrongMip", SSR + "/ssr_trace.comp",
         DEVICE_OPTIONS + ("-DSSR_DIAGNOSTICS", "-DSEEDED_SSR_WRONG_MIP")),
        ("kSsrTraceHardSwitch", SSR + "/ssr_trace.comp",
         DEVICE_OPTIONS + ("-DSSR_DIAGNOSTICS", "-DSEEDED_SSR_HARD_SWITCH")))),
    "output_defects_spv.h": ("rendertest::output::spirv",
        "the output suite's seeded fragment programs (render.output)", (
        ("kOutputAlwaysCompress", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_ALWAYS_COMPRESS",)),
        ("kOutputPerChannel", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_PER_CHANNEL",)),
        ("kOutputKneeAtPeak", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_KNEE_AT_PEAK",)),
        ("kOutputHeadroomIgnored", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_HEADROOM_IGNORED",)),
        ("kOutputDebugViewToneMapped", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_VIEW_TONE_MAPPED",)),
        ("kOutputSrgbOnLinear", OUTPUT + "/output.frag",
         DEVICE_OPTIONS + ("-DSEEDED_SRGB_ON_LINEAR",)))),
    "shadow_defects_spv.h": ("rendertest::shadows::spirv",
        "the shadow suite's seeded receiver (render.shadows.pixels)", (
        ("kShadowReceiverDepthReversed", SHADOWS + "/shadow_receiver.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEPTH_REVERSED",)),)),
    "clustered_light_defects_spv.h": ("render::lab::spirv",
        "render_lab's clustered-light suite's seeded programs (render.lab.clustered-lights "
        "sensitivity, RFC 0016 K11)", (
        ("kSurfaceClusterSliceOffByOne", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_CLUSTER_SLICE_OFF_BY_ONE",)),
        ("kSurfaceClusterSkipsFirst", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_CLUSTER_SKIPS_FIRST",)),
        ("kSurfaceRuntimeFalloffUnwindowed", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_RUNTIME_FALLOFF_UNWINDOWED",)),
        ("kSurfaceSpotNoCosine", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_SPOT_NO_COSINE",)))),
    "shadowed_light_defects_spv.h": ("render::lab::spirv",
        "render_lab's shadowed-light suite's seeded programs (render.lab.shadowed-lights "
        "sensitivity, RFC 0016 K11)", (
        ("kSurfaceShadowIgnored", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_SHADOW_IGNORED",)),
        ("kSurfaceShadowTileNext", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_SHADOW_TILE_NEXT",)),
        ("kSurfaceShadowDepthReversed", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEPTH_REVERSED",)))),
    "area_light_defects_spv.h": ("render::lab::spirv",
        "render_lab's area-light suite's seeded programs (render.lab.area-lights sensitivity, "
        "RFC 0016 K11)", (
        ("kSurfaceLtcNoHorizonClip", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_LTC_NO_HORIZON_CLIP",)),
        ("kSurfaceLtcTransposed", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_LTC_TRANSPOSED",)),
        ("kSurfaceLtcNoMagnitude", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_LTC_NO_MAGNITUDE",)))),
    "volumetric_defects_spv.h": ("render::lab::spirv",
        "render_lab's volumetric suite's seeded stages (render.lab.volumetric sensitivity, "
        "RFC 0016 K11)", (
        ("kVolumetricPhaseIgnored", VOLUMETRIC + "/volumetric_inject.comp",
         DEVICE_OPTIONS + ("-DSEEDED_PHASE_IGNORED",)),
        ("kVolumetricAlbedoIgnored", VOLUMETRIC + "/volumetric_inject.comp",
         DEVICE_OPTIONS + ("-DSEEDED_ALBEDO_IGNORED",)),
        ("kVolumetricExtinctionTwice", VOLUMETRIC + "/volumetric_composite.frag",
         DEVICE_OPTIONS + ("-DSEEDED_EXTINCTION_TWICE",)),
        ("kVolumetricSliceOffByOne", VOLUMETRIC + "/volumetric_composite.frag",
         DEVICE_OPTIONS + ("-DSEEDED_SLICE_OFF_BY_ONE",)))),
    "lightmap_basis_check_spv.h": ("render::lab::spirv",
        "render_lab's lightmap-basis suite's check kernel and its seeded variants "
        "(render.lab.lightmap-basis, RFC 0016 K11)", (
        ("kLightmapBasisCheck", LAB + "/lightmap_basis_check.comp", DEVICE_OPTIONS),
        ("kLightmapBasisNoSmoothNormal", LAB + "/lightmap_basis_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_LIGHTMAP_NO_SMOOTH_NORMAL",)),
        ("kLightmapBasisNoGainClamp", LAB + "/lightmap_basis_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_LIGHTMAP_NO_GAIN_CLAMP",)),
        ("kLightmapBasisRnmUnsquared", LAB + "/lightmap_basis_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_LIGHTMAP_RNM_UNSQUARED",)),
        ("kLightmapBasisRnmOffsetFromZero", LAB + "/lightmap_basis_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_LIGHTMAP_RNM_OFFSET_FROM_ZERO",)))),
    "probe_volume_check_spv.h": ("render::lab::spirv",
        "render_lab's probe-volume suite's check kernel and its seeded variants "
        "(render.lab.probe-volume, RFC 0016 K11)", (
        ("kProbeVolumeCheck", LAB + "/probe_volume_check.comp", DEVICE_OPTIONS),
        ("kProbeVolumeNoNormalBias", LAB + "/probe_volume_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_PROBE_NO_NORMAL_BIAS",)),
        ("kProbeVolumeStateIgnored", LAB + "/probe_volume_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_PROBE_STATE_IGNORED",)),
        ("kProbeVolumeVisibilityIgnored", LAB + "/probe_volume_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_PROBE_VISIBILITY_IGNORED",)),
        ("kProbeVolumeNoCrush", LAB + "/probe_volume_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_PROBE_NO_CRUSH",)))),
    "reflection_probes_check_spv.h": ("render::lab::spirv",
        "render_lab's reflection-probes suite's check kernel and its seeded variants "
        "(render.lab.reflection-probes, RFC 0016 K11)", (
        ("kReflectionProbesCheck", LAB + "/reflection_probes_check.comp", DEVICE_OPTIONS),
        ("kReflectionProbesNoDistanceRoughness", LAB + "/reflection_probes_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_RPRB_NO_DISTANCE_ROUGHNESS",)),
        ("kReflectionProbesNoFacing", LAB + "/reflection_probes_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_RPRB_NO_FACING",)),
        ("kReflectionProbesRelightAddedOnly", LAB + "/reflection_probes_check.comp",
         DEVICE_OPTIONS + ("-DSEEDED_RPRB_RELIGHT_ADDED_ONLY",)))),
    "debug_view_defects_spv.h": ("render::lab::spirv",
        "render_lab's debug-view suite's seeded programs (render.debug-views sensitivity, "
        "RFC 0014)", (
        ("kLightmappedSwappedNormal", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_SWAPPED_NORMAL",)),
        ("kLightmappedToneMaps", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_TONE_MAPS",)),
        ("kLightmappedMissesNan", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_MISS_NAN",)),
        ("kLightmappedNoHatch", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_NO_HATCH",)),
        ("kLightmappedTermIgnored", FAMILIES + "/surface.frag",
         DEVICE_OPTIONS + ("-DSEEDED_DEBUG_TERM_IGNORED",)))),
}

# GENERATED headers holding material-family programs: artifact units that a
# layouts.json family owns (tools/render/shader_artifacts.py).
FAMILY_HEADERS = ("families_spv.h",)

# GLSL 4.50 headers (RFC 0016 K10): the OpenGL adapter's artifacts of SPIR-V
# rows, cross-compiled by tools/render/shader_artifacts.py with the pinned
# SPIRV-Cross (its cross_compile owns the artifacts' form). name: (namespace,
# purpose, ((array, GLSL source, glslc options), ...)); each array is a char
# array of GLSL text.
GLSL_GENERATED = {
    "device_fixtures_glsl.h": ("rendertest::glsl",
        "the render.device.v2 suite's fixtures (EMBEDDED) for the OpenGL adapter",
        tuple((array, source, options) for _, array, source, options in EMBEDDED)),
}
# The core's programs (the GENERATED headers the render passes and material
# families embed) each have a GLSL 4.50 twin: <stem>_glsl.h in the namespace's
# ::glsl sibling, with the same array names.
CORE_PROGRAM_HEADERS = ("cluster_assign_spv.h", "debug_spv.h", "families_spv.h", "lines_spv.h",
                        "output_spv.h", "shadow_spv.h", "skin_spv.h", "volumetric_spv.h",
                        "ssr_spv.h")
for _header in CORE_PROGRAM_HEADERS:
    _namespace, _purpose, _rows = GENERATED[_header]
    GLSL_GENERATED[_header.replace("_spv.h", "_glsl.h")] = (
        _namespace.replace("::spirv", "::glsl"), _purpose, _rows)
del _header, _namespace, _purpose, _rows

# The core artifact store's table (public/render/shaderlib/core_artifacts.h):
# every CORE_PROGRAM_HEADERS row in both formats with its reflection, written
# by tools/render/shader_artifacts.py store_header.
STORE_HEADER = "core_artifact_table.h"

# Every generated header's name (the regenerators', GENERATED's,
# GLSL_GENERATED's and the store's).
GENERATED_NAMES = tuple(sorted({n for _, _, names in REGENERATORS for n in names} |
                               set(GENERATED) | set(GLSL_GENERATED) | {STORE_HEADER}))


def generated_rows():
    """(header, array, GLSL source, options) of every GENERATED row."""
    return tuple((header, array, source, options)
                 for header, (_, _, rows) in sorted(GENERATED.items())
                 for array, source, options in rows)


def render_generated(header, words_of):
    """The text of a GENERATED header; words_of(array) gives each array's
    SPIR-V words."""
    namespace, purpose, rows = GENERATED[header]
    guard = "GENERATED_SPV_" + re.sub(r"[^A-Z0-9]", "_", header.upper())
    out = ["//========= Copyright Valve Corporation, All rights reserved. ============//\n",
           "//\n",
           "// Purpose: SPIR-V of %s. GENERATED by\n" % purpose,
           "//          tools/render/shader_artifacts.py headers with the pinned glslc\n",
           "//          (quality/toolchain/shader-compiler.json); not committed, do not edit.\n",
           "//\n",
           "//=============================================================================//\n\n",
           "#ifndef %s\n#define %s\n\n#include <cstdint>\n\n" % (guard, guard)]
    if namespace:
        out.append("namespace %s\n{\n\n" % namespace)
    for array, source, options in rows:
        words = words_of(array)
        out.append("// %s%s\n" % (source, (" " + " ".join(options)) if options else ""))
        declaration = ("inline constexpr std::uint32_t %s[] = {" if namespace
                       else "static const uint32_t %s[] = {") % array
        out.append(declaration + "\n")
        for i in range(0, len(words), 8):
            out.append("    " + ", ".join("0x%08xu" % w for w in words[i:i + 8]) + ",\n")
        out.append("};\n\n")
    if namespace:
        out.append("} // namespace %s\n\n" % namespace)
    out.append("#endif // %s\n" % guard)
    return "".join(out)


def render_glsl(header, text_of):
    """The text of a GLSL_GENERATED header; text_of(array) gives each array's
    GLSL 4.50."""
    namespace, purpose, rows = GLSL_GENERATED[header]
    guard = "GENERATED_GLSL_" + re.sub(r"[^A-Z0-9]", "_", header.upper())
    out = ["//========= Copyright Valve Corporation, All rights reserved. ============//\n",
           "//\n",
           "// Purpose: GLSL 4.50 of %s. GENERATED by\n" % purpose,
           "//          tools/render/shader_artifacts.py headers with the pinned glslc and\n",
           "//          SPIRV-Cross (quality/toolchain/shader-compiler.json); not committed,\n",
           "//          do not edit.\n",
           "//\n",
           "//=============================================================================//\n\n",
           "#ifndef %s\n#define %s\n\n" % (guard, guard),
           "namespace %s\n{\n\n" % namespace]
    for array, source, options in rows:
        text = text_of(array)
        if ")glsl\"" in text:
            raise ToolchainError("%s: the GLSL holds the raw string's delimiter" % array)
        out.append("// %s%s\n" % (source, (" " + " ".join(options)) if options else ""))
        out.append("inline constexpr char %s[] = R\"glsl(%s)glsl\";\n\n" % (array, text))
    out.append("} // namespace %s\n\n" % namespace)
    out.append("#endif // %s\n" % guard)
    return "".join(out)


# Files whose SPIR-V-looking initializers are not compiler output.
EXEMPT = {
    "unittests/shaderapivulkantest/test_vulkan_debug_tools.cpp":
        "hand-written magic/version words that exercise the debug-variant table's validation",
}

HEX = r"0[xX][0-9a-fA-F]+[uU]?"
ARRAY = re.compile(r"(\w+)\s*\[\s*\]\s*=\s*\{(\s*%s(?:\s*,\s*%s)*\s*,?\s*)\}" % (HEX, HEX))
MAGIC_INITIALIZER = re.compile(r"\{\s*0[xX]0*7230203[uU]?\b")
SCANNED_SUFFIXES = (".h", ".hpp", ".c", ".cc", ".cpp", ".inc")


class ToolchainError(Exception):
    pass


class RecordingChecks(Checks):
    """Checks that also keep each check's name and outcome for the evidence."""

    def __init__(self, stream=None):
        super().__init__(stream)
        self.results = []

    def check(self, condition, name, detail=""):
        self.results.append({"name": name, "ok": bool(condition)})
        return super().check(condition, name, detail)


def log(message):
    print("shader_toolchain: " + message, file=sys.stderr, flush=True)


# ---------------------------------------------------------------------------
# The pin and compiler identity


def load_pin(path=PIN):
    pin = json.loads(Path(path).read_text())
    if pin.get("schema") != SCHEMA:
        raise ToolchainError("%s: schema %r is not %s" % (path, pin.get("schema"), SCHEMA))
    for key in ("directory", "compiler", "debug_compiler", "components", "build"):
        if key not in pin:
            raise ToolchainError("%s: no %r" % (path, key))
    for name, component in pin["components"].items():
        for key in ("commit", "url", "sha256", "cache_archive", "extracted_directory"):
            if not component.get(key):
                raise ToolchainError("%s: component %s has no %r" % (path, name, key))
    return pin


def pinned_bin(pin, root=ROOT):
    return Path(root) / pin["directory"] / "bin"


def resolve(executable, environment, pin, root=ROOT):
    """(path, how) of a compiler: the environment override, the pinned build,
    then PATH. Identity is verified separately."""
    override = os.environ.get(environment)
    if override:
        return override, "$" + environment
    built = pinned_bin(pin, root) / executable
    if built.is_file():
        return str(built), "pinned build"
    found = shutil.which(executable)
    if found:
        return found, "PATH"
    raise ToolchainError("no %s: build the pinned compiler with "
                         "'python3 tools/render/shader_toolchain.py build'" % executable)


def version_lines(path, argument="--version"):
    try:
        result = subprocess.run([path, argument], capture_output=True, text=True, timeout=60)
    except OSError as error:
        raise ToolchainError("cannot run %s: %s" % (path, error)) from error
    if result.returncode != 0:
        raise ToolchainError("%s %s exited %d: %s" % (path, argument, result.returncode,
                                                      (result.stdout + result.stderr).strip()))
    return [line.rstrip() for line in result.stdout.strip().splitlines()]


def identity_problem(path, pin):
    """None when glslc at `path` reports the pinned identity, else the reason,
    naming both identities."""
    expected = pin["compiler"]["identity"]
    actual = version_lines(path, pin["compiler"].get("version_argument", "--version"))
    if actual == expected:
        return None
    return ("%s reports %s; the pin (%s) is %s. Build the pinned compiler with "
            "'python3 tools/render/shader_toolchain.py build'"
            % (path, json.dumps(" / ".join(actual)), PIN.relative_to(ROOT),
               json.dumps(" / ".join(expected))))


def debug_identity_problem(path, pin):
    expected = pin["debug_compiler"]["identity_first_line"]
    lines = version_lines(path, pin["debug_compiler"].get("version_argument", "--version"))
    actual = lines[0] if lines else ""
    if actual == expected:
        return None
    return "%s reports %r; the pin is %r" % (path, actual, expected)


@functools.lru_cache(maxsize=None)
def glslc():
    """The verified pinned glslc (path)."""
    pin = load_pin()
    path, _ = resolve(pin["compiler"]["executable"], ENV_GLSLC, pin)
    problem = identity_problem(path, pin)
    if problem:
        raise ToolchainError(problem)
    return path


def cross_identity_problem(path, pin):
    """None when spirv-cross at `path` reports the pinned identity (it prints
    its revision to stderr), else the reason."""
    expected = pin["cross_compiler"]["identity"]
    try:
        result = subprocess.run([path, pin["cross_compiler"].get("version_argument",
                                                                 "--revision")],
                                capture_output=True, text=True, timeout=60)
    except OSError as error:
        return "cannot run %s: %s" % (path, error)
    actual = [line.rstrip() for line in (result.stdout + result.stderr).strip().splitlines()]
    if actual == expected:
        return None
    return ("%s reports %s; the pin (%s) is %s. Build it with "
            "'python3 tools/render/shader_toolchain.py build-cross'"
            % (path, json.dumps(" / ".join(actual)), PIN.relative_to(ROOT),
               json.dumps(" / ".join(expected))))


@functools.lru_cache(maxsize=None)
def spirv_cross():
    """The verified pinned spirv-cross (path)."""
    pin = load_pin()
    if "cross_compiler" not in pin:
        raise ToolchainError("%s has no cross_compiler" % PIN.relative_to(ROOT))
    path, _ = resolve(pin["cross_compiler"]["executable"], ENV_SPIRV_CROSS, pin)
    problem = cross_identity_problem(path, pin)
    if problem:
        raise ToolchainError(problem)
    return path


@functools.lru_cache(maxsize=None)
def glslang_validator():
    """The verified pinned glslangValidator (path)."""
    pin = load_pin()
    path, _ = resolve(pin["debug_compiler"]["executable"], ENV_GLSLANG, pin)
    problem = debug_identity_problem(path, pin)
    if problem:
        raise ToolchainError(problem)
    return path


# ---------------------------------------------------------------------------
# Committed modules


def embedded_arrays(text):
    """{name: words} of the SPIR-V arrays (first word the magic) in C/C++ text."""
    arrays = {}
    for match in ARRAY.finditer(text):
        words = [int(item.strip().rstrip("uU"), 16)
                 for item in match.group(2).split(",") if item.strip()]
        if words and words[0] == SPIRV_MAGIC:
            arrays[match.group(1)] = words
    return arrays


def compile_module(compiler, source, options):
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "module.spv"
        result = subprocess.run([compiler, *options, str(source), "-o", str(out)],
                                capture_output=True, text=True)
        if result.returncode != 0:
            raise ToolchainError("%s %s failed: %s" % (compiler, source,
                                                       (result.stdout + result.stderr).strip()))
        data = out.read_bytes()
    return [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]


def first_difference(actual, expected):
    for index, (a, b) in enumerate(zip(actual, expected)):
        if a != b:
            return "word %d is 0x%08x, the compiler builds 0x%08x" % (index, a, b)
    return "%d words, the compiler builds %d" % (len(actual), len(expected))


def check_embedded(checks, compiler, source_root, committed_root, table=EMBEDDED):
    """One check per table row: the committed array equals the rebuilt module."""
    texts = {}
    for path in sorted({row[0] for row in table}):
        texts[path] = embedded_arrays((Path(committed_root) / path).read_text())
    jobs = [(compiler, Path(source_root) / source, list(options))
            for _, _, source, options in table]
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        futures = [pool.submit(compile_module, *job) for job in jobs]
    for (path, array, source, options), future in zip(table, futures):
        name = "module.%s:%s" % (path, array)
        try:
            built = future.result()
        except ToolchainError as error:
            checks.check(False, name, str(error))
            continue
        committed = texts[path].get(array)
        if committed is None:
            checks.check(False, name, "no SPIR-V array %s in %s" % (array, path))
            continue
        checks.check(committed == built, name, "%s (from %s %s)" % (
            first_difference(committed, built), source, " ".join(options)))
    # Every SPIR-V array in a table file must have a row.
    for path, arrays in texts.items():
        rows = {row[1] for row in table if row[0] == path}
        unlisted = sorted(set(arrays) - rows)
        checks.check(not unlisted, "inventory.arrays.%s" % path,
                     "SPIR-V arrays without a GLSL source row in EMBEDDED: %s"
                     % ", ".join(unlisted))


def spirv_files(root):
    """Repository-relative paths of C/C++ files holding an initializer that
    starts with the SPIR-V magic."""
    root = Path(root)
    found = set()
    if (root / ".git").exists():
        # git grep matches lines; MAGIC_INITIALIZER below spans the newline
        # between an initializer's brace and its first word.
        result = subprocess.run(["git", "-C", str(root), "grep", "--untracked", "-l", "-I", "-E",
                                 "0[xX]0*7230203"], capture_output=True, text=True)
        if result.returncode not in (0, 1):
            raise ToolchainError("git grep failed: " + result.stderr.strip())
        candidates = result.stdout.splitlines()
    else:
        candidates = [str(path.relative_to(root)) for path in root.rglob("*")
                      if path.is_file()]
    for relative in candidates:
        if not relative.endswith(SCANNED_SUFFIXES):
            continue
        text = (root / relative).read_text(errors="replace")
        if MAGIC_INITIALIZER.search(text):
            found.add(relative.replace(os.sep, "/"))
    return found


def check_inventory(checks, root, table=EMBEDDED, exempt=None, names=GENERATED_NAMES):
    exempt = EXEMPT if exempt is None else exempt
    covered = {row[0] for row in table}
    for relative in sorted(spirv_files(root)):
        checks.check(relative in covered or relative in exempt, "inventory.files.%s" % relative,
                     "committed SPIR-V that no EMBEDDED row rebuilds (generated headers "
                     "are written by the build, not committed)")
    tracked = subprocess.run(["git", "-C", str(root), "ls-files"], capture_output=True,
                             text=True).stdout.splitlines()
    for name in names:
        copies = [path for path in tracked if Path(path).name == name]
        checks.check(not copies, "inventory.generated.%s.not-committed" % name,
                     "committed copies of a generated header: %s" % ", ".join(copies))


def check_generated(checks, compiler, source_root, generated_dir):
    """One check per GENERATED row: the array in the generated header equals
    the module the compiler builds."""
    rows = generated_rows()
    texts = {}
    for header in sorted({row[0] for row in rows}):
        path = Path(generated_dir) / header
        texts[header] = embedded_arrays(path.read_text()) if path.is_file() else {}
    jobs = [(compiler, Path(source_root) / source, list(options)) for _, _, source, options in rows]
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        futures = [pool.submit(compile_module, *job) for job in jobs]
    for (header, array, source, options), future in zip(rows, futures):
        name = "generated.%s:%s" % (header, array)
        try:
            built = future.result()
        except ToolchainError as error:
            checks.check(False, name, str(error))
            continue
        written = texts[header].get(array)
        if written is None:
            checks.check(False, name, "no SPIR-V array %s in the generated %s" % (array, header))
            continue
        checks.check(written == built, name, "%s (from %s %s)" % (
            first_difference(written, built), source, " ".join(options)))


def run_regenerator(checks, name, script, compiler, compare_dir=None, root=ROOT):
    env = dict(os.environ)
    env[ENV_GLSLC] = compiler
    argv = [sys.executable, str(Path(root) / script), "--check"]
    if compare_dir:
        argv += ["--compare-dir", str(compare_dir)]
    result = subprocess.run(argv, capture_output=True, text=True, env=env)
    detail = (result.stdout + result.stderr).strip().splitlines()
    checks.check(result.returncode == 0, "generator.%s" % name,
                 "%s --check exited %d: %s" % (script, result.returncode,
                                               " | ".join(detail[-4:])))


# ---------------------------------------------------------------------------
# Seeded faults


FOREIGN_IDENTITY = ("shaderc v2025.5 v2025.5", "spirv-tools v2025.5 v2025.5", "glslang 16.1.0",
                    "", "Target: SPIR-V 1.0")


def write_foreign_compiler(directory, real):
    """A glslc that compiles with `real` but reports another version."""
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    wrapper = directory / "glslc"
    wrapper.write_text("#!/bin/sh\n"
                       "if [ \"$1\" = --version ]; then\n"
                       "  printf '%s\\n'\n"
                       "  exit 0\n"
                       "fi\n"
                       "exec %s \"$@\"\n"
                       % ("\\n".join(FOREIGN_IDENTITY), shlex.quote(real)))
    wrapper.chmod(0o755)
    return str(wrapper)


def flip_one_byte(path):
    """Change one byte of the first SPIR-V module in a C/C++ file: the low bit
    of its sixth word. Returns (array, word index)."""
    path = Path(path)
    text = path.read_text()
    for match in ARRAY.finditer(text):
        items = list(re.finditer(HEX, match.group(2)))
        if len(items) < 6 or int(items[0].group(0).rstrip("uU"), 16) != SPIRV_MAGIC:
            continue
        item = items[5]
        literal = item.group(0)
        suffix = literal[len(literal.rstrip("uU")):]
        digits = literal[2:len(literal) - len(suffix)]
        flipped = int(digits, 16) ^ 1
        replacement = literal[:2] + ("%0*x" % (len(digits), flipped)) + suffix
        start = match.start(2) + item.start()
        path.write_text(text[:start] + replacement + text[start + len(literal):])
        return match.group(1), 5
    raise ToolchainError("%s has no SPIR-V array to seed" % path)


def seed_copies(seeded_root, generated_dir, root=ROOT):
    """Copies of every committed module file under seeded_root, and the
    generated headers in generated_dir, each with one byte changed (the index
    is left alone); returns {file: (array, word)}."""
    seeded = {}
    for relative in sorted({row[0] for row in EMBEDDED}):
        target = Path(seeded_root) / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(Path(root) / relative, target)
        seeded[relative] = flip_one_byte(target)
    for name in GENERATED_NAMES:
        # The GLSL headers and the store's table hold no SPIR-V: the GL suites
        # compile the former, the store's users read the latter.
        if not name.endswith("_index.h") and name not in GLSL_GENERATED and \
                name != STORE_HEADER:
            seeded[name] = flip_one_byte(Path(generated_dir) / name)
    return seeded


def write_generated(generated_dir, root=ROOT):
    """The generated headers as the build writes them (lazy import: the
    artifact tool imports this module)."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    try:
        import shader_artifacts
    finally:
        sys.path.pop(0)
    shader_artifacts.generate_headers(generated_dir, root)


# ---------------------------------------------------------------------------
# check


def git_revision(root):
    result = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                            capture_output=True, text=True)
    return result.stdout.strip() or None


def run_check(out=None, seed_fault=None, root=ROOT, stream=None):
    """The shader.toolchain-pin gate; returns (Checks, evidence dict)."""
    checks = RecordingChecks(stream)
    evidence = {"schema": "shader-toolchain-evidence/v1", "revision": git_revision(root),
                "pin": str(PIN.relative_to(ROOT)), "seed_fault": seed_fault}
    try:
        pin = load_pin()
    except (OSError, ValueError, ToolchainError) as error:
        checks.check(False, "toolchain.pin-record", str(error))
        return checks, evidence
    checks.check(True, "toolchain.pin-record")
    evidence["pinned_identity"] = pin["compiler"]["identity"]

    try:
        compiler, how = resolve(pin["compiler"]["executable"], ENV_GLSLC, pin, root)
    except ToolchainError as error:
        checks.check(False, "toolchain.identity", str(error))
        return checks, evidence
    scratch = Path(out) if out else Path(tempfile.mkdtemp(prefix="shader-toolchain-"))
    scratch.mkdir(parents=True, exist_ok=True)
    if seed_fault == "foreign-compiler":
        compiler, how = write_foreign_compiler(scratch / "foreign", compiler), "seeded wrapper"
    evidence.update(compiler=compiler, resolved_from=how)
    print("compiler: %s (%s)" % (compiler, how), file=checks.stream)
    try:
        evidence["observed_identity"] = version_lines(compiler)
        problem = identity_problem(compiler, pin)
    except ToolchainError as error:
        problem = str(error)
    if not checks.check(problem is None, "toolchain.identity", problem or ""):
        print("rebuild checks not run: the compiler is not the pin", file=checks.stream)
        return checks, evidence

    try:
        debug, _ = resolve(pin["debug_compiler"]["executable"], ENV_GLSLANG, pin, root)
        problem = debug_identity_problem(debug, pin)
    except ToolchainError as error:
        problem = str(error)
    checks.check(problem is None, "toolchain.debug-identity", problem or "")

    generated = scratch / "generated"
    try:
        write_generated(generated, root)
        checks.check(True, "generated.headers-written")
    except Exception as error:  # the generator's own failure is this check's detail
        checks.check(False, "generated.headers-written", str(error))
        return checks, evidence
    committed_root = Path(root)
    if seed_fault == "flip-byte":
        committed_root = scratch / "seeded"
        evidence["seeded"] = {path: "%s word %d" % seed
                              for path, seed in seed_copies(committed_root, generated,
                                                            root).items()}
    for name, script, _ in REGENERATORS:
        run_regenerator(checks, name, script, compiler, generated, root)
    check_generated(checks, compiler, root, generated)
    check_embedded(checks, compiler, root, committed_root)
    check_inventory(checks, root)
    return checks, evidence


def command_check(args):
    checks, evidence = run_check(args.out, args.seed_fault)
    evidence.update(checks=checks.checks, failures=checks.failures, results=checks.results)
    if args.out:
        Path(args.out).mkdir(parents=True, exist_ok=True)
        (Path(args.out) / "shader-toolchain-evidence.json").write_text(
            json.dumps(evidence, indent=1) + "\n")
    return checks.report()


def failed_names(checks):
    return {result["name"] for result in checks.results if not result["ok"]}


def command_sensitivity(args):
    """The check must pass unseeded and reject each seeded fault for exactly
    the defects it seeded."""
    checks = Checks()
    out = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="shader-toolchain-"))
    runs = {}
    for fault in (None, "foreign-compiler", "flip-byte"):
        log_stream = io.StringIO()
        runs[fault] = run_check(out / (fault or "control"), fault, stream=log_stream)
        (out / ((fault or "control") + ".log")).write_text(log_stream.getvalue())
    control = runs[None][0]
    checks.check(control.checks > 0 and control.failures == 0, "control.passes",
                 "the unseeded check failed: %s" % ", ".join(sorted(failed_names(control))))
    checks.equal(sorted(failed_names(runs["foreign-compiler"][0])), ["toolchain.identity"],
                 "foreign-compiler.rejected-for-identity")
    flipped, evidence = runs["flip-byte"]
    expected = {"generator.%s" % name for name, _, _ in REGENERATORS}
    seeded = evidence.get("seeded", {})
    for path in sorted({row[0] for row in EMBEDDED}):
        array = seeded.get(path, "? word").split(" word ")[0]
        expected.add("module.%s:%s" % (path, array))
    for header in sorted(GENERATED):
        array = seeded.get(header, "? word").split(" word ")[0]
        expected.add("generated.%s:%s" % (header, array))
    failed = failed_names(flipped)
    for name in sorted(expected):
        checks.check(name in failed, "flip-byte.detected." + name, "the seeded byte passed")
    checks.check(failed <= expected, "flip-byte.only-seeded",
                 "unseeded checks failed: %s" % ", ".join(sorted(failed - expected)))
    return checks.report()


def command_identity(_args):
    pin = load_pin()
    status = 0
    for key, env, problem_of in (("compiler", ENV_GLSLC, identity_problem),
                                 ("debug_compiler", ENV_GLSLANG, debug_identity_problem)):
        try:
            path, how = resolve(pin[key]["executable"], env, pin)
            problem = problem_of(path, pin)
        except ToolchainError as error:
            path, how, problem = "-", "-", str(error)
        print("%s: %s (%s): %s" % (pin[key]["executable"], path, how,
                                   "pinned" if problem is None else "NOT PINNED: " + problem))
        status |= problem is not None
    if "cross_compiler" in pin:
        try:
            path, how = resolve(pin["cross_compiler"]["executable"], ENV_SPIRV_CROSS, pin)
            problem = cross_identity_problem(path, pin)
        except ToolchainError as error:
            path, how, problem = "-", "-", str(error)
        print("%s: %s (%s): %s" % (pin["cross_compiler"]["executable"], path, how,
                                   "pinned" if problem is None else "NOT PINNED: " + problem))
        status |= problem is not None
    return status


# ---------------------------------------------------------------------------
# build


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def fetch(component, archives):
    archives.mkdir(parents=True, exist_ok=True)
    path = archives / component["cache_archive"]
    if not path.is_file():
        log("downloading " + component["url"])
        partial = path.with_suffix(path.suffix + ".partial")
        with urllib.request.urlopen(component["url"]) as response, open(partial, "wb") as out:
            shutil.copyfileobj(response, out)
        partial.rename(path)
    actual = sha256(path)
    if actual != component["sha256"]:
        raise ToolchainError("%s: sha256 %s does not match the pin %s"
                             % (path, actual, component["sha256"]))
    return path


VERSION_SCRIPT = '''#!/usr/bin/env python3
# Written by tools/render/shader_toolchain.py: the pinned identity lines
# (quality/toolchain/shader-compiler.json) instead of 'git describe'.
import os, sys
CONTENT = %r
path = sys.argv[4]
os.makedirs(os.path.dirname(path), exist_ok=True)
if not os.path.isfile(path) or open(path).read() != CONTENT:
    with open(path, "w") as out:
        out.write(CONTENT)
'''


def build_version_content(pin):
    """build-version.inc: the identity's component lines as C string literals."""
    lines = []
    for line in pin["compiler"]["identity"]:
        if not line:
            break
        lines.append('"%s\\n"\n' % line.replace('"', '\\"'))
    return "".join(lines)


def build_stamp(pin):
    inputs = {key: pin[key] for key in ("compiler", "debug_compiler", "components",
                                        "patches", "build")}
    inputs["recipe"] = RECIPE
    return hashlib.sha256(json.dumps(inputs, sort_keys=True).encode()).hexdigest()


def first_line(argv):
    try:
        result = subprocess.run(argv, capture_output=True, text=True)
    except OSError:
        return None
    return (result.stdout or result.stderr).strip().splitlines()[0] if result.returncode == 0 \
        else None


def build(pin, jobs=None, fetch_only=False, root=ROOT):
    base = Path(root) / pin["directory"]
    archives, sources, build_dir = base / "archives", base / "src", base / "build"
    bin_dir, manifest_path = base / "bin", base / "manifest.json"
    stamp = build_stamp(pin)
    paths = {}
    for name, component in pin["components"].items():
        paths[name] = fetch(component, archives)
    if fetch_only:
        log("archives verified in %s" % archives)
        return 0
    if manifest_path.is_file():
        manifest = json.loads(manifest_path.read_text())
        if manifest.get("stamp") == stamp and \
                all((base / out).is_file() for out in pin["build"]["outputs"]):
            log("up to date: %s" % bin_dir)
            return 0
    for directory in (sources, build_dir, bin_dir):
        shutil.rmtree(directory, ignore_errors=True)
    sources.mkdir(parents=True)
    extracted = {}
    for name, component in pin["components"].items():
        with tarfile.open(paths[name]) as archive:
            archive.extractall(sources, filter="data")
        extracted[name] = sources / component["extracted_directory"]
        if not extracted[name].is_dir():
            raise ToolchainError("%s did not extract to %s" % (paths[name], extracted[name]))
    (extracted["shaderc"] / "utils" / "update_build_version.py").write_text(
        VERSION_SCRIPT % build_version_content(pin))

    env = dict(os.environ)
    # No source directory may be described by an enclosing repository.
    env["GIT_CEILING_DIRECTORIES"] = str(sources)
    configure = ["cmake", "-S", str(extracted["shaderc"]), "-B", str(build_dir),
                 "-G", pin["build"]["generator"],
                 "-DCMAKE_BUILD_TYPE=" + pin["build"]["build_type"],
                 "-DSHADERC_GLSLANG_DIR=" + str(extracted["glslang"]),
                 "-DSHADERC_SPIRV_TOOLS_DIR=" + str(extracted["spirv-tools"]),
                 "-DSHADERC_SPIRV_HEADERS_DIR=" + str(extracted["spirv-headers"]),
                 *pin["build"]["cmake_options"]]
    log("configuring shaderc")
    subprocess.run(configure, check=True, env=env, stdout=subprocess.DEVNULL)
    compile_ = ["cmake", "--build", str(build_dir), "--target", *pin["build"]["targets"]]
    if jobs:
        compile_ += ["--parallel", str(jobs)]
    log("building %s" % ", ".join(pin["build"]["targets"]))
    subprocess.run(compile_, check=True, env=env, stdout=subprocess.DEVNULL)
    bin_dir.mkdir(parents=True)
    for output, built in pin["build"]["outputs"].items():
        shutil.copy2(build_dir / built, base / output)

    glslc_path = str(bin_dir / pin["compiler"]["executable"])
    problem = identity_problem(glslc_path, pin)
    debug_problem = debug_identity_problem(str(bin_dir / pin["debug_compiler"]["executable"]),
                                           pin)
    if problem or debug_problem:
        raise ToolchainError("the pinned build is not the pin: %s" % (problem or debug_problem))
    manifest = {
        "schema": "shader-toolchain-build/v1",
        "stamp": stamp,
        "recipe": RECIPE,
        "pin": str(PIN.relative_to(ROOT)),
        "components": {name: {"commit": c["commit"], "sha256": c["sha256"]}
                       for name, c in pin["components"].items()},
        "identity": version_lines(glslc_path),
        "host": {"c++": first_line(["c++", "--version"]),
                 "cmake": first_line(["cmake", "--version"]),
                 "ninja": first_line(["ninja", "--version"])},
        "outputs": {output: sha256(base / output) for output in pin["build"]["outputs"]},
    }
    manifest_path.write_text(json.dumps(manifest, indent=1) + "\n")
    log("built %s" % bin_dir)
    return 0


def cross_stamp(pin):
    inputs = dict(pin["cross_compiler"])
    inputs["recipe"] = CROSS_RECIPE
    return hashlib.sha256(json.dumps(inputs, sort_keys=True).encode()).hexdigest()


def build_cross(pin, jobs=None, fetch_only=False, root=ROOT):
    """The SPIRV-Cross stage: its archive, a patched version header and the
    CLI, under the same directory as glslc with a stamp of its own."""
    cross = pin.get("cross_compiler")
    if not cross:
        raise ToolchainError("%s has no cross_compiler" % PIN.relative_to(ROOT))
    base = Path(root) / pin["directory"]
    archive = fetch(cross["component"], base / "archives")
    if fetch_only:
        return 0
    sources, build_dir = base / "src-spirv-cross", base / "build-spirv-cross"
    manifest_path = base / CROSS_MANIFEST
    stamp = cross_stamp(pin)
    if manifest_path.is_file():
        manifest = json.loads(manifest_path.read_text())
        if manifest.get("stamp") == stamp and \
                all((base / out).is_file() for out in cross["build"]["outputs"]):
            log("up to date: %s" % ", ".join(cross["build"]["outputs"]))
            return 0
    for directory in (sources, build_dir):
        shutil.rmtree(directory, ignore_errors=True)
    sources.mkdir(parents=True)
    with tarfile.open(archive) as stream:
        stream.extractall(sources, filter="data")
    source = sources / cross["component"]["extracted_directory"]
    if not source.is_dir():
        raise ToolchainError("%s did not extract to %s" % (archive, source))
    (source / "cmake" / "gitversion.in.h").write_text(
        "// Written by tools/render/shader_toolchain.py: the pinned identity\n"
        "// (quality/toolchain/shader-compiler.json cross_compiler).\n"
        "#ifndef SPIRV_CROSS_GIT_VERSION_H_\n#define SPIRV_CROSS_GIT_VERSION_H_\n"
        "#define SPIRV_CROSS_GIT_REVISION \"%s\"\n#endif\n"
        % cross["identity"][0].replace('"', '\\"'))
    env = dict(os.environ)
    env["GIT_CEILING_DIRECTORIES"] = str(sources)
    log("configuring SPIRV-Cross")
    subprocess.run(["cmake", "-S", str(source), "-B", str(build_dir),
                    "-G", cross["build"]["generator"],
                    "-DCMAKE_BUILD_TYPE=" + cross["build"]["build_type"],
                    *cross["build"]["cmake_options"]],
                   check=True, env=env, stdout=subprocess.DEVNULL)
    compile_ = ["cmake", "--build", str(build_dir), "--target", *cross["build"]["targets"]]
    if jobs:
        compile_ += ["--parallel", str(jobs)]
    log("building %s" % ", ".join(cross["build"]["targets"]))
    subprocess.run(compile_, check=True, env=env, stdout=subprocess.DEVNULL)
    (base / "bin").mkdir(parents=True, exist_ok=True)
    for output, built in cross["build"]["outputs"].items():
        shutil.copy2(build_dir / built, base / output)
    problem = cross_identity_problem(str(base / "bin" / cross["executable"]), pin)
    if problem:
        raise ToolchainError("the pinned SPIRV-Cross build is not the pin: %s" % problem)
    manifest = {
        "schema": "shader-toolchain-build/v1",
        "stamp": stamp,
        "recipe": CROSS_RECIPE,
        "pin": str(PIN.relative_to(ROOT)) + "#cross_compiler",
        "component": {"commit": cross["component"]["commit"],
                      "sha256": cross["component"]["sha256"]},
        "identity": cross["identity"],
        "host": {"c++": first_line(["c++", "--version"]),
                 "cmake": first_line(["cmake", "--version"]),
                 "ninja": first_line(["ninja", "--version"])},
        "outputs": {output: sha256(base / output) for output in cross["build"]["outputs"]},
    }
    manifest_path.write_text(json.dumps(manifest, indent=1) + "\n")
    log("built %s" % ", ".join(cross["build"]["outputs"]))
    return 0


def command_build(args):
    pin = load_pin()
    status = build(pin, args.jobs, args.fetch_only)
    if status == 0 and "cross_compiler" in pin:
        status = build_cross(pin, args.jobs, args.fetch_only)
    return status


def command_build_cross(args):
    return build_cross(load_pin(), args.jobs)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    build_parser = commands.add_parser("build", help="build the pinned compiler")
    build_parser.add_argument("--jobs", type=int)
    build_parser.add_argument("--fetch-only", action="store_true",
                              help="only download and verify the archives")
    build_parser.set_defaults(run=command_build)
    cross_parser = commands.add_parser("build-cross", help="build only the pinned SPIRV-Cross")
    cross_parser.add_argument("--jobs", type=int)
    cross_parser.set_defaults(run=command_build_cross)
    commands.add_parser("identity", help="print the resolved compiler and its pin status") \
        .set_defaults(run=command_identity)
    check_parser = commands.add_parser("check", help="the shader.toolchain-pin gate")
    check_parser.add_argument("--out", help="evidence and scratch directory")
    check_parser.add_argument("--seed-fault", choices=("foreign-compiler", "flip-byte"))
    check_parser.set_defaults(run=command_check)
    sensitivity_parser = commands.add_parser("sensitivity",
                                             help="the check against its seeded faults")
    sensitivity_parser.add_argument("--out", help="scratch directory")
    sensitivity_parser.set_defaults(run=command_sensitivity)
    args = parser.parse_args(argv)
    try:
        return args.run(args)
    except (ToolchainError, subprocess.CalledProcessError) as error:
        log(str(error))
        return 2


if __name__ == "__main__":
    sys.exit(main())
