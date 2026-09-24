//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "modellight" family of the material pixel conformance harness
//          (VertexLitGeneric model lighting: ambient cube, local lights and
//          static vertex lighting).
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_MODELLIGHT_H
#define MATERIAL_PIXEL_MODELLIGHT_H

#include <cstdio>

// Renders the model lighting cases, writing the report to `out` and each case's
// frame to "<outPath>.<case>.rgb" (tightly packed 8-bit RGB, top row first).
// `writeClearProbe` writes the harness's shared renderer/readback fields.
// `toneScale` is the linear tone-mapping scale set for every case (integer HDR).
bool RunModelLightCases(
    FILE *out, const char *outPath, void ( *writeClearProbe )( FILE * ), float toneScale );
// The "pbr-model" family: the same quads, lights and placements drawn with
// RFC 0007 PBRMetalRough materials (native Vulkan's model pipeline).
bool RunPbrModelCases(
    FILE *out, const char *outPath, void ( *writeClearProbe )( FILE * ), float toneScale );

#endif // MATERIAL_PIXEL_MODELLIGHT_H
