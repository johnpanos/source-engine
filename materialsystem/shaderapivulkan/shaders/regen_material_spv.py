#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Regenerates material_spv.h: the embedded SPIR-V of the native material
shaders that are compiled with options (demo_triangle_spv.h holds the rest).

    python3 materialsystem/shaderapivulkan/shaders/regen_material_spv.py

Needs glslc on PATH. The output is written in the layout clang-format keeps.
"""

import pathlib
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
OUTPUT = HERE.parent / "material_spv.h"
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


def compile_words(source, extra):
    with tempfile.TemporaryDirectory() as tmp:
        out = pathlib.Path(tmp) / "shader.spv"
        subprocess.run(["glslc", "-O", *extra, str(HERE / source), "-o", str(out)], check=True)
        data = out.read_bytes()
    return [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]


def main():
    subprocess.run([sys.executable, str(HERE / "gen_pbr_split_sum.py"), "--check"],
                   check=True)
    parts = [HEADER]
    for name, source, extra in SHADERS:
        words = compile_words(source, extra)
        parts.append("\n// %s%s\n" % (source, (" " + " ".join(extra)) if extra else ""))
        parts.append("static const uint32_t %s[] = {\n" % name)
        parts.extend("    0x%08xu,\n" % word for word in words)
        parts.append("};\n")
    parts.append("\n#endif // SHADERAPIVULKAN_MATERIAL_SPV_H\n")
    OUTPUT.write_text("".join(parts))


if __name__ == "__main__":
    main()
