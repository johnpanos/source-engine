//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The "softparticle" family of the material pixel conformance harness
//          (SpriteCard's DEPTHBLEND against the engine's full-frame depth copy).
//
//=============================================================================//

#ifndef MATERIAL_PIXEL_SOFTPARTICLE_H
#define MATERIAL_PIXEL_SOFTPARTICLE_H

#include <cstdio>

// Renders the soft particle cases, writing the report to `out`.
// `writeClearProbe` writes the harness's shared renderer/readback fields.
bool RunSoftParticleCases( FILE *out, void ( *writeClearProbe )( FILE * ) );

#endif // MATERIAL_PIXEL_SOFTPARTICLE_H
