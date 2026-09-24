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
#include "engine/IEngineTrace.h"

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

// The surface flag Portal 2 tests for "no paint" (bspflags.h in the Portal 2
// engine), and the CEG constant accessor its paint code reads it through.
#ifndef SURF_NOPAINT
#define SURF_NOPAINT SURF_NODECALS
#endif
inline unsigned short SurfNoPaintFlag() { return SURF_NOPAINT; }

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


//-----------------------------------------------------------------------------
// UTIL_FindClosestPassableSpace with trace adapters (later Portal 2 base
// game util_shared.h). The 2010 retail build took a trace filter and mask; the
// retained Portal 2 source routes every trace and world test through the
// adapter so portal-aware callers can substitute UTIL_Portal_TraceRay. The
// search is the 2010 algorithm (also Portal 1's FindClosestPassableSpace):
// grow a box from the center, score the eight corners by how far each can
// see the others, and move toward the free corners.
//-----------------------------------------------------------------------------
struct FindClosestPassableSpace_TraceAdapter_t;
typedef void ( *FN_RayTraceAdapterFunc )( const Ray_t &ray, trace_t *pResult, FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter );
typedef bool ( *FN_PointIsOutsideWorld )( const Vector &vTest, FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter );

struct FindClosestPassableSpace_TraceAdapter_t
{
	FN_RayTraceAdapterFunc pTraceFunc;
	FN_PointIsOutsideWorld pPointOutsideWorldFunc;
	ITraceFilter *pTraceFilter;
	unsigned int fMask;
};

// Axis restriction flags: a set bit forbids moving the center along that
// signed axis direction.
enum
{
	FL_AXIS_DIRECTION_NONE = 0,
	FL_AXIS_DIRECTION_X = ( 1 << 0 ),
	FL_AXIS_DIRECTION_NX = ( 1 << 1 ),
	FL_AXIS_DIRECTION_Y = ( 1 << 2 ),
	FL_AXIS_DIRECTION_NY = ( 1 << 3 ),
	FL_AXIS_DIRECTION_Z = ( 1 << 4 ),
	FL_AXIS_DIRECTION_NZ = ( 1 << 5 ),
};

// Finds the closest space where a box of half-size vExtents centered near
// vCenter is not solid according to the adapter. Returns false after
// iIterations failed attempts (vCenterOut then holds the last candidate).
bool UTIL_FindClosestPassableSpace( const Vector &vCenter, const Vector &vExtents, const Vector &vIndecisivePush,
									unsigned int iIterations, Vector &vCenterOut, int nAxisRestrictionFlags,
									FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter );

// The same search with enginetrace->TraceRay() and PointOutsideWorld().
bool UTIL_FindClosestPassableSpace( const Vector &vCenter, const Vector &vExtents, const Vector &vIndecisivePush,
									ITraceFilter *pTraceFilter, unsigned int fMask, unsigned int iIterations,
									Vector &vCenterOut, int nAxisRestrictionFlags = FL_AXIS_DIRECTION_NONE );

//-----------------------------------------------------------------------------
// Engine extensions Portal 2 requires that the frozen engine interfaces lack:
// the single-player shared memory registry (sharedvar.h) and the game time
// scale. A module that cannot connect both fails its init.
//-----------------------------------------------------------------------------
typedef void *( *CreateInterfaceFn )( const char *pName, int *pReturnCode );
bool Portal2_ConnectEngineInterfaces( CreateInterfaceFn engineFactory );

#ifdef CLIENT_DLL
// Borrows the matchmaking framework the server module published through the
// engine's IMatchFrameworkHost and sets g_pMatchFramework. The engine
// initializes the server module before its GameUI and the client, so the
// GameUI (CGameUI::Initialize) and the client's Init both connect here; a
// module that cannot connect fails its init. Idempotent.
bool Portal2_ConnectMatchFramework( CreateInterfaceFn engineFactory );
// Stops borrowing the framework (client shutdown; the server outlives it).
void Portal2_DisconnectMatchFramework();
// Once per client frame: reports signon changes to the framework and runs it.
void Portal2_MatchFrameworkClientFrame();
#endif

// Replace engine->GetTimescale()/SetTimescale() (IEngineGameTimescale). The
// scale is engine-wide: client and server of one process share it.
float Portal2_GetTimescale();
void Portal2_SetTimescale( float flTimescale );

#ifdef GAME_DLL
// Replaces physenv->DestroyCollideOnDeadObjectFlush(). Objects destroyed during
// a simulation step stay queued in this engine until the step ends, and may
// still reference the collide, so a collide released while the environment is
// simulating is destroyed after the step (at the next server frame). Outside a
// simulation step it is destroyed immediately.
void Portal2_DestroyCollideOnDeadObjectFlush( CPhysCollide *pCollide );
#endif

#endif // PORTAL2_SHARED_COMPAT_H
