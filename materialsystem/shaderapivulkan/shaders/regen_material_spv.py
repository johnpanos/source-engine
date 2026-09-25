#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Regenerates material_spv.h: the embedded SPIR-V of the native material
shaders that are compiled with options (demo_triangle_spv.h holds the rest),
and material_spv_index.h: each array's content hash and source, which names
the shader modules the backend creates (vulkan_shader_library.cpp).

    python3 materialsystem/shaderapivulkan/shaders/regen_material_spv.py
    python3 .../regen_material_spv.py --check-index
    python3 .../regen_material_spv.py --debug-out DIR

Needs glslc on PATH. The output is written in the layout clang-format keeps.
--check-index verifies the index against material_spv.h (no compiler).
--debug-out writes, for every array still built from the current GLSL, a
named variant with source-level debug information (glslangValidator -gVS:
NonSemantic.Shader.DebugInfo.100, unoptimized) as DIR/<array>.<hash>.spv,
keyed by the embedded array's hash; `-vkshaderdir DIR` makes the backend use
them (RenderDoc then shows GLSL names and steps source). Arrays whose GLSL
changed since material_spv.h was generated are skipped and reported.
"""

import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
OUTPUT = HERE.parent / "material_spv.h"
INDEX_OUTPUT = HERE.parent / "material_spv_index.h"
# (array name, source, extra glslc arguments)
SHADERS = (
    ("g_materialTexVertSpv", "demo_dyn_tex.vert", []),
    ("g_materialTexClipVertSpv", "demo_dyn_tex.vert", ["-DCLIP_PLANES"]),
    ("g_worldMeshVertSpv", "world_mesh.vert", []),
    ("g_worldMeshClipVertSpv", "world_mesh.vert", ["-DCLIP_PLANES"]),
    ("g_worldPbrVertSpv", "world_pbr.vert", []),
    ("g_worldPbrClipVertSpv", "world_pbr.vert", ["-DCLIP_PLANES"]),
    ("g_worldPbrFragSpv", "world_pbr.frag", []),
    ("g_worldPbrClipFragSpv", "world_pbr.frag", ["-DCLIP_PLANES"]),
    ("g_worldPbrIndirectFragSpv", "world_pbr.frag", ["-DINDIRECT_VIEW"]),
    # RFC 0011 G2: the frame's unbaked lights (set 7).
    ("g_worldPbrLightFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS"]),
    ("g_worldPbrLightClipFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS", "-DCLIP_PLANES"]),
    # RFC 0011 G2: the RuntimeIndirect policy (set 8), with and without lights.
    ("g_worldPbrRuntimeFragSpv", "world_pbr.frag", ["-DRUNTIME_INDIRECT"]),
    ("g_worldPbrRuntimeClipFragSpv", "world_pbr.frag", ["-DRUNTIME_INDIRECT", "-DCLIP_PLANES"]),
    ("g_worldPbrLightRuntimeFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS",
                                                          "-DRUNTIME_INDIRECT"]),
    ("g_worldPbrLightRuntimeClipFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS",
                                                              "-DRUNTIME_INDIRECT",
                                                              "-DCLIP_PLANES"]),
    ("g_worldPbrIndirectClipFragSpv", "world_pbr.frag", ["-DINDIRECT_VIEW", "-DCLIP_PLANES"]),
    # RFC 0011 G4: BakedPlusDelta, the producer's change volume (sets 8 and 9),
    # with and without lights, and in the indirect-light view.
    ("g_worldPbrDeltaFragSpv", "world_pbr.frag", ["-DDELTA_VOLUME"]),
    ("g_worldPbrDeltaClipFragSpv", "world_pbr.frag", ["-DDELTA_VOLUME", "-DCLIP_PLANES"]),
    ("g_worldPbrLightDeltaFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS", "-DDELTA_VOLUME"]),
    ("g_worldPbrLightDeltaClipFragSpv", "world_pbr.frag", ["-DDIRECT_LIGHTS", "-DDELTA_VOLUME",
                                                            "-DCLIP_PLANES"]),
    ("g_worldPbrIndirectDeltaFragSpv", "world_pbr.frag", ["-DINDIRECT_VIEW", "-DDELTA_VOLUME"]),
    ("g_worldPbrIndirectDeltaClipFragSpv", "world_pbr.frag", ["-DINDIRECT_VIEW",
                                                               "-DDELTA_VOLUME",
                                                               "-DCLIP_PLANES"]),
    # RFC 0011 G5: the compute conformance suite's program.
    ("g_computeFillSpv", "compute_fill.comp", []),
    # RFC 0011 G6: the SDF-traced producer's probe update.
    ("g_sdfProbeTraceSpv", "sdf_probe_trace.comp", []),
    # R50-PARALLAX: the reflection-probe GLSL suite's program.
    ("g_reflectionProbesCheckSpv", "reflection_probes_check.comp", []),
    # RFC 0011 G7: the same update traced with ray queries (SPIR-V 1.5).
    ("g_rayQueryProbeTraceSpv", "sdf_probe_trace.comp", ["--target-env=vulkan1.2",
                                                         "-DRAY_QUERY"]),
    ("g_worldGlassFragSpv", "world_pbr_glass.frag", []),
    ("g_worldGlassClipFragSpv", "world_pbr_glass.frag", ["-DCLIP_PLANES"]),
    ("g_materialTexFragSpv", "demo_dyn_tex.frag", []),
    ("g_portalRefractVertSpv", "portal_refract.vert", []),
    ("g_portalRefractFragSpv", "portal_refract.frag", []),
    ("g_skinVertSpv", "skin.vert", []),
    ("g_skinFragSpv", "skin.frag", []),
    # LightmappedGeneric / WorldVertexTransition (lightmappedgeneric_vs20 / _ps2_3_x).
    ("g_lightmappedVertSpv", "lightmapped.vert", []),
    ("g_lightmappedFragSpv", "lightmapped.frag", []),
    # The bloom and color-correction passes (Downsample_nohdr, BlurFilter, Engine_Post).
    ("g_postVertSpv", "screenspace_post.vert", []),
    ("g_postFragSpv", "screenspace_post.frag", []),
    ("g_solidEnergyVertSpv", "solidenergy.vert", []),
    ("g_solidEnergyFragSpv", "solidenergy.frag", []),
    # Portal 2 paint blobs (paintblob_ps20b), after skin.vert.
    ("g_paintBlobFragSpv", "paintblob.frag", []),
    ("g_modelPbrFragSpv", "model_pbr.frag", []),
    ("g_modelPbrEnvFragSpv", "model_pbr.frag", ["-DENV_CUBE"]),
    ("g_modelPbrIndirectFragSpv", "model_pbr.frag", ["-DINDIRECT_VIEW"]),
    # RFC 0011: the map's probe volume sampled per pixel (sets 7 and 8).
    ("g_modelPbrProbeFragSpv", "model_pbr.frag", ["-DPROBE_VOLUME"]),
    ("g_modelPbrEnvProbeFragSpv", "model_pbr.frag", ["-DENV_CUBE", "-DPROBE_VOLUME"]),
    ("g_modelPbrIndirectProbeFragSpv", "model_pbr.frag", ["-DINDIRECT_VIEW", "-DPROBE_VOLUME"]),
    ("g_pbrDirectFragSpv", "pbr_direct.frag", []),
    ("g_pbrDirectClipFragSpv", "pbr_direct.frag", ["-DCLIP_PLANES"]),
    # render.pbr-brdf.glsl: pbr_brdf.glsl evaluated beside public/render/pbr_brdf.h.
    ("g_pbrBrdfCheckSpv", "pbr_brdf_check.comp", []),
    ("g_presentGammaVertSpv", "present_gamma.vert", []),
    ("g_presentGammaFragSpv", "present_gamma.frag", []),
)

HEADER = """//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Embedded SPIR-V of the native material shaders: the textured stages
//          (with and without D3D9 user clip planes), PortalRefract (a port of
//          portal_refract_vs20.fxc / portal_refract_ps2x.fxc) and VertexLitGeneric's
//          $phong path (skin_vs20.fxc / skin_ps20b.fxc), SolidEnergy
//          (solidenergy_vs20.fxc / solidenergy_ps20b.fxc), paint blobs
//          (paintblob_ps20b.fxc), PBRMetalRough on
//          models (model_pbr.frag, both probe sources), and the RFC 0007
//          synthetic PBR direct-light pixel stage, the WMSH PBR and glass
//          stages, and the present-time monitor gamma pass. GENERATED
//          by shaders/regen_material_spv.py from the GLSL in shaders/; do not edit.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_MATERIAL_SPV_H
#define SHADERAPIVULKAN_MATERIAL_SPV_H

#include <cstdint>
"""


INDEX_HEADER = """//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The content hash (FNV-1a 64 over the bytes) and GLSL source of
//          every array in material_spv.h, so the backend names the shader
//          modules it creates and matches debug variants to them
//          (vulkan_shader_library.cpp). GENERATED by
//          shaders/regen_material_spv.py with material_spv.h; do not edit.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_MATERIAL_SPV_INDEX_H
#define SHADERAPIVULKAN_MATERIAL_SPV_INDEX_H

#include <cstdint>

struct MaterialSpvIndexEntry
{
\tuint64_t hash;
\tconst char *array;
\tconst char *source; // the GLSL file and its compile options
};

"""


def spirv_hash(words):
    """FNV-1a 64 over the little-endian bytes of `words`
    (render_vulkan::SpirvHash in vulkan_shader_library.cpp)."""
    value = 0xCBF29CE484222325
    for word in words:
        for byte in word.to_bytes(4, "little"):
            value = ((value ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return value


def describe(source, extra):
    return source + ((" " + " ".join(extra)) if extra else "")


def compile_words(source, extra):
    with tempfile.TemporaryDirectory() as tmp:
        out = pathlib.Path(tmp) / "shader.spv"
        subprocess.run(["glslc", "-O", *extra, str(HERE / source), "-o", str(out)], check=True)
        data = out.read_bytes()
    return [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]


def debug_arguments(extra):
    """glslc options as glslangValidator's (glslc's default is Vulkan 1.0)."""
    arguments, environment = [], "vulkan1.0"
    for option in extra:
        if option.startswith("-D"):
            arguments.append(option)
        elif option.startswith("--target-env="):
            environment = option.split("=", 1)[1]
        else:
            raise ValueError("no glslangValidator equivalent for glslc option %s" % option)
    # glslc enables #include implicitly; glslangValidator needs the extension.
    return ["-V", "-gVS", "--target-env", environment,
            "-P#extension GL_GOOGLE_include_directive : require"] + arguments


def compile_debug(source, extra):
    with tempfile.TemporaryDirectory() as tmp:
        out = pathlib.Path(tmp) / "shader.spv"
        result = subprocess.run(["glslangValidator", *debug_arguments(extra), str(HERE / source),
                                 "-o", str(out)], capture_output=True, text=True)
        if result.returncode != 0:
            raise RuntimeError("glslangValidator %s failed:\n%s" % (
                describe(source, extra), result.stdout + result.stderr))
        return out.read_bytes()


def parallel(function, items):
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        return list(pool.map(lambda item: function(*item), items))


def embedded_arrays(path=OUTPUT):
    """{array name: words} as material_spv.h holds them."""
    arrays, name = {}, None
    for line in pathlib.Path(path).read_text().splitlines():
        match = re.match(r"static const uint32_t (\w+)\[\] = \{$", line)
        if match:
            name = match.group(1)
            arrays[name] = []
        elif name and line == "};":
            name = None
        elif name:
            arrays[name].append(int(line.strip().rstrip(",").rstrip("u"), 16))
    return arrays


# .clang-format's column limit; the index is written in the layout it keeps.
COLUMNS = 100


def macro_line(text):
    """`text` with its continuation backslash at the last column (tabs are 4)."""
    width = len(text.expandtabs(4))
    return text + " " * (COLUMNS - 1 - width) + "\\\n"


def index_text(arrays):
    parts = [INDEX_HEADER, "static const MaterialSpvIndexEntry g_materialSpvIndex[] = {\n"]
    for name, source, extra in SHADERS:
        head = "    { 0x%016xull, \"%s\"," % (spirv_hash(arrays[name]), name)
        tail = "\"%s\" }," % describe(source, extra)
        if len(head) + 1 + len(tail) <= COLUMNS:
            parts.append(head + " " + tail + "\n")
        else:
            parts.append(head + "\n        " + tail + "\n")
    parts.append("};\n\n")
    parts.append("// X( array ) for every array, for suites that include material_spv.h.\n")
    parts.append(macro_line("#define MATERIAL_SPV_ARRAYS( X )"))
    names = [name for name, _, _ in SHADERS]
    parts.extend(macro_line("\tX( %s )" % name) for name in names[:-1])
    parts.append("\tX( %s )\n" % names[-1])
    parts.append("\n#endif // SHADERAPIVULKAN_MATERIAL_SPV_INDEX_H\n")
    return "".join(parts)


def regenerate():
    subprocess.run([sys.executable, str(HERE / "gen_pbr_split_sum.py"), "--check"],
                   check=True)
    compiled = parallel(compile_words, [(source, extra) for _, source, extra in SHADERS])
    parts = [HEADER]
    for (name, source, extra), words in zip(SHADERS, compiled):
        parts.append("\n// %s\n" % describe(source, extra))
        parts.append("static const uint32_t %s[] = {\n" % name)
        parts.extend("    0x%08xu,\n" % word for word in words)
        parts.append("};\n")
    parts.append("\n#endif // SHADERAPIVULKAN_MATERIAL_SPV_H\n")
    OUTPUT.write_text("".join(parts))
    INDEX_OUTPUT.write_text(index_text(dict(zip((n for n, _, _ in SHADERS), compiled))))


def check_index():
    arrays = embedded_arrays()
    missing = [name for name, _, _ in SHADERS if name not in arrays]
    if missing:
        print("material_spv.h lacks %s; regenerate it" % ", ".join(missing), file=sys.stderr)
        return 1
    if INDEX_OUTPUT.read_text() != index_text(arrays):
        print("%s does not match material_spv.h; regenerate both" % INDEX_OUTPUT.name,
              file=sys.stderr)
        return 1
    return 0


DEBUG_FILE = re.compile(r"^\w+\.[0-9a-f]{16}\.spv$")


def write_debug(directory):
    """Debug variants of the embedded arrays still built from the current GLSL;
    returns the manifest written beside them."""
    directory = pathlib.Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    for old in directory.iterdir():
        if DEBUG_FILE.match(old.name):
            old.unlink()
    arrays = embedded_arrays()
    jobs = [(source, extra) for _, source, extra in SHADERS]
    current = parallel(compile_words, jobs)
    debug = parallel(compile_debug, jobs)
    entries = []
    for (name, source, extra), words, spirv in zip(SHADERS, current, debug):
        entry = {"array": name, "source": describe(source, extra)}
        if arrays.get(name) != words:
            entry["status"] = "stale: material_spv.h was not regenerated after its GLSL changed"
        else:
            entry["hash"] = "%016x" % spirv_hash(words)
            entry["file"] = "%s.%s.spv" % (name, entry["hash"])
            entry["status"] = "written"
            (directory / entry["file"]).write_bytes(spirv)
        entries.append(entry)
    manifest = {"schema": "vulkan-shader-debug/v1", "entries": entries}
    (directory / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
    return manifest


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check-index", action="store_true",
                      help="verify material_spv_index.h against material_spv.h")
    mode.add_argument("--debug-out", type=pathlib.Path,
                      help="write the named debug variants to this directory")
    args = parser.parse_args(argv)
    if args.check_index:
        return check_index()
    if args.debug_out:
        manifest = write_debug(args.debug_out)
        stale = [e for e in manifest["entries"] if e["status"] != "written"]
        print("%d debug shaders in %s" % (len(manifest["entries"]) - len(stale), args.debug_out))
        for entry in stale:
            print("skipped %s (%s): %s" % (entry["array"], entry["source"], entry["status"]),
                  file=sys.stderr)
        return 0
    regenerate()
    return 0


if __name__ == "__main__":
    sys.exit(main())
