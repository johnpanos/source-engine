//========= Portal 2 port ======================================================//
//
// Purpose: Implementation of the client/server adapters declared in
//          portal2_shared_compat.h. See that header for each contract.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_shared_compat.h"
#include "engine/IEngineTrace.h"
#include "vphysics_interface.h"
#include "cmodel.h"
#include "igamesystem.h"
#include "collisionutils.h"
#include "mathlib/vmatrix.h"
#include "particle_parse.h"
#include "effect_dispatch_data.h"
#include "itempents.h"
#include "ispsharedmemory.h"
#include "engine/igametimescale.h"
#include "engine/ienginepaint.h"
#include "paint_enum.h"
#include "engine/imatchframeworkhost.h"
#include "matchmaking/imatchframework.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IPhysicsCollision *physcollision;

//-----------------------------------------------------------------------------
// One-time report of a CS:GO-era engine feature this engine does not provide.
//-----------------------------------------------------------------------------
#define PORTAL2_SHARED_UNSUPPORTED( feature ) \
	do \
	{ \
		static bool s_bWarned = false; \
		if ( !s_bWarned ) \
		{ \
			s_bWarned = true; \
			DevWarning( "Portal 2: " feature " is not supported by this engine\n" ); \
		} \
	} while ( 0 )

//-----------------------------------------------------------------------------
// Matchmaking. The server module owns the process's framework
// (game/server/portal2/portal2_matchmaking.cpp) and sets this pointer; the
// client borrows the framework the server publishes through the engine.
//-----------------------------------------------------------------------------
IMatchFramework *g_pMatchFramework = NULL;

#ifdef CLIENT_DLL
bool Portal2_ConnectMatchFramework( CreateInterfaceFn engineFactory )
{
	if ( g_pMatchFramework )
		return true;

	IMatchFrameworkHost *pHost = (IMatchFrameworkHost *)engineFactory( VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION, NULL );
	if ( !pHost )
	{
		Warning( "Portal 2: the engine does not provide %s\n", VENGINE_MATCHFRAMEWORKHOST_INTERFACE_VERSION );
		return false;
	}

	g_pMatchFramework = pHost->GetMatchFramework();
	if ( !g_pMatchFramework )
	{
		Warning( "Portal 2: the server game module has not published a matchmaking framework\n" );
		return false;
	}
	return true;
}

void Portal2_DisconnectMatchFramework()
{
	g_pMatchFramework = NULL;
}

// Signon states the matchmaking framework distinguishes (common/protocol.h).
enum
{
	PORTAL2_SIGNONSTATE_NONE = 0,
	PORTAL2_SIGNONSTATE_CONNECTED = 2,
	PORTAL2_SIGNONSTATE_FULL = 6,
};

void Portal2_MatchFrameworkClientFrame()
{
	if ( !g_pMatchFramework )
		return;

	// The CS:GO engine broadcast OnEngineClientSignonStateChange from
	// CClientState::SetSignonState. This engine does not tell the client
	// module about signon changes, so the states the framework acts on
	// (connected / fully in game / disconnected) are sampled once a frame.
	// Intermediate signon steps and changes that begin and end within one
	// frame are not reported.
	static int s_nSignonState = PORTAL2_SIGNONSTATE_NONE;
	int nSignonState = engine->IsInGame() ? PORTAL2_SIGNONSTATE_FULL :
		( engine->IsConnected() ? PORTAL2_SIGNONSTATE_CONNECTED : PORTAL2_SIGNONSTATE_NONE );
	if ( nSignonState != s_nSignonState )
	{
		KeyValues *pEvent = new KeyValues( "OnEngineClientSignonStateChange" );
		pEvent->SetInt( "slot", 0 );
		pEvent->SetInt( "old", s_nSignonState );
		pEvent->SetInt( "new", nSignonState );
		pEvent->SetInt( "count", 1 );
		s_nSignonState = nSignonState;
		g_pMatchFramework->GetEventsSubscription()->BroadcastEvent( pEvent );
	}

	g_pMatchFramework->RunFrame();
}
#endif

#ifdef GAME_DLL
// The server's view of the client-side Portal2Engine adapters it calls.
namespace Portal2Engine
{
	bool HasPaintmap()
	{
		return Portal2_HasPaintmap();
	}
}
#endif

//-----------------------------------------------------------------------------
// Single-player shared memory
//-----------------------------------------------------------------------------
static ISPSharedMemoryManager *s_pSPSharedMemoryManager = NULL;
static IEngineGameTimescale *s_pEngineGameTimescale = NULL;
static IEnginePaint *s_pEnginePaint = NULL;

// The engine stores paint_enum.h powers; its bare value is NO_POWER.
static_assert( NO_POWER == ENGINE_PAINT_NO_POWER && PAINT_POWER_TYPE_COUNT == ENGINE_PAINT_POWER_COUNT,
			   "the engine paint map and paint_enum.h disagree on the paint powers" );

bool Portal2_ConnectEngineInterfaces( CreateInterfaceFn engineFactory )
{
	// Optional: an engine without paint maps (a dedicated server) reports
	// every map as unpaintable.
	s_pEnginePaint = (IEnginePaint *)engineFactory( VENGINE_PAINT_INTERFACE_VERSION, NULL );
	if ( !s_pEnginePaint )
	{
		DevWarning( "Portal 2: the engine does not provide %s; maps cannot be painted\n", VENGINE_PAINT_INTERFACE_VERSION );
	}

	s_pSPSharedMemoryManager = (ISPSharedMemoryManager *)engineFactory( VENGINE_SPSHAREDMEMORY_INTERFACE_VERSION, NULL );
	if ( !s_pSPSharedMemoryManager )
	{
		Warning( "Portal 2: the engine does not provide %s\n", VENGINE_SPSHAREDMEMORY_INTERFACE_VERSION );
		return false;
	}

	s_pEngineGameTimescale = (IEngineGameTimescale *)engineFactory( VENGINE_GAMETIMESCALE_INTERFACE_VERSION, NULL );
	if ( !s_pEngineGameTimescale )
	{
		Warning( "Portal 2: the engine does not provide %s\n", VENGINE_GAMETIMESCALE_INTERFACE_VERSION );
		return false;
	}
	return true;
}

float Portal2_GetTimescale()
{
	// Connected in the module's init, which fails without it.
	Assert( s_pEngineGameTimescale );
	return s_pEngineGameTimescale->GetTimescale();
}

void Portal2_SetTimescale( float flTimescale )
{
	Assert( s_pEngineGameTimescale );
	s_pEngineGameTimescale->SetTimescale( flTimescale );
}

ISPSharedMemory *Portal2_GetSinglePlayerSharedMemorySpace( const char *szName, int nEntNum )
{
	// Connected in the module's init, which fails without it.
	Assert( s_pSPSharedMemoryManager );
	return s_pSPSharedMemoryManager->GetSharedMemory( szName, nEntNum );
}

//-----------------------------------------------------------------------------
// Paint maps
//-----------------------------------------------------------------------------
bool Portal2_HasPaintmap()
{
	return s_pEnginePaint && s_pEnginePaint->HasPaintmap();
}

bool Portal2_SpherePaintSurface( const model_t *pModel, const Vector &vPosition, unsigned char color,
								 float flSphereRadius, float flPaintCoatPercent )
{
	if ( !s_pEnginePaint )
	{
		PORTAL2_SHARED_UNSUPPORTED( "painting map surfaces" );
		return false;
	}
	return s_pEnginePaint->SpherePaintSurface( pModel, vPosition, color, flSphereRadius, flPaintCoatPercent );
}

void Portal2_SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition, const Vector &vContactNormal,
									  float flSphereRadius, CUtlVector<unsigned char> &surfColor )
{
	surfColor.RemoveAll();
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->SphereTracePaintSurface( pModel, vPosition, vContactNormal, flSphereRadius, surfColor );
	}
}

void Portal2_RemovePaint( const model_t *pModel )
{
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->RemovePaint( pModel );
	}
}

void Portal2_RemoveAllPaint()
{
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->RemoveAllPaint();
	}
}

void Portal2_PaintAllSurfaces( unsigned char color )
{
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->PaintAllSurfaces( color );
	}
}

void Portal2_GetPaintmapDataRLE( CUtlVector<uint32> &data )
{
	data.RemoveAll();
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->GetPaintmapDataRLE( data );
	}
}

void Portal2_LoadPaintmapDataRLE( const CUtlVector<uint32> &data )
{
	if ( s_pEnginePaint )
	{
		s_pEnginePaint->LoadPaintmapDataRLE( data );
	}
}

//-----------------------------------------------------------------------------
// Brush queries
//-----------------------------------------------------------------------------
void Portal2_GetBrushesInAABB( const Vector &vMins, const Vector &vMaxs, CBrushQuery &BrushQuery,
							   int iContentsMask, int cmodelIndex )
{
	BrushQuery.ReleasePrivateData();

	if ( cmodelIndex != 0 )
	{
		PORTAL2_SHARED_UNSUPPORTED( "brush queries against brush entity models" );
		return;
	}

	CUtlVector<int> brushes;
	enginetrace->GetBrushesInAABB( vMins, vMaxs, &brushes, iContentsMask );

	CUtlVector<Vector4D> planes;
	for ( int i = 0; i != brushes.Count(); ++i )
	{
		BrushQuery.m_Brushes.AddToTail( (uint32)brushes[i] );

		int iContents;
		planes.RemoveAll();
		if ( enginetrace->GetBrushInfo( brushes[i], &planes, &iContents ) && planes.Count() > BrushQuery.m_iMaxBrushSides )
			BrushQuery.m_iMaxBrushSides = planes.Count();
	}
}

void Portal2_GetBrushesInCollideable( ICollideable *pCollideable, CBrushQuery &BrushQuery )
{
	BrushQuery.ReleasePrivateData();
	PORTAL2_SHARED_UNSUPPORTED( "brush queries against collideables" );
}

int Portal2_GetBrushInfo( int iBrush, int &ContentsOut, BrushSideInfo_t *pBrushSideInfoOut, int iBrushSideInfoArraySize )
{
	CUtlVector<Vector4D> planes;
	int iContents = 0;
	if ( !enginetrace->GetBrushInfo( iBrush, &planes, &iContents ) )
		return 0;

	ContentsOut = iContents;
	if ( planes.Count() > iBrushSideInfoArraySize || pBrushSideInfoOut == NULL )
		return -planes.Count();

	for ( int i = 0; i != planes.Count(); ++i )
	{
		BrushSideInfo_t &side = pBrushSideInfoOut[i];
		memset( &side, 0, sizeof( side ) );
		side.plane.normal.Init( planes[i].x, planes[i].y, planes[i].z );
		side.plane.dist = planes[i].w;
		side.plane.type = 3; // PLANE_ANYX: the engine does not report axial types
		side.plane.signbits = SignbitsForPlane( &side.plane );
		side.bevel = 0;
		side.thin = 0;
	}
	return planes.Count();
}

int Portal2_GetMeshesFromDisplacementsInAABB( const Vector &vMins, const Vector &vMaxs,
											  virtualmeshlist_t *pOutputMeshes, int iMaxOutputMeshes )
{
	PORTAL2_SHARED_UNSUPPORTED( "displacement mesh queries" );
	return 0;
}

//-----------------------------------------------------------------------------
// Trace list data
//-----------------------------------------------------------------------------
namespace
{
	class CPortal2TraceListData : public ITraceListData
	{
	public:
		CPortal2TraceListData() : m_bHasBox( false ) {}

		virtual void Reset()
		{
			m_Data.Reset();
			m_bHasBox = false;
		}

		virtual bool IsEmpty()
		{
			return m_Data.IsEmpty();
		}

		virtual bool CanTraceRay( const Ray_t &ray )
		{
			if ( !m_bHasBox )
				return false;

			Vector vRayMins, vRayMaxs;
			Vector vEnd = ray.m_Start + ray.m_Delta;
			VectorMin( ray.m_Start, vEnd, vRayMins );
			VectorMax( ray.m_Start, vEnd, vRayMaxs );
			vRayMins -= ray.m_Extents;
			vRayMaxs += ray.m_Extents;

			return vRayMins.x >= m_vBoxMin.x && vRayMins.y >= m_vBoxMin.y && vRayMins.z >= m_vBoxMin.z &&
				   vRayMaxs.x <= m_vBoxMax.x && vRayMaxs.y <= m_vBoxMax.y && vRayMaxs.z <= m_vBoxMax.z;
		}

		CTraceListData m_Data;
		Vector m_vBoxMin;
		Vector m_vBoxMax;
		bool m_bHasBox;
	};
}

ITraceListData *Portal2_AllocTraceListData()
{
	return new CPortal2TraceListData;
}

void Portal2_FreeTraceListData( ITraceListData *pTraceListData )
{
	delete pTraceListData;
}

void Portal2_SetupLeafAndEntityListBox( const Vector &vecBoxMin, const Vector &vecBoxMax, ITraceListData *pTraceData )
{
	CPortal2TraceListData *pData = static_cast<CPortal2TraceListData *>( pTraceData );
	pData->m_Data.Reset();
	enginetrace->SetupLeafAndEntityListBox( vecBoxMin, vecBoxMax, pData->m_Data );
	pData->m_vBoxMin = vecBoxMin;
	pData->m_vBoxMax = vecBoxMax;
	pData->m_bHasBox = true;
}

void Portal2_TraceRayAgainstLeafAndEntityList( const Ray_t &ray, ITraceListData *pTraceData, unsigned int fMask,
											   ITraceFilter *pTraceFilter, trace_t *pTrace )
{
	CPortal2TraceListData *pData = static_cast<CPortal2TraceListData *>( pTraceData );
	enginetrace->TraceRayAgainstLeafAndEntityList( ray, pData->m_Data, fMask, pTraceFilter, pTrace );
}

//-----------------------------------------------------------------------------
// IPhysicsCollision helpers
//-----------------------------------------------------------------------------
bool Portal2_TraceBoxAA( const Ray_t &ray, const CPhysCollide *pCollide, trace_t *ptr )
{
	physcollision->TraceBox( ray, pCollide, vec3_origin, vec3_angle, ptr );
	return ptr->startsolid || ptr->fraction < 1.0f;
}

float Portal2_CollideGetRadius( const CPhysCollide *pCollide )
{
	Vector vMins, vMaxs;
	physcollision->CollideGetAABB( &vMins, &vMaxs, pCollide, vec3_origin, vec3_angle );

	Vector vFarthest;
	for ( int i = 0; i != 3; ++i )
		vFarthest[i] = MAX( fabsf( vMins[i] ), fabsf( vMaxs[i] ) );
	return vFarthest.Length();
}

//-----------------------------------------------------------------------------
// DispatchParticleEffect with recipients (portal2_util_compat.h)
//-----------------------------------------------------------------------------
void DispatchParticleEffect( const char *pszParticleName, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pEntity,
							 int nSplitScreenPlayerSlot, IRecipientFilter *filter )
{
	if ( !filter )
	{
		DispatchParticleEffect( pszParticleName, vecOrigin, vecAngles, pEntity );
		return;
	}

	// The same effect data as this tree's DispatchParticleEffect( int, ... ).
	CEffectData data;
	data.m_nHitBox = GetParticleSystemIndex( pszParticleName );
	data.m_vOrigin = vecOrigin;
	data.m_vStart = vecOrigin;
	data.m_vAngles = vecAngles;
	if ( pEntity )
	{
#ifdef CLIENT_DLL
		data.m_hEntity = pEntity;
#else
		data.m_nEntIndex = pEntity->entindex();
#endif
		data.m_fFlags |= PARTICLE_DISPATCH_FROM_ENTITY;
		data.m_nDamageType = PATTACH_CUSTOMORIGIN;
	}

	te->DispatchEffect( *filter, 0.0f, data.m_vOrigin, "ParticleEffect", data );
}

//-----------------------------------------------------------------------------
// CTraceFilterNoPlayers
//-----------------------------------------------------------------------------
bool CTraceFilterNoPlayers::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	if ( !CTraceFilterSimple::ShouldHitEntity( pHandleEntity, contentsMask ) )
		return false;

	CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
	return pEntity && !pEntity->IsPlayer();
}

#ifdef GAME_DLL
extern IPhysicsEnvironment *physenv;

namespace
{
	// Owns collides released while the physics environment was simulating.
	class CPortal2DeferredCollideDestroyer : public CAutoGameSystemPerFrame
	{
	public:
		CPortal2DeferredCollideDestroyer() : CAutoGameSystemPerFrame( "CPortal2DeferredCollideDestroyer" ) {}

		void Add( CPhysCollide *pCollide ) { m_Pending.AddToTail( pCollide ); }

		void Flush()
		{
			for ( int i = 0; i != m_Pending.Count(); ++i )
				physcollision->DestroyCollide( m_Pending[i] );
			m_Pending.RemoveAll();
		}

		virtual void FrameUpdatePostEntityThink() { Flush(); }
		virtual void LevelShutdownPostEntity() { Flush(); }

	private:
		CUtlVector<CPhysCollide *> m_Pending;
	};

	CPortal2DeferredCollideDestroyer s_DeferredCollideDestroyer;
}

void UTIL_RecordAchievementEvent( const char *pszAchievementname, CBasePlayer *pPlayer )
{
	PORTAL2_SHARED_UNSUPPORTED( "Steam achievement events" );
	DevMsg( 2, "Portal 2: achievement event %s not recorded\n", pszAchievementname );
}

void Portal2_DestroyCollideOnDeadObjectFlush( CPhysCollide *pCollide )
{
	if ( physenv && physenv->IsInSimulation() )
		s_DeferredCollideDestroyer.Add( pCollide );
	else
		physcollision->DestroyCollide( pCollide );
}
#endif // GAME_DLL

//-----------------------------------------------------------------------------
// UTIL_FindClosestPassableSpace
//-----------------------------------------------------------------------------
static void ApplyAxisRestrictions( Vector &vMove, int nAxisRestrictionFlags )
{
	for ( int i = 0; i != 3; ++i )
	{
		if ( ( vMove[i] > 0.0f ) && ( nAxisRestrictionFlags & ( FL_AXIS_DIRECTION_X << ( i * 2 ) ) ) )
			vMove[i] = 0.0f;
		else if ( ( vMove[i] < 0.0f ) && ( nAxisRestrictionFlags & ( FL_AXIS_DIRECTION_NX << ( i * 2 ) ) ) )
			vMove[i] = 0.0f;
	}
}

bool UTIL_FindClosestPassableSpace( const Vector &vOriginalCenter, const Vector &vExtents, const Vector &vIndecisivePush,
									unsigned int iIterations, Vector &vCenterOut, int nAxisRestrictionFlags,
									FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter )
{
	Assert( pTraceAdapter && pTraceAdapter->pTraceFunc && pTraceAdapter->pPointOutsideWorldFunc );

	Vector ptExtents[8]; // 3 bits per corner: 0 is the min and 1 the max on x, y, z
	float fExtentsValidation[8]; // how far each corner can see the others

	Vector vCenter = vOriginalCenter;
	vCenter.z += 0.001f; // so the first trace is swept

	trace_t traces[2];
	Ray_t entRay;
	entRay.m_Extents = vExtents;
	entRay.m_IsRay = false;
	entRay.m_IsSwept = true;
	entRay.m_StartOffset = vec3_origin;

	Vector vOriginalExtents = vExtents;
	Vector vGrowSize = vExtents / 101.0f;
	Vector vCurrentExtents = vExtents - vGrowSize;

	Ray_t testRay;
	testRay.m_Extents = vGrowSize;
	testRay.m_IsRay = false;
	testRay.m_IsSwept = true;
	testRay.m_StartOffset = vec3_origin;

	for ( unsigned int iFailCount = 0; iFailCount != iIterations; ++iFailCount )
	{
		entRay.m_Start = vCenter;
		entRay.m_Delta = vOriginalCenter - vCenter;

		pTraceAdapter->pTraceFunc( entRay, &traces[0], pTraceAdapter );
		if ( !traces[0].startsolid )
		{
			vCenterOut = traces[0].endpos;
			return true; // current placement worked
		}

		bool bExtentInvalid[8];
		for ( int i = 0; i != 8; ++i )
		{
			fExtentsValidation[i] = 0.0f;
			ptExtents[i] = vCenter;
			ptExtents[i].x += ( ( i & ( 1 << 0 ) ) ? vCurrentExtents.x : -vCurrentExtents.x );
			ptExtents[i].y += ( ( i & ( 1 << 1 ) ) ? vCurrentExtents.y : -vCurrentExtents.y );
			ptExtents[i].z += ( ( i & ( 1 << 2 ) ) ? vCurrentExtents.z : -vCurrentExtents.z );

			bExtentInvalid[i] = pTraceAdapter->pPointOutsideWorldFunc( ptExtents[i], pTraceAdapter );
		}

		for ( unsigned int counter = 0; counter != 7; ++counter )
		{
			for ( unsigned int counter2 = counter + 1; counter2 != 8; ++counter2 )
			{
				testRay.m_Delta = ptExtents[counter2] - ptExtents[counter];

				if ( bExtentInvalid[counter] )
				{
					traces[0].startsolid = true;
				}
				else
				{
					testRay.m_Start = ptExtents[counter];
					pTraceAdapter->pTraceFunc( testRay, &traces[0], pTraceAdapter );
				}

				if ( bExtentInvalid[counter2] )
				{
					traces[1].startsolid = true;
				}
				else
				{
					testRay.m_Start = ptExtents[counter2];
					testRay.m_Delta = -testRay.m_Delta;
					pTraceAdapter->pTraceFunc( testRay, &traces[1], pTraceAdapter );
				}

				float fDistance = testRay.m_Delta.Length();

				for ( int i = 0; i != 2; ++i )
				{
					int iExtent = ( i == 0 ) ? counter : counter2;

					if ( traces[i].startsolid )
						fExtentsValidation[iExtent] -= 100.0f;
					else
						fExtentsValidation[iExtent] += traces[i].fraction * fDistance;
				}
			}
		}

		Vector vNewOriginDirection( 0.0f, 0.0f, 0.0f );
		float fTotalValidation = 0.0f;
		for ( int i = 0; i != 8; ++i )
		{
			if ( fExtentsValidation[i] > 0.0f )
			{
				vNewOriginDirection += ( ptExtents[i] - vCenter ) * fExtentsValidation[i];
				fTotalValidation += fExtentsValidation[i];
			}
		}

		if ( fTotalValidation != 0.0f )
		{
			Vector vMove = vNewOriginDirection / fTotalValidation;
			ApplyAxisRestrictions( vMove, nAxisRestrictionFlags );
			vCenter += vMove;

			// increase sizing
			testRay.m_Extents += vGrowSize;
			vCurrentExtents -= vGrowSize;
		}
		else
		{
			// no point was valid, apply the indecisive vector
			Vector vMove = vIndecisivePush;
			ApplyAxisRestrictions( vMove, nAxisRestrictionFlags );
			vCenter += vMove;

			// reset sizing
			testRay.m_Extents = vGrowSize;
			vCurrentExtents = vOriginalExtents;
		}
	}

	vCenterOut = vCenter;
	return false;
}

static void EngineTraceFunc( const Ray_t &ray, trace_t *pResult, FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter )
{
	enginetrace->TraceRay( ray, pTraceAdapter->fMask, pTraceAdapter->pTraceFilter, pResult );
}

static bool EnginePointOutsideWorldFunc( const Vector &vTest, FindClosestPassableSpace_TraceAdapter_t *pTraceAdapter )
{
	return enginetrace->PointOutsideWorld( vTest );
}

bool UTIL_FindClosestPassableSpace( const Vector &vCenter, const Vector &vExtents, const Vector &vIndecisivePush,
									ITraceFilter *pTraceFilter, unsigned int fMask, unsigned int iIterations,
									Vector &vCenterOut, int nAxisRestrictionFlags )
{
	FindClosestPassableSpace_TraceAdapter_t adapter;
	adapter.pTraceFunc = EngineTraceFunc;
	adapter.pPointOutsideWorldFunc = EnginePointOutsideWorldFunc;
	adapter.pTraceFilter = pTraceFilter;
	adapter.fMask = fMask;
	return UTIL_FindClosestPassableSpace( vCenter, vExtents, vIndecisivePush, iIterations, vCenterOut, nAxisRestrictionFlags, &adapter );
}

//-----------------------------------------------------------------------------
// Later mathlib and collision helpers (portal2_util_compat.h)
//-----------------------------------------------------------------------------
VMatrix SetupMatrixAxisToAxisRot( const Vector &vFromAxis, const Vector &vToAxis )
{
	Vector vAxis = vFromAxis.Cross( vToAxis );

	vec_t s = vAxis.Length(); // sin(theta)
	vec_t c = vFromAxis.Dot( vToAxis ); // cos(theta)
	vec_t t = 1.0f - c;

	if ( s <= 0 )
		return SetupMatrixIdentity();

	vAxis *= 1.0f / s;

	vec_t tx = t * vAxis.x, ty = t * vAxis.y, tz = t * vAxis.z;
	vec_t sx = s * vAxis.x, sy = s * vAxis.y, sz = s * vAxis.z;

	return VMatrix(
		tx * vAxis.x + c, tx * vAxis.y - sz, tx * vAxis.z + sy, 0.0f,
		tx * vAxis.y + sz, ty * vAxis.y + c, ty * vAxis.z - sx, 0.0f,
		tx * vAxis.z - sy, ty * vAxis.z + sx, tz * vAxis.z + c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
}

// Parametric entry/exit of the 2D line vStart + t * vDelta through the circle
// of radius flRadius at the origin. False when the line misses the circle.
static bool Portal2_LineCircleIntersection( const Vector2D &vStart, const Vector2D &vDelta, float flRadius,
											float *pflEnter, float *pflLeave )
{
	float a = vDelta.Dot( vDelta );
	float b = 2.0f * vStart.Dot( vDelta );
	float c = vStart.Dot( vStart ) - flRadius * flRadius;

	if ( a < 1e-12f )
	{
		// No motion in the plane: inside for the whole ray or never.
		if ( c > 0.0f )
			return false;
		*pflEnter = 0.0f;
		*pflLeave = 1.0f;
		return true;
	}

	float flDiscriminant = b * b - 4.0f * a * c;
	if ( flDiscriminant < 0.0f )
		return false;

	float flRoot = sqrtf( flDiscriminant );
	*pflEnter = ( -b - flRoot ) / ( 2.0f * a );
	*pflLeave = ( -b + flRoot ) / ( 2.0f * a );
	return true;
}

bool IntersectRayWithAACylinder( const Ray_t &ray, const Vector &center, float radius, float height, CBaseTrace *pTrace )
{
	Assert( ray.m_IsRay );

	// Clear the trace as CS:GO's Collision_ClearTrace() does.
	pTrace->startpos = ray.m_Start;
	pTrace->endpos = ray.m_Start + ray.m_Delta;
	pTrace->fraction = 1.0f;
	pTrace->startsolid = false;
	pTrace->allsolid = false;
	pTrace->contents = 0;
	pTrace->plane.normal.Init();

	// First intersect the ray with the top + bottom planes
	float halfHeight = height * 0.5f;

	Vector vStart = ray.m_Start - center;
	Vector vEnd = vStart + ray.m_Delta;

	float flEnterFrac, flLeaveFrac;
	if ( FloatMakePositive( ray.m_Delta.z ) < 1e-8 )
	{
		// Parallel to the caps
		if ( ( vStart.z < -halfHeight ) || ( vStart.z > halfHeight ) )
			return false;
		flEnterFrac = 0.0f;
		flLeaveFrac = 1.0f;
	}
	else
	{
		flEnterFrac = IntersectRayWithAAPlane( vStart, vEnd, 2, 1, halfHeight );
		flLeaveFrac = IntersectRayWithAAPlane( vStart, vEnd, 2, 1, -halfHeight );

		if ( flLeaveFrac < flEnterFrac )
		{
			float temp = flLeaveFrac;
			flLeaveFrac = flEnterFrac;
			flEnterFrac = temp;
		}

		if ( flLeaveFrac < 0 || flEnterFrac > 1 )
			return false;
	}

	// Intersect with circle
	float flCircleEnterFrac, flCircleLeaveFrac;
	if ( !Portal2_LineCircleIntersection( vStart.AsVector2D(), ray.m_Delta.AsVector2D(), radius, &flCircleEnterFrac, &flCircleLeaveFrac ) )
		return false;

	if ( flCircleLeaveFrac < 0 || flCircleEnterFrac > 1 )
		return false;

	if ( flEnterFrac < flCircleEnterFrac )
		flEnterFrac = flCircleEnterFrac;
	if ( flLeaveFrac > flCircleLeaveFrac )
		flLeaveFrac = flCircleLeaveFrac;

	if ( flLeaveFrac < flEnterFrac )
		return false;

	VectorMA( ray.m_Start, flEnterFrac, ray.m_Delta, pTrace->endpos );
	pTrace->fraction = flEnterFrac;
	pTrace->contents = CONTENTS_SOLID;

	// The normal points from the nearest point on the center line to the hit.
	Vector collisionCenter;
	CalcClosestPointOnLineSegment( pTrace->endpos, center + Vector( 0, 0, halfHeight ), center - Vector( 0, 0, halfHeight ), collisionCenter );
	pTrace->plane.normal = pTrace->endpos - collisionCenter;
	VectorNormalize( pTrace->plane.normal );

	return true;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// UTIL_FindWaterSurface (client), the server util.cpp version.
//-----------------------------------------------------------------------------
namespace
{
	// Hits only entities with volume contents (water).
	class CPortal2WaterTraceFilter : public CTraceFilter
	{
	public:
		virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
		{
			C_BaseEntity *pCollide = EntityFromEntityHandle( pHandleEntity );
			return pCollide && ( pCollide->GetSolidFlags() & FSOLID_VOLUME_CONTENTS );
		}
	};
}

float UTIL_FindWaterSurface( const Vector &position, float minz, float maxz )
{
	Vector vecStart, vecEnd;
	vecStart.Init( position.x, position.y, maxz );
	vecEnd.Init( position.x, position.y, minz );

	Ray_t ray;
	trace_t tr;
	CPortal2WaterTraceFilter waterTraceFilter;
	ray.Init( vecStart, vecEnd );
	enginetrace->TraceRay( ray, MASK_WATER, &waterTraceFilter, &tr );

	return tr.endpos.z;
}
#endif // CLIENT_DLL
