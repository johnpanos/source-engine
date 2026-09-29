//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Textures the engine decodes on the CPU to sample their color
//          (vtf/vtf_sample.h): projected lights' cookies and the emission of
//          self-illuminated world faces and overlays. Cached per map by name;
//          thread-safe (lightmap builds may run on the material system's
//          thread).
//
//===========================================================================//

#ifndef ENGINE_SAMPLE_TEXTURES_H
#define ENGINE_SAMPLE_TEXTURES_H

#include "vtf/vtf_sample.h"

// The texture `materials/<name>.vtf` at its first mip no larger than 256, or
// null when it cannot be read. The pointer stays valid until the map changes.
const vtf_sample::Texture *EngineSampleTexture( const char *pName );

#endif // ENGINE_SAMPLE_TEXTURES_H
