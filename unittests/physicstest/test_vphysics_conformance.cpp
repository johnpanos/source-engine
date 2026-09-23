//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared VPhysics provider conformance suite (RFC 0004 / RFC 0005
//          Q-PHYSICS). Loads ONE provider module per process through its
//          exported CreateInterface -- exactly as the engine does -- and runs
//          the same public-contract checks against it. IVP is the legacy oracle:
//          any check that fails on IVP is a wrong check, not an IVP bug.
//
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//          Runner:          tools/quality/physics_conformance.py
//
// Output protocol (stdout, one record per line):
//   PASS <tier> <check>
//   FAIL <tier> <check>: <detail>
//   OBS <key> <tolerance> <value>...   observation for cross-provider comparison
//   TIER <tier> <checks> <failed>
//   CONFORMANCE <checks> <failed>     (checks-v1 summary)
//
// Tiers: "boot" is what a map load needs to complete without crashing
// (non-null results, coherent bookkeeping, authored data). "gameplay" is
// correct physical behavior (collision geometry, traces, simulation, events).
//
// --fault <name> deliberately breaks one provider behavior (a decorator over
// the real provider, or suppression at the provider call boundary) so the
// runner can prove each check family detects a non-conforming provider.
//
//=============================================================================//

#include <dlfcn.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tier0/dbg.h"
#include "tier1/interface.h"
#include "tier1/utlvector.h"
#include "mathlib/mathlib.h"
#include "vstdlib/cvar.h"
#include "vphysics_interface.h"
#include "vphysics/object_hash.h"
#include "vphysics/collision_set.h"
#include "vphysics/constraints.h"
#include "vphysics/friction.h"
#include "vphysics/performance.h"
#include "vphysics/player_controller.h"
#include "vcollide_parse.h"
#include "cmodel.h"
#include "gametrace.h"
#include "phyfile.h"
#include "bspflags.h"
#include "mathlib/polyhedron.h"

#include "vphysics_conformance.h"

//-----------------------------------------------------------------------------
// Result reporting
//-----------------------------------------------------------------------------
static const char *s_tierNames[TIER_COUNT] = { "boot", "gameplay" };
static int s_checks[TIER_COUNT];
static int s_failed[TIER_COUNT];

bool Check( Tier_t tier, const char *pName, bool ok, const char *pFormat, ... )
{
	s_checks[tier]++;
	if ( ok )
	{
		printf( "PASS %s %s\n", s_tierNames[tier], pName );
		return true;
	}
	s_failed[tier]++;
	char detail[512] = "";
	if ( pFormat )
	{
		va_list args;
		va_start( args, pFormat );
		vsnprintf( detail, sizeof( detail ), pFormat, args );
		va_end( args );
	}
	printf( "FAIL %s %s: %s\n", s_tierNames[tier], pName, detail );
	return false;
}

// Observation tolerances: "x" exact, "a<abs>" absolute, "r<frac>" relative.
void ObsFloats( const char *pKey, const char *pTol, int count, const float *pValues )
{
	printf( "OBS %s %s", pKey, pTol );
	for ( int i = 0; i < count; i++ )
		printf( " %.4f", pValues[i] );
	printf( "\n" );
}

void ObsVector( const char *pKey, const char *pTol, const Vector &v )
{
	float values[3] = { v.x, v.y, v.z };
	ObsFloats( pKey, pTol, 3, values );
}

void ObsString( const char *pKey, const char *pValue )
{
	char clean[512];
	V_strncpy( clean, pValue ? pValue : "(null)", sizeof( clean ) );
	for ( char *p = clean; *p; p++ )
	{
		if ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' )
			*p = '_';
	}
	printf( "OBS %s x %s\n", pKey, clean[0] ? clean : "(empty)" );
}

bool Near( float a, float b, float tol )
{
	return fabsf( a - b ) <= tol;
}

bool NearVec( const Vector &a, const Vector &b, float tol )
{
	return Near( a.x, b.x, tol ) && Near( a.y, b.y, tol ) && Near( a.z, b.z, tol );
}

// Engine targets build with fast-math, where isfinite() may fold to true. A
// magnitude comparison is false for NaN and infinity under any FP mode.
bool IsFiniteVec( const Vector &v )
{
	return fabsf( v.x ) < 1e30f && fabsf( v.y ) < 1e30f && fabsf( v.z ) < 1e30f;
}

//-----------------------------------------------------------------------------
// Fault injection (sensitivity). Each decorator forwards to the real provider
// and breaks exactly one behavior.
//-----------------------------------------------------------------------------
static const char *s_pFault = "";

bool FaultIs( const char *pName )
{
	return !V_strcmp( s_pFault, pName );
}

class CFaultSurfaceProps : public IPhysicsSurfaceProps
{
public:
	explicit CFaultSurfaceProps( IPhysicsSurfaceProps *pInner ) : m_pInner( pInner ) {}
	virtual int ParseSurfaceData( const char *pFilename, const char *pTextfile ) { return m_pInner->ParseSurfaceData( pFilename, pTextfile ); }
	virtual int SurfacePropCount( void ) const { return m_pInner->SurfacePropCount(); }
	virtual int GetSurfaceIndex( const char *pName ) const
	{
		if ( FaultIs( "surfaceprops-index" ) )
			return 0;
		return m_pInner->GetSurfaceIndex( pName );
	}
	virtual void GetPhysicsProperties( int i, float *d, float *t, float *f, float *e ) const { m_pInner->GetPhysicsProperties( i, d, t, f, e ); }
	virtual surfacedata_t *GetSurfaceData( int i )
	{
		if ( FaultIs( "surfaceprops-null" ) && i != 0 )
			return NULL;
		return m_pInner->GetSurfaceData( i );
	}
	virtual const char *GetString( unsigned short i ) const { return m_pInner->GetString( i ); }
	virtual const char *GetPropName( int i ) const { return m_pInner->GetPropName( i ); }
	virtual void SetWorldMaterialIndexTable( int *pMap, int size ) { m_pInner->SetWorldMaterialIndexTable( pMap, size ); }
	virtual void GetPhysicsParameters( int i, surfacephysicsparams_t *pOut ) const { m_pInner->GetPhysicsParameters( i, pOut ); }

private:
	IPhysicsSurfaceProps *m_pInner;
};

class CFaultCollision : public IPhysicsCollision
{
public:
	explicit CFaultCollision( IPhysicsCollision *pInner ) : m_pInner( pInner ) {}

	virtual CPhysConvex *ConvexFromVerts( Vector **pVerts, int vertCount ) { return m_pInner->ConvexFromVerts( pVerts, vertCount ); }
	virtual CPhysConvex *ConvexFromPlanes( float *pPlanes, int planeCount, float mergeDistance ) { return m_pInner->ConvexFromPlanes( pPlanes, planeCount, mergeDistance ); }
	virtual float ConvexVolume( CPhysConvex *pConvex ) { return m_pInner->ConvexVolume( pConvex ); }
	virtual float ConvexSurfaceArea( CPhysConvex *pConvex ) { return m_pInner->ConvexSurfaceArea( pConvex ); }
	virtual void SetConvexGameData( CPhysConvex *pConvex, unsigned int gameData ) { m_pInner->SetConvexGameData( pConvex, gameData ); }
	virtual void ConvexFree( CPhysConvex *pConvex ) { m_pInner->ConvexFree( pConvex ); }
	virtual CPhysConvex *BBoxToConvex( const Vector &mins, const Vector &maxs ) { return m_pInner->BBoxToConvex( mins, maxs ); }
	virtual CPhysConvex *ConvexFromConvexPolyhedron( const CPolyhedron &poly ) { return m_pInner->ConvexFromConvexPolyhedron( poly ); }
	virtual void ConvexesFromConvexPolygon( const Vector &n, const Vector *pPoints, int count, CPhysConvex **pOut ) { m_pInner->ConvexesFromConvexPolygon( n, pPoints, count, pOut ); }
	virtual CPhysPolysoup *PolysoupCreate( void ) { return m_pInner->PolysoupCreate(); }
	virtual void PolysoupDestroy( CPhysPolysoup *pSoup ) { m_pInner->PolysoupDestroy( pSoup ); }
	virtual void PolysoupAddTriangle( CPhysPolysoup *pSoup, const Vector &a, const Vector &b, const Vector &c, int m ) { m_pInner->PolysoupAddTriangle( pSoup, a, b, c, m ); }
	virtual CPhysCollide *ConvertPolysoupToCollide( CPhysPolysoup *pSoup, bool useMOPP ) { return m_pInner->ConvertPolysoupToCollide( pSoup, useMOPP ); }
	virtual CPhysCollide *ConvertConvexToCollide( CPhysConvex **pConvex, int count ) { return m_pInner->ConvertConvexToCollide( pConvex, count ); }
	virtual CPhysCollide *ConvertConvexToCollideParams( CPhysConvex **pConvex, int count, const convertconvexparams_t &p ) { return m_pInner->ConvertConvexToCollideParams( pConvex, count, p ); }
	virtual void DestroyCollide( CPhysCollide *pCollide ) { m_pInner->DestroyCollide( pCollide ); }
	virtual int CollideSize( CPhysCollide *pCollide ) { return m_pInner->CollideSize( pCollide ); }
	virtual int CollideWrite( char *pDest, CPhysCollide *pCollide, bool bSwap ) { return m_pInner->CollideWrite( pDest, pCollide, bSwap ); }
	virtual CPhysCollide *UnserializeCollide( char *pBuffer, int size, int index ) { return m_pInner->UnserializeCollide( pBuffer, size, index ); }
	virtual float CollideVolume( CPhysCollide *pCollide ) { return m_pInner->CollideVolume( pCollide ); }
	virtual float CollideSurfaceArea( CPhysCollide *pCollide ) { return m_pInner->CollideSurfaceArea( pCollide ); }
	virtual Vector CollideGetExtent( const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles, const Vector &dir ) { return m_pInner->CollideGetExtent( pCollide, origin, angles, dir ); }
	virtual void CollideGetAABB( Vector *pMins, Vector *pMaxs, const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles )
	{
		if ( FaultIs( "collision-unit-aabb" ) )
		{
			*pMins = origin - Vector( 1, 1, 1 );
			*pMaxs = origin + Vector( 1, 1, 1 );
			return;
		}
		m_pInner->CollideGetAABB( pMins, pMaxs, pCollide, origin, angles );
	}
	virtual void CollideGetMassCenter( CPhysCollide *pCollide, Vector *pOut ) { m_pInner->CollideGetMassCenter( pCollide, pOut ); }
	virtual void CollideSetMassCenter( CPhysCollide *pCollide, const Vector &c ) { m_pInner->CollideSetMassCenter( pCollide, c ); }
	virtual Vector CollideGetOrthographicAreas( const CPhysCollide *pCollide ) { return m_pInner->CollideGetOrthographicAreas( pCollide ); }
	virtual void CollideSetOrthographicAreas( CPhysCollide *pCollide, const Vector &areas ) { m_pInner->CollideSetOrthographicAreas( pCollide, areas ); }
	virtual int CollideIndex( const CPhysCollide *pCollide ) { return m_pInner->CollideIndex( pCollide ); }
	virtual CPhysCollide *BBoxToCollide( const Vector &mins, const Vector &maxs ) { return m_pInner->BBoxToCollide( mins, maxs ); }
	virtual int GetConvexesUsedInCollideable( const CPhysCollide *pCollide, CPhysConvex **pOut, int limit ) { return m_pInner->GetConvexesUsedInCollideable( pCollide, pOut, limit ); }

	virtual void TraceBox( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles, trace_t *ptr )
	{
		m_pInner->TraceBox( start, end, mins, maxs, pCollide, origin, angles, ptr );
		BreakTrace( ptr );
	}
	virtual void TraceBox( const Ray_t &ray, const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles, trace_t *ptr )
	{
		m_pInner->TraceBox( ray, pCollide, origin, angles, ptr );
		BreakTrace( ptr );
	}
	virtual void TraceBox( const Ray_t &ray, unsigned int mask, IConvexInfo *pInfo, const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles, trace_t *ptr )
	{
		m_pInner->TraceBox( ray, mask, pInfo, pCollide, origin, angles, ptr );
		BreakTrace( ptr );
	}
	virtual void TraceCollide( const Vector &start, const Vector &end, const CPhysCollide *pSweep, const QAngle &sweepAngles, const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles, trace_t *ptr )
	{
		m_pInner->TraceCollide( start, end, pSweep, sweepAngles, pCollide, origin, angles, ptr );
		BreakTrace( ptr );
	}
	virtual bool IsBoxIntersectingCone( const Vector &mins, const Vector &maxs, const truncatedcone_t &cone ) { return m_pInner->IsBoxIntersectingCone( mins, maxs, cone ); }
	virtual void VCollideLoad( vcollide_t *pOutput, int solidCount, const char *pBuffer, int size, bool swap )
	{
		m_pInner->VCollideLoad( pOutput, solidCount, pBuffer, size, swap );
		if ( FaultIs( "vcollide-keyvalues" ) && pOutput->pKeyValues )
			pOutput->pKeyValues[0] = '\0';
	}
	virtual void VCollideUnload( vcollide_t *pVCollide ) { m_pInner->VCollideUnload( pVCollide ); }
	virtual IVPhysicsKeyParser *VPhysicsKeyParserCreate( const char *pKeyData ) { return m_pInner->VPhysicsKeyParserCreate( pKeyData ); }
	virtual void VPhysicsKeyParserDestroy( IVPhysicsKeyParser *pParser ) { m_pInner->VPhysicsKeyParserDestroy( pParser ); }
	virtual int CreateDebugMesh( CPhysCollide const *pCollide, Vector **outVerts ) { return m_pInner->CreateDebugMesh( pCollide, outVerts ); }
	virtual void DestroyDebugMesh( int vertCount, Vector *outVerts ) { m_pInner->DestroyDebugMesh( vertCount, outVerts ); }
	virtual ICollisionQuery *CreateQueryModel( CPhysCollide *pCollide ) { return m_pInner->CreateQueryModel( pCollide ); }
	virtual void DestroyQueryModel( ICollisionQuery *pQuery ) { m_pInner->DestroyQueryModel( pQuery ); }
	virtual IPhysicsCollision *ThreadContextCreate( void ) { return m_pInner->ThreadContextCreate(); }
	virtual void ThreadContextDestroy( IPhysicsCollision *pContext ) { m_pInner->ThreadContextDestroy( pContext ); }
	virtual CPhysCollide *CreateVirtualMesh( const virtualmeshparams_t &params ) { return m_pInner->CreateVirtualMesh( params ); }
	virtual bool SupportsVirtualMesh() { return m_pInner->SupportsVirtualMesh(); }
	virtual bool GetBBoxCacheSize( int *pSize, int *pCount ) { return m_pInner->GetBBoxCacheSize( pSize, pCount ); }
	virtual CPolyhedron *PolyhedronFromConvex( CPhysConvex * const pConvex, bool bTemp ) { return m_pInner->PolyhedronFromConvex( pConvex, bTemp ); }
	virtual void OutputDebugInfo( const CPhysCollide *pCollide ) { m_pInner->OutputDebugInfo( pCollide ); }
	virtual unsigned int ReadStat( int statID ) { return m_pInner->ReadStat( statID ); }

private:
	// The stub Box3D behavior this fault models: a trace that never hits.
	static void BreakTrace( trace_t *ptr )
	{
		if ( !FaultIs( "collision-trace-miss" ) )
			return;
		ptr->fraction = 1.0f;
		ptr->startsolid = false;
		ptr->allsolid = false;
	}

	IPhysicsCollision *m_pInner;
};

class CFaultPairHash : public IPhysicsObjectPairHash
{
public:
	explicit CFaultPairHash( IPhysicsObjectPairHash *pInner ) : m_pInner( pInner ) {}
	virtual void AddObjectPair( void *p0, void *p1 ) { m_pInner->AddObjectPair( p0, p1 ); }
	virtual void RemoveObjectPair( void *p0, void *p1 ) { m_pInner->RemoveObjectPair( p0, p1 ); }
	virtual bool IsObjectPairInHash( void *p0, void *p1 )
	{
		// Order-sensitive lookup: game code queries pairs in either order.
		if ( p0 > p1 )
			return false;
		return m_pInner->IsObjectPairInHash( p0, p1 );
	}
	virtual void RemoveAllPairsForObject( void *p0 ) { m_pInner->RemoveAllPairsForObject( p0 ); }
	virtual bool IsObjectInHash( void *p0 ) { return m_pInner->IsObjectInHash( p0 ); }
	virtual int GetPairCountForObject( void *p0 ) { return m_pInner->GetPairCountForObject( p0 ); }
	virtual int GetPairListForObject( void *p0, int max, void **ppList ) { return m_pInner->GetPairListForObject( p0, max, ppList ); }

private:
	IPhysicsObjectPairHash *m_pInner;
};

//-----------------------------------------------------------------------------
// Provider under test
//-----------------------------------------------------------------------------
static CreateInterfaceFn s_providerFactory;
IPhysics *s_pPhysics;
IPhysicsCollision *s_pCollision;
IPhysicsSurfaceProps *s_pProps;

static void *HostFactory( const char *pName, int *pReturnCode )
{
	void *pResult = s_providerFactory ? s_providerFactory( pName, NULL ) : NULL;
	if ( !pResult )
	{
		CreateInterfaceFn cvarFactory = VStdLib_GetICVarFactory();
		pResult = cvarFactory ? cvarFactory( pName, NULL ) : NULL;
	}
	if ( pReturnCode )
		*pReturnCode = pResult ? IFACE_OK : IFACE_FAILED;
	return pResult;
}

// Every provider Simulate call goes through here so the "sim-noop" fault can
// model a provider whose step does not advance the world.
extern const float kTick = 0.015f;

void Step( IPhysicsEnvironment *pEnv, float seconds )
{
	int ticks = (int)( seconds / kTick + 0.5f );
	for ( int i = 0; i < ticks; i++ )
	{
		if ( !FaultIs( "sim-noop" ) )
			pEnv->Simulate( kTick );
	}
}

static char *ReadFile( const char *pPath, int *pSize )
{
	FILE *fp = fopen( pPath, "rb" );
	if ( !fp )
		return NULL;
	fseek( fp, 0, SEEK_END );
	long size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	char *pData = (char *)malloc( size + 1 );
	if ( fread( pData, 1, size, fp ) != (size_t)size )
	{
		fclose( fp );
		free( pData );
		return NULL;
	}
	fclose( fp );
	pData[size] = '\0';
	if ( pSize )
		*pSize = (int)size;
	return pData;
}

objectparams_t DefaultParams( float mass, void *pGameData )
{
	// Mirrors game/shared/physics_shared.cpp g_PhysDefaultObjectParams.
	objectparams_t params;
	memset( &params, 0, sizeof( params ) );
	params.mass = mass;
	params.inertia = 1.0f;
	params.damping = 0.1f;
	params.rotdamping = 0.1f;
	params.rotInertiaLimit = 0.05f;
	params.pName = "conformance";
	params.pGameData = pGameData;
	params.volume = 0.0f;
	params.dragCoefficient = 1.0f;
	params.enableCollisions = true;
	return params;
}

static void ResetTrace( trace_t *pTrace )
{
	memset( (void *)pTrace, 0, sizeof( *pTrace ) );
	// Sentinel: a provider that never writes the trace is detected.
	pTrace->fraction = -1.0f;
}

//-----------------------------------------------------------------------------
// Module / interfaces
//-----------------------------------------------------------------------------
static bool TestModule( const char *pProviderPath )
{
	void *pModule = dlopen( pProviderPath, RTLD_NOW | RTLD_LOCAL );
	if ( !Check( TIER_BOOT, "module.load", pModule != NULL, "%s", dlerror() ) )
		return false;
	s_providerFactory = (CreateInterfaceFn)dlsym( pModule, CREATEINTERFACE_PROCNAME );
	if ( !Check( TIER_BOOT, "module.create-interface-export", s_providerFactory != NULL ) )
		return false;

	s_pPhysics = (IPhysics *)s_providerFactory( VPHYSICS_INTERFACE_VERSION, NULL );
	IPhysicsCollision *pCollision = (IPhysicsCollision *)s_providerFactory( VPHYSICS_COLLISION_INTERFACE_VERSION, NULL );
	IPhysicsSurfaceProps *pProps = (IPhysicsSurfaceProps *)s_providerFactory( VPHYSICS_SURFACEPROPS_INTERFACE_VERSION, NULL );
	Check( TIER_BOOT, "module.exports-physics", s_pPhysics != NULL );
	Check( TIER_BOOT, "module.exports-collision", pCollision != NULL );
	Check( TIER_BOOT, "module.exports-surfaceprops", pProps != NULL );
	if ( !s_pPhysics || !pCollision || !pProps )
		return false;

	// The app framework resolves sibling interfaces through the primary
	// AppSystem's QueryInterface, not the module factory.
	Check( TIER_BOOT, "module.query-interface-collision",
		s_pPhysics->QueryInterface( VPHYSICS_COLLISION_INTERFACE_VERSION ) == pCollision );
	Check( TIER_BOOT, "module.query-interface-surfaceprops",
		s_pPhysics->QueryInterface( VPHYSICS_SURFACEPROPS_INTERFACE_VERSION ) == pProps );

	Check( TIER_BOOT, "module.connect", s_pPhysics->Connect( HostFactory ) );
	Check( TIER_BOOT, "module.init", s_pPhysics->Init() == INIT_OK );

	s_pCollision = new CFaultCollision( pCollision );
	s_pProps = new CFaultSurfaceProps( pProps );
	return true;
}

//-----------------------------------------------------------------------------
// Surface properties
//-----------------------------------------------------------------------------
static void TestSurfaceProps( const CUtlVector<const char *> &files )
{
	int parsed = 0;
	for ( int i = 0; i < files.Count(); i++ )
	{
		char *pText = ReadFile( files[i], NULL );
		if ( !Check( TIER_BOOT, "surfaceprops.read-fixture", pText != NULL, "%s", files[i] ) )
			continue;
		parsed = s_pProps->ParseSurfaceData( files[i], pText );
		free( pText );
	}

	int count = s_pProps->SurfacePropCount();
	Check( TIER_BOOT, "surfaceprops.parse-count", count > 1 && parsed == count,
		"parse returned %d, count %d", parsed, count );
	Check( TIER_BOOT, "surfaceprops.default-index-zero", s_pProps->GetSurfaceIndex( "default" ) == 0,
		"got %d", s_pProps->GetSurfaceIndex( "default" ) );
	Check( TIER_BOOT, "surfaceprops.unknown-name", s_pProps->GetSurfaceIndex( "no_such_surface_prop" ) == -1 );

	bool roundTrip = true;
	for ( int i = 0; i < count; i++ )
	{
		const char *pName = s_pProps->GetPropName( i );
		if ( !pName || s_pProps->GetSurfaceIndex( pName ) != i )
		{
			// Reserved (shadow) records resolve to their reserved index instead.
			if ( pName && pName[0] == '$' )
				continue;
			roundTrip = false;
			Check( TIER_BOOT, "surfaceprops.name-roundtrip", false, "index %d name %s", i, pName ? pName : "(null)" );
			break;
		}
	}
	if ( roundTrip )
		Check( TIER_BOOT, "surfaceprops.name-roundtrip", true );

	// Game code dereferences GetSurfaceData unconditionally (player
	// CategorizePosition, footsteps, impact sounds).
	const int probe[] = { -1, 0, 1, count - 1, count, count + 100, 127, 128, 0xF000 };
	bool neverNull = true;
	for ( int i = 0; i < (int)ARRAYSIZE( probe ); i++ )
	{
		if ( !s_pProps->GetSurfaceData( probe[i] ) )
		{
			neverNull = false;
			Check( TIER_BOOT, "surfaceprops.data-never-null", false, "index %d", probe[i] );
			break;
		}
	}
	if ( neverNull )
		Check( TIER_BOOT, "surfaceprops.data-never-null", true );

	// Re-parsing an already-loaded file is a no-op returning 0.
	if ( files.Count() )
	{
		char *pText = ReadFile( files[0], NULL );
		int again = pText ? s_pProps->ParseSurfaceData( files[0], pText ) : -1;
		free( pText );
		Check( TIER_BOOT, "surfaceprops.duplicate-file", again == 0 && s_pProps->SurfacePropCount() == count,
			"returned %d", again );
	}

	int shadow = s_pProps->GetSurfaceIndex( "$MATERIAL_INDEX_SHADOW" );
	surfacedata_t *pShadow = s_pProps->GetSurfaceData( shadow );
	Check( TIER_BOOT, "surfaceprops.shadow-reserved",
		shadow == 0xF000 && pShadow && Near( pShadow->physics.friction, 0.8f, 1e-4f ) && Near( pShadow->physics.elasticity, 1e-3f, 1e-5f ),
		"index %d", shadow );

	int concrete = s_pProps->GetSurfaceIndex( "concrete" );
	surfacedata_t *pConcrete = s_pProps->GetSurfaceData( concrete );
	const char *pStep = pConcrete ? s_pProps->GetString( pConcrete->sounds.stepleft ) : NULL;
	Check( TIER_BOOT, "surfaceprops.sound-strings", concrete > 0 && pStep && pStep[0],
		"concrete %d step %s", concrete, pStep ? pStep : "(null)" );

	surfacephysicsparams_t params;
	memset( &params, 0, sizeof( params ) );
	s_pProps->GetPhysicsParameters( concrete, &params );
	Check( TIER_BOOT, "surfaceprops.physics-parameters",
		pConcrete && params.friction == pConcrete->physics.friction && params.density == pConcrete->physics.density );

	// Full authored table for exact cross-provider comparison.
	for ( int i = 0; i < count; i++ )
	{
		surfacedata_t *pData = s_pProps->GetSurfaceData( i );
		char key[64];
		V_snprintf( key, sizeof( key ), "surfaceprops.%03d.name", i );
		ObsString( key, s_pProps->GetPropName( i ) );
		if ( !pData )
			continue;
		float values[] = {
			pData->physics.friction, pData->physics.elasticity, pData->physics.density,
			pData->physics.thickness, pData->physics.dampening,
			pData->game.maxSpeedFactor, pData->game.jumpFactor,
			(float)pData->game.material, (float)pData->game.climbable,
			pData->audio.reflectivity, pData->audio.hardnessFactor };
		V_snprintf( key, sizeof( key ), "surfaceprops.%03d.values", i );
		ObsFloats( key, "a0.0001", ARRAYSIZE( values ), values );
		V_snprintf( key, sizeof( key ), "surfaceprops.%03d.stepleft", i );
		ObsString( key, s_pProps->GetString( pData->sounds.stepleft ) );
		V_snprintf( key, sizeof( key ), "surfaceprops.%03d.impacthard", i );
		ObsString( key, s_pProps->GetString( pData->sounds.impactHard ) );
	}
}

//-----------------------------------------------------------------------------
// Object pair hash and collision sets (game-side collision rules)
//-----------------------------------------------------------------------------
static void TestPairHashAndSets()
{
	IPhysicsObjectPairHash *pRealHash = s_pPhysics->CreateObjectPairHash();
	if ( !Check( TIER_BOOT, "pairhash.create", pRealHash != NULL ) )
		return;
	IPhysicsObjectPairHash *pHash = pRealHash;
	CFaultPairHash faultHash( pRealHash );
	if ( FaultIs( "pairhash-ordered" ) )
		pHash = &faultHash;

	int a, b, c;
	pHash->AddObjectPair( &a, &b );
	Check( TIER_BOOT, "pairhash.symmetric-lookup",
		pHash->IsObjectPairInHash( &a, &b ) && pHash->IsObjectPairInHash( &b, &a ) );
	Check( TIER_BOOT, "pairhash.absent-pair", !pHash->IsObjectPairInHash( &a, &c ) );
	pHash->AddObjectPair( &a, &c );
	Check( TIER_BOOT, "pairhash.pair-count", pHash->GetPairCountForObject( &a ) == 2 && pHash->GetPairCountForObject( &b ) == 1,
		"a %d b %d", pHash->GetPairCountForObject( &a ), pHash->GetPairCountForObject( &b ) );
	void *list[4] = {};
	int listed = pHash->GetPairListForObject( &a, 4, list );
	bool hasB = false, hasC = false;
	for ( int i = 0; i < listed; i++ )
	{
		hasB |= list[i] == &b;
		hasC |= list[i] == &c;
	}
	Check( TIER_BOOT, "pairhash.pair-list", listed == 2 && hasB && hasC, "listed %d", listed );
	Check( TIER_BOOT, "pairhash.object-in-hash", pHash->IsObjectInHash( &a ) && pHash->IsObjectInHash( &c ) );
	pHash->RemoveObjectPair( &b, &a );
	Check( TIER_BOOT, "pairhash.remove-either-order", !pHash->IsObjectPairInHash( &a, &b ) && pHash->IsObjectPairInHash( &a, &c ) );
	pHash->RemoveAllPairsForObject( &a );
	Check( TIER_BOOT, "pairhash.remove-all", !pHash->IsObjectPairInHash( &a, &c ) && pHash->GetPairCountForObject( &a ) == 0 );
	s_pPhysics->DestroyObjectPairHash( pRealHash );

	IPhysicsCollisionSet *pSet = s_pPhysics->FindOrCreateCollisionSet( 0xC0FFEE, 32 );
	Check( TIER_BOOT, "collisionset.create", pSet != NULL );
	if ( !pSet )
		return;
	Check( TIER_BOOT, "collisionset.find-or-create-stable", s_pPhysics->FindOrCreateCollisionSet( 0xC0FFEE, 32 ) == pSet );
	Check( TIER_BOOT, "collisionset.find", s_pPhysics->FindCollisionSet( 0xC0FFEE ) == pSet && !s_pPhysics->FindCollisionSet( 0xBADF00D ) );
	// Ragdoll sets start with every pair disabled; the model's authored
	// "collisionpair" entries enable the few limbs that may touch.
	Check( TIER_BOOT, "collisionset.default-disabled", !pSet->ShouldCollide( 1, 2 ) && !pSet->ShouldCollide( 0, 31 ) );
	pSet->EnableCollisions( 1, 2 );
	Check( TIER_BOOT, "collisionset.enable-symmetric", pSet->ShouldCollide( 1, 2 ) && pSet->ShouldCollide( 2, 1 ) && !pSet->ShouldCollide( 1, 3 ) );
	pSet->DisableCollisions( 2, 1 );
	Check( TIER_BOOT, "collisionset.disable", !pSet->ShouldCollide( 1, 2 ) && !pSet->ShouldCollide( 2, 1 ) );
	s_pPhysics->DestroyAllCollisionSets();
	Check( TIER_BOOT, "collisionset.destroy-all", s_pPhysics->FindCollisionSet( 0xC0FFEE ) == NULL );
}

//-----------------------------------------------------------------------------
// Collision models
//-----------------------------------------------------------------------------
static void TraceRay( const CPhysCollide *pCollide, const Vector &origin, const QAngle &angles,
	const Vector &start, const Vector &end, const Vector &extent, trace_t *pTrace )
{
	Ray_t ray;
	if ( extent == vec3_origin )
		ray.Init( start, end );
	else
		ray.Init( start, end, -extent, extent );
	ResetTrace( pTrace );
	s_pCollision->TraceBox( ray, pCollide, origin, angles, pTrace );
}

static void TestBoxCollide()
{
	const Vector mins( -10, -5, -2 ), maxs( 10, 5, 2 );
	CPhysCollide *pBox = s_pCollision->BBoxToCollide( mins, maxs );
	if ( !Check( TIER_BOOT, "collide.bbox-create", pBox != NULL ) )
		return;

	const Vector origin( 100, 0, 0 );
	const QAngle identity( 0, 0, 0 ), yaw90( 0, 90, 0 );
	Vector aMins, aMaxs;
	s_pCollision->CollideGetAABB( &aMins, &aMaxs, pBox, vec3_origin, identity );
	Check( TIER_GAMEPLAY, "collide.bbox-aabb", NearVec( aMins, mins, 0.5f ) && NearVec( aMaxs, maxs, 0.5f ),
		"(%.2f %.2f %.2f)-(%.2f %.2f %.2f)", aMins.x, aMins.y, aMins.z, aMaxs.x, aMaxs.y, aMaxs.z );
	s_pCollision->CollideGetAABB( &aMins, &aMaxs, pBox, origin, yaw90 );
	Check( TIER_GAMEPLAY, "collide.bbox-aabb-rotated",
		NearVec( aMins, origin + Vector( -5, -10, -2 ), 0.5f ) && NearVec( aMaxs, origin + Vector( 5, 10, 2 ), 0.5f ),
		"(%.2f %.2f %.2f)-(%.2f %.2f %.2f)", aMins.x, aMins.y, aMins.z, aMaxs.x, aMaxs.y, aMaxs.z );

	Vector extent = s_pCollision->CollideGetExtent( pBox, origin, identity, Vector( 1, 0, 0 ) );
	Check( TIER_GAMEPLAY, "collide.bbox-extent", Near( extent.x, 110.0f, 0.5f ), "x %.2f", extent.x );
	extent = s_pCollision->CollideGetExtent( pBox, origin, yaw90, Vector( 1, 0, 0 ) );
	Check( TIER_GAMEPLAY, "collide.bbox-extent-rotated", Near( extent.x, 105.0f, 0.5f ), "x %.2f", extent.x );
	extent = s_pCollision->CollideGetExtent( pBox, origin, identity, Vector( 0, 0, -1 ) );
	Check( TIER_GAMEPLAY, "collide.bbox-extent-down", Near( extent.z, -2.0f, 0.5f ), "z %.2f", extent.z );

	float volume = s_pCollision->CollideVolume( pBox );
	Check( TIER_GAMEPLAY, "collide.bbox-volume", Near( volume, 800.0f, 40.0f ), "%.2f", volume );

	// Traces (33 engine/game call sites; SOLID_VPHYSICS player/bullet/portal
	// placement collision all route through TraceBox).
	trace_t tr;
	TraceRay( pBox, origin, identity, Vector( 0, 0, 0 ), Vector( 200, 0, 0 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-hit",
		Near( tr.fraction, 0.45f, 0.01f ) && !tr.startsolid && Near( tr.plane.normal.x, -1.0f, 0.01f ) && Near( tr.endpos.x, 90.0f, 1.0f ),
		"fraction %.3f normal (%.2f %.2f %.2f) end x %.2f startsolid %d", tr.fraction,
		tr.plane.normal.x, tr.plane.normal.y, tr.plane.normal.z, tr.endpos.x, tr.startsolid );
	printf( "OBS trace.ray-hit.contents x %d\n", tr.contents );

	TraceRay( pBox, origin, identity, Vector( 0, 50, 0 ), Vector( 200, 50, 0 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-miss", tr.fraction == 1.0f && !tr.startsolid && Near( tr.endpos.x, 200.0f, 0.01f ),
		"fraction %.3f end x %.2f", tr.fraction, tr.endpos.x );

	TraceRay( pBox, origin, identity, Vector( 0, 0, 0 ), Vector( 50, 0, 0 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-short", tr.fraction == 1.0f, "fraction %.3f", tr.fraction );

	TraceRay( pBox, origin, identity, Vector( 100, 0, 0 ), Vector( 300, 0, 0 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-start-inside", tr.startsolid, "startsolid %d fraction %.3f", tr.startsolid, tr.fraction );

	TraceRay( pBox, origin, identity, Vector( 0, 0, 0 ), Vector( 200, 0, 0 ), Vector( 2, 2, 2 ), &tr );
	Check( TIER_GAMEPLAY, "trace.box-hit", Near( tr.fraction, 0.44f, 0.01f ) && !tr.startsolid,
		"fraction %.3f", tr.fraction );

	TraceRay( pBox, origin, yaw90, Vector( 0, 0, 0 ), Vector( 200, 0, 0 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-hit-rotated", Near( tr.fraction, 0.475f, 0.01f ), "fraction %.3f", tr.fraction );

	TraceRay( pBox, origin, identity, Vector( 100, 0, 50 ), Vector( 100, 0, -50 ), vec3_origin, &tr );
	Check( TIER_GAMEPLAY, "trace.ray-hit-top", Near( tr.fraction, 0.48f, 0.01f ) && Near( tr.plane.normal.z, 1.0f, 0.01f ),
		"fraction %.3f normal z %.2f", tr.fraction, tr.plane.normal.z );

	CPhysCollide *pSmall = s_pCollision->BBoxToCollide( Vector( -2, -2, -2 ), Vector( 2, 2, 2 ) );
	ResetTrace( &tr );
	s_pCollision->TraceCollide( Vector( 0, 0, 0 ), Vector( 200, 0, 0 ), pSmall, identity, pBox, origin, identity, &tr );
	Check( TIER_GAMEPLAY, "trace.collide-sweep", Near( tr.fraction, 0.44f, 0.01f ), "fraction %.3f", tr.fraction );
	s_pCollision->DestroyCollide( pSmall );

	// Convex construction paths used by the engine and tools.
	CPhysConvex *pConvex = s_pCollision->BBoxToConvex( mins, maxs );
	CPhysCollide *pFromConvex = pConvex ? s_pCollision->ConvertConvexToCollide( &pConvex, 1 ) : NULL;
	if ( Check( TIER_GAMEPLAY, "collide.bbox-convex", pFromConvex != NULL ) )
	{
		s_pCollision->CollideGetAABB( &aMins, &aMaxs, pFromConvex, vec3_origin, identity );
		Check( TIER_GAMEPLAY, "collide.bbox-convex-aabb", NearVec( aMins, mins, 0.5f ) && NearVec( aMaxs, maxs, 0.5f ) );
		s_pCollision->DestroyCollide( pFromConvex );
	}

	Vector corners[8];
	Vector *pCorners[8];
	for ( int i = 0; i < 8; i++ )
	{
		corners[i] = Vector( ( i & 1 ) ? maxs.x : mins.x, ( i & 2 ) ? maxs.y : mins.y, ( i & 4 ) ? maxs.z : mins.z );
		pCorners[i] = &corners[i];
	}
	CPhysConvex *pHull = s_pCollision->ConvexFromVerts( pCorners, 8 );
	Check( TIER_GAMEPLAY, "collide.convex-from-verts", pHull != NULL );
	if ( pHull )
	{
		Check( TIER_GAMEPLAY, "collide.convex-volume", Near( s_pCollision->ConvexVolume( pHull ), 800.0f, 40.0f ),
			"%.2f", s_pCollision->ConvexVolume( pHull ) );
		CPhysCollide *pHullCollide = s_pCollision->ConvertConvexToCollide( &pHull, 1 );
		if ( Check( TIER_GAMEPLAY, "collide.convex-to-collide", pHullCollide != NULL ) )
		{
			TraceRay( pHullCollide, origin, identity, Vector( 0, 0, 0 ), Vector( 200, 0, 0 ), vec3_origin, &tr );
			Check( TIER_GAMEPLAY, "trace.convex-hull-hit", Near( tr.fraction, 0.45f, 0.01f ), "fraction %.3f", tr.fraction );
			s_pCollision->DestroyCollide( pHullCollide );
		}
	}

	ICollisionQuery *pQuery = s_pCollision->CreateQueryModel( pBox );
	if ( Check( TIER_GAMEPLAY, "collide.query-model", pQuery != NULL ) )
	{
		int convexes = pQuery->ConvexCount();
		int triangles = convexes > 0 ? pQuery->TriangleCount( 0 ) : 0;
		Check( TIER_GAMEPLAY, "collide.query-model-box", convexes == 1 && triangles == 12, "convexes %d triangles %d", convexes, triangles );
		s_pCollision->DestroyQueryModel( pQuery );
	}

	Vector *pVerts = NULL;
	int vertCount = s_pCollision->CreateDebugMesh( pBox, &pVerts );
	bool inside = vertCount > 0 && ( vertCount % 3 ) == 0;
	for ( int i = 0; inside && i < vertCount; i++ )
		inside = pVerts[i].WithinAABox( mins - Vector( 1, 1, 1 ), maxs + Vector( 1, 1, 1 ) );
	Check( TIER_GAMEPLAY, "collide.debug-mesh", inside, "verts %d", vertCount );
	if ( pVerts )
		s_pCollision->DestroyDebugMesh( vertCount, pVerts );

	s_pCollision->DestroyCollide( pBox );
}

//-----------------------------------------------------------------------------
// Portal collision paths: portal holes and the static-prop collision cache
// convert between CPolyhedron and convex pieces
// (game/shared/portal/PortalSimulation.cpp, StaticCollisionPolyhedronCache.cpp).
//-----------------------------------------------------------------------------
static void TestPortalPolyhedra()
{
	// Outward-facing planes of the box (-10,-5,-2)-(10,5,2).
	float planes[6 * 4] = {
		1, 0, 0, 10,	-1, 0, 0, 10,
		0, 1, 0, 5,		0, -1, 0, 5,
		0, 0, 1, 2,		0, 0, -1, 2 };
	CPolyhedron *pPolyhedron = GeneratePolyhedronFromPlanes( planes, 6, 0.01f );
	if ( !Check( TIER_GAMEPLAY, "portal.fixture-polyhedron", pPolyhedron && pPolyhedron->iVertexCount == 8 ) )
		return;

	CPhysConvex *pConvex = s_pCollision->ConvexFromConvexPolyhedron( *pPolyhedron );
	pPolyhedron->Release();
	if ( !Check( TIER_GAMEPLAY, "portal.convex-from-polyhedron", pConvex != NULL ) )
		return;
	Check( TIER_GAMEPLAY, "portal.convex-from-polyhedron-volume", Near( s_pCollision->ConvexVolume( pConvex ), 800.0f, 40.0f ),
		"%.2f", s_pCollision->ConvexVolume( pConvex ) );

	// The face partition is unspecified (IVP returns the box as 12 triangles);
	// the vertex set must be the box's corners.
	CPolyhedron *pBack = s_pCollision->PolyhedronFromConvex( pConvex, false );
	bool corners = pBack && pBack->iVertexCount == 8 && pBack->iPolygonCount >= 6;
	for ( int i = 0; corners && i < pBack->iVertexCount; i++ )
	{
		const Vector &v = pBack->pVertices[i];
		corners = Near( fabsf( v.x ), 10.0f, 0.05f ) && Near( fabsf( v.y ), 5.0f, 0.05f ) && Near( fabsf( v.z ), 2.0f, 0.05f );
	}
	Check( TIER_GAMEPLAY, "portal.polyhedron-from-convex", corners,
		"vertices %d polygons %d", pBack ? pBack->iVertexCount : -1, pBack ? pBack->iPolygonCount : -1 );
	if ( pBack )
		pBack->Release();
	s_pCollision->ConvexFree( pConvex );
}

struct PhyFixture_t
{
	const char *pPath;
	vcollide_t collide;
	bool loaded;
};

static void TestPhyFixture( PhyFixture_t &fixture, int fixtureIndex )
{
	fixture.loaded = false;
	int size = 0;
	char *pData = ReadFile( fixture.pPath, &size );
	if ( !Check( TIER_BOOT, "vcollide.read-fixture", pData != NULL && size > (int)sizeof( phyheader_t ), "%s", fixture.pPath ) )
		return;
	phyheader_t header;
	memcpy( &header, pData, sizeof( header ) );
	Check( TIER_BOOT, "vcollide.fixture-header", header.size == sizeof( header ) && header.solidCount > 0, "%s", fixture.pPath );

	const char *pBuffer = pData + sizeof( header );
	int bufferSize = size - (int)sizeof( header );

	// Independent layout walk: each solid is length-prefixed; the remainder is
	// the authored keyvalue text.
	int position = 0;
	for ( int i = 0; i < header.solidCount && position + 4 <= bufferSize; i++ )
	{
		int solidSize;
		memcpy( &solidSize, pBuffer + position, sizeof( int ) );
		position += sizeof( int ) + solidSize;
	}
	const char *pExpectedText = pBuffer + position;

	memset( &fixture.collide, 0, sizeof( fixture.collide ) );
	s_pCollision->VCollideLoad( &fixture.collide, header.solidCount, pBuffer, bufferSize );
	vcollide_t &vc = fixture.collide;
	bool solids = vc.solidCount == header.solidCount && vc.solids != NULL;
	for ( int i = 0; solids && i < vc.solidCount; i++ )
		solids = vc.solids[i] != NULL;
	Check( TIER_BOOT, "vcollide.load-solids", solids, "%s: count %d expected %d", fixture.pPath, vc.solidCount, header.solidCount );
	Check( TIER_BOOT, "vcollide.load-keyvalues",
		vc.pKeyValues && !strncmp( vc.pKeyValues, pExpectedText, bufferSize - position ) && vc.pKeyValues[0],
		"%s", fixture.pPath );
	fixture.loaded = solids;

	char key[128];
	V_snprintf( key, sizeof( key ), "vcollide.%d.solid-count", fixtureIndex );
	printf( "OBS %s x %d\n", key, vc.solidCount );

	// Authored keyvalues through the shared parser: every block, and every
	// solid's parsed fields, compared exactly across providers.
	if ( vc.pKeyValues )
	{
		IVPhysicsKeyParser *pParser = s_pCollision->VPhysicsKeyParserCreate( vc.pKeyValues );
		int blocks = 0, solidBlocks = 0, ragdollBlocks = 0;
		bool solidsValid = true, ragdollValid = true;
		while ( pParser && !pParser->Finished() && blocks < 256 )
		{
			const char *pBlock = pParser->GetCurrentBlockName();
			V_snprintf( key, sizeof( key ), "keyparser.%d.block.%d", fixtureIndex, blocks );
			ObsString( key, pBlock );
			if ( !V_stricmp( pBlock, "solid" ) )
			{
				solid_t solid;
				memset( &solid, 0, sizeof( solid ) );
				pParser->ParseSolid( &solid, NULL );
				solidsValid &= solid.index >= 0 && solid.index < vc.solidCount && solid.params.mass > 0 && solid.surfaceprop[0];
				V_snprintf( key, sizeof( key ), "keyparser.%d.solid.%d", fixtureIndex, solidBlocks );
				float values[] = { (float)solid.index, solid.params.mass, solid.params.damping, solid.params.rotdamping, solid.params.inertia, solid.params.volume };
				ObsFloats( key, "a0.0001", ARRAYSIZE( values ), values );
				V_snprintf( key, sizeof( key ), "keyparser.%d.solid.%d.surfaceprop", fixtureIndex, solidBlocks );
				ObsString( key, solid.surfaceprop );
				solidBlocks++;
			}
			else if ( !V_stricmp( pBlock, "ragdollconstraint" ) )
			{
				constraint_ragdollparams_t ragdoll;
				ragdoll.Defaults();
				pParser->ParseRagdollConstraint( &ragdoll, NULL );
				ragdollValid &= ragdoll.parentIndex >= 0 && ragdoll.parentIndex < vc.solidCount &&
					ragdoll.childIndex >= 0 && ragdoll.childIndex < vc.solidCount;
				V_snprintf( key, sizeof( key ), "keyparser.%d.ragdoll.%d", fixtureIndex, ragdollBlocks );
				float values[] = { (float)ragdoll.parentIndex, (float)ragdoll.childIndex,
					ragdoll.axes[0].minRotation, ragdoll.axes[0].maxRotation, ragdoll.axes[0].torque,
					ragdoll.axes[1].minRotation, ragdoll.axes[1].maxRotation, ragdoll.axes[1].torque,
					ragdoll.axes[2].minRotation, ragdoll.axes[2].maxRotation, ragdoll.axes[2].torque };
				ObsFloats( key, "a0.0001", ARRAYSIZE( values ), values );
				ragdollBlocks++;
			}
			else
			{
				pParser->SkipBlock();
			}
			blocks++;
		}
		Check( TIER_BOOT, "keyparser.solid-blocks", pParser && solidBlocks == vc.solidCount && solidsValid,
			"%s: %d solid blocks for %d solids", fixture.pPath, solidBlocks, vc.solidCount );
		// A ragdoll joins its solids into one tree: solidCount - 1 constraints.
		Check( TIER_BOOT, "keyparser.ragdoll-constraints",
			ragdollValid && ( vc.solidCount == 1 ? ragdollBlocks == 0 : ragdollBlocks == vc.solidCount - 1 ),
			"%s: %d ragdoll constraints for %d solids", fixture.pPath, ragdollBlocks, vc.solidCount );
		if ( pParser )
			s_pCollision->VPhysicsKeyParserDestroy( pParser );
	}

	// Decoded geometry per solid (RFC 0004 B3 legacy geometry decoder).
	for ( int s = 0; fixture.loaded && s < vc.solidCount; s++ )
	{
		CPhysCollide *pSolid = vc.solids[s];
		Vector mins, maxs;
		s_pCollision->CollideGetAABB( &mins, &maxs, pSolid, vec3_origin, vec3_angle );
		Vector size3 = maxs - mins;
		bool sane = IsFiniteVec( mins ) && IsFiniteVec( maxs ) && size3.x > 0.1f && size3.y > 0.1f && size3.z > 0.1f
			&& size3.x < 4096 && size3.y < 4096 && size3.z < 4096;
		Check( TIER_GAMEPLAY, "vcollide.solid-aabb", sane, "%s solid %d (%.2f %.2f %.2f)-(%.2f %.2f %.2f)", fixture.pPath, s,
			mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.aabb-mins", fixtureIndex, s );
		ObsVector( key, "a0.5", mins );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.aabb-maxs", fixtureIndex, s );
		ObsVector( key, "a0.5", maxs );

		float volume = s_pCollision->CollideVolume( pSolid );
		Check( TIER_GAMEPLAY, "vcollide.solid-volume", volume > 0 && volume <= size3.x * size3.y * size3.z * 1.01f,
			"%s solid %d volume %.2f", fixture.pPath, s, volume );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.volume", fixtureIndex, s );
		ObsFloats( key, "r0.05", 1, &volume );

		Vector massCenter;
		s_pCollision->CollideGetMassCenter( pSolid, &massCenter );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.mass-center", fixtureIndex, s );
		ObsVector( key, "a0.5", massCenter );

		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.collide-index", fixtureIndex, s );
		printf( "OBS %s x %d\n", key, s_pCollision->CollideIndex( pSolid ) );

		ICollisionQuery *pQuery = s_pCollision->CreateQueryModel( pSolid );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.convex-count", fixtureIndex, s );
		printf( "OBS %s x %d\n", key, pQuery ? pQuery->ConvexCount() : -1 );
		if ( pQuery )
			s_pCollision->DestroyQueryModel( pQuery );

		// A ray dropped from above the AABB onto an upward-facing surface of
		// the solid hits it. The aim point comes from the solid's own triangles
		// (the AABB center can fall in a hole, e.g. the button base ring).
		Vector *pVerts = NULL;
		int vertCount = s_pCollision->CreateDebugMesh( pSolid, &pVerts );
		bool inside = vertCount > 0 && ( vertCount % 3 ) == 0;
		for ( int i = 0; inside && i < vertCount; i++ )
			inside = pVerts[i].WithinAABox( mins - Vector( 1, 1, 1 ), maxs + Vector( 1, 1, 1 ) );
		Check( TIER_GAMEPLAY, "vcollide.solid-debug-mesh", inside, "%s solid %d verts %d", fixture.pPath, s, vertCount );

		// Aim at the highest upward-facing triangle so nothing else occludes it.
		Vector aim = ( mins + maxs ) * 0.5f;
		float bestZ = -1e30f;
		for ( int i = 0; inside && i + 2 < vertCount; i += 3 )
		{
			Vector normal = CrossProduct( pVerts[i + 1] - pVerts[i], pVerts[i + 2] - pVerts[i] );
			if ( VectorNormalize( normal ) < 1e-3f || fabsf( normal.z ) < 0.7f )
				continue;
			Vector centroid = ( pVerts[i] + pVerts[i + 1] + pVerts[i + 2] ) / 3.0f;
			if ( centroid.z > bestZ )
			{
				bestZ = centroid.z;
				aim = centroid;
			}
		}
		if ( pVerts )
			s_pCollision->DestroyDebugMesh( vertCount, pVerts );

		trace_t tr;
		TraceRay( pSolid, vec3_origin, vec3_angle, Vector( aim.x, aim.y, maxs.z + 64 ), Vector( aim.x, aim.y, mins.z - 64 ), vec3_origin, &tr );
		Check( TIER_GAMEPLAY, "vcollide.solid-trace-down", tr.fraction > 0 && tr.fraction < 1 && tr.plane.normal.z > 0.2f && tr.endpos.z <= maxs.z + 0.5f,
			"%s solid %d fraction %.3f normal z %.2f", fixture.pPath, s, tr.fraction, tr.plane.normal.z );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.trace-down-z", fixtureIndex, s );
		ObsFloats( key, "a0.5", 1, &tr.endpos.z );

		TraceRay( pSolid, vec3_origin, vec3_angle, Vector( maxs.x + 64, maxs.y + 64, maxs.z + 64 ), Vector( maxs.x + 200, maxs.y + 64, maxs.z + 64 ), vec3_origin, &tr );
		Check( TIER_GAMEPLAY, "vcollide.solid-trace-miss", tr.fraction == 1.0f, "%s solid %d fraction %.3f", fixture.pPath, s, tr.fraction );

		// A player-sized hull (16x16x36 half extents) walking into the solid
		// placed with a 30 degree yaw, as the engine clips player movement
		// against SOLID_VPHYSICS props. The sweep is movement-sized: IVP misses
		// some long (~300 unit) tall-box sweeps against rotated boxes that it
		// hits when shorter, a legacy defect this contract does not require.
		const QAngle yaw30( 0, 30, 0 );
		Vector rotatedMins, rotatedMaxs;
		s_pCollision->CollideGetAABB( &rotatedMins, &rotatedMaxs, pSolid, vec3_origin, yaw30 );
		Vector middle = ( rotatedMins + rotatedMaxs ) * 0.5f;
		Vector hullStart( rotatedMins.x - 40, middle.y, middle.z );
		Vector hullEnd( rotatedMaxs.x + 40, middle.y, middle.z );
		TraceRay( pSolid, vec3_origin, yaw30, hullStart, hullEnd, Vector( 16, 16, 36 ), &tr );
		Check( TIER_GAMEPLAY, "vcollide.solid-player-hull", tr.fraction > 0 && tr.fraction < 1 && !tr.startsolid && tr.plane.normal.x < 0,
			"%s solid %d fraction %.3f normal x %.2f", fixture.pPath, s, tr.fraction, tr.plane.normal.x );
		V_snprintf( key, sizeof( key ), "vcollide.%d.%d.player-hull-x", fixtureIndex, s );
		ObsFloats( key, "a0.75", 1, &tr.endpos.x );
	}

	// Every solid as the game creates it (authored solid block: mass,
	// inertia scale, damping, surface property), dropped onto the floor:
	// its inertia follows the model, and it stays finite and comes to rest.
	if ( fixture.loaded && vc.pKeyValues )
	{
		World_t world;
		CreateWorld( world, NULL );
		CUtlVector<IPhysicsObject *> objects;
		objects.SetCount( vc.solidCount );
		for ( int i = 0; i < vc.solidCount; i++ )
			objects[i] = NULL;
		IVPhysicsKeyParser *pParser = s_pCollision->VPhysicsKeyParserCreate( vc.pKeyValues );
		while ( pParser && !pParser->Finished() )
		{
			if ( V_stricmp( pParser->GetCurrentBlockName(), "solid" ) )
			{
				pParser->SkipBlock();
				continue;
			}
			solid_t solid;
			memset( &solid, 0, sizeof( solid ) );
			pParser->ParseSolid( &solid, NULL );
			if ( solid.index < 0 || solid.index >= vc.solidCount || objects[solid.index] )
				continue;
			objectparams_t params = DefaultParams( solid.params.mass > 0 ? solid.params.mass : 1.0f, NULL );
			params.inertia = solid.params.inertia > 0 ? solid.params.inertia : 1.0f;
			params.damping = solid.params.damping;
			params.rotdamping = solid.params.rotdamping;
			params.volume = solid.params.volume;
			int material = s_pProps->GetSurfaceIndex( solid.surfaceprop );
			objects[solid.index] = world.pEnv->CreatePolyObject( vc.solids[solid.index], material >= 0 ? material : 0,
				Vector( ( solid.index % 6 ) * 150.0f - 375.0f, ( solid.index / 6 ) * 150.0f - 225.0f, 200 ), QAngle( 10, 20, 5 ), &params );
			if ( objects[solid.index] )
				objects[solid.index]->Wake();
		}
		if ( pParser )
			s_pCollision->VPhysicsKeyParserDestroy( pParser );

		for ( int i = 0; i < vc.solidCount; i++ )
		{
			if ( !objects[i] )
				continue;
			V_snprintf( key, sizeof( key ), "vcollide.%d.%d.object-inertia", fixtureIndex, i );
			ObsVector( key, "r0.1", objects[i]->GetInertia() );
		}
		Step( world.pEnv, 5.0f );
		bool finite = true, rested = true;
		int created = 0;
		for ( int i = 0; i < vc.solidCount; i++ )
		{
			if ( !objects[i] )
				continue;
			created++;
			Vector position = PositionOf( objects[i] );
			finite &= IsFiniteVec( position ) && IsFiniteVec( VelocityOf( objects[i] ) );
			// Loose ragdoll limbs may still roll; single models must settle.
			bool restedHere = position.z > -1.0f && position.z < 150.0f &&
				( vc.solidCount > 1 || VelocityOf( objects[i] ).Length() < 20.0f );
			rested &= restedHere;
		}
		Check( TIER_GAMEPLAY, "vcollide.model-simulates", created == vc.solidCount && finite && rested,
			"%s created %d finite %d rested %d", fixture.pPath, created, finite, rested );
		DestroyWorld( world );
	}
	free( pData );
}

//-----------------------------------------------------------------------------
// Corpus sweep: every shipped collision model decodes to sane geometry. The
// per-solid observations are compared with the oracle by the runner, so a
// crash, empty solid, or divergent shape on any model fails the gate.
//-----------------------------------------------------------------------------
struct CorpusTally_t
{
	CorpusTally_t() : total( 0 ), failed( 0 ) { first[0] = '\0'; }
	void Record( bool ok, const char *pWhat )
	{
		total++;
		if ( ok )
			return;
		if ( !failed )
			V_strncpy( first, pWhat, sizeof( first ) );
		failed++;
	}
	int total, failed;
	char first[256];
};

static void TestCorpus( const char *pListFile )
{
	FILE *fp = fopen( pListFile, "r" );
	if ( !Check( TIER_BOOT, "corpus.list", fp != NULL, "%s", pListFile ) )
		return;
	CorpusTally_t loads, extents, volumes, traces;
	char line[1024];
	int models = 0, flatSolids = 0;
	while ( fgets( line, sizeof( line ), fp ) )
	{
		char *pTab = strchr( line, '\t' );
		if ( !pTab )
			continue;
		*pTab = '\0';
		char *pPath = pTab + 1;
		pPath[strcspn( pPath, "\r\n" )] = '\0';
		const char *pName = line;

		int size = 0;
		char *pData = ReadFile( pPath, &size );
		phyheader_t header;
		if ( !pData || size < (int)sizeof( header ) )
		{
			loads.Record( false, pName );
			free( pData );
			continue;
		}
		memcpy( &header, pData, sizeof( header ) );
		if ( header.size != sizeof( header ) || header.solidCount <= 0 )
		{
			free( pData );
			continue;	// not a loadable model; the engine skips it too
		}
		models++;
		vcollide_t vc;
		memset( &vc, 0, sizeof( vc ) );
		s_pCollision->VCollideLoad( &vc, header.solidCount, pData + sizeof( header ), size - (int)sizeof( header ) );
		loads.Record( vc.solidCount == header.solidCount && vc.solids != NULL, pName );
		for ( int s = 0; vc.solids && s < vc.solidCount; s++ )
		{
			CPhysCollide *pSolid = vc.solids[s];
			char what[512];
			V_snprintf( what, sizeof( what ), "%s solid %d", pName, s );
			Vector mins, maxs;
			s_pCollision->CollideGetAABB( &mins, &maxs, pSolid, vec3_origin, vec3_angle );
			Vector size3 = maxs - mins;
			float volume = s_pCollision->CollideVolume( pSolid );
			// Shipped content includes flat solids (cardboard gibs, wing
			// planes): zero volume or under a quarter unit thick. Their bounds
			// and volume are still compared; IVP's traces against them are
			// erratic, so they are excluded from the solid-shape checks.
			bool flat = volume <= 0.0f || size3.x < 0.25f || size3.y < 0.25f || size3.z < 0.25f;
			flatSolids += flat ? 1 : 0;
			if ( !flat )
			{
				extents.Record( IsFiniteVec( mins ) && IsFiniteVec( maxs ), what );
				// Authored convex pieces may overlap, so the summed volume can
				// exceed the bounding box; only finiteness is a shape property.
				volumes.Record( volume > 0.0f && volume < 1e12f, what );
			}

			char key[600];
			V_snprintf( key, sizeof( key ), "corpus.%s.%d.aabb", pName, s );
			float box[6] = { mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z };
			ObsFloats( key, "a0.5", 6, box );
			V_snprintf( key, sizeof( key ), "corpus.%s.%d.volume", pName, s );
			// Tiny solids get an absolute allowance for tolerance-scale effects.
			printf( "OBS %s %s %.4f\n", key, volume < 64.0f ? "a4" : "r0.05", volume );

			// Straight down through the center of the highest upward face.
			Vector *pVerts = NULL;
			int vertCount = s_pCollision->CreateDebugMesh( pSolid, &pVerts );
			Vector aim = ( mins + maxs ) * 0.5f;
			float bestZ = -1e30f;
			for ( int i = 0; i + 2 < vertCount; i += 3 )
			{
				Vector normal = CrossProduct( pVerts[i + 1] - pVerts[i], pVerts[i + 2] - pVerts[i] );
				if ( VectorNormalize( normal ) < 1e-3f || fabsf( normal.z ) < 0.7f )
					continue;
				Vector centroid = ( pVerts[i] + pVerts[i + 1] + pVerts[i + 2] ) / 3.0f;
				if ( centroid.z > bestZ )
				{
					bestZ = centroid.z;
					aim = centroid;
				}
			}
			if ( pVerts )
				s_pCollision->DestroyDebugMesh( vertCount, pVerts );
			if ( flat )
				continue;
			trace_t tr;
			TraceRay( pSolid, vec3_origin, vec3_angle, Vector( aim.x, aim.y, maxs.z + 16 ), Vector( aim.x, aim.y, mins.z - 16 ), vec3_origin, &tr );
			traces.Record( tr.fraction > 0 && tr.fraction < 1 && !tr.startsolid, what );
			V_snprintf( key, sizeof( key ), "corpus.%s.%d.trace-down-z", pName, s );
			ObsFloats( key, "a0.75", 1, &tr.endpos.z );
		}
		s_pCollision->VCollideUnload( &vc );
		free( pData );
	}
	fclose( fp );
	Check( TIER_BOOT, "corpus.models", models > 0, "no models listed" );
	Check( TIER_BOOT, "corpus.load", loads.failed == 0, "%d/%d failed, first %s", loads.failed, loads.total, loads.first );
	Check( TIER_GAMEPLAY, "corpus.solid-aabb", extents.failed == 0, "%d/%d failed, first %s", extents.failed, extents.total, extents.first );
	Check( TIER_GAMEPLAY, "corpus.solid-volume", volumes.failed == 0, "%d/%d failed, first %s", volumes.failed, volumes.total, volumes.first );
	Check( TIER_GAMEPLAY, "corpus.solid-trace-down", traces.failed == 0, "%d/%d failed, first %s", traces.failed, traces.total, traces.first );
	printf( "OBS corpus.flat-solids x %d\n", flatSolids );
	printf( "CORPUS %d models %d solids (%d flat)\n", models, extents.total + flatSolids, flatSolids );
}

//-----------------------------------------------------------------------------
// Environment, objects, controllers, simulation
//-----------------------------------------------------------------------------
class CCollisionEvents : public IPhysicsCollisionEvent
{
public:
	CCollisionEvents() : m_pre( 0 ), m_post( 0 ), m_pWatchA( NULL ), m_pWatchB( NULL ), m_watchedPair( 0 ) {}
	virtual void PreCollision( vcollisionevent_t *pEvent ) { m_pre++; Watch( pEvent ); }
	virtual void PostCollision( vcollisionevent_t *pEvent ) { m_post++; }
	virtual void Friction( IPhysicsObject *, float, int, int, IPhysicsCollisionData * ) {}
	virtual void StartTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void EndTouch( IPhysicsObject *, IPhysicsObject *, IPhysicsCollisionData * ) {}
	virtual void FluidStartTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void FluidEndTouch( IPhysicsObject *, IPhysicsFluidController * ) {}
	virtual void PostSimulationFrame() {}

	void Watch( vcollisionevent_t *pEvent )
	{
		if ( ( pEvent->pObjects[0] == m_pWatchA && pEvent->pObjects[1] == m_pWatchB ) ||
			( pEvent->pObjects[0] == m_pWatchB && pEvent->pObjects[1] == m_pWatchA ) )
			m_watchedPair++;
	}

	int m_pre, m_post;
	IPhysicsObject *m_pWatchA, *m_pWatchB;
	int m_watchedPair;
};

class CObjectEvents : public IPhysicsObjectEvent
{
public:
	CObjectEvents() : m_pWatch( NULL ), m_sleeps( 0 ) {}
	virtual void ObjectWake( IPhysicsObject * ) {}
	virtual void ObjectSleep( IPhysicsObject *pObject ) { if ( pObject == m_pWatch ) m_sleeps++; }
	IPhysicsObject *m_pWatch;
	int m_sleeps;
};

class CFilterSolver : public IPhysicsCollisionSolver
{
public:
	CFilterSolver() : m_pIgnoreA( NULL ), m_pIgnoreB( NULL ), m_calls( 0 ) {}
	virtual int ShouldCollide( IPhysicsObject *p0, IPhysicsObject *p1, void *, void * )
	{
		m_calls++;
		if ( ( p0 == m_pIgnoreA && p1 == m_pIgnoreB ) || ( p0 == m_pIgnoreB && p1 == m_pIgnoreA ) )
			return 0;
		return 1;
	}
	virtual int ShouldSolvePenetration( IPhysicsObject *, IPhysicsObject *, void *, void *, float ) { return 1; }
	virtual bool ShouldFreezeObject( IPhysicsObject * ) { return false; }
	virtual int AdditionalCollisionChecksThisTick( int ) { return 0; }
	virtual bool ShouldFreezeContacts( IPhysicsObject **, int ) { return false; }
	IPhysicsObject *m_pIgnoreA, *m_pIgnoreB;
	int m_calls;
};

class CAntiGravity : public IMotionEvent
{
public:
	CAntiGravity() : m_calls( 0 ) {}
	virtual simresult_e Simulate( IPhysicsMotionController *, IPhysicsObject *, float, Vector &linear, AngularImpulse &angular )
	{
		m_calls++;
		linear = Vector( 0, 0, 600 );
		angular.Init();
		return SIM_GLOBAL_ACCELERATION;
	}
	int m_calls;
};

bool CreateWorld( World_t &world, IPhysicsCollisionSolver *pSolver )
{
	memset( &world, 0, sizeof( world ) );
	world.pEnv = s_pPhysics->CreateEnvironment();
	if ( !world.pEnv )
		return false;
	physics_performanceparams_t perf;
	perf.Defaults();
	world.pEnv->SetPerformanceSettings( &perf );
	world.pEnv->SetSimulationTimestep( kTick );
	world.pEnv->SetGravity( Vector( 0, 0, -600 ) );
	if ( pSolver )
		world.pEnv->SetCollisionSolver( pSolver );
	world.material = s_pProps->GetSurfaceIndex( "metal" );
	if ( world.material < 0 )
		world.material = 0;
	world.pFloorCollide = s_pCollision->BBoxToCollide( Vector( -512, -512, -16 ), Vector( 512, 512, 0 ) );
	world.pCubeCollide = s_pCollision->BBoxToCollide( Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );
	objectparams_t params = DefaultParams( 1.0f, NULL );
	world.pFloor = world.pEnv->CreatePolyObjectStatic( world.pFloorCollide, world.material, vec3_origin, vec3_angle, &params );
	return world.pFloor != NULL;
}

IPhysicsObject *CreateCube( World_t &world, const Vector &position, float mass, void *pGameData )
{
	objectparams_t params = DefaultParams( mass, pGameData );
	IPhysicsObject *pObject = world.pEnv->CreatePolyObject( world.pCubeCollide, world.material, position, vec3_angle, &params );
	if ( pObject )
	{
		pObject->EnableMotion( true );
		pObject->EnableGravity( true );
		pObject->Wake();
	}
	return pObject;
}

void DestroyWorld( World_t &world )
{
	if ( !world.pEnv )
		return;
	s_pPhysics->DestroyEnvironment( world.pEnv );
	s_pCollision->DestroyCollide( world.pFloorCollide );
	s_pCollision->DestroyCollide( world.pCubeCollide );
	memset( &world, 0, sizeof( world ) );
}

Vector PositionOf( IPhysicsObject *pObject )
{
	Vector position;
	QAngle angles;
	pObject->GetPosition( &position, &angles );
	return position;
}

Vector VelocityOf( IPhysicsObject *pObject )
{
	Vector velocity;
	AngularImpulse angular;
	pObject->GetVelocity( &velocity, &angular );
	return velocity;
}

AngularImpulse AngularVelocityOf( IPhysicsObject *pObject )
{
	Vector velocity;
	AngularImpulse angular;
	pObject->GetVelocity( &velocity, &angular );
	return angular;
}

Vector AxisOf( IPhysicsObject *pObject, int axis )
{
	matrix3x4_t matrix;
	pObject->GetPositionMatrix( &matrix );
	Vector out;
	MatrixGetColumn( matrix, axis, out );
	return out;
}

static void TestObjects()
{
	World_t world;
	if ( !Check( TIER_BOOT, "env.create", CreateWorld( world, NULL ) ) )
	{
		DestroyWorld( world );
		return;
	}
	IPhysicsEnvironment *pEnv = world.pEnv;
	Vector gravity;
	pEnv->GetGravity( &gravity );
	Check( TIER_BOOT, "env.gravity-roundtrip", NearVec( gravity, Vector( 0, 0, -600 ), 1e-3f ) );
	pEnv->SetAirDensity( 3.0f );
	Check( TIER_BOOT, "env.air-density-roundtrip", Near( pEnv->GetAirDensity(), 3.0f, 1e-4f ) );
	pEnv->SetAirDensity( 2.0f );
	Check( TIER_BOOT, "env.static-floor", world.pFloor && world.pFloor->IsStatic() && !world.pFloor->IsMoveable() );

	int gameData = 0;
	const Vector position( 10, 20, 300 );
	const QAngle angles( 0, 45, 0 );
	objectparams_t params = DefaultParams( 50.0f, &gameData );
	IPhysicsObject *pObject = pEnv->CreatePolyObject( world.pCubeCollide, world.material, position, angles, &params );
	if ( !Check( TIER_BOOT, "object.create", pObject != NULL ) )
	{
		DestroyWorld( world );
		return;
	}
	Check( TIER_BOOT, "object.game-data", pObject->GetGameData() == &gameData );
	Check( TIER_BOOT, "object.mass", Near( pObject->GetMass(), 50.0f, 0.01f ) && Near( pObject->GetInvMass(), 1.0f / 50.0f, 1e-4f ),
		"mass %.3f inv %.5f", pObject->GetMass(), pObject->GetInvMass() );
	Check( TIER_BOOT, "object.not-static", !pObject->IsStatic() );
	Check( TIER_BOOT, "object.collide", pObject->GetCollide() == world.pCubeCollide );
	Check( TIER_BOOT, "object.material", pObject->GetMaterialIndex() == world.material );

	Vector gotPosition;
	QAngle gotAngles;
	pObject->GetPosition( &gotPosition, &gotAngles );
	Check( TIER_BOOT, "object.initial-transform", NearVec( gotPosition, position, 0.01f ) && Near( gotAngles.y, 45.0f, 0.1f ),
		"(%.2f %.2f %.2f) yaw %.2f", gotPosition.x, gotPosition.y, gotPosition.z, gotAngles.y );

	matrix3x4_t matrix, expected;
	pObject->GetPositionMatrix( &matrix );
	AngleMatrix( angles, position, expected );
	bool matrixOk = true;
	for ( int r = 0; r < 3; r++ )
		for ( int c = 0; c < 4; c++ )
			matrixOk &= Near( matrix[r][c], expected[r][c], 1e-2f );
	Check( TIER_BOOT, "object.position-matrix", matrixOk );

	Vector world0, local0;
	pObject->LocalToWorld( &world0, Vector( 16, 0, 0 ) );
	pObject->WorldToLocal( &local0, world0 );
	Vector expectedWorld;
	VectorTransform( Vector( 16, 0, 0 ), expected, expectedWorld );
	Check( TIER_BOOT, "object.local-world", NearVec( world0, expectedWorld, 0.05f ) && NearVec( local0, Vector( 16, 0, 0 ), 0.05f ) );

	pObject->SetPosition( Vector( 0, 0, 400 ), vec3_angle, true );
	Check( TIER_BOOT, "object.teleport", NearVec( PositionOf( pObject ), Vector( 0, 0, 400 ), 0.01f ) );

	pObject->SetGameFlags( 0x1234 );
	pObject->SetContents( CONTENTS_SOLID | CONTENTS_MONSTER );
	pObject->SetCallbackFlags( CALLBACK_GLOBAL_COLLISION | CALLBACK_GLOBAL_TOUCH );
	Check( TIER_BOOT, "object.flags-roundtrip", pObject->GetGameFlags() == 0x1234 &&
		pObject->GetContents() == ( CONTENTS_SOLID | CONTENTS_MONSTER ) &&
		( pObject->GetCallbackFlags() & ( CALLBACK_GLOBAL_COLLISION | CALLBACK_GLOBAL_TOUCH ) ) == ( CALLBACK_GLOBAL_COLLISION | CALLBACK_GLOBAL_TOUCH ) );

	int listCount = 0;
	const IPhysicsObject **ppList = pEnv->GetObjectList( &listCount );
	bool listed = false;
	for ( int i = 0; ppList && i < listCount; i++ )
		listed |= ppList[i] == pObject;
	Check( TIER_BOOT, "env.object-list", listCount == 2 && listed, "count %d", listCount );

	IPhysicsObject *pSphere = pEnv->CreateSphereObject( 8.0f, world.material, Vector( 200, 0, 100 ), vec3_angle, &params, false );
	Check( TIER_BOOT, "object.sphere-create", pSphere != NULL );
	if ( pSphere )
	{
		Check( TIER_GAMEPLAY, "object.sphere-radius", Near( pSphere->GetSphereRadius(), 8.0f, 0.1f ), "%.2f", pSphere->GetSphereRadius() );
		pEnv->DestroyObject( pSphere );
	}

	// Shadow controllers drive players, NPCs, and held objects.
	pObject->SetShadow( 1e4f, 1e4f, false, false );
	IPhysicsShadowController *pShadow = pObject->GetShadowController();
	if ( Check( TIER_BOOT, "shadow.set-shadow", pShadow != NULL ) )
	{
		pObject->UpdateShadow( Vector( 50, 0, 400 ), vec3_angle, false, kTick );
		Vector target;
		QAngle targetAngles;
		pShadow->GetTargetPosition( &target, &targetAngles );
		Check( TIER_BOOT, "shadow.target-roundtrip", NearVec( target, Vector( 50, 0, 400 ), 0.01f ) );
		Check( TIER_BOOT, "shadow.allows", !pShadow->AllowsTranslation() && !pShadow->AllowsRotation() );
	}

	IPhysicsObject *pOther = CreateCube( world, Vector( 100, 0, 400 ) );
	IPhysicsShadowController *pEnvShadow = pOther ? pEnv->CreateShadowController( pOther, true, true ) : NULL;
	Check( TIER_BOOT, "shadow.env-create", pEnvShadow != NULL && pEnvShadow->AllowsTranslation() && pEnvShadow->AllowsRotation() );
	if ( pEnvShadow )
		pEnv->DestroyShadowController( pEnvShadow );

	IPhysicsPlayerController *pPlayer = pOther ? pEnv->CreatePlayerController( pOther ) : NULL;
	Check( TIER_BOOT, "player.create", pPlayer != NULL );
	if ( pPlayer )
		pEnv->DestroyPlayerController( pPlayer );

	CAntiGravity handler;
	IPhysicsMotionController *pMotion = pEnv->CreateMotionController( &handler );
	if ( Check( TIER_BOOT, "motion.create", pMotion != NULL ) && pOther )
	{
		pMotion->AttachObject( pOther, true );
		pMotion->AttachObject( pOther, true );
		Check( TIER_BOOT, "motion.attach-once", pMotion->CountObjects() == 1, "%d", pMotion->CountObjects() );
		pMotion->DetachObject( pOther );
		Check( TIER_BOOT, "motion.detach", pMotion->CountObjects() == 0 );
		pEnv->DestroyMotionController( pMotion );
	}

	if ( pOther )
	{
		constraint_fixedparams_t fixed;
		fixed.Defaults();
		fixed.InitWithCurrentObjectState( pObject, pOther );
		IPhysicsConstraint *pConstraint = pEnv->CreateFixedConstraint( pObject, pOther, NULL, fixed );
		if ( Check( TIER_BOOT, "constraint.fixed-create", pConstraint != NULL ) )
		{
			pConstraint->SetGameData( &gameData );
			Check( TIER_BOOT, "constraint.objects", pConstraint->GetReferenceObject() == pObject && pConstraint->GetAttachedObject() == pOther &&
				pConstraint->GetGameData() == &gameData );
			pEnv->DestroyConstraint( pConstraint );
		}
		constraint_groupparams_t groupParams;
		groupParams.Defaults();
		IPhysicsConstraintGroup *pGroup = pEnv->CreateConstraintGroup( groupParams );
		Check( TIER_BOOT, "constraint.group-create", pGroup != NULL );
		if ( pGroup )
			pEnv->DestroyConstraintGroup( pGroup );
		pEnv->DestroyObject( pOther );
	}

	pEnv->DestroyObject( pObject );
	pEnv->GetObjectList( &listCount );
	Check( TIER_BOOT, "env.destroy-object", listCount == 1, "count %d", listCount );

	pEnv->ResetSimulationClock();
	float before = pEnv->GetSimulationTime();
	Step( pEnv, 0.3f );
	Check( TIER_GAMEPLAY, "env.simulation-clock", Near( pEnv->GetSimulationTime() - before, 0.3f, 0.02f ) && !pEnv->IsInSimulation(),
		"advanced %.3f", pEnv->GetSimulationTime() - before );
	DestroyWorld( world );
}

static void TestSimulation()
{
	// Free fall: 0.5 * 600 * 1^2 = 300 units, reduced by damping/drag.
	{
		World_t world;
		CreateWorld( world, NULL );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 2000 ) );
		IPhysicsObject *pFrozen = CreateCube( world, Vector( 200, 0, 2000 ) );
		pFrozen->EnableMotion( false );
		Step( world.pEnv, 0.5f );
		int active = world.pEnv->GetActiveObjectCount();
		bool activeListed = false;
		if ( active > 0 && active < 64 )
		{
			IPhysicsObject *pActive[64];
			world.pEnv->GetActiveObjects( pActive );
			for ( int i = 0; i < active; i++ )
				activeListed |= pActive[i] == pCube;
		}
		// Entities copy physics state back only for active objects.
		Check( TIER_GAMEPLAY, "sim.active-objects", activeListed, "active %d", active );
		Step( world.pEnv, 0.5f );
		float drop = 2000.0f - PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "sim.free-fall", drop > 180.0f && drop < 320.0f, "dropped %.2f", drop );
		Check( TIER_GAMEPLAY, "sim.free-fall-velocity", VelocityOf( pCube ).z < -400.0f, "vz %.2f", VelocityOf( pCube ).z );
		ObsFloats( "sim.free-fall-drop", "r0.25", 1, &drop );
		Check( TIER_GAMEPLAY, "sim.motion-disabled-stays", NearVec( PositionOf( pFrozen ), Vector( 200, 0, 2000 ), 0.01f ) );
		Check( TIER_GAMEPLAY, "sim.static-stays", NearVec( PositionOf( world.pFloor ), vec3_origin, 0.01f ) );
		DestroyWorld( world );
	}

	// Landing, resting contact, impact events, sleep, friction snapshot.
	{
		CCollisionEvents collisions;
		CObjectEvents objects;
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetCollisionEventHandler( &collisions );
		world.pEnv->SetObjectEventHandler( &objects );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 100 ) );
		pCube->SetCallbackFlags( pCube->GetCallbackFlags() | CALLBACK_GLOBAL_COLLISION | CALLBACK_GLOBAL_COLLIDE_STATIC );
		collisions.m_pWatchA = pCube;
		collisions.m_pWatchB = world.pFloor;
		objects.m_pWatch = pCube;
		Step( world.pEnv, 3.0f );
		Vector rest = PositionOf( pCube );
		Check( TIER_GAMEPLAY, "sim.rests-on-floor", rest.z > 14.0f && rest.z < 19.0f && VelocityOf( pCube ).Length() < 5.0f,
			"z %.2f speed %.2f", rest.z, VelocityOf( pCube ).Length() );
		Check( TIER_GAMEPLAY, "events.collision-pairs", collisions.m_pre > 0 && collisions.m_pre == collisions.m_post,
			"pre %d post %d", collisions.m_pre, collisions.m_post );
		Check( TIER_GAMEPLAY, "events.collision-objects", collisions.m_watchedPair > 0, "matching events %d", collisions.m_watchedPair );

		IPhysicsFrictionSnapshot *pSnapshot = pCube->CreateFrictionSnapshot();
		bool touchingFloor = false;
		for ( ; pSnapshot && pSnapshot->IsValid(); pSnapshot->NextFrictionData() )
			touchingFloor |= pSnapshot->GetObject( 1 ) == world.pFloor;
		Check( TIER_GAMEPLAY, "contacts.friction-snapshot", touchingFloor );
		if ( pSnapshot )
			pCube->DestroyFrictionSnapshot( pSnapshot );

		Step( world.pEnv, 5.0f );
		Check( TIER_GAMEPLAY, "events.sleep", objects.m_sleeps > 0 && pCube->IsAsleep(), "sleeps %d asleep %d", objects.m_sleeps, pCube->IsAsleep() );
		world.pEnv->SetCollisionEventHandler( NULL );
		world.pEnv->SetObjectEventHandler( NULL );
		DestroyWorld( world );
	}

	// Portal builds portal-hole collision from polyhedra as static geometry
	// in its simulation environment: a cube must come to rest on such a slab.
	{
		float planes[6 * 4] = {
			1, 0, 0, 128,	-1, 0, 0, 128,
			0, 1, 0, 128,	0, -1, 0, 128,
			0, 0, 1, 0,		0, 0, -1, 8 };
		CPolyhedron *pPolyhedron = GeneratePolyhedronFromPlanes( planes, 6, 0.01f );
		CPhysConvex *pConvex = pPolyhedron ? s_pCollision->ConvexFromConvexPolyhedron( *pPolyhedron ) : NULL;
		if ( pPolyhedron )
			pPolyhedron->Release();
		CPhysCollide *pSlab = pConvex ? s_pCollision->ConvertConvexToCollide( &pConvex, 1 ) : NULL;
		if ( Check( TIER_GAMEPLAY, "portal.polyhedron-collide", pSlab != NULL ) )
		{
			World_t world;
			CreateWorld( world, NULL );
			objectparams_t params = DefaultParams( 1.0f, NULL );
			// Well above the world floor, so only the slab can hold the cube.
			IPhysicsObject *pSlabObject = world.pEnv->CreatePolyObjectStatic( pSlab, world.material, Vector( 0, 0, 500 ), vec3_angle, &params );
			IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 600 ) );
			Step( world.pEnv, 3.0f );
			float z = PositionOf( pCube ).z;
			Check( TIER_GAMEPLAY, "portal.polyhedron-supports", pSlabObject && z > 514.0f && z < 519.0f, "z %.2f", z );
			DestroyWorld( world );
			s_pCollision->DestroyCollide( pSlab );
		}
	}

	// Game collision rules (portal environments, player/prop filtering) are
	// expressed through the collision solver.
	{
		CFilterSolver solver;
		World_t world;
		CreateWorld( world, &solver );
		IPhysicsObject *pCube = CreateCube( world, Vector( 0, 0, 100 ) );
		solver.m_pIgnoreA = pCube;
		solver.m_pIgnoreB = world.pFloor;
		Step( world.pEnv, 1.5f );
		float z = PositionOf( pCube ).z;
		Check( TIER_GAMEPLAY, "rules.solver-filter", z < -100.0f && solver.m_calls > 0, "z %.2f calls %d", z, solver.m_calls );
		DestroyWorld( world );
	}

	// Velocity, impulses, shadow following, constraints, motion controllers.
	{
		World_t world;
		CreateWorld( world, NULL );
		world.pEnv->SetGravity( vec3_origin );

		IPhysicsObject *pMover = CreateCube( world, Vector( 0, 0, 500 ) );
		Vector velocity( 100, 0, 0 );
		pMover->SetVelocity( &velocity, NULL );
		Check( TIER_GAMEPLAY, "object.velocity-roundtrip", NearVec( VelocityOf( pMover ), velocity, 1.0f ) );
		Step( world.pEnv, 0.5f );
		float moved = PositionOf( pMover ).x;
		Check( TIER_GAMEPLAY, "sim.velocity-integrates", moved > 35.0f && moved < 55.0f, "moved %.2f", moved );

		IPhysicsObject *pPushed = CreateCube( world, Vector( 0, 200, 500 ) );
		pPushed->ApplyForceCenter( Vector( pPushed->GetMass() * 100.0f, 0, 0 ) );
		Step( world.pEnv, kTick );
		Check( TIER_GAMEPLAY, "object.impulse", Near( VelocityOf( pPushed ).x, 100.0f, 15.0f ), "vx %.2f", VelocityOf( pPushed ).x );

		IPhysicsObject *pHeld = CreateCube( world, Vector( 0, 400, 500 ) );
		pHeld->SetShadow( 1e4f, 1e4f, false, false );
		const Vector target( 100, 400, 500 );
		for ( int i = 0; i < 40; i++ )
		{
			pHeld->UpdateShadow( target, vec3_angle, true, kTick );
			Step( world.pEnv, kTick );
		}
		float error = ( PositionOf( pHeld ) - target ).Length();
		Check( TIER_GAMEPLAY, "shadow.follows-target", error < 5.0f, "error %.2f", error );

		IPhysicsObject *pRef = CreateCube( world, Vector( 0, 800, 500 ) );
		IPhysicsObject *pAttached = CreateCube( world, Vector( 40, 800, 500 ) );
		constraint_fixedparams_t fixed;
		fixed.Defaults();
		fixed.InitWithCurrentObjectState( pRef, pAttached );
		IPhysicsConstraint *pConstraint = world.pEnv->CreateFixedConstraint( pRef, pAttached, NULL, fixed );
		pRef->ApplyForceCenter( Vector( 0, 0, pRef->GetMass() * 200.0f ) );
		Step( world.pEnv, 0.5f );
		float separation = ( PositionOf( pAttached ) - PositionOf( pRef ) ).Length();
		float carried = PositionOf( pAttached ).z - 500.0f;
		Check( TIER_GAMEPLAY, "constraint.fixed-holds", pConstraint && Near( separation, 40.0f, 2.0f ) && carried > 10.0f,
			"separation %.2f carried %.2f", separation, carried );
		if ( pConstraint )
			world.pEnv->DestroyConstraint( pConstraint );

		world.pEnv->SetGravity( Vector( 0, 0, -600 ) );
		CAntiGravity handler;
		IPhysicsMotionController *pMotion = world.pEnv->CreateMotionController( &handler );
		IPhysicsObject *pFloating = CreateCube( world, Vector( 0, 1200, 1000 ) );
		pMotion->AttachObject( pFloating, true );
		Step( world.pEnv, 1.0f );
		float drift = fabsf( PositionOf( pFloating ).z - 1000.0f );
		Check( TIER_GAMEPLAY, "motion.acceleration-applied", handler.m_calls > 0 && drift < 10.0f, "calls %d drift %.2f", handler.m_calls, drift );
		world.pEnv->DestroyMotionController( pMotion );
		DestroyWorld( world );
	}
}

//-----------------------------------------------------------------------------
int main( int argc, char **argv )
{
	const char *pProvider = NULL;
	const char *pCorpus = NULL;
	CUtlVector<const char *> surfaceFiles;
	CUtlVector<PhyFixture_t> fixtures;
	for ( int i = 1; i < argc; i++ )
	{
		if ( !V_strcmp( argv[i], "--provider" ) && i + 1 < argc )
			pProvider = argv[++i];
		else if ( !V_strcmp( argv[i], "--surfaceprops" ) && i + 1 < argc )
			surfaceFiles.AddToTail( argv[++i] );
		else if ( !V_strcmp( argv[i], "--phy" ) && i + 1 < argc )
		{
			PhyFixture_t fixture;
			memset( &fixture, 0, sizeof( fixture ) );
			fixture.pPath = argv[++i];
			fixtures.AddToTail( fixture );
		}
		else if ( !V_strcmp( argv[i], "--fault" ) && i + 1 < argc )
			s_pFault = argv[++i];
		else if ( !V_strcmp( argv[i], "--corpus" ) && i + 1 < argc )
			pCorpus = argv[++i];
		else
		{
			fprintf( stderr, "usage: %s --provider <lib.so> --surfaceprops <file>... --phy <file>... [--corpus <list>] [--fault <name>]\n", argv[0] );
			return 2;
		}
	}
	// Required inputs are not optional coverage: a run without content fails.
	if ( !pProvider || !surfaceFiles.Count() || !fixtures.Count() )
	{
		fprintf( stderr, "missing --provider, --surfaceprops, or --phy\n" );
		return 2;
	}

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
	printf( "PROVIDER %s\nFAULT %s\n", pProvider, s_pFault[0] ? s_pFault : "none" );

	if ( TestModule( pProvider ) )
	{
		TestSurfaceProps( surfaceFiles );
		TestPairHashAndSets();
		TestBoxCollide();
		TestPortalPolyhedra();
		for ( int i = 0; i < fixtures.Count(); i++ )
			TestPhyFixture( fixtures[i], i );
		if ( pCorpus )
			TestCorpus( pCorpus );
		TestObjects();
		TestSimulation();
		TestConstraints();
		TestObjectsAndEvents();
		TestFluidsAndSprings();
		TestSaveRestore();
		for ( int i = 0; i < fixtures.Count(); i++ )
		{
			if ( fixtures[i].collide.solids )
			{
				s_pCollision->VCollideUnload( &fixtures[i].collide );
				Check( TIER_BOOT, "vcollide.unload", fixtures[i].collide.solidCount == 0 && fixtures[i].collide.solids == NULL );
			}
		}
		s_pPhysics->Shutdown();
		s_pPhysics->Disconnect();
	}

	int checks = 0, failed = 0;
	for ( int t = 0; t < TIER_COUNT; t++ )
	{
		printf( "TIER %s %d %d\n", s_tierNames[t], s_checks[t], s_failed[t] );
		checks += s_checks[t];
		failed += s_failed[t];
	}
	printf( "CONFORMANCE %d %d\n", checks, failed );
	fflush( stdout );
	return failed ? 1 : 0;
}
