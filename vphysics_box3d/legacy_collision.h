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
	// IVP's per-unit-mass rotational inertia (Source axes, square meters), as
	// the compact builder stored it.
	Vector rotationInertia;
};

// Decodes one serialized solid. Returns false (and leaves pOut empty) on an
// unknown, byte-swapped, or malformed buffer; never reads outside the buffer.
bool DecodeLegacyCollide( const char *pBuffer, int size, LegacyCollide_t *pOut );

// Encodes a solid in the same format (header, compact surface, ledge tree,
// terminal ledges with edge topology and pierce triangles) so tools and IVP
// can read it back. Triangles are re-oriented to IVP's winding; a single
// triangle becomes a two-sided flat ledge. Writes nothing when pDest is NULL
// and returns the byte size either way; returns 0 if a convex is not a
// closed triangle mesh or exceeds the format's limits. swap writes the
// opposite byte order.
int EncodeLegacyCollide( const LegacyCollide_t &collide, int vcollideIndex, char *pDest, bool swap );

#endif // LEGACY_COLLISION_H
