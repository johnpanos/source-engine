//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy VPhysics collision-model decoder (RFC 0004 B3).
//
// Reads the serialized collide format that studiomdl/vbsp wrote through IVP
// (an optional "VPHY" header followed by an IVP compact surface: a ledge tree
// whose terminal ledges are the convex pieces) and produces plain convex
// geometry in Source units. It parses bytes with explicit bounds checks and
// includes no IVP headers, so the Box3D provider carries no IVP code.
//
//=============================================================================//
#ifndef LEGACY_COLLISION_H
#define LEGACY_COLLISION_H

#include "mathlib/vector.h"
#include "utlvector.h"

struct LegacyConvex_t
{
	CUtlVector<Vector> points;		// unique vertices, Source units, collide space
	CUtlVector<unsigned short> triangles;	// 3 indices per triangle into points
	CUtlVector<unsigned char> triangleMaterials;	// 7-bit material per triangle
	unsigned int gameData;			// ledge client data (convex game data)
};

struct LegacyCollide_t
{
	CUtlVector<LegacyConvex_t> convexes;
	Vector massCenter;
	Vector orthoAreas;
};

// Decodes one serialized solid. Returns false (and leaves pOut empty) on an
// unknown, byte-swapped, or malformed buffer; never reads outside the buffer.
bool DecodeLegacyCollide( const char *pBuffer, int size, LegacyCollide_t *pOut );

#endif // LEGACY_COLLISION_H
