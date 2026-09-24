//========= Portal 2 port ======================================================//
//
// Purpose: Adapters from the CS:GO-era engine API that the Portal 2 client and
//          server share (paint maps, IEngineTrace brush and trace-list queries,
//          IPhysicsCollision helpers) to this engine's frozen interfaces.
//
// Each adapter either implements the CS:GO behavior with this engine's API or
// returns the documented unsupported result and reports it once with
// DevWarning( "Portal 2: <feature> is not supported by this engine\n" ). The
// Portal 2 call sites use these functions instead of the missing interface
// members, so the difference from retail Portal 2 stays explicit.
//
//=============================================================================//

#ifndef PORTAL2_SHARED_COMPAT_H
#define PORTAL2_SHARED_COMPAT_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "tier1/utlvector.h"
#include "cmodel.h"

struct model_t;
struct Ray_t;
class CGameTrace;
typedef CGameTrace trace_t;
class ICollideable;
class ITraceFilter;
class CPhysCollide;
struct virtualmeshlist_t;

//-----------------------------------------------------------------------------
// Paint maps. This engine's BSP loader and renderer have no paint map data, so
// every map behaves as a map without a paint map: nothing can be painted and
// every surface reports no paint.
//-----------------------------------------------------------------------------

// Replaces engine->HasPaintmap(). Always false.
bool Portal2_HasPaintmap();

// Replaces engine->SpherePaintSurface(). Paints nothing and returns false.
bool Portal2_SpherePaintSurface( const model_t *pModel, const Vector &vPosition, unsigned char color,
								 float flSphereRadius, float flPaintCoatPercent );

// Replaces engine->SphereTracePaintSurface(). Leaves surfColor empty (no paint found).
void Portal2_SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition, const Vector &vContactNormal,
									  float flSphereRadius, CUtlVector<unsigned char> &surfColor );

// Replaces engine->RemovePaint( pModel ). No-op: there is no paint to remove.
void Portal2_RemovePaint( const model_t *pModel );

//-----------------------------------------------------------------------------
// IEngineTrace brush queries (CS:GO IEngineTrace.h types).
//-----------------------------------------------------------------------------
struct BrushSideInfo_t
{
	cplane_t plane;			// The plane of the brush side
	unsigned short bevel;	// Bevel plane?
	unsigned short thin;	// Thin?
};

// Results of a brush query. Owns its storage (the CS:GO version borrowed
// engine memory and released it through a callback).
class CBrushQuery
{
public:
	CBrushQuery( void ) : m_iMaxBrushSides( 0 ) {}
	~CBrushQuery( void ) { ReleasePrivateData(); }

	void ReleasePrivateData( void )
	{
		m_Brushes.Purge();
		m_iMaxBrushSides = 0;
	}

	inline int Count( void ) const { return m_Brushes.Count(); }
	inline uint32 *Base( void ) { return m_Brushes.Base(); }
	inline uint32 operator[]( int iIndex ) const { return m_Brushes[iIndex]; }
	inline uint32 GetBrushNumber( int iIndex ) const { return m_Brushes[iIndex]; }

	// maximum number of sides of any 1 brush in the query results
	inline int MaxBrushSides( void ) const { return m_iMaxBrushSides; }

private:
	friend void Portal2_GetBrushesInAABB( const Vector &, const Vector &, CBrushQuery &, int, int );

	CUtlVector<uint32> m_Brushes;
	int m_iMaxBrushSides;

	CBrushQuery( const CBrushQuery & );
	CBrushQuery &operator=( const CBrushQuery & );
};

// Replaces enginetrace->GetBrushesInAABB( ..., CBrushQuery &, mask, cmodelIndex ).
// World brushes (cmodelIndex 0) come from this engine's query. Brushes of other
// brush models are not supported: the query is left empty for them.
void Portal2_GetBrushesInAABB( const Vector &vMins, const Vector &vMaxs, CBrushQuery &BrushQuery,
							   int iContentsMask = 0xFFFFFFFF, int cmodelIndex = 0 );

// Replaces enginetrace->GetBrushesInCollideable(). This engine cannot enumerate
// the brushes of a brush entity's model, so the query is left empty.
void Portal2_GetBrushesInCollideable( ICollideable *pCollideable, CBrushQuery &BrushQuery );

// Replaces enginetrace->GetBrushInfo( iBrush, contents, sides, size ) with the
// CS:GO contract: returns 0 if the brush does not exist, the number of sides
// written if the array holds them all, or the negative number of slots needed.
// This engine does not mark bevel or thin sides; they are reported as regular
// sides (a bevel plane touches but never cuts a convex brush).
int Portal2_GetBrushInfo( int iBrush, int &ContentsOut, BrushSideInfo_t *pBrushSideInfoOut,
						  int iBrushSideInfoArraySize );

// Replaces enginetrace->GetMeshesFromDisplacementsInAABB(). This engine only
// exposes displacements as a combined collideable
// (GetCollidableFromDisplacementsInAABB), not as meshes, so this returns 0.
int Portal2_GetMeshesFromDisplacementsInAABB( const Vector &vMins, const Vector &vMaxs,
											  virtualmeshlist_t *pOutputMeshes, int iMaxOutputMeshes );

//-----------------------------------------------------------------------------
// Trace list data (CS:GO ITraceListData). Wraps this engine's CTraceListData
// and remembers the volume it was built for, so CanTraceRay() can tell whether
// a ray may use the culled lists.
//-----------------------------------------------------------------------------
class ITraceListData
{
public:
	virtual ~ITraceListData() {}

	virtual void Reset() = 0;
	virtual bool IsEmpty() = 0;
	// CanTraceRay will return true if the current volume encloses the ray
	// NOTE: The leaflist trace will NOT check this.  Traces are intersected
	// against the culled volume exclusively.
	virtual bool CanTraceRay( const Ray_t &ray ) = 0;
};

// Replace enginetrace->AllocTraceListData() / FreeTraceListData().
ITraceListData *Portal2_AllocTraceListData();
void Portal2_FreeTraceListData( ITraceListData *pTraceListData );

// Replace the ITraceListData overloads of enginetrace->SetupLeafAndEntityListBox()
// and TraceRayAgainstLeafAndEntityList().
void Portal2_SetupLeafAndEntityListBox( const Vector &vecBoxMin, const Vector &vecBoxMax, ITraceListData *pTraceData );
void Portal2_TraceRayAgainstLeafAndEntityList( const Ray_t &ray, ITraceListData *pTraceData, unsigned int fMask,
											   ITraceFilter *pTraceFilter, trace_t *pTrace );

//-----------------------------------------------------------------------------
// IPhysicsCollision helpers.
//-----------------------------------------------------------------------------

// Replaces physcollision->TraceBoxAA(): an axis-aligned box sweep against a
// collide at the origin, implemented with TraceBox. Returns true when the trace
// hit something (started solid or stopped short of the end).
bool Portal2_TraceBoxAA( const Ray_t &ray, const CPhysCollide *pCollide, trace_t *ptr );

// Replaces physcollision->CollideGetRadius(): the radius of a sphere around the
// collide's origin that encloses it, from the collide's local bounding box.
float Portal2_CollideGetRadius( const CPhysCollide *pCollide );

#endif // PORTAL2_SHARED_COMPAT_H
