//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Box3D collision models and queries (RFC 0004 B3/B4).
//
// Legacy .phy solids are decoded into convex pieces (legacy_collision.cpp),
// each backed by a Box3D hull. Queries follow the VPhysics trace contract as
// the IVP provider implements it: traces start from a cleared result
// (fraction 1, "**empty**" surface), report CONTENTS_SOLID unless an
// IConvexInfo supplies per-convex contents, report startsolid/allsolid with
// fraction 0 when the swept shape starts inside, and derive endpos and
// plane.dist from the hit fraction.
//
//=============================================================================//
#include "physics_collision.h"
#include "vphysics/virtualmesh.h"

#include <string.h>

#include "box3d/collision.h"
#include "box3d/constants.h"
#include "box3d_convert.h"
#include "cmodel.h"
#include "gametrace.h"
#include "bspflags.h"
#include "coordsize.h"
#include "legacy_collision.h"
#include "mathlib/polyhedron.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
// Box3D bounds a hull's vertex, face and edge counts; see BuildHulls for how
// larger convexes are covered. These budgets are the points-only fallback.
const int kHullVertexBudgets[] = { 64, 32, 16 };

csurface_t s_nullSurface = { "**empty**", 0, 0 };

void ClearTrace( trace_t *ptr )
{
	memset( (void *)ptr, 0, sizeof( *ptr ) );
	ptr->fraction = 1.0f;
	ptr->fractionleftsolid = 0.0f;
	ptr->surface = s_nullSurface;
}

// Fan-triangulates a hull's faces into index triples over its points.
void TriangulateHull( const b3HullData *pHull, CPhysConvexBox3D *pConvex )
{
	const b3Vec3 *pPoints = b3GetHullPoints( pHull );
	const b3HullHalfEdge *pEdges = b3GetHullEdges( pHull );
	const b3HullFace *pFaces = b3GetHullFaces( pHull );
	int base = pConvex->points.Count();
	for ( int i = 0; i < pHull->vertexCount; i++ )
		pConvex->points.AddToTail( FromB3( pPoints[i] ) );
	for ( int f = 0; f < pHull->faceCount; f++ )
	{
		int first = pFaces[f].edge;
		int anchor = pEdges[first].origin;
		int edge = pEdges[first].next;
		for ( int guard = 0; guard < pHull->edgeCount; guard++ )
		{
			int next = pEdges[edge].next;
			if ( next == first )
				break;
			pConvex->triangles.AddToTail( (unsigned short)( base + anchor ) );
			pConvex->triangles.AddToTail( (unsigned short)( base + pEdges[edge].origin ) );
			pConvex->triangles.AddToTail( (unsigned short)( base + pEdges[next].origin ) );
			pConvex->triangleMaterials.AddToTail( 0 );
			edge = next;
		}
	}
}

b3HullData *CreateHull( const Vector *pPoints, int count, int maxVertices )
{
	if ( count < 4 )
		return NULL;
	CUtlVector<b3Vec3> points;
	points.SetCount( count );
	for ( int i = 0; i < count; i++ )
		points[i] = ToB3( pPoints[i] );
	return b3CreateHull( points.Base(), count, maxVertices );
}

bool AddHull( const Vector *pPoints, int count, int maxVertices, CUtlVector<b3HullData *> &out )
{
	b3HullData *pHull = CreateHull( pPoints, count, maxVertices );
	if ( pHull )
		out.AddToTail( pHull );
	return pHull != NULL;
}

// Last resort for a convex given only as points: Box3D's vertex-reducing
// simplification (slightly inscribed).
void AddSimplifiedHull( const Vector *pPoints, int count, CUtlVector<b3HullData *> &out )
{
	for ( int i = 0; i < (int)ARRAYSIZE( kHullVertexBudgets ); i++ )
	{
		if ( AddHull( pPoints, count, kHullVertexBudgets[i], out ) )
			return;
	}
}

// Covers a patch of the convex's surface triangles with the hull of the
// interior point plus the patch vertices, halving the patch (by the widest
// axis of its triangle centers) until the hull fits Box3D's limits. For a
// convex solid and an interior point, each such hull lies inside the solid
// and contains the cone over its patch, so the pieces cover the solid exactly
// (overlapping only inside it). A single triangle yields a tetrahedron.
void CoverPatch( const Vector *pPoints, const unsigned short *pTriangles, CUtlVector<int> &patch,
	const Vector &interior, CUtlVector<b3HullData *> &out )
{
	CUtlVector<Vector> vertices;
	vertices.AddToTail( interior );
	for ( int t = 0; t < patch.Count(); t++ )
	{
		for ( int k = 0; k < 3; k++ )
		{
			const Vector &v = pPoints[pTriangles[patch[t] * 3 + k]];
			if ( vertices.Find( v ) == vertices.InvalidIndex() )
				vertices.AddToTail( v );
		}
	}
	if ( AddHull( vertices.Base(), vertices.Count(), B3_MAX_HULL_VERTICES, out ) || patch.Count() <= 1 )
		return;	// a sub-tolerance single triangle contributes nothing

	Vector mins( FLT_MAX, FLT_MAX, FLT_MAX ), maxs( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	CUtlVector<Vector> centers;
	for ( int t = 0; t < patch.Count(); t++ )
	{
		const unsigned short *pTri = &pTriangles[patch[t] * 3];
		Vector center = ( pPoints[pTri[0]] + pPoints[pTri[1]] + pPoints[pTri[2]] ) / 3.0f;
		centers.AddToTail( center );
		VectorMin( mins, center, mins );
		VectorMax( maxs, center, maxs );
	}
	Vector size = maxs - mins;
	int axis = ( size.x >= size.y && size.x >= size.z ) ? 0 : ( size.y >= size.z ? 1 : 2 );
	float split = ( mins[axis] + maxs[axis] ) * 0.5f;
	CUtlVector<int> low, high;
	for ( int t = 0; t < patch.Count(); t++ )
		( centers[t][axis] <= split ? low : high ).AddToTail( patch[t] );
	if ( !low.Count() || !high.Count() )
	{
		// Coincident centers: split the list in half instead.
		low.RemoveAll();
		high.RemoveAll();
		for ( int t = 0; t < patch.Count(); t++ )
			( t < patch.Count() / 2 ? low : high ).AddToTail( patch[t] );
	}
	CoverPatch( pPoints, pTriangles, low, interior, out );
	CoverPatch( pPoints, pTriangles, high, interior, out );
}

const float kFlatSlabHalfThickness = 0.25f;
// IVP's global collision tolerance, the clip distance for collide sweeps.
const float kCollideSweepEpsilon = 0.25f;

// True when every point lies within a small distance of one plane.
bool IsFlat( const Vector *pPoints, int count )
{
	if ( count < 4 )
		return true;
	Vector normal( 0, 0, 0 );
	for ( int i = 1; i + 1 < count && normal.LengthSqr() < 1e-8f; i++ )
		normal = CrossProduct( pPoints[i] - pPoints[0], pPoints[i + 1] - pPoints[0] );
	if ( VectorNormalize( normal ) < 1e-6f )
		return true;
	for ( int i = 0; i < count; i++ )
	{
		if ( fabsf( DotProduct( pPoints[i] - pPoints[0], normal ) ) > 0.01f )
			return false;
	}
	return true;
}

void AddFlatSlab( const Vector *pPoints, int count, CUtlVector<b3HullData *> &out )
{
	Vector normal( 0, 0, 0 );
	for ( int i = 1; i + 1 < count && normal.LengthSqr() < 1e-8f; i++ )
		normal = CrossProduct( pPoints[i] - pPoints[0], pPoints[i + 1] - pPoints[0] );
	if ( VectorNormalize( normal ) < 1e-6f )
		return;	// collinear or coincident: nothing to collide with
	CUtlVector<Vector> slab;
	for ( int i = 0; i < count; i++ )
	{
		slab.AddToTail( pPoints[i] + normal * kFlatSlabHalfThickness );
		slab.AddToTail( pPoints[i] - normal * kFlatSlabHalfThickness );
	}
	if ( !AddHull( slab.Base(), slab.Count(), B3_MAX_HULL_VERTICES, out ) )
		AddSimplifiedHull( slab.Base(), slab.Count(), out );
}

// Builds the Box3D hulls for one convex: normally a single hull. A convex over
// Box3D's per-hull limits is covered exactly by patch hulls when its surface
// triangles are known (legacy .phy data), and simplified otherwise. Volume
// and center come from the triangles, so overlapping pieces do not skew them.
void BuildHulls( const Vector *pPoints, int count, const unsigned short *pTriangles, int triangleIndexCount,
	CPhysConvexBox3D *pConvex )
{
	pConvex->volume = 0.0f;
	pConvex->center.Init();
	if ( AddHull( pPoints, count, B3_MAX_HULL_VERTICES, pConvex->hulls ) )
	{
		pConvex->volume = pConvex->hulls[0]->volume;
		pConvex->center = FromB3( pConvex->hulls[0]->center );
		return;
	}
	if ( count < 3 )
		return;
	if ( IsFlat( pPoints, count ) )
	{
		// A flat piece (IVP's double-sided triangle ledges) has no volume but
		// still blocks traces and objects: collide with a thin slab around it.
		AddFlatSlab( pPoints, count, pConvex->hulls );
		return;
	}
	if ( !pTriangles || triangleIndexCount < 12 )
	{
		AddSimplifiedHull( pPoints, count, pConvex->hulls );
		for ( int i = 0; i < pConvex->hulls.Count(); i++ )
		{
			pConvex->volume = pConvex->hulls[i]->volume;
			pConvex->center = FromB3( pConvex->hulls[i]->center );
		}
		return;
	}

	Vector interior( 0, 0, 0 );
	for ( int i = 0; i < count; i++ )
		interior += pPoints[i];
	interior /= (float)count;

	int triangleCount = triangleIndexCount / 3;
	Vector weighted( 0, 0, 0 );
	for ( int t = 0; t < triangleCount; t++ )
	{
		const Vector &a = pPoints[pTriangles[t * 3 + 0]];
		const Vector &b = pPoints[pTriangles[t * 3 + 1]];
		const Vector &c = pPoints[pTriangles[t * 3 + 2]];
		float volume = fabsf( DotProduct( a - interior, CrossProduct( b - interior, c - interior ) ) ) / 6.0f;
		pConvex->volume += volume;
		weighted += ( interior + a + b + c ) * ( 0.25f * volume );
	}
	pConvex->center = pConvex->volume > 0.0f ? weighted / pConvex->volume : interior;

	CUtlVector<int> patch;
	for ( int t = 0; t < triangleCount; t++ )
		patch.AddToTail( t );
	CoverPatch( pPoints, pTriangles, patch, interior, pConvex->hulls );
}

// IVP_Rot_Inertia_Solver: mean squared extents about the mass center over the
// solid volume, combined per axis as sqrt(b^2 + c^2) (not the rigid-body
// b + c), in square meters. Without volume, IVP falls back to half the
// squared bounding radius.
Vector ComputeRotationInertia( const CPhysCollideBox3D *pCollide )
{
	const float kMetersPerInch = 0.0254f;
	double volume = 0.0, first[3] = { 0, 0, 0 }, second[3] = { 0, 0, 0 };
	for ( int c = 0; c < pCollide->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pCollide->convexes[c];
		int pointCount = pConvex->points.Count();
		if ( pointCount < 4 )
			continue;
		Vector interior( 0, 0, 0 );
		for ( int i = 0; i < pointCount; i++ )
			interior += pConvex->points[i];
		interior /= (float)pointCount;
		for ( int t = 0; t + 2 < pConvex->triangles.Count(); t += 3 )
		{
			const Vector *pTet[4] = { &interior, &pConvex->points[pConvex->triangles[t]],
				&pConvex->points[pConvex->triangles[t + 1]], &pConvex->points[pConvex->triangles[t + 2]] };
			double v = fabs( DotProduct( *pTet[1] - interior, CrossProduct( *pTet[2] - interior, *pTet[3] - interior ) ) ) / 6.0;
			if ( v <= 0 )
				continue;
			volume += v;
			for ( int axis = 0; axis < 3; axis++ )
			{
				double sum = 0, squares = 0;
				for ( int k = 0; k < 4; k++ )
				{
					double x = ( *pTet[k] )[axis];
					sum += x;
					squares += x * x;
				}
				first[axis] += v * sum * 0.25;
				// Integral of x^2 over a tetrahedron: V/20 * (sum x_k^2 + (sum x_k)^2).
				second[axis] += v * ( squares + sum * sum ) / 20.0;
			}
		}
	}
	if ( volume <= 1e-6 )
	{
		Vector extent = ( pCollide->maxs - pCollide->mins ) * ( 0.5f * kMetersPerInch );
		float radiusSq = extent.LengthSqr();
		return Vector( radiusSq, radiusSq, radiusSq ) * 0.5f;
	}
	double meanSq[3];
	for ( int axis = 0; axis < 3; axis++ )
	{
		double center = first[axis] / volume;
		meanSq[axis] = ( second[axis] / volume - center * center ) * kMetersPerInch * kMetersPerInch;
	}
	return Vector( (float)sqrt( meanSq[1] * meanSq[1] + meanSq[2] * meanSq[2] ),
		(float)sqrt( meanSq[0] * meanSq[0] + meanSq[2] * meanSq[2] ),
		(float)sqrt( meanSq[0] * meanSq[0] + meanSq[1] * meanSq[1] ) );
}

void FinalizeCollide( CPhysCollideBox3D *pCollide, bool computeMassCenter )
{
	pCollide->mins.Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pCollide->maxs.Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	pCollide->volume = 0.0f;
	Vector weighted( 0, 0, 0 );
	for ( int c = 0; c < pCollide->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pCollide->convexes[c];
		for ( int i = 0; i < pConvex->points.Count(); i++ )
		{
			VectorMin( pCollide->mins, pConvex->points[i], pCollide->mins );
			VectorMax( pCollide->maxs, pConvex->points[i], pCollide->maxs );
		}
		pCollide->volume += pConvex->volume;
		weighted += pConvex->center * pConvex->volume;
	}
	if ( pCollide->convexes.Count() == 0 )
	{
		pCollide->mins.Init();
		pCollide->maxs.Init();
	}
	if ( computeMassCenter )
	{
		pCollide->massCenter = pCollide->volume > 0.0f ? weighted / pCollide->volume
			: ( pCollide->mins + pCollide->maxs ) * 0.5f;
		pCollide->rotationInertia = ComputeRotationInertia( pCollide );
	}
}

CPhysCollideBox3D *DecodeSolid( const char *pBuffer, int size, int index )
{
	LegacyCollide_t legacy;
	if ( !DecodeLegacyCollide( pBuffer, size, &legacy ) )
		return NULL;
	CPhysCollideBox3D *pCollide = new CPhysCollideBox3D;
	pCollide->index = index;
	pCollide->massCenter = legacy.massCenter;
	pCollide->orthoAreas = legacy.orthoAreas;
	pCollide->rotationInertia = legacy.rotationInertia;
	for ( int i = 0; i < legacy.convexes.Count(); i++ )
	{
		const LegacyConvex_t &source = legacy.convexes[i];
		CPhysConvexBox3D *pConvex = new CPhysConvexBox3D;
		pConvex->points.CopyArray( source.points.Base(), source.points.Count() );
		pConvex->triangles.CopyArray( source.triangles.Base(), source.triangles.Count() );
		pConvex->triangleMaterials.CopyArray( source.triangleMaterials.Base(), source.triangleMaterials.Count() );
		pConvex->gameData = source.gameData;
		BuildHulls( source.points.Base(), source.points.Count(), source.triangles.Base(), source.triangles.Count(), pConvex );
		pCollide->convexes.AddToTail( pConvex );
	}
	FinalizeCollide( pCollide, false );
	return pCollide;
}

class CDefaultConvexInfo : public IConvexInfo
{
public:
	virtual unsigned int GetContents( int convexGameData ) { return CONTENTS_SOLID; }
};

CDefaultConvexInfo s_defaultConvexInfo;

// Point sets that stand for a ray (1 point) or a swept box (8 corners).
int RayProxyPoints( const Ray_t &ray, Vector *pOut )
{
	if ( ray.m_IsRay )
	{
		pOut[0] = ray.m_Start;
		return 1;
	}
	for ( int i = 0; i < 8; i++ )
	{
		pOut[i] = ray.m_Start + Vector( ( i & 1 ) ? ray.m_Extents.x : -ray.m_Extents.x,
			( i & 2 ) ? ray.m_Extents.y : -ray.m_Extents.y, ( i & 4 ) ? ray.m_Extents.z : -ray.m_Extents.z );
	}
	return 8;
}
}

CPhysConvexBox3D::~CPhysConvexBox3D()
{
	for ( int i = 0; i < hulls.Count(); i++ )
		b3DestroyHull( hulls[i] );
}

CPhysCollideBox3D::~CPhysCollideBox3D()
{
	convexes.PurgeAndDeleteElements();
}

bool ComputeSolidInertia( const CPhysCollideBox3D *pCollide, const Vector &about, double *pVolume,
    double inertia[3][3] )
{
	// Volume, first moments and second moments (the integrals of x_a x_b)
	// about the collide origin, then shifted to `about`.
	double volume = 0.0;
	double first[3] = { 0, 0, 0 };
	double second[3][3] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
	for ( int c = 0; c < pCollide->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pCollide->convexes[c];
		if ( pConvex->volume <= 0.0f )
			continue;
		if ( pConvex->hulls.Count() == 1 )
		{
			b3MassData mass = b3ComputeHullMass( pConvex->hulls[0], 1.0f );
			if ( mass.mass <= 0.0f )
				continue;
			const b3Vec3 *pColumns[3] = { &mass.inertia.cx, &mass.inertia.cy, &mass.inertia.cz };
			double tensor[3][3];
			for ( int col = 0; col < 3; col++ )
			{
				tensor[0][col] = pColumns[col]->x;
				tensor[1][col] = pColumns[col]->y;
				tensor[2][col] = pColumns[col]->z;
			}
			// A tensor about the centroid is trace(S) E - S, S the centroid's
			// second moments, so S = trace(I) / 2 E - I.
			double halfTrace = 0.5 * ( tensor[0][0] + tensor[1][1] + tensor[2][2] );
			double center[3] = { mass.center.x, mass.center.y, mass.center.z };
			for ( int a = 0; a < 3; a++ )
			{
				first[a] += mass.mass * center[a];
				for ( int b = 0; b < 3; b++ )
				{
					second[a][b] += ( a == b ? halfTrace : 0.0 ) - tensor[a][b] +
					                mass.mass * center[a] * center[b];
				}
			}
			volume += mass.mass;
			continue;
		}
		// Tetrahedra from an interior point to each source surface triangle:
		// the integral of x_a x_b over one is V/20 (sum_k x_ka x_kb + X_a X_b),
		// X the vertex sums.
		int pointCount = pConvex->points.Count();
		if ( pointCount < 4 )
			continue;
		Vector interior( 0, 0, 0 );
		for ( int i = 0; i < pointCount; i++ )
			interior += pConvex->points[i];
		interior /= (float)pointCount;
		for ( int t = 0; t + 2 < pConvex->triangles.Count(); t += 3 )
		{
			const Vector *pTet[4] = { &interior, &pConvex->points[pConvex->triangles[t]],
				&pConvex->points[pConvex->triangles[t + 1]],
				&pConvex->points[pConvex->triangles[t + 2]] };
			double v = fabs( DotProduct( *pTet[1] - interior,
			               CrossProduct( *pTet[2] - interior, *pTet[3] - interior ) ) ) /
			           6.0;
			if ( v <= 0.0 )
				continue;
			double sums[3] = { 0, 0, 0 };
			for ( int k = 0; k < 4; k++ )
			{
				for ( int a = 0; a < 3; a++ )
					sums[a] += ( *pTet[k] )[a];
			}
			for ( int a = 0; a < 3; a++ )
			{
				first[a] += v * sums[a] * 0.25;
				for ( int b = 0; b < 3; b++ )
				{
					double products = 0.0;
					for ( int k = 0; k < 4; k++ )
						products += (double)( *pTet[k] )[a] * ( *pTet[k] )[b];
					second[a][b] += v * ( products + sums[a] * sums[b] ) / 20.0;
				}
			}
			volume += v;
		}
	}
	if ( volume <= 1e-6 )
		return false;
	double p[3] = { about.x, about.y, about.z };
	double shifted[3][3];
	for ( int a = 0; a < 3; a++ )
	{
		for ( int b = 0; b < 3; b++ )
			shifted[a][b] = second[a][b] - first[a] * p[b] - p[a] * first[b] + volume * p[a] * p[b];
	}
	double trace = shifted[0][0] + shifted[1][1] + shifted[2][2];
	for ( int a = 0; a < 3; a++ )
	{
		for ( int b = 0; b < 3; b++ )
			inertia[a][b] = ( a == b ? trace : 0.0 ) - shifted[a][b];
	}
	*pVolume = volume;
	return true;
}

CPhysConvexBox3D *CreateConvexBox3D( const Vector *pPoints, int pointCount )
{
	CPhysConvexBox3D *pConvex = new CPhysConvexBox3D;
	BuildHulls( pPoints, pointCount, NULL, 0, pConvex );
	if ( !pConvex->hulls.Count() )
	{
		delete pConvex;
		return NULL;
	}
	for ( int i = 0; i < pConvex->hulls.Count(); i++ )
		TriangulateHull( pConvex->hulls[i], pConvex );
	return pConvex;
}

//-----------------------------------------------------------------------------
// Convex construction
//-----------------------------------------------------------------------------
CPhysConvex *CPhysicsCollisionBox3D::ConvexFromVerts( Vector **pVerts, int vertCount )
{
	CUtlVector<Vector> points;
	for ( int i = 0; i < vertCount; i++ )
		points.AddToTail( *pVerts[i] );
	return reinterpret_cast<CPhysConvex *>( CreateConvexBox3D( points.Base(), points.Count() ) );
}

CPhysConvex *CPhysicsCollisionBox3D::ConvexFromPlanes( float *pPlanes, int planeCount, float mergeDistance )
{
	CPolyhedron *pPolyhedron = GeneratePolyhedronFromPlanes( pPlanes, planeCount, mergeDistance, true );
	if ( !pPolyhedron )
		return NULL;
	CPhysConvex *pConvex = ConvexFromConvexPolyhedron( *pPolyhedron );
	pPolyhedron->Release();
	return pConvex;
}

CPhysConvex *CPhysicsCollisionBox3D::ConvexFromConvexPolyhedron( const CPolyhedron &polyhedron )
{
	return reinterpret_cast<CPhysConvex *>( CreateConvexBox3D( polyhedron.pVertices, polyhedron.iVertexCount ) );
}

void CPhysicsCollisionBox3D::ConvexesFromConvexPolygon( const Vector &vPolyNormal, const Vector *pPoints, int iPointCount, CPhysConvex **pOutput )
{
	// A flat polygon has no volume; Box3D hulls require one. Give each fan
	// triangle a thin backing along -normal so it still blocks sweeps.
	const float kThickness = 1.0f;
	for ( int i = 0; i < iPointCount - 2; i++ )
	{
		Vector prism[6] = { pPoints[0], pPoints[i + 1], pPoints[i + 2],
			pPoints[0] - vPolyNormal * kThickness, pPoints[i + 1] - vPolyNormal * kThickness, pPoints[i + 2] - vPolyNormal * kThickness };
		pOutput[i] = reinterpret_cast<CPhysConvex *>( CreateConvexBox3D( prism, 6 ) );
	}
}

float CPhysicsCollisionBox3D::ConvexVolume( CPhysConvex *pConvex )
{
	return pConvex ? ToBox3D( pConvex )->volume : 0.0f;
}

float CPhysicsCollisionBox3D::ConvexSurfaceArea( CPhysConvex *pConvex )
{
	CPhysConvexBox3D *pBox = pConvex ? ToBox3D( pConvex ) : NULL;
	float area = 0.0f;
	for ( int i = 0; pBox && i < pBox->hulls.Count(); i++ )
		area += pBox->hulls[i]->surfaceArea;
	return area;
}

void CPhysicsCollisionBox3D::SetConvexGameData( CPhysConvex *pConvex, unsigned int gameData )
{
	if ( pConvex )
		ToBox3D( pConvex )->gameData = gameData;
}

void CPhysicsCollisionBox3D::ConvexFree( CPhysConvex *pConvex )
{
	delete ToBox3D( pConvex );
}

CPhysConvex *CPhysicsCollisionBox3D::BBoxToConvex( const Vector &mins, const Vector &maxs )
{
	Vector corners[8];
	for ( int i = 0; i < 8; i++ )
		corners[i] = Vector( ( i & 1 ) ? maxs.x : mins.x, ( i & 2 ) ? maxs.y : mins.y, ( i & 4 ) ? maxs.z : mins.z );
	return reinterpret_cast<CPhysConvex *>( CreateConvexBox3D( corners, 8 ) );
}

CPolyhedron *CPhysicsCollisionBox3D::PolyhedronFromConvex( CPhysConvex * const pConvex, bool bUseTempPolyhedron )
{
	CPhysConvexBox3D *pBox = pConvex ? ToBox3D( pConvex ) : NULL;
	if ( !pBox || pBox->triangles.Count() < 3 )
		return NULL;
	// The convex's own surface triangles, as IVP builds the polyhedron from its
	// ledge. Rebuilding it from the hull pieces' planes
	// (GeneratePolyhedronFromPlanes) overflowed the stack on shipped models whose
	// pieces share nearly coincident planes (physics conformance
	// corpus.convex-polyhedron).
	CUtlVector<int> triangles;
	triangles.SetCount( pBox->triangles.Count() - pBox->triangles.Count() % 3 );
	for ( int i = 0; i < triangles.Count(); i++ )
		triangles[i] = pBox->triangles[i];
	return ConvertTriangleMeshToPolyhedron( pBox->points.Base(), pBox->points.Count(), triangles.Base(), triangles.Count() / 3, bUseTempPolyhedron );
}

//-----------------------------------------------------------------------------
// Collides
//-----------------------------------------------------------------------------
CPhysCollide *CPhysicsCollisionBox3D::ConvertConvexToCollide( CPhysConvex **pConvex, int convexCount )
{
	CPhysCollideBox3D *pCollide = new CPhysCollideBox3D;
	for ( int i = 0; i < convexCount; i++ )
	{
		// The collide takes ownership of the convex pieces.
		if ( pConvex[i] )
			pCollide->convexes.AddToTail( ToBox3D( pConvex[i] ) );
	}
	if ( !pCollide->convexes.Count() )
	{
		delete pCollide;
		return NULL;
	}
	FinalizeCollide( pCollide, true );
	return reinterpret_cast<CPhysCollide *>( pCollide );
}

CPhysCollide *CPhysicsCollisionBox3D::ConvertConvexToCollideParams( CPhysConvex **pConvex, int convexCount, const convertconvexparams_t &convertParams )
{
	return ConvertConvexToCollide( pConvex, convexCount );
}

void CPhysicsCollisionBox3D::DestroyCollide( CPhysCollide *pCollide )
{
	for ( int i = 0; i < m_bboxCache.Count(); i++ )
	{
		if ( m_bboxCache[i].pCollide == pCollide )
			return;
	}
	delete ToBox3D( pCollide );
}

namespace
{
void CollideToLegacy( const CPhysCollideBox3D *pCollide, LegacyCollide_t *pOut )
{
	pOut->massCenter = pCollide->massCenter;
	pOut->orthoAreas = pCollide->orthoAreas;
	pOut->rotationInertia = pCollide->rotationInertia;
	for ( int i = 0; i < pCollide->convexes.Count(); i++ )
	{
		const CPhysConvexBox3D *pSource = pCollide->convexes[i];
		LegacyConvex_t &convex = pOut->convexes[pOut->convexes.AddToTail()];
		convex.points.CopyArray( pSource->points.Base(), pSource->points.Count() );
		convex.triangles.CopyArray( pSource->triangles.Base(), pSource->triangles.Count() );
		convex.triangleMaterials.CopyArray( pSource->triangleMaterials.Base(), pSource->triangleMaterials.Count() );
		convex.gameData = pSource->gameData;
	}
}
}

// The serialized form is the legacy IVP compact surface, so .phy/BSP data
// written through this provider stays loadable by either provider.
int CPhysicsCollisionBox3D::CollideSize( CPhysCollide *pCollide )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollide );
	if ( !pBox )
		return 0;
	LegacyCollide_t legacy;
	CollideToLegacy( pBox, &legacy );
	return EncodeLegacyCollide( legacy, pBox->index, NULL, false );
}

int CPhysicsCollisionBox3D::CollideWrite( char *pDest, CPhysCollide *pCollide, bool bSwap )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollide );
	if ( !pBox || !pDest )
		return 0;
	LegacyCollide_t legacy;
	CollideToLegacy( pBox, &legacy );
	return EncodeLegacyCollide( legacy, pBox->index, pDest, bSwap );
}

CPhysCollide *CPhysicsCollisionBox3D::UnserializeCollide( char *pBuffer, int size, int index )
{
	return reinterpret_cast<CPhysCollide *>( DecodeSolid( pBuffer, size, index ) );
}

CPhysCollide *CPhysicsCollisionBox3D::BBoxToCollide( const Vector &mins, const Vector &maxs )
{
	// Can't create a collision model for an empty box.
	if ( mins == maxs )
		return NULL;
	for ( int i = 0; i < m_bboxCache.Count(); i++ )
	{
		if ( m_bboxCache[i].mins == mins && m_bboxCache[i].maxs == maxs )
			return m_bboxCache[i].pCollide;
	}
	CPhysConvex *pConvex = BBoxToConvex( mins, maxs );
	CPhysCollide *pCollide = pConvex ? ConvertConvexToCollide( &pConvex, 1 ) : NULL;
	if ( pCollide )
	{
		BBoxCache_t entry = { mins, maxs, pCollide };
		m_bboxCache.AddToTail( entry );
	}
	return pCollide;
}

bool CPhysicsCollisionBox3D::GetBBoxCacheSize( int *pCachedSize, int *pCachedCount )
{
	*pCachedCount = m_bboxCache.Count();
	*pCachedSize = 0;
	for ( int i = 0; i < m_bboxCache.Count(); i++ )
		*pCachedSize += CollideSize( m_bboxCache[i].pCollide );
	return true;
}

//-----------------------------------------------------------------------------
// Triangle collides: polysoups and virtual meshes
//-----------------------------------------------------------------------------
CPhysCollideBox3D *CreateTriangleCollide( const Vector *pVertices, const int *pIndices, int triangleCount,
	const unsigned char *pMaterials )
{
	CPhysCollideBox3D *pCollide = new CPhysCollideBox3D;
	for ( int t = 0; t < triangleCount; t++ )
	{
		Vector points[3] = { pVertices[pIndices[t * 3]], pVertices[pIndices[t * 3 + 1]], pVertices[pIndices[t * 3 + 2]] };
		if ( CrossProduct( points[1] - points[0], points[2] - points[0] ).LengthSqr() < 1e-8f )
			continue;	// degenerate
		CPhysConvexBox3D *pConvex = new CPhysConvexBox3D;
		for ( int k = 0; k < 3; k++ )
		{
			pConvex->points.AddToTail( points[k] );
			pConvex->triangles.AddToTail( (unsigned short)k );
		}
		pConvex->triangleMaterials.AddToTail( pMaterials ? pMaterials[t] : 0 );
		BuildHulls( points, 3, pConvex->triangles.Base(), 3, pConvex );
		pCollide->convexes.AddToTail( pConvex );
	}
	FinalizeCollide( pCollide, true );
	return pCollide;
}

struct CPhysPolysoupBox3D
{
	CUtlVector<Vector> vertices;
	CUtlVector<unsigned char> materials;
};

CPhysPolysoup *CPhysicsCollisionBox3D::PolysoupCreate( void )
{
	return reinterpret_cast<CPhysPolysoup *>( new CPhysPolysoupBox3D );
}

void CPhysicsCollisionBox3D::PolysoupDestroy( CPhysPolysoup *pSoup )
{
	delete reinterpret_cast<CPhysPolysoupBox3D *>( pSoup );
}

void CPhysicsCollisionBox3D::PolysoupAddTriangle( CPhysPolysoup *pSoup, const Vector &a, const Vector &b, const Vector &c, int materialIndex7bits )
{
	CPhysPolysoupBox3D *pBox = reinterpret_cast<CPhysPolysoupBox3D *>( pSoup );
	if ( !pBox )
		return;
	pBox->vertices.AddToTail( a );
	pBox->vertices.AddToTail( b );
	pBox->vertices.AddToTail( c );
	pBox->materials.AddToTail( (unsigned char)( materialIndex7bits & 0x7F ) );
}

CPhysCollide *CPhysicsCollisionBox3D::ConvertPolysoupToCollide( CPhysPolysoup *pSoup, bool useMOPP )
{
	CPhysPolysoupBox3D *pBox = reinterpret_cast<CPhysPolysoupBox3D *>( pSoup );
	int triangleCount = pBox ? pBox->materials.Count() : 0;
	if ( !triangleCount )
		return NULL;
	CUtlVector<int> indices;
	for ( int i = 0; i < triangleCount * 3; i++ )
		indices.AddToTail( i );
	CPhysCollideBox3D *pCollide = CreateTriangleCollide( pBox->vertices.Base(), indices.Base(), triangleCount, pBox->materials.Base() );
	if ( !pCollide->convexes.Count() )
	{
		delete pCollide;
		return NULL;
	}
	return reinterpret_cast<CPhysCollide *>( pCollide );
}

// IVP builds the virtual mesh lazily from the handler; its whole triangle
// list is available up front, so the collide is built once here.
CPhysCollide *CPhysicsCollisionBox3D::CreateVirtualMesh( const virtualmeshparams_t &params )
{
	if ( !params.pMeshEventHandler )
		return NULL;
	virtualmeshlist_t *pList = new virtualmeshlist_t;
	memset( pList, 0, sizeof( *pList ) );
	params.pMeshEventHandler->GetVirtualMesh( params.userData, pList );
	CPhysCollide *pResult = NULL;
	if ( pList->pVerts && pList->triangleCount > 0 )
	{
		int triangleCount = MIN( pList->triangleCount, MAX_VIRTUAL_TRIANGLES );
		CUtlVector<int> indices;
		for ( int i = 0; i < triangleCount * 3; i++ )
			indices.AddToTail( pList->indices[i] );
		CPhysCollideBox3D *pCollide = CreateTriangleCollide( pList->pVerts, indices.Base(), triangleCount, NULL );
		if ( pCollide->convexes.Count() )
			pResult = reinterpret_cast<CPhysCollide *>( pCollide );
		else
			delete pCollide;
	}
	delete pList;
	return pResult;
}

// IVP sweeps the box against the cone and reports a start-solid overlap;
// the same question as a GJK distance between the box corners and the cone
// (apex plus a fine polygon for its base rim).
bool CPhysicsCollisionBox3D::IsBoxIntersectingCone( const Vector &boxAbsMins, const Vector &boxAbsMaxs, const truncatedcone_t &cone )
{
	const int kRimPoints = 32;
	b3Vec3 box[8];
	for ( int i = 0; i < 8; i++ )
	{
		Vector corner( ( i & 1 ) ? boxAbsMaxs.x : boxAbsMins.x, ( i & 2 ) ? boxAbsMaxs.y : boxAbsMins.y, ( i & 4 ) ? boxAbsMaxs.z : boxAbsMins.z );
		box[i] = ToB3( corner );
	}
	Vector axis = cone.normal;
	if ( VectorNormalize( axis ) < 1e-6f )
		return false;
	Vector right, up;
	VectorVectors( axis, right, up );
	float radius = cone.h * tanf( DEG2RAD( cone.theta ) );
	Vector baseCenter = cone.origin + axis * cone.h;
	b3Vec3 conePoints[kRimPoints + 1];
	conePoints[0] = ToB3( cone.origin );
	for ( int i = 0; i < kRimPoints; i++ )
	{
		float angle = 2.0f * M_PI_F * i / kRimPoints;
		conePoints[i + 1] = ToB3( baseCenter + ( right * cosf( angle ) + up * sinf( angle ) ) * radius );
	}
	b3DistanceInput input;
	memset( &input, 0, sizeof( input ) );
	input.proxyA.points = box;
	input.proxyA.count = 8;
	input.proxyB.points = conePoints;
	input.proxyB.count = kRimPoints + 1;
	input.transform = b3Transform_identity;
	input.useRadii = false;
	b3SimplexCache cache;
	memset( &cache, 0, sizeof( cache ) );
	b3DistanceOutput output = b3ShapeDistance( &input, &cache, NULL, 0 );
	return output.distance <= 1e-3f;
}

void CPhysicsCollisionBox3D::OutputDebugInfo( const CPhysCollide *pCollide )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollide );
	if ( !pBox )
		return;
	int hulls = 0;
	for ( int i = 0; i < pBox->convexes.Count(); i++ )
		hulls += pBox->convexes[i]->hulls.Count();
	Msg( "Box3D collide: %d convexes (%d hulls), volume %.2f, bounds (%.2f %.2f %.2f)-(%.2f %.2f %.2f)\n", pBox->convexes.Count(), hulls,
		pBox->volume, pBox->mins.x, pBox->mins.y, pBox->mins.z, pBox->maxs.x, pBox->maxs.y, pBox->maxs.z );
}

int CPhysicsCollisionBox3D::GetConvexesUsedInCollideable( const CPhysCollide *pCollideable, CPhysConvex **pOutputArray, int iOutputArrayLimit )
{
	const CPhysCollideBox3D *pCollide = ToBox3D( pCollideable );
	int count = 0;
	for ( ; pCollide && count < pCollide->convexes.Count() && count < iOutputArrayLimit; count++ )
		pOutputArray[count] = reinterpret_cast<CPhysConvex *>( pCollide->convexes[count] );
	return count;
}

float CPhysicsCollisionBox3D::CollideVolume( CPhysCollide *pCollide )
{
	return pCollide ? ToBox3D( pCollide )->volume : 0.0f;
}

float CPhysicsCollisionBox3D::CollideSurfaceArea( CPhysCollide *pCollide )
{
	float area = 0.0f;
	const CPhysCollideBox3D *pBox = pCollide ? ToBox3D( pCollide ) : NULL;
	for ( int i = 0; pBox && i < pBox->convexes.Count(); i++ )
		area += ConvexSurfaceArea( reinterpret_cast<CPhysConvex *>( pBox->convexes[i] ) );
	return area;
}

Vector CPhysicsCollisionBox3D::CollideGetExtent( const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, const Vector &direction )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollide );
	if ( !pBox || !pBox->convexes.Count() )
		return collideOrigin;
	matrix3x4_t xform;
	AngleMatrix( collideAngles, collideOrigin, xform );
	Vector best = collideOrigin;
	float bestDot = -FLT_MAX;
	for ( int c = 0; c < pBox->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pBox->convexes[c];
		for ( int i = 0; i < pConvex->points.Count(); i++ )
		{
			Vector world;
			VectorTransform( pConvex->points[i], xform, world );
			float dot = DotProduct( world, direction );
			if ( dot > bestDot )
			{
				bestDot = dot;
				best = world;
			}
		}
	}
	return best;
}

void CPhysicsCollisionBox3D::CollideGetAABB( Vector *pMins, Vector *pMaxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollide );
	if ( !pBox || !pBox->convexes.Count() )
	{
		*pMins = *pMaxs = collideOrigin;
		return;
	}
	if ( collideAngles == vec3_angle )
	{
		*pMins = pBox->mins + collideOrigin;
		*pMaxs = pBox->maxs + collideOrigin;
		return;
	}
	matrix3x4_t xform;
	AngleMatrix( collideAngles, collideOrigin, xform );
	pMins->Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pMaxs->Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	for ( int c = 0; c < pBox->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pBox->convexes[c];
		for ( int i = 0; i < pConvex->points.Count(); i++ )
		{
			Vector world;
			VectorTransform( pConvex->points[i], xform, world );
			VectorMin( *pMins, world, *pMins );
			VectorMax( *pMaxs, world, *pMaxs );
		}
	}
}

void CPhysicsCollisionBox3D::CollideGetMassCenter( CPhysCollide *pCollide, Vector *pOutMassCenter )
{
	*pOutMassCenter = pCollide ? ToBox3D( pCollide )->massCenter : vec3_origin;
}

void CPhysicsCollisionBox3D::CollideSetMassCenter( CPhysCollide *pCollide, const Vector &massCenter )
{
	if ( pCollide )
		ToBox3D( pCollide )->massCenter = massCenter;
}

Vector CPhysicsCollisionBox3D::CollideGetOrthographicAreas( const CPhysCollide *pCollide )
{
	return pCollide ? ToBox3D( pCollide )->orthoAreas : Vector( 1, 1, 1 );
}

void CPhysicsCollisionBox3D::CollideSetOrthographicAreas( CPhysCollide *pCollide, const Vector &areas )
{
	if ( pCollide )
		ToBox3D( pCollide )->orthoAreas = areas;
}

int CPhysicsCollisionBox3D::CollideIndex( const CPhysCollide *pCollide )
{
	return pCollide ? ToBox3D( pCollide )->index : 0;
}

//-----------------------------------------------------------------------------
// Traces
//-----------------------------------------------------------------------------
namespace
{
// IVP ends a sweep clipEpsilon from the surface, measured along the hit
// normal (DIST_EPSILON for traces, the collision tolerance for collide
// sweeps). A ray cast is exact: back it off along the normal.
float ClipRayToEpsilon( const b3Vec3 &localDelta, const b3CastOutput &output, float clipEpsilon )
{
	float length = b3Length( localDelta );
	if ( length <= 0.0f )
		return output.fraction;
	float cosine = fabsf( b3Dot( localDelta, output.normal ) ) / length;
	if ( cosine < 1e-3f )
		return output.fraction;
	float fraction = output.fraction - clipEpsilon / ( cosine * length );
	return fraction < 0.0f ? 0.0f : fraction;
}

// True when the point set penetrates the hull. As IVP, a trace starts solid
// only inside a surface: b3OverlapHull also counts anything within Box3D's
// overlap slop (about 0.02 inch here), less than DIST_EPSILON, so a prop
// settling against a mover would leave all of the mover's traces solid.
// GJK reports a few millionths of an inch, not zero, for some overlaps.
const float kPenetrationDistance = 1e-4f;

bool PointsPenetrateHull( const b3HullData *pHull, const b3ShapeProxy &proxy )
{
	b3DistanceInput input;
	memset( &input, 0, sizeof( input ) );
	input.proxyA.points = b3GetHullPoints( pHull );
	input.proxyA.count = pHull->vertexCount;
	input.proxyB = proxy;
	input.transform = b3Transform_identity;
	input.useRadii = false;
	b3SimplexCache cache;
	memset( &cache, 0, sizeof( cache ) );
	return b3ShapeDistance( &input, &cache, NULL, 0 ).distance <= kPenetrationDistance;
}

// Sweeps a point set (not overlapping the hull) until it is clipEpsilon from
// the hull, following IVP's CTraceSolver::SweepSingleConvex. Box3D's own
// shape cast is not used: it reports any start within its linear slop
// (about a quarter inch here) as an initial overlap, a hit at fraction 0 with
// no normal, even when moving away. Traces leave movers DIST_EPSILON from
// surfaces, so every following trace from there would be blocked.
bool SweepPointsToSeparation( const b3HullData *pHull, const b3Vec3 *pLocalPoints, int pointCount,
    const b3Vec3 &localDelta, float clipEpsilon, float maxFraction, float *pFraction,
    b3Vec3 *pNormal )
{
	const float kTolerance = 0.25f * clipEpsilon;
	const int kMaxIterations = 32;
	b3Vec3 moved[B3_MAX_SHAPE_CAST_POINTS];
	b3DistanceInput input;
	memset( &input, 0, sizeof( input ) );
	input.proxyA.points = b3GetHullPoints( pHull );
	input.proxyA.count = pHull->vertexCount;
	input.proxyB.points = moved;
	input.transform = b3Transform_identity;
	input.useRadii = false;
	b3SimplexCache cache;

	// The volume a translated convex point set sweeps is the hull of its
	// start and end points. When that stays apart from the hull, the sweep
	// hits only if it ends within clipEpsilon while still closing on the
	// surface; one that merely grazes past (or runs along a surface it starts
	// against) misses. Closing is measured over the whole sweep.
	if ( 2 * pointCount <= B3_MAX_SHAPE_CAST_POINTS )
	{
		for ( int i = 0; i < pointCount; i++ )
		{
			moved[i] = pLocalPoints[i];
			moved[pointCount + i] = b3Add( pLocalPoints[i], localDelta );
		}
		input.proxyB.count = 2 * pointCount;
		memset( &cache, 0, sizeof( cache ) );
		b3DistanceOutput swept = b3ShapeDistance( &input, &cache, NULL, 0 );
		if ( swept.distance >= clipEpsilon )
			return false;
		if ( swept.distance > kPenetrationDistance )
		{
			float closing = -b3Dot( localDelta, swept.normal );
			if ( !( closing > 0.1f * clipEpsilon ||
			         ( closing > 1e-4f && swept.distance < 0.9f * clipEpsilon ) ) )
				return false;
			float fraction = 1.0f - ( clipEpsilon - swept.distance ) / closing;
			*pFraction = fraction > 0.0f ? fraction : 0.0f;
			*pNormal = swept.normal;
			return *pFraction < maxFraction;
		}
	}

	// The sweep reaches the hull: advance conservatively on the exact
	// distance until it is clipEpsilon.
	input.proxyB.count = pointCount;
	memset( &cache, 0, sizeof( cache ) );
	float alpha = 0.0f;
	for ( int iteration = 0; iteration < kMaxIterations; iteration++ )
	{
		for ( int i = 0; i < pointCount; i++ )
			moved[i] = b3MulAdd( pLocalPoints[i], alpha, localDelta );
		b3DistanceOutput output = b3ShapeDistance( &input, &cache, NULL, 0 );
		// The normal points from the hull toward the swept points.
		float closing = -b3Dot( localDelta, output.normal ) * ( 1.0f - alpha );
		if ( output.distance < clipEpsilon + kTolerance &&
		     ( iteration > 0 || output.distance <= clipEpsilon ) )
		{
			if ( closing <= 0.0f )
				return false; // touching, but moving away or along it
			*pFraction = alpha;
			*pNormal = output.normal;
			return true;
		}
		if ( closing <= 0.0f )
			return false;
		alpha += ( output.distance - clipEpsilon ) / closing * ( 1.0f - alpha );
		if ( alpha >= maxFraction )
			return false;
	}
	return false;
}
}

void SweepPointsAgainstCollide( const Vector *pWorldPoints, int pointCount, const Vector &delta,
	const CPhysCollideBox3D *pCollide, const Vector &origin, const QAngle &angles,
	unsigned int contentsMask, IConvexInfo *pConvexInfo, float clipEpsilon, trace_t *ptr )
{
	if ( !pCollide || pointCount <= 0 || pointCount > B3_MAX_SHAPE_CAST_POINTS )
		return;
	if ( !pConvexInfo )
		pConvexInfo = &s_defaultConvexInfo;

	b3Transform xform = ToB3Transform( origin, angles );
	b3Vec3 localPoints[B3_MAX_SHAPE_CAST_POINTS];
	b3Vec3 endPoints[B3_MAX_SHAPE_CAST_POINTS];
	b3Vec3 localDelta = b3InvRotateVector( xform.q, ToB3( delta ) );
	for ( int i = 0; i < pointCount; i++ )
	{
		localPoints[i] = b3InvTransformPoint( xform, ToB3( pWorldPoints[i] ) );
		endPoints[i] = b3Add( localPoints[i], localDelta );
	}

	b3ShapeProxy startProxy;
	memset( &startProxy, 0, sizeof( startProxy ) );
	startProxy.points = localPoints;
	startProxy.count = pointCount;
	b3ShapeProxy endProxy = startProxy;
	endProxy.points = endPoints;

	for ( int c = 0; c < pCollide->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pCollide->convexes[c];
		if ( !pConvex->hulls.Count() )
			continue;
		unsigned int contents = pConvexInfo->GetContents( pConvex->gameData );
		if ( !( contents & contentsMask ) )
			continue;

		for ( int h = 0; h < pConvex->hulls.Count(); h++ )
		{
			const b3HullData *pHull = pConvex->hulls[h];
			if ( PointsPenetrateHull( pHull, startProxy ) )
			{
				ptr->startsolid = true;
				ptr->fraction = 0.0f;
				ptr->contents = contents;
				if ( PointsPenetrateHull( pHull, endProxy ) )
					ptr->allsolid = true;
				continue;
			}
			if ( ptr->startsolid )
				continue;

			float fraction;
			b3Vec3 normal;
			if ( pointCount == 1 )
			{
				b3RayCastInput ray;
				ray.origin = localPoints[0];
				ray.translation = localDelta;
				ray.maxFraction = 1.0f;
				b3CastOutput output = b3RayCastHull( pHull, &ray );
				if ( !output.hit )
					continue;
				fraction = ClipRayToEpsilon( localDelta, output, clipEpsilon );
				normal = output.normal;
			}
			else if ( !SweepPointsToSeparation( pHull, localPoints, pointCount, localDelta,
			              clipEpsilon, ptr->fraction, &fraction, &normal ) )
			{
				continue;
			}
			if ( fraction < ptr->fraction )
			{
				ptr->fraction = fraction;
				ptr->plane.normal = FromB3( b3RotateVector( xform.q, normal ) );
				ptr->contents = contents;
			}
		}
	}
}

namespace
{
// Box3D rests and pushes bodies on soft contacts, which pitch a prop the
// player pushes by a fraction of a milliradian (IVP's stiff contacts do not).
// Movement slides along the hit plane, and any rise that gives is taken for a
// step, which stops CBasePlayer::PostThinkVPhysics from pushing: report
// near-vertical planes of swept traces as vertical.
const float kVerticalNormalTolerance = 0.005f;

void SnapNearVerticalNormal( trace_t *ptr )
{
	if ( ptr->DidHit() && !ptr->startsolid && ptr->plane.normal.z != 0.0f &&
	     fabsf( ptr->plane.normal.z ) < kVerticalNormalTolerance )
	{
		ptr->plane.normal.z = 0.0f;
		VectorNormalize( ptr->plane.normal );
	}
}

void FinishTrace( const Vector &startPos, const Vector &delta, trace_t *ptr )
{
	ptr->startpos = startPos;
	VectorMA( startPos, ptr->fraction, delta, ptr->endpos );
	if ( ptr->DidHit() && !ptr->startsolid )
		ptr->plane.dist = DotProduct( ptr->endpos, ptr->plane.normal );
}
}

void CPhysicsCollisionBox3D::TraceBox( const Vector &start, const Vector &end, const Vector &mins, const Vector &maxs, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr )
{
	Ray_t ray;
	ray.Init( start, end, mins, maxs );
	TraceBox( ray, MASK_ALL, NULL, pCollide, collideOrigin, collideAngles, ptr );
}

void CPhysicsCollisionBox3D::TraceBox( const Ray_t &ray, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr )
{
	TraceBox( ray, MASK_ALL, NULL, pCollide, collideOrigin, collideAngles, ptr );
}

void CPhysicsCollisionBox3D::TraceBox( const Ray_t &ray, unsigned int contentsMask, IConvexInfo *pConvexInfo, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr )
{
	ClearTrace( ptr );
	Vector points[8];
	int count = RayProxyPoints( ray, points );
	SweepPointsAgainstCollide( points, count, ray.m_Delta, ToBox3D( pCollide ), collideOrigin, collideAngles, contentsMask, pConvexInfo, DIST_EPSILON, ptr );
	if ( !ray.m_IsRay )
		SnapNearVerticalNormal( ptr );
	FinishTrace( ray.m_Start + ray.m_StartOffset, ray.m_Delta, ptr );
}

void CPhysicsCollisionBox3D::TraceCollide( const Vector &start, const Vector &end, const CPhysCollide *pSweepCollide, const QAngle &sweepAngles, const CPhysCollide *pCollide, const Vector &collideOrigin, const QAngle &collideAngles, trace_t *ptr )
{
	ClearTrace( ptr );
	const CPhysCollideBox3D *pSweep = ToBox3D( pSweepCollide );
	Vector delta = end - start;
	matrix3x4_t sweepXform;
	AngleMatrix( sweepAngles, start, sweepXform );
	for ( int c = 0; pSweep && c < pSweep->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pSweep->convexes[c];
		// Cast each hull piece with its (at most B3_MAX_SHAPE_CAST_POINTS) vertices.
		for ( int h = 0; h < pConvex->hulls.Count(); h++ )
		{
			const b3HullData *pHull = pConvex->hulls[h];
			const b3Vec3 *pPoints = b3GetHullPoints( pHull );
			CUtlVector<Vector> world;
			for ( int i = 0; i < pHull->vertexCount; i++ )
			{
				Vector point;
				VectorTransform( FromB3( pPoints[i] ), sweepXform, point );
				world.AddToTail( point );
			}
			trace_t tr;
			ClearTrace( &tr );
			SweepPointsAgainstCollide( world.Base(), world.Count(), delta, ToBox3D( pCollide ), collideOrigin, collideAngles, MASK_ALL, NULL, kCollideSweepEpsilon, &tr );
			if ( tr.startsolid || tr.fraction < ptr->fraction )
			{
				bool wasStartSolid = ptr->startsolid;
				*ptr = tr;
				ptr->startsolid |= wasStartSolid;
			}
		}
	}
	FinishTrace( start, delta, ptr );
}

//-----------------------------------------------------------------------------
// VCollide
//-----------------------------------------------------------------------------
void CPhysicsCollisionBox3D::VCollideLoad( vcollide_t *pOutput, int solidCount, const char *pBuffer, int bufferSize, bool swap )
{
	// Same on-disk layout the IVP provider reads: each solid is length-prefixed
	// and the trailing bytes are the authored keyvalue text.
	memset( pOutput, 0, sizeof( *pOutput ) );
	pOutput->solidCount = solidCount;
	pOutput->solids = new CPhysCollide *[solidCount];

	int position = 0;
	for ( int i = 0; i < solidCount; i++ )
	{
		int solidSize = 0;
		if ( position + (int)sizeof( int ) <= bufferSize )
			memcpy( &solidSize, pBuffer + position, sizeof( int ) );
		position += sizeof( int );
		if ( solidSize < 0 || solidSize > bufferSize - position )
			solidSize = 0;
		CPhysCollideBox3D *pSolid = swap ? NULL : DecodeSolid( pBuffer + position, solidSize, i );
		if ( !pSolid )
		{
			// Unsupported or malformed geometry keeps the vcollide shape (one
			// entry per solid) but has no convex pieces, so it never collides.
			Warning( "vphysics_box3d: solid %d of %d could not be decoded\n", i, solidCount );
			pSolid = new CPhysCollideBox3D;
			pSolid->index = i;
		}
		pOutput->solids[i] = reinterpret_cast<CPhysCollide *>( pSolid );
		position += solidSize;
	}

	pOutput->isPacked = false;
	int keySize = bufferSize - position;
	if ( keySize < 0 )
		keySize = 0;
	pOutput->pKeyValues = new char[keySize + 1];
	if ( keySize > 0 )
		memcpy( pOutput->pKeyValues, pBuffer + position, keySize );
	pOutput->pKeyValues[keySize] = '\0';
	pOutput->descSize = 0;
}

void CPhysicsCollisionBox3D::VCollideUnload( vcollide_t *pVCollide )
{
	for ( int i = 0; i < pVCollide->solidCount; i++ )
		delete ToBox3D( pVCollide->solids[i] );
	delete[] pVCollide->solids;
	delete[] pVCollide->pKeyValues;
	memset( pVCollide, 0, sizeof( *pVCollide ) );
}

//-----------------------------------------------------------------------------
// Debug and query models
//-----------------------------------------------------------------------------
int CPhysicsCollisionBox3D::CreateDebugMesh( CPhysCollide const *pCollisionModel, Vector **outVerts )
{
	const CPhysCollideBox3D *pBox = ToBox3D( pCollisionModel );
	int count = 0;
	for ( int c = 0; pBox && c < pBox->convexes.Count(); c++ )
		count += pBox->convexes[c]->triangles.Count();
	*outVerts = count ? new Vector[count] : NULL;
	int out = 0;
	for ( int c = 0; pBox && c < pBox->convexes.Count(); c++ )
	{
		const CPhysConvexBox3D *pConvex = pBox->convexes[c];
		for ( int i = 0; i < pConvex->triangles.Count(); i++ )
			( *outVerts )[out++] = pConvex->points[pConvex->triangles[i]];
	}
	return count;
}

void CPhysicsCollisionBox3D::DestroyDebugMesh( int vertCount, Vector *outVerts )
{
	delete[] outVerts;
}

namespace
{
class CCollisionQueryBox3D : public ICollisionQuery
{
public:
	explicit CCollisionQueryBox3D( CPhysCollideBox3D *pCollide ) : m_pCollide( pCollide ) {}

	virtual int ConvexCount( void ) { return m_pCollide->convexes.Count(); }
	virtual int TriangleCount( int convexIndex ) { return Convex( convexIndex ) ? Convex( convexIndex )->triangles.Count() / 3 : 0; }
	virtual unsigned int GetGameData( int convexIndex ) { return Convex( convexIndex ) ? Convex( convexIndex )->gameData : 0; }
	virtual void GetTriangleVerts( int convexIndex, int triangleIndex, Vector *verts )
	{
		CPhysConvexBox3D *pConvex = Convex( convexIndex );
		if ( !pConvex || triangleIndex < 0 || triangleIndex * 3 + 2 >= pConvex->triangles.Count() )
			return;
		for ( int i = 0; i < 3; i++ )
			verts[i] = pConvex->points[pConvex->triangles[triangleIndex * 3 + i]];
	}
	virtual void SetTriangleVerts( int convexIndex, int triangleIndex, const Vector *verts ) {}
	virtual int GetTriangleMaterialIndex( int convexIndex, int triangleIndex )
	{
		CPhysConvexBox3D *pConvex = Convex( convexIndex );
		return pConvex && pConvex->triangleMaterials.IsValidIndex( triangleIndex ) ? pConvex->triangleMaterials[triangleIndex] : 0;
	}
	virtual void SetTriangleMaterialIndex( int convexIndex, int triangleIndex, int index7bits )
	{
		CPhysConvexBox3D *pConvex = Convex( convexIndex );
		if ( pConvex && pConvex->triangleMaterials.IsValidIndex( triangleIndex ) )
			pConvex->triangleMaterials[triangleIndex] = (unsigned char)( index7bits & 0x7F );
	}

private:
	CPhysConvexBox3D *Convex( int index )
	{
		return m_pCollide->convexes.IsValidIndex( index ) ? m_pCollide->convexes[index] : NULL;
	}

	CPhysCollideBox3D *m_pCollide;
};
}

ICollisionQuery *CPhysicsCollisionBox3D::CreateQueryModel( CPhysCollide *pCollide )
{
	return pCollide ? new CCollisionQueryBox3D( ToBox3D( pCollide ) ) : NULL;
}

void CPhysicsCollisionBox3D::DestroyQueryModel( ICollisionQuery *pQuery )
{
	delete pQuery;
}
