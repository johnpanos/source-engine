#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H

#include "vphysics_interface.h"

class CPhysicsCollisionBox3D : public IPhysicsCollision
{
public:
	virtual ~CPhysicsCollisionBox3D() {}

	virtual CPhysConvex *ConvexFromVerts( Vector **pVerts, int vertCount ) override { return nullptr; }
	virtual CPhysConvex *ConvexFromPlanes( float *pPlanes, int planeCount, float mergeDistance ) override { return nullptr; }
	virtual float ConvexVolume( CPhysConvex *pConvex ) override { return 1.0f; }
	virtual float ConvexSurfaceArea( CPhysConvex *pConvex ) override { return 1.0f; }
	virtual void SetConvexGameData( CPhysConvex *pConvex, unsigned int gameData ) override {}
	virtual void ConvexFree( CPhysConvex *pConvex ) override {}
	virtual CPhysConvex *BBoxToConvex( const Vector &mins, const Vector &maxs ) override { return nullptr; }
	virtual CPhysConvex *ConvexFromConvexPolyhedron( const CPolyhedron &ConvexPolyhedron ) override { return nullptr; }
	virtual void ConvexesFromConvexPolygon( const Vector &vPolyNormal, const Vector *pPoints, int iPointCount, CPhysConvex **pOutput ) override {}

	virtual CPhysPolysoup *PolysoupCreate( void ) override { return nullptr; }
	virtual void PolysoupDestroy( CPhysPolysoup *pSoup ) override {}
	virtual void PolysoupAddTriangle( CPhysPolysoup *pSoup, const Vector &a, const Vector &b, const Vector &c, int materialIndex7bits ) override {}
	virtual CPhysCollide *ConvertPolysoupToCollide( CPhysPolysoup *pSoup, bool useMOPP ) override { return nullptr; }

	virtual CPhysCollide *ConvertConvexToCollide( CPhysConvex **pConvex, int convexCount ) override { return nullptr; }
	virtual CPhysCollide *ConvertConvexToCollideParams( CPhysConvex **pConvex, int convexCount, const convertconvexparams_t &convertParams ) override { return nullptr; }
	virtual void DestroyCollide( CPhysCollide *pCollide ) override {}

	virtual int CollideSize( CPhysCollide *pCollide ) override { return 0; }
	virtual int CollideWrite( char *pDest, CPhysCollide *pCollide, bool bSwap = false ) override { return 0; }
	virtual CPhysCollide *UnserializeCollide( char *pBuffer, int size, int index ) override { return (CPhysCollide*)1; /* Return a non-null dummy to avoid asserts */ }

	virtual float CollideVolume( CPhysCollide *pCollide ) override { return 1.0f; }
	virtual float CollideSurfaceArea( CPhysCollide *pCollide ) override { return 1.0f; }

	virtual Vector CollideGetExtent( const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, const Vector &direction ) override { return Vector(0,0,0); }
	virtual void CollideGetAABB( Vector *pMins, Vector *pMaxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles ) override { *pMins = Vector(-1,-1,-1); *pMaxs = Vector(1,1,1); }

	virtual void CollideGetMassCenter( CPhysCollide *pCollide, Vector *pOutMassCenter ) override { *pOutMassCenter = Vector(0,0,0); }
	virtual void CollideSetMassCenter( CPhysCollide *pCollide, const Vector &massCenter ) override {}
	virtual Vector CollideGetOrthographicAreas( const CPhysCollide *pCollide ) override { return Vector(1,1,1); }
	virtual void CollideSetOrthographicAreas( CPhysCollide *pCollide, const Vector &areas ) override {}

	virtual int CollideIndex( const CPhysCollide *pCollide ) override { return 0; }

	virtual CPhysCollide *BBoxToCollide( const Vector &mins, const Vector &maxs ) override { return nullptr; }
	virtual int GetConvexesUsedInCollideable( const CPhysCollide *pCollideable, CPhysConvex **pOutputArray, int iOutputArrayLimit ) override { return 0; }

	virtual void TraceBox( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override {}
	virtual void TraceBox( const Ray_t &ray, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override {}
	virtual void TraceBox( const Ray_t &ray, unsigned int contentsMask, IConvexInfo *pConvexInfo, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override {}

	virtual void TraceCollide( const Vector &start, const Vector &end, const CPhysCollide *pSweepCollide, const QAngle &sweepAngles, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr ) override {}

	virtual void VCollideLoad( vcollide_t *pOutput, int solidCount, const char *pBuffer, int size, bool swap ) override { 
		pOutput->solidCount = solidCount;
		pOutput->solids = new CPhysCollide*[solidCount];
		for (int i=0; i<solidCount; ++i) {
			pOutput->solids[i] = (CPhysCollide*)1; // Dummy
		}
		pOutput->isPacked = false;
	}
	virtual void VCollideUnload( vcollide_t *pVCollide ) override {
		delete[] pVCollide->solids;
		pVCollide->solidCount = 0;
	}

	virtual bool IsBoxIntersectingCone( const Vector &boxAbsMins, const Vector &boxAbsMaxs, const truncatedcone_t &cone ) override { return false; }
	virtual IVPhysicsKeyParser *VPhysicsKeyParserCreate( const char *pKeyData ) override { return nullptr; }
	virtual void VPhysicsKeyParserDestroy( IVPhysicsKeyParser *pParser ) override {}
	virtual int CreateDebugMesh( CPhysCollide const *pCollisionModel, Vector **outVerts ) override { return 0; }
	virtual void DestroyDebugMesh( int vertCount, Vector *outVerts ) override {}
	virtual ICollisionQuery *CreateQueryModel( CPhysCollide *pCollide ) override { return nullptr; }
	virtual void DestroyQueryModel( ICollisionQuery *pQuery ) override {}
	virtual IPhysicsCollision *ThreadContextCreate( void ) override { return nullptr; }
	virtual void ThreadContextDestroy( IPhysicsCollision *pThreadContex ) override {}
	virtual CPhysCollide *CreateVirtualMesh( const virtualmeshparams_t &params ) override { return nullptr; }
	virtual bool SupportsVirtualMesh() override { return false; }
	virtual bool GetBBoxCacheSize( int *pCachedSize, int *pCachedCount ) override { return false; }
	virtual CPolyhedron *PolyhedronFromConvex( CPhysConvex * const pConvex, bool bUseTempPolyhedron ) override { return nullptr; }
	virtual void OutputDebugInfo( const CPhysCollide *pCollide ) override {}
	virtual unsigned int ReadStat( int statID ) override { return 0; }
};

#endif // PHYSICS_COLLISION_H
