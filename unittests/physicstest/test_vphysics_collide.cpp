//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VPhysics provider conformance: triangle collides (polysoups and
//          virtual meshes), the shared bbox cache, the box/cone query, and
//          collide serialization in the legacy compact-surface format.
//          Contract record: unittests/physicstest/contracts/vphysics.provider.v1.md
//
//          The serialized bytes are checked by an independent reader of the
//          IVP layout (see vphysics_box3d/legacy_collision.cpp for the field
//          table) so the same structural rules are proven against IVP's own
//          writer before they are required of another provider.
//
//=============================================================================//

#include <math.h>
#include <string.h>

#include "vphysics_conformance.h"
#include "vphysics/virtualmesh.h"
#include "vcollide.h"
#include "cmodel.h"
#include "gametrace.h"
#include "utlvector.h"

namespace
{
// A 5x5-vertex grid 512 units wide with the center vertex raised 32 units:
// the eight triangles around the center slope, the rest are flat at z = 0.
const int kGridSide = 5;
const float kGridSpacing = 128.0f;
const float kSpikeHeight = 32.0f;
const Vector kMeshOrigin( -4000, 0, 0 );

struct Grid_t
{
	Vector verts[kGridSide * kGridSide];
	unsigned short indices[( kGridSide - 1 ) * ( kGridSide - 1 ) * 6];
	int triangleCount;

	Grid_t()
	{
		for ( int j = 0; j < kGridSide; j++ )
		{
			for ( int i = 0; i < kGridSide; i++ )
			{
				bool center = i == kGridSide / 2 && j == kGridSide / 2;
				verts[j * kGridSide + i] = Vector( ( i - kGridSide / 2 ) * kGridSpacing, ( j - kGridSide / 2 ) * kGridSpacing,
					center ? kSpikeHeight : 0.0f );
			}
		}
		triangleCount = 0;
		for ( int j = 0; j < kGridSide - 1; j++ )
		{
			for ( int i = 0; i < kGridSide - 1; i++ )
			{
				unsigned short a = j * kGridSide + i, b = a + 1, c = a + kGridSide, d = c + 1;
				unsigned short quad[6] = { a, b, d, a, d, c };
				memcpy( &indices[triangleCount * 3], quad, sizeof( quad ) );
				triangleCount += 2;
			}
		}
	}
};

Grid_t s_grid;

class CGridMesh : public IVirtualMeshEvent
{
public:
	CGridMesh() : m_calls( 0 ) {}
	virtual void GetVirtualMesh( void *userData, virtualmeshlist_t *pList )
	{
		m_calls++;
		pList->pVerts = s_grid.verts;
		pList->vertexCount = kGridSide * kGridSide;
		pList->triangleCount = s_grid.triangleCount;
		pList->indexCount = s_grid.triangleCount * 3;
		pList->surfacePropsIndex = 0;
		pList->pHull = NULL;
		memcpy( pList->indices, s_grid.indices, sizeof( unsigned short ) * s_grid.triangleCount * 3 );
	}
	virtual void GetWorldspaceBounds( void *userData, Vector *pMins, Vector *pMaxs )
	{
		// "Worldspace" is the mesh object's space: the engine's displacement
		// meshes belong to the world object at the origin.
		*pMins = Vector( -256, -256, 0 );
		*pMaxs = Vector( 256, 256, kSpikeHeight );
	}
	virtual void GetTrianglesInSphere( void *userData, const Vector &center, float radius, virtualmeshtrianglelist_t *pList )
	{
		// Triangle numbers, one per triangle (all of them: the mesh is small).
		pList->triangleCount = s_grid.triangleCount;
		for ( int i = 0; i < s_grid.triangleCount; i++ )
			pList->triangleIndices[i] = (unsigned short)i;
	}
	int m_calls;
};

// A 17x17-vertex grid (512 triangles, a power-4 displacement's size) 512
// units wide, flat but for a raised center vertex (IVP's bounding hull of a
// perfectly planar mesh is degenerate and collides with nothing).
class CLargeGridMesh : public IVirtualMeshEvent
{
public:
	CLargeGridMesh()
	{
		for ( int j = 0; j < kSide; j++ )
		{
			for ( int i = 0; i < kSide; i++ )
				m_verts[j * kSide + i] = Vector( i * 32.0f - 256.0f, j * 32.0f - 256.0f, i == kSide / 2 && j == kSide / 2 ? 8.0f : 0.0f );
		}
		m_triangleCount = 0;
		for ( int j = 0; j < kSide - 1; j++ )
		{
			for ( int i = 0; i < kSide - 1; i++ )
			{
				unsigned short a = j * kSide + i, b = a + 1, c = a + kSide, d = c + 1;
				unsigned short quad[6] = { a, b, d, a, d, c };
				memcpy( &m_indices[m_triangleCount * 3], quad, sizeof( quad ) );
				m_triangleCount += 2;
			}
		}
	}
	virtual void GetVirtualMesh( void *userData, virtualmeshlist_t *pList )
	{
		pList->pVerts = m_verts;
		pList->vertexCount = kSide * kSide;
		pList->triangleCount = m_triangleCount;
		pList->indexCount = m_triangleCount * 3;
		pList->surfacePropsIndex = 0;
		pList->pHull = NULL;
		memcpy( pList->indices, m_indices, sizeof( unsigned short ) * m_triangleCount * 3 );
	}
	virtual void GetWorldspaceBounds( void *userData, Vector *pMins, Vector *pMaxs )
	{
		*pMins = Vector( -256, -256, 0 );
		*pMaxs = Vector( 256, 256, 8 );
	}
	virtual void GetTrianglesInSphere( void *userData, const Vector &center, float radius, virtualmeshtrianglelist_t *pList )
	{
		pList->triangleCount = m_triangleCount;
		for ( int i = 0; i < m_triangleCount; i++ )
			pList->triangleIndices[i] = (unsigned short)i;
	}

private:
	static const int kSide = 17;
	Vector m_verts[kSide * kSide];
	unsigned short m_indices[( kSide - 1 ) * ( kSide - 1 ) * 6];
	int m_triangleCount;
};

// Sensitivity: "soup-null" models the former provider that built no
// triangle collides; "collide-write-stub" one that serializes nothing.
CPhysCollide *Soup( CPhysCollide *pCollide )
{
	if ( pCollide && FaultIs( "soup-null" ) )
	{
		s_pCollision->DestroyCollide( pCollide );
		return NULL;
	}
	return pCollide;
}

int WriteSize( CPhysCollide *pCollide )
{
	return FaultIs( "collide-write-stub" ) ? 0 : s_pCollision->CollideSize( pCollide );
}

float TraceDown( CPhysCollide *pCollide, const Vector &origin, float x, float y )
{
	trace_t tr;
	Vector start = origin + Vector( x, y, 256 ), end = origin + Vector( x, y, -256 );
	// A small box: IVP's zero-extent rays can slip past thin flat ledges.
	Vector half( 0.5f, 0.5f, 0.5f );
	s_pCollision->TraceBox( start, end, -half, half, pCollide, origin, vec3_angle, &tr );
	return tr.fraction < 1.0f ? tr.endpos.z - half.z - origin.z : -1000.0f;
}

// Shared triangle-collide clauses for a polysoup and a virtual mesh built
// from the same grid.
// IVP answers no bounds or trace queries on a virtual mesh (the engine
// traces displacements itself), so those clauses apply to polysoups only.
void CheckTriangleCollide( const char *pFamily, CPhysCollide *pCollide, bool queries )
{
	char name[128];
	V_snprintf( name, sizeof( name ), "%s.create", pFamily );
	if ( !Check( TIER_GAMEPLAY, name, pCollide != NULL ) )
		return;

	if ( queries )
	{
		Vector mins, maxs;
		s_pCollision->CollideGetAABB( &mins, &maxs, pCollide, kMeshOrigin, vec3_angle );
		V_snprintf( name, sizeof( name ), "%s.aabb", pFamily );
		// Flat pieces may carry a thin collision skin; the authored bounds hold.
		Check( TIER_GAMEPLAY, name, NearVec( mins, kMeshOrigin + Vector( -256, -256, 0 ), 1.0f ) &&
			NearVec( maxs, kMeshOrigin + Vector( 256, 256, kSpikeHeight ), 1.0f ),
			"(%.2f %.2f %.2f)-(%.2f %.2f %.2f)", mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z );
		V_snprintf( name, sizeof( name ), "%s.aabb-mins", pFamily );
		ObsVector( name, "a1", mins - kMeshOrigin );
		V_snprintf( name, sizeof( name ), "%s.aabb-maxs", pFamily );
		ObsVector( name, "a1", maxs - kMeshOrigin );

		// Traces meet the authored surface: flat, the spike and a slope.
		float flat = TraceDown( pCollide, kMeshOrigin, -200, -200 );
		// On a sloped triangle next to the spike: z = 32 - x / 4 + y / 4. (IVP's traces
	// miss some flat polysoup triangles outright, so the samples are points
	// it resolves.)
	float spike = TraceDown( pCollide, kMeshOrigin, 40, -40 );
		float slope = TraceDown( pCollide, kMeshOrigin, 64, 0 );
		V_snprintf( name, sizeof( name ), "%s.trace-surface", pFamily );
		Check( TIER_GAMEPLAY, name, Near( flat, 0.0f, 1.0f ) && Near( spike, kSpikeHeight - 20.0f, 1.5f ) && Near( slope, kSpikeHeight * 0.5f, 1.5f ),
			"flat %.2f spike %.2f slope %.2f", flat, spike, slope );
		float heights[3] = { flat, spike, slope };
		V_snprintf( name, sizeof( name ), "%s.trace-heights", pFamily );
		ObsFloats( name, "a1", 3, heights );
		V_snprintf( name, sizeof( name ), "%s.trace-outside", pFamily );
		Check( TIER_GAMEPLAY, name, TraceDown( pCollide, kMeshOrigin, 300, 0 ) < -999.0f );
	}

	// A static object of the mesh holds a cube resting on its flat corner.
	World_t world;
	if ( CreateWorld( world, NULL ) )
	{
		objectparams_t params = DefaultParams( 1.0f, NULL );
		IPhysicsObject *pMesh = world.pEnv->CreatePolyObjectStatic( pCollide, world.material, kMeshOrigin, vec3_angle, &params );
		IPhysicsObject *pCube = CreateCube( world, kMeshOrigin + Vector( -192, -192, 64 ) );
		Step( world.pEnv, 3.0f );
		Vector rest = PositionOf( pCube ) - kMeshOrigin;
		V_snprintf( name, sizeof( name ), "%s.supports-object", pFamily );
		Check( TIER_GAMEPLAY, name, pMesh != NULL && Near( rest.z, 16.0f, 1.0f ) && Near( rest.x, -192.0f, 4.0f ),
			"cube at (%.2f %.2f %.2f)", rest.x, rest.y, rest.z );
		V_snprintf( name, sizeof( name ), "%s.rest-height", pFamily );
		ObsFloats( name, "a1", 1, &rest.z );
		world.pEnv->DestroyObject( pCube );
		world.pEnv->DestroyObject( pMesh );
		DestroyWorld( world );
	}
}

void TestPolysoup()
{
	CPhysPolysoup *pSoup = s_pCollision->PolysoupCreate();
	Check( TIER_GAMEPLAY, "polysoup.create-soup", pSoup != NULL );
	if ( !pSoup )
		return;
	for ( int t = 0; t < s_grid.triangleCount; t++ )
	{
		const unsigned short *pTri = &s_grid.indices[t * 3];
		s_pCollision->PolysoupAddTriangle( pSoup, s_grid.verts[pTri[0]], s_grid.verts[pTri[1]], s_grid.verts[pTri[2]], t & 3 );
	}
	CPhysCollide *pCollide = Soup( s_pCollision->ConvertPolysoupToCollide( pSoup, false ) );
	s_pCollision->PolysoupDestroy( pSoup );
	CheckTriangleCollide( "polysoup", pCollide, true );
	if ( !pCollide )
		return;

	// One convex piece per triangle.
	CPhysConvex *pConvexes[64];
	int convexCount = s_pCollision->GetConvexesUsedInCollideable( pCollide, pConvexes, 64 );
	Check( TIER_GAMEPLAY, "polysoup.convex-per-triangle", convexCount == s_grid.triangleCount, "got %d", convexCount );
	printf( "OBS polysoup.convex-count x %d\n", convexCount );

	// Serializes and reads back like any other collide.
	int size = WriteSize( pCollide );
	CUtlVector<char> buffer;
	buffer.SetCount( MAX( size, 1 ) );
	int written = size > 0 ? s_pCollision->CollideWrite( buffer.Base(), pCollide, false ) : 0;
	CPhysCollide *pRead = written > 0 ? s_pCollision->UnserializeCollide( buffer.Base(), written, 0 ) : NULL;
	Check( TIER_GAMEPLAY, "collide.write-polysoup", pRead != NULL && Near( TraceDown( pRead, kMeshOrigin, 64, 0 ), kSpikeHeight * 0.5f, 1.5f ),
		"size %d written %d", size, written );
	if ( pRead )
		s_pCollision->DestroyCollide( pRead );
	s_pCollision->DestroyCollide( pCollide );

	// An empty soup has nothing to build.
	CPhysPolysoup *pEmpty = s_pCollision->PolysoupCreate();
	CPhysCollide *pNone = s_pCollision->ConvertPolysoupToCollide( pEmpty, false );
	Check( TIER_BOOT, "polysoup.empty-null", pNone == NULL );
	if ( pNone )
		s_pCollision->DestroyCollide( pNone );
	s_pCollision->PolysoupDestroy( pEmpty );
}

void TestVirtualMesh()
{
	Check( TIER_BOOT, "virtualmesh.supported", s_pCollision->SupportsVirtualMesh() );
	CGridMesh mesh;
	virtualmeshparams_t params;
	params.pMeshEventHandler = &mesh;
	params.userData = NULL;
	params.buildOuterHull = true;
	CPhysCollide *pCollide = Soup( s_pCollision->CreateVirtualMesh( params ) );
	Check( TIER_GAMEPLAY, "virtualmesh.queries-handler", mesh.m_calls > 0 );
	CheckTriangleCollide( "virtualmesh", pCollide, false );
	if ( pCollide )
		s_pCollision->DestroyCollide( pCollide );

	// A displacement-sized mesh works over its whole extent, and object
	// state (collisions off) reaches every triangle.
	CLargeGridMesh large;
	params.pMeshEventHandler = &large;
	CPhysCollide *pLarge = Soup( s_pCollision->CreateVirtualMesh( params ) );
	World_t world;
	if ( Check( TIER_GAMEPLAY, "virtualmesh.large-create", pLarge != NULL ) && CreateWorld( world, NULL ) )
	{
		objectparams_t meshParams = DefaultParams( 1.0f, NULL );
		float rest[2];
		for ( int pass = 0; pass < 2; pass++ )
		{
			IPhysicsObject *pMesh = world.pEnv->CreatePolyObjectStatic( pLarge, world.material, kMeshOrigin, vec3_angle, &meshParams );
			if ( pass == 1 )
				pMesh->EnableCollisions( false );
			IPhysicsObject *pCube = CreateCube( world, kMeshOrigin + Vector( 232, 232, 40 ) );
			Step( world.pEnv, 1.5f );
			rest[pass] = PositionOf( pCube ).z - kMeshOrigin.z;
			world.pEnv->DestroyObject( pCube );
			world.pEnv->DestroyObject( pMesh );
		}
		Check( TIER_GAMEPLAY, "virtualmesh.large-supports", Near( rest[0], 16.0f, 1.0f ), "cube at z %.2f", rest[0] );
		Check( TIER_GAMEPLAY, "virtualmesh.large-collisions-off", rest[1] < -100.0f, "cube at z %.2f", rest[1] );
		DestroyWorld( world );
	}
	if ( pLarge )
		s_pCollision->DestroyCollide( pLarge );
}

void TestBBoxCache()
{
	Vector mins( -3, -5, -7 ), maxs( 11, 13, 17 );
	CPhysCollide *pFirst = s_pCollision->BBoxToCollide( mins, maxs );
	int sizeBefore = 0, countBefore = 0;
	bool reported = s_pCollision->GetBBoxCacheSize( &sizeBefore, &countBefore );
	CPhysCollide *pAgain = s_pCollision->BBoxToCollide( mins, maxs );
	Check( TIER_BOOT, "bboxcache.shared", pFirst != NULL && pFirst == pAgain );

	CPhysCollide *pOther = s_pCollision->BBoxToCollide( mins, maxs + Vector( 1, 0, 0 ) );
	int sizeAfter = 0, countAfter = 0;
	s_pCollision->GetBBoxCacheSize( &sizeAfter, &countAfter );
	Check( TIER_BOOT, "bboxcache.size", reported && countBefore > 0 && countAfter == countBefore + 1 && sizeAfter > sizeBefore,
		"count %d -> %d size %d -> %d", countBefore, countAfter, sizeBefore, sizeAfter );
	printf( "OBS bboxcache.count-delta x %d\n", countAfter - countBefore );

	// Cached boxes are shared, so destroying one keeps it usable.
	s_pCollision->DestroyCollide( pFirst );
	s_pCollision->DestroyCollide( pOther );
	float volume = s_pCollision->CollideVolume( pAgain );
	Check( TIER_BOOT, "bboxcache.destroy-keeps", Near( volume, 14.0f * 18.0f * 24.0f, 1.0f ) &&
		s_pCollision->BBoxToCollide( mins, maxs ) == pFirst, "volume %.2f", volume );
	Check( TIER_BOOT, "bboxcache.empty-null", s_pCollision->BBoxToCollide( mins, mins ) == NULL );
}

void TestBoxCone()
{
	truncatedcone_t cone;
	cone.origin.Init( 100, 200, 300 );
	cone.normal.Init( 1, 0, 0 );
	cone.h = 400.0f;
	cone.theta = 30.0f;	// base radius 400 * tan(30) = 230.9

	struct Case_t
	{
		const char *pName;
		Vector center;
		float half;
		bool expected;
	};
	const Case_t cases[] = {
		{ "apex", Vector( 0, 0, 0 ), 4, true },
		{ "on-axis", Vector( 200, 0, 0 ), 8, true },
		{ "inside-rim", Vector( 350, 150, 0 ), 8, true },
		{ "outside-rim", Vector( 350, 250, 0 ), 8, false },
		{ "behind-apex", Vector( -40, 0, 0 ), 8, false },
		{ "beyond-base", Vector( 440, 0, 0 ), 8, false },
		{ "straddles-side", Vector( 200, 115.5f, 0 ), 8, true },
		{ "above-axis", Vector( 100, 0, 80 ), 8, false },
		{ "point-inside", Vector( 300, 0, 50 ), 0, true },
	};
	// IVP's cone sweep reports every box as intersecting (its cone support
	// mapping is incomplete; no engine code calls this), so only the
	// intersecting cases are required and compared. Separated cases are
	// reported for information.
	int required = 0, correct = 0;
	char observed[32];
	int observedCount = 0;
	for ( int i = 0; i < (int)ARRAYSIZE( cases ); i++ )
	{
		Vector center = cone.origin + cases[i].center;
		Vector half( cases[i].half, cases[i].half, cases[i].half );
		bool hit = s_pCollision->IsBoxIntersectingCone( center - half, center + half, cone );
		if ( !cases[i].expected )
		{
			printf( "  cone case %s (separated): reported %d\n", cases[i].pName, hit );
			continue;
		}
		required++;
		observed[observedCount++] = hit ? '1' : '0';
		if ( hit )
			correct++;
		else
			printf( "  cone case %s: got %d\n", cases[i].pName, hit );
	}
	observed[observedCount] = 0;
	Check( TIER_GAMEPLAY, "cone.box-intersection", correct == required, "%d of %d", correct, required );
	ObsString( "cone.box-intersection", observed );
}

//-----------------------------------------------------------------------------
// Independent reader for the serialized compact surface
//-----------------------------------------------------------------------------
int ReadInt( const char *p, int offset )
{
	int value;
	memcpy( &value, p + offset, 4 );
	return value;
}

float ReadFloat( const char *p, int offset )
{
	float value;
	memcpy( &value, p + offset, 4 );
	return value;
}

struct Structure_t
{
	int ledges;
	int triangles;
	int points;
	bool valid;
	bool windingOutward;	// (p1-p0)x(p2-p0) points out of every solid ledge
	char error[128];
};

bool ReadLedge( const char *pSurface, int surfaceSize, int ledge, const Vector &nodeMins, const Vector &nodeMaxs, Structure_t *pOut )
{
	if ( ledge < 0 || ledge + 16 > surfaceSize )
		return false;
	int pointOffset = ReadInt( pSurface, ledge );
	unsigned int flags = (unsigned int)ReadInt( pSurface, ledge + 8 );
	short triangleCount;
	memcpy( &triangleCount, pSurface + ledge + 12, 2 );
	int size = (int)( flags >> 8 ) * 16;
	int pointCount = size / 16 - triangleCount - 1;
	if ( ( flags & 3 ) != 0 || triangleCount < 2 || pointCount < 3 || pointOffset != 16 + triangleCount * 16 || ledge + size > surfaceSize )
	{
		V_snprintf( pOut->error, sizeof( pOut->error ), "ledge header flags %x tris %d points %d", flags, triangleCount, pointCount );
		return false;
	}
	pOut->ledges++;
	pOut->triangles += triangleCount;
	pOut->points += pointCount;
	CUtlVector<Vector> points;
	Vector centroid( 0, 0, 0 );
	for ( int i = 0; i < pointCount; i++ )
	{
		int p = ledge + pointOffset + i * 16;
		Vector point( ReadFloat( pSurface, p ), ReadFloat( pSurface, p + 4 ), ReadFloat( pSurface, p + 8 ) );
		points.AddToTail( point );
		centroid += point;
		for ( int k = 0; k < 3; k++ )
		{
			if ( point[k] < nodeMins[k] - 1e-3f || point[k] > nodeMaxs[k] + 1e-3f )
			{
				V_snprintf( pOut->error, sizeof( pOut->error ), "point outside its tree node" );
				return false;
			}
		}
	}
	centroid /= (float)pointCount;
	bool flat = pointCount == 3;
	for ( int t = 0; t < triangleCount; t++ )
	{
		int triangle = ledge + 16 + t * 16;
		unsigned int header = (unsigned int)ReadInt( pSurface, triangle );
		int pierce = ( header >> 12 ) & 0xFFF;
		if ( (int)( header & 0xFFF ) != t || pierce >= triangleCount || pierce == t )
		{
			V_snprintf( pOut->error, sizeof( pOut->error ), "triangle %d index %d pierce %d", t, header & 0xFFF, pierce );
			return false;
		}
		int start[3];
		for ( int e = 0; e < 3; e++ )
		{
			unsigned int edge = (unsigned int)ReadInt( pSurface, triangle + 4 + e * 4 );
			start[e] = edge & 0xFFFF;
			int opposite = (int)( edge << 1 ) >> 17;	// signed 15 bits at bit 16
			int slot = t * 4 + 1 + e + opposite;
			int u = slot / 4, f = slot % 4 - 1;
			if ( start[e] >= pointCount || opposite == 0 || u < 0 || u >= triangleCount || f < 0 )
			{
				V_snprintf( pOut->error, sizeof( pOut->error ), "triangle %d edge %d opposite %d", t, e, opposite );
				return false;
			}
			// The opposite edge runs the other way and points back here.
			unsigned int back = (unsigned int)ReadInt( pSurface, ledge + 16 + u * 16 + 4 + f * 4 );
			unsigned int nextEdge = (unsigned int)ReadInt( pSurface, triangle + 4 + ( ( e + 1 ) % 3 ) * 4 );
			unsigned int backNext = (unsigned int)ReadInt( pSurface, ledge + 16 + u * 16 + 4 + ( ( f + 1 ) % 3 ) * 4 );
			if ( ( (int)( back << 1 ) >> 17 ) != -opposite || ( back & 0xFFFF ) != ( nextEdge & 0xFFFF ) || ( backNext & 0xFFFF ) != (unsigned int)start[e] )
			{
				V_snprintf( pOut->error, sizeof( pOut->error ), "triangle %d edge %d does not pair", t, e );
				return false;
			}
		}
		if ( !flat )
		{
			Vector normal = CrossProduct( points[start[1]] - points[start[0]], points[start[2]] - points[start[0]] );
			if ( DotProduct( normal, centroid - points[start[0]] ) >= 0.0f )
				pOut->windingOutward = false;
		}
	}
	return true;
}

bool ReadTree( const char *pSurface, int surfaceSize, int node, int depth, Structure_t *pOut )
{
	if ( depth > 64 || node < 0 || node + 28 > surfaceSize )
		return false;
	Vector center( ReadFloat( pSurface, node + 8 ), ReadFloat( pSurface, node + 12 ), ReadFloat( pSurface, node + 16 ) );
	float radius = ReadFloat( pSurface, node + 20 );
	Vector half;
	for ( int k = 0; k < 3; k++ )
		half[k] = (unsigned char)pSurface[node + 24 + k] * radius / 250.0f;
	int right = ReadInt( pSurface, node );
	if ( right != 0 )
		return ReadTree( pSurface, surfaceSize, node + 28, depth + 1, pOut ) && ReadTree( pSurface, surfaceSize, node + right, depth + 1, pOut );
	// Node boxes are conservative (rounded up) around their ledge.
	return ReadLedge( pSurface, surfaceSize, node + ReadInt( pSurface, node + 4 ), center - half, center + half, pOut );
}

Structure_t ReadStructure( const char *pBuffer, int size )
{
	Structure_t out;
	memset( &out, 0, sizeof( out ) );
	out.windingOutward = true;
	const int kVPhysicsId = ( 'Y' << 24 ) | ( 'H' << 16 ) | ( 'P' << 8 ) | 'V';
	if ( size < 28 + 48 || ReadInt( pBuffer, 0 ) != kVPhysicsId )
	{
		V_snprintf( out.error, sizeof( out.error ), "no VPHY header" );
		return out;
	}
	int surfaceSize = ReadInt( pBuffer, 8 );
	const char *pSurface = pBuffer + 28;
	if ( surfaceSize + 28 != size || ( (unsigned int)ReadInt( pSurface, 28 ) >> 8 ) != (unsigned int)surfaceSize )
	{
		V_snprintf( out.error, sizeof( out.error ), "sizes: buffer %d surface %d", size, surfaceSize );
		return out;
	}
	out.valid = ReadTree( pSurface, surfaceSize, ReadInt( pSurface, 32 ), 0, &out );
	return out;
}

void CheckSerialization( const char *pName, CPhysCollide *pCollide, bool observeCounts )
{
	char name[128];
	int size = WriteSize( pCollide );
	CUtlVector<char> buffer;
	buffer.SetCount( size + 64 );
	memset( buffer.Base(), 0x5A, buffer.Count() );
	int written = size > 0 ? s_pCollision->CollideWrite( buffer.Base(), pCollide, false ) : 0;
	bool tailUntouched = true;
	for ( int i = MAX( written, 0 ); i < buffer.Count(); i++ )
		tailUntouched = tailUntouched && buffer[i] == 0x5A;
	V_snprintf( name, sizeof( name ), "collide.write-size.%s", pName );
	Check( TIER_BOOT, name, size > 0 && written == size && tailUntouched, "size %d written %d", size, written );
	if ( written <= 0 )
		return;

	Structure_t structure = ReadStructure( buffer.Base(), written );
	V_snprintf( name, sizeof( name ), "collide.write-structure.%s", pName );
	Check( TIER_BOOT, name, structure.valid, "%s", structure.error );
	V_snprintf( name, sizeof( name ), "collide.write-winding.%s", pName );
	Check( TIER_BOOT, name, structure.valid && structure.windingOutward );
	if ( observeCounts )
	{
		int counts[3] = { structure.ledges, structure.triangles, structure.points };
		printf( "OBS collide.write-counts.%s x %d %d %d\n", pName, counts[0], counts[1], counts[2] );
	}

	// Reads back to the same solid.
	CPhysCollide *pRead = s_pCollision->UnserializeCollide( buffer.Base(), written, 7 );
	V_snprintf( name, sizeof( name ), "collide.write-roundtrip.%s", pName );
	if ( !Check( TIER_BOOT, name, pRead != NULL ) )
		return;
	Vector mins, maxs, readMins, readMaxs, center, readCenter;
	s_pCollision->CollideGetAABB( &mins, &maxs, pCollide, vec3_origin, vec3_angle );
	s_pCollision->CollideGetAABB( &readMins, &readMaxs, pRead, vec3_origin, vec3_angle );
	s_pCollision->CollideGetMassCenter( pCollide, &center );
	s_pCollision->CollideGetMassCenter( pRead, &readCenter );
	float volume = s_pCollision->CollideVolume( pCollide ), readVolume = s_pCollision->CollideVolume( pRead );
	Vector areas = s_pCollision->CollideGetOrthographicAreas( pCollide ), readAreas = s_pCollision->CollideGetOrthographicAreas( pRead );
	V_snprintf( name, sizeof( name ), "collide.write-preserves.%s", pName );
	Check( TIER_BOOT, name, NearVec( mins, readMins, 0.01f ) && NearVec( maxs, readMaxs, 0.01f ) && NearVec( center, readCenter, 0.01f ) &&
		Near( volume, readVolume, MAX( 0.01f, volume * 1e-4f ) ) && NearVec( areas, readAreas, 1e-4f ) && s_pCollision->CollideIndex( pRead ) == 7,
		"volume %.3f -> %.3f center (%.2f %.2f %.2f) -> (%.2f %.2f %.2f) index %d", volume, readVolume,
		center.x, center.y, center.z, readCenter.x, readCenter.y, readCenter.z, s_pCollision->CollideIndex( pRead ) );
	s_pCollision->DestroyCollide( pRead );

	// The opposite byte order writes the same bytes word-swapped.
	CUtlVector<char> swapped;
	swapped.SetCount( size );
	int swappedSize = s_pCollision->CollideWrite( swapped.Base(), pCollide, true );
	int id = ReadInt( swapped.Base(), 0 ), native = ReadInt( buffer.Base(), 0 );
	int swappedSurface = ReadInt( swapped.Base(), 8 ), nativeSurface = ReadInt( buffer.Base(), 8 );
	V_snprintf( name, sizeof( name ), "collide.write-swapped.%s", pName );
	Check( TIER_BOOT, name, swappedSize == size && id == (int)DWordSwapC( (unsigned int)native ) && swappedSurface == (int)DWordSwapC( (unsigned int)nativeSurface ) );
}

void TestSerialization( const vcollide_t *pFixture )
{
	CPhysCollide *pBox = s_pCollision->BBoxToCollide( Vector( -16, -24, -8 ), Vector( 16, 24, 40 ) );
	CheckSerialization( "box", pBox, true );

	Vector wedge[6] = { Vector( 0, 0, 0 ), Vector( 64, 0, 0 ), Vector( 0, 64, 0 ), Vector( 0, 0, 32 ), Vector( 64, 0, 32 ), Vector( 0, 64, 32 ) };
	Vector *pWedge[6];
	for ( int i = 0; i < 6; i++ )
		pWedge[i] = &wedge[i];
	CPhysConvex *pConvex = s_pCollision->ConvexFromVerts( pWedge, 6 );
	CPhysCollide *pWedgeCollide = pConvex ? s_pCollision->ConvertConvexToCollide( &pConvex, 1 ) : NULL;
	if ( pWedgeCollide )
	{
		CheckSerialization( "wedge", pWedgeCollide, false );
		s_pCollision->DestroyCollide( pWedgeCollide );
	}

	// Authored model data rewrites losslessly with its authored structure.
	if ( pFixture && pFixture->solidCount > 0 && pFixture->solids[0] )
		CheckSerialization( "fixture", pFixture->solids[0], true );
	else
		Check( TIER_BOOT, "collide.write-fixture-available", false );
}
}

namespace
{
// A map's world collision, as the engine loads it from the BSP's physics
// lump (dphysmodel_t records: model index, data size, keydata size, solid
// count, then size-prefixed solids and keydata; model -1 ends the lump).
void TestBspWorld( const char *pPath )
{
	int size = 0;
	char *pData = ReadFixtureFile( pPath, &size );
	const int kHeaderLumps = 64, kPhysCollideLump = 29, kLumpOffset = 8;
	if ( !Check( TIER_BOOT, "bsp.read", pData && size > kLumpOffset + kHeaderLumps * 16, "%s", pPath ) )
		return;
	int lumpOffset, lumpLength, compressed;
	memcpy( &lumpOffset, pData + kLumpOffset + kPhysCollideLump * 16, 4 );
	memcpy( &lumpLength, pData + kLumpOffset + kPhysCollideLump * 16 + 4, 4 );
	memcpy( &compressed, pData + kLumpOffset + kPhysCollideLump * 16 + 12, 4 );
	vcollide_t world;
	memset( &world, 0, sizeof( world ) );
	bool found = false;
	int position = lumpOffset;
	while ( !compressed && position + 16 <= lumpOffset + lumpLength && position + 16 <= size )
	{
		int header[4];
		memcpy( header, pData + position, sizeof( header ) );
		position += sizeof( header );
		if ( header[0] == -1 )
			break;
		if ( header[0] == 0 )
		{
			s_pCollision->VCollideLoad( &world, header[3], pData + position, header[1] + header[2] );
			found = world.solidCount == header[3] && world.solidCount > 0;
			break;
		}
		position += header[1] + header[2];
	}
	if ( !Check( TIER_BOOT, "bsp.world-solids", found, "%s solids %d", pPath, world.solidCount ) )
	{
		free( pData );
		return;
	}
	printf( "OBS bsp.world-solid-count x %d\n", world.solidCount );

	Vector mins( FLT_MAX, FLT_MAX, FLT_MAX ), maxs( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	float volume = 0.0f;
	for ( int i = 0; i < world.solidCount; i++ )
	{
		Vector solidMins, solidMaxs;
		s_pCollision->CollideGetAABB( &solidMins, &solidMaxs, world.solids[i], vec3_origin, vec3_angle );
		VectorMin( mins, solidMins, mins );
		VectorMax( maxs, solidMaxs, maxs );
		volume += s_pCollision->CollideVolume( world.solids[i] );
	}
	ObsVector( "bsp.world-mins", "a0.5", mins );
	ObsVector( "bsp.world-maxs", "a0.5", maxs );
	ObsFloats( "bsp.world-volume", "r0.02", 1, &volume );

	// Vertical traces from a grid of points at several heights through the
	// map: the first surface below, or where the start is inside a brush.
	const int kGrid = 12, kLevels = 4;
	const float kStartSolid = -88888.0f, kMiss = -99999.0f;
	float heights[kGrid * kGrid * kLevels];
	int hits = 0, solid = 0;
	for ( int level = 0; level < kLevels; level++ )
	{
		float z = mins.z + ( maxs.z - mins.z ) * ( level + 0.5f ) / kLevels;
		for ( int j = 0; j < kGrid; j++ )
		{
			for ( int i = 0; i < kGrid; i++ )
			{
				Vector top( mins.x + ( maxs.x - mins.x ) * ( i + 0.5f ) / kGrid, mins.y + ( maxs.y - mins.y ) * ( j + 0.5f ) / kGrid, z );
				Vector bottom( top.x, top.y, mins.z - 64 );
				float best = 1.0f;
				bool startSolid = false;
				for ( int k = 0; k < world.solidCount; k++ )
				{
					trace_t tr;
					s_pCollision->TraceBox( top, bottom, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), world.solids[k], vec3_origin, vec3_angle, &tr );
					startSolid = startSolid || tr.startsolid;
					if ( tr.fraction < best && !tr.startsolid )
						best = tr.fraction;
				}
				float &out = heights[( level * kGrid + j ) * kGrid + i];
				if ( startSolid )
				{
					out = kStartSolid;
					solid++;
				}
				else if ( best < 1.0f )
				{
					out = top.z + ( bottom.z - top.z ) * best;
					hits++;
				}
				else
				{
					out = kMiss;
				}
			}
		}
	}
	Check( TIER_BOOT, "bsp.world-traces", hits > 0 && solid > 0, "%d hit, %d start solid of %d", hits, solid, kGrid * kGrid * kLevels );
	printf( "OBS bsp.world-trace-counts x %d %d\n", hits, solid );
	ObsFloats( "bsp.world-trace-heights", "a1", kGrid * kGrid * kLevels, heights );
	s_pCollision->VCollideUnload( &world );
	free( pData );
}
}

void TestCollideModels( const vcollide_t *pFixture, const char *pBsp )
{
	if ( pBsp )
		TestBspWorld( pBsp );
	else
		Check( TIER_BOOT, "bsp.fixture-present", false );
	TestBBoxCache();
	TestPolysoup();
	TestVirtualMesh();
	TestBoxCone();
	TestSerialization( pFixture );

	// Diagnostics hooks: must be callable; IVP reports no statistics.
	CPhysCollide *pBox = s_pCollision->BBoxToCollide( Vector( -1, -1, -1 ), Vector( 1, 1, 1 ) );
	s_pCollision->OutputDebugInfo( pBox );
	Check( TIER_BOOT, "collide.read-stat-zero", s_pCollision->ReadStat( 0 ) == 0 );
	IPhysicsCollision *pContext = s_pCollision->ThreadContextCreate();
	Check( TIER_BOOT, "collide.thread-context", pContext != NULL );
	if ( pContext )
		s_pCollision->ThreadContextDestroy( pContext );
}
