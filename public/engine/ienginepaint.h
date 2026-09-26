//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's paint map service (Portal 2 gel on world surfaces).
//          The CS:GO-era engine exposed these as IVEngineClient and
//          IVEngineServer members. Those interfaces are frozen here, so the
//          engine exposes the same service as its own interface; the Portal 2
//          client and server connect to it (portal2_shared_compat.cpp).
//
// A paint map holds one byte per lightmap luxel of the map's lightmapped
// brush surfaces: a paint power in the top three bits (paint_enum.h, NO_POWER
// for bare) and a coverage alpha in the low five. It exists only for maps
// whose worldspawn sets "paintinmap" 1, from the moment the client has laid
// out the map's lightmap pages until the map unloads; before and after that
// every call reports no paint. Server and client of one process share it.
//
//=============================================================================//

#ifndef IENGINEPAINT_H
#define IENGINEPAINT_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier1/utlvector.h"
#include "mathlib/vector.h"

struct model_t;

// The paint power values the engine stores are Portal 2's PaintPowerType
// (game/shared/portal2/paint_enum.h): 0..3 are powers, and this one, its
// NO_POWER, is bare (or erased) paint. The game asserts that they match.
enum
{
	ENGINE_PAINT_POWER_COUNT = 4,
	ENGINE_PAINT_NO_POWER = ENGINE_PAINT_POWER_COUNT,
};

abstract_class IEnginePaint
{
public:
	// True while the current map has a paint map.
	virtual bool HasPaintmap( void ) = 0;

	// Paints (or, with NO_POWER, erases) the brush model's surfaces that a
	// sphere at vPosition (model space) touches. flPaintCoatPercent scales
	// how much coverage one call adds. Returns true when the paint power of
	// any luxel changed.
	virtual bool SpherePaintSurface( const model_t *pModel, const Vector &vPosition, unsigned char color,
									 float flSphereRadius, float flPaintCoatPercent ) = 0;

	// Appends the paint power of every well-covered luxel of the painted
	// surfaces, facing vContactNormal, that a sphere at vPosition touches.
	// surfColors is emptied first; it stays empty where there is no paint.
	virtual void SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition,
										  const Vector &vContactNormal, float flSphereRadius,
										  CUtlVector<unsigned char> &surfColors ) = 0;

	virtual void RemoveAllPaint( void ) = 0;
	virtual void PaintAllSurfaces( unsigned char color ) = 0;
	virtual void RemovePaint( const model_t *pModel ) = 0;

	// The painted surfaces as run-length encoded records (surface index, run
	// count, runs), for save games and joining clients; valid only for the
	// map they were read from.
	virtual void GetPaintmapDataRLE( CUtlVector<uint32> &data ) = 0;
	virtual void LoadPaintmapDataRLE( const CUtlVector<uint32> &data ) = 0;
};

#define VENGINE_PAINT_INTERFACE_VERSION "VEnginePaint001"

#endif // IENGINEPAINT_H
