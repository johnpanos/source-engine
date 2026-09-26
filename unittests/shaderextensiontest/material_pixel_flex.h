//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "flex" family of the material pixel conformance harness
//          (studiorender's delta-flexed faces: flex position deltas and
//          wrinkle maps through IMesh::SetFlexMesh).
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_FLEX_H
#define MATERIAL_PIXEL_FLEX_H

#include <cstdio>

// Renders the flex cases, writing the report to `out`. `writeClearProbe`
// writes the harness's shared renderer/readback fields.
bool RunFlexCases( FILE *out, void ( *writeClearProbe )( FILE * ) );

#endif // MATERIAL_PIXEL_FLEX_H
