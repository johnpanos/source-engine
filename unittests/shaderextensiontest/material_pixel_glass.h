//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "glass" family of the material pixel conformance harness
//          (window and model glass must show the scene behind it).
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_GLASS_H
#define MATERIAL_PIXEL_GLASS_H

#include <cstdio>

// Renders the glass cases, writing the report to `out`. `writeClearProbe`
// writes the harness's shared renderer/readback fields.
bool RunGlassCases( FILE *out, void ( *writeClearProbe )( FILE * ) );

#endif // MATERIAL_PIXEL_GLASS_H
