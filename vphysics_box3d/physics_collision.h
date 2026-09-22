#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H

#include "vphysics_interface.h"
#include "mathlib/vector.h"
#include "utlvector.h"

struct b3HullData;

// The authored keyvalue parser is owned by the shared vcollide text parser
// (vphysics/vcollide_parse.cpp), reused here so the Box3D provider does not
// keep a second copy of solid/constraint/vehicle keyvalue semantics.
IVPhysicsKeyParser *CreateVPhysicsKeyParser( const char *pKeyData );
void DestroyVPhysicsKeyParser( IVPhysicsKeyParser *pParser );

//-----------------------------------------------------------------------------
// One convex piece. The engine sees it as an opaque CPhysConvex*.
// Geometry is in Source units in the owning collide's space. The Box3D hulls
// are the same shape for queries and simulation: normally one hull, or
// several pieces that exactly cover it when the convex exceeds Box3D's
// per-hull vertex/face limits. A degenerate (flat or sub-tolerance) piece
// has no hulls; it keeps its points for bounds and debug output but does not
// collide.
//-----------------------------------------------------------------------------
struct CPhysConvexBox3D
{
	CPhysConvexBox3D() : gameData( 0 ), volume( 0.0f ) { center.Init(); }
	~CPhysConvexBox3D();

	CUtlVector<Vector> points;
	CUtlVector<unsigned short> triangles;		// 3 indices per triangle
	CUtlVector<unsigned char> triangleMaterials;
	CUtlVector<b3HullData *> hulls;
	unsigned int gameData;
	float volume;			// exact solid volume (cubic inches)
	Vector center;			// volume centroid
};

//-----------------------------------------------------------------------------
// A collision model: convex pieces plus derived properties. Opaque to the
// engine as CPhysCollide*.
//-----------------------------------------------------------------------------
struct CPhysCollideBox3D
{
	CPhysCollideBox3D() : volume( 0.0f ), index( 0 )
	{
		mins.Init();
		maxs.Init();
		massCenter.Init();
		orthoAreas.Init( 1, 1, 1 );
		rotationInertia.Init( 1, 1, 1 );
	}
	~CPhysCollideBox3D();

	CUtlVector<CPhysConvexBox3D *> convexes;
	Vector mins, maxs;		// collide-space bounds
	Vector massCenter;
	Vector orthoAreas;
	// Per-unit-mass rotational inertia (Source axes, square meters) by IVP's
	// definition (vphysics objects report inertia in these units): with
	// a, b, c the mean squared extents <x^2>, <y^2>, <z^2> about the mass
	// center, I = ( sqrt(b^2 + c^2), sqrt(a^2 + c^2), sqrt(a^2 + b^2) ).
	// Decoded models carry the value the tools stored.
	Vector rotationInertia;
	float volume;
	int index;
};

inline CPhysCollideBox3D *ToBox3D( CPhysCollide *pCollide ) { return reinterpret_cast<CPhysCollideBox3D *>( pCollide ); }
inline const CPhysCollideBox3D *ToBox3D( const CPhysCollide *pCollide ) { return reinterpret_cast<const CPhysCollideBox3D *>( pCollide ); }
inline CPhysConvexBox3D *ToBox3D( CPhysConvex *pConvex ) { return reinterpret_cast<CPhysConvexBox3D *>( pConvex ); }

// Builds a convex from points; triangles come from the hull when not given.
CPhysConvexBox3D *CreateConvexBox3D( const Vector *pPoints, int pointCount );

class CPhysicsCollisionBox3D : public IPhysicsCollision
{
public:
	virtual ~CPhysicsCollisionBox3D() {}

	virtual CPhysConvex *ConvexFromVerts( Vector **pVerts, int vertCount ) override;
	virtual CPhysConvex *ConvexFromPlanes( float *pPlanes, int planeCount, float mergeDistance ) override;
	virtual float ConvexVolume( CPhysConvex *pConvex ) override;
	virtual float ConvexSurfaceArea( CPhysConvex *pConvex ) override;
	virtual void SetConvexGameData( CPhysConvex *pConvex, unsigned int gameData ) override;
	virtual void ConvexFree( CPhysConvex *pConvex ) override;
	virtual CPhysConvex *BBoxToConvex( const Vector &mins, const Vector &maxs ) override;
	virtual CPhysConvex *ConvexFromConvexPolyhedron( const CPolyhedron &ConvexPolyhedron ) override;
	virtual void ConvexesFromConvexPolygon( const Vector &vPolyNormal, const Vector *pPoints, int iPointCount, CPhysConvex **pOutput ) override;

	virtual CPhysPolysoup *PolysoupCreate( void ) override { return nullptr; }
	virtual void PolysoupDestroy( CPhysPolysoup *pSoup ) override {}
	virtual void PolysoupAddTriangle( CPhysPolysoup *pSoup, const Vector &a, const Vector &b, const Vector &c, int materialIndex7bits ) override {}
	virtual CPhysCollide *ConvertPolysoupToCollide( CPhysPolysoup *pSoup, bool useMOPP ) override { return nullptr; }

	virtual CPhysCollide *ConvertConvexToCollide( CPhysConvex **pConvex, int convexCount ) override;
	virtual CPhysCollide *ConvertConvexToCollideParams( CPhysConvex **pConvex, int convexCount, const convertconvexparams_t &convertParams ) override;
	virtual void DestroyCollide( CPhysCollide *pCollide ) override;

	virtual int CollideSize( CPhysCollide *pCollide ) override { return 0; }
	virtual int CollideWrite( char *pDest, CPhysCollide *pCollide, bool bSwap = false ) override { return 0; }
	virtual CPhysCollide *UnserializeCollide( char *pBuffer, int size, int index ) override;

	virtual float CollideVolume( CPhysCollide *pCollide ) override;
	virtual float CollideSurfaceArea( CPhysCollide *pCollide ) override;

	virtual Vector CollideGetExtent( const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, const Vector &direction ) override;
	virtual void CollideGetAABB( Vector *pMins, Vector *pMaxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles ) override;

	virtual void CollideGetMassCenter( CPhysCollide *pCollide, Vector *pOutMassCenter ) override;
	virtual void CollideSetMassCenter( CPhysCollide *pCollide, const Vector &massCenter ) override;
	virtual Vector CollideGetOrthographicAreas( const CPhysCollide *pCollide ) override;
	virtual void CollideSetOrthographicAreas( CPhysCollide *pCollide, const Vector &areas ) override;

	virtual int CollideIndex( const CPhysCollide *pCollide ) override;

	virtual CPhysCollide *BBoxToCollide( const Vector &mins, const Vector &maxs ) override;
	virtual int GetConvexesUsedInCollideable( const CPhysCollide *pCollideable, CPhysConvex **pOutputArray, int iOutputArrayLimit ) override;

	virtual void TraceBox( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override;
	virtual void TraceBox( const Ray_t &ray, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override;
	virtual void TraceBox( const Ray_t &ray, unsigned int contentsMask, IConvexInfo *pConvexInfo, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override;
	virtual void TraceCollide( const Vector &start, const Vector &end, const CPhysCollide *pSweepCollide, const QAngle &sweepAngles, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override;

	virtual void VCollideLoad( vcollide_t *pOutput, int solidCount, const char *pBuffer, int bufferSize, bool swap ) override;
	virtual void VCollideUnload( vcollide_t *pVCollide ) override;

	virtual bool IsBoxIntersectingCone( const Vector &boxAbsMins, const Vector &boxAbsMaxs, const truncatedcone_t &cone ) override { return false; }
	virtual IVPhysicsKeyParser *VPhysicsKeyParserCreate( const char *pKeyData ) override { return CreateVPhysicsKeyParser( pKeyData ); }
	virtual void VPhysicsKeyParserDestroy( IVPhysicsKeyParser *pParser ) override { DestroyVPhysicsKeyParser( pParser ); }
	virtual int CreateDebugMesh( CPhysCollide const *pCollisionModel, Vector **outVerts ) override;
	virtual void DestroyDebugMesh( int vertCount, Vector *outVerts ) override;
	virtual ICollisionQuery *CreateQueryModel( CPhysCollide *pCollide ) override;
	virtual void DestroyQueryModel( ICollisionQuery *pQuery ) override;
	virtual IPhysicsCollision *ThreadContextCreate( void ) override { return this; }
	virtual void ThreadContextDestroy( IPhysicsCollision *pThreadContex ) override {}
	virtual CPhysCollide *CreateVirtualMesh( const virtualmeshparams_t &params ) override { return nullptr; }
	virtual bool SupportsVirtualMesh() override { return false; }
	virtual bool GetBBoxCacheSize( int *pCachedSize, int *pCachedCount ) override { return false; }
	virtual CPolyhedron *PolyhedronFromConvex( CPhysConvex * const pConvex, bool bUseTempPolyhedron ) override;
	virtual void OutputDebugInfo( const CPhysCollide *pCollide ) override {}
	virtual unsigned int ReadStat( int statID ) override { return 0; }
};

// Shared sweep used by IPhysicsCollision traces: sweeps a point set (world
// space, at the start position) by delta against a collide at a pose.
void SweepPointsAgainstCollide( const Vector *pWorldPoints, int pointCount, const Vector &delta,
	const CPhysCollideBox3D *pCollide, const Vector &origin, const QAngle &angles,
	unsigned int contentsMask, IConvexInfo *pConvexInfo, float clipEpsilon, trace_t *ptr );

#endif // PHYSICS_COLLISION_H
