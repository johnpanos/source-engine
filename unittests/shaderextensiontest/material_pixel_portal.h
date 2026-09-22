//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "portal" family of the material pixel conformance harness
//          (stencil portal recursion with the real Portal materials).
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_PORTAL_H
#define MATERIAL_PIXEL_PORTAL_H

#include <cstdio>

// Renders the portal cases, writing the report to `out` and each case's frame
// to "<outPath>.<case>.rgb" (tightly packed 8-bit RGB, top row first).
// `writeClearProbe` writes the harness's shared renderer/readback fields.
bool RunPortalCases( FILE *out, const char *outPath, void ( *writeClearProbe )( FILE * ) );

#endif // MATERIAL_PIXEL_PORTAL_H
