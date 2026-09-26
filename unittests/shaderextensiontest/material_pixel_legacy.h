//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "legacy" family of the material pixel conformance harness: the
//          native Vulkan backend's legacy shader ports (vulkan_legacy_programs.h)
//          drawn through the real material system from a case file.
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_LEGACY_H
#define MATERIAL_PIXEL_LEGACY_H

#include <cstdio>

// Renders every case of the KeyValues case file `casesPath`, writing the report
// to `out`. `writeClearProbe` writes the harness's shared renderer/readback
// fields. The backend records each pass's inputs when the harness runs with
// -vklegacycapture <file>; tools/quality/legacy_shader_oracle.py replays them.
bool RunLegacyCases( FILE *out, const char *casesPath, void ( *writeClearProbe )( FILE * ) );

#endif // MATERIAL_PIXEL_LEGACY_H
