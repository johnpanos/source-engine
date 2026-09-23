//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Legacy VPhysics collision-model decoder (RFC 0004 B3).
//
// Layout (little-endian, all offsets relative to the structure that holds
// them), as written by vphysics/physics_collide.cpp and IVP's compact builder:
//
//   compactsurfaceheader_t (28 bytes, optional; absent in old .phy files)
//     int vphysicsID ('VPHY'), short version, short modelType (0 = poly),
//     int surfaceSize, float dragAxisAreas[3], int axisMapSize
//   IVP_Compact_Surface (48 bytes)
//     float mass_center[3], float rotation_inertia[3], float radius,
//     uint32 { deviation:8, byte_size:24 }, int offset_ledgetree_root,
//     int dummy[3] (dummy[2] = 'IVPS' or 0)
//   IVP_Compact_Ledgetree_Node (28 bytes)
//     int offset_right_node (0 = terminal), int offset_compact_ledge,
//     float center[3], float radius, uchar box_sizes[3], uchar pad
//     left child = this + 28, right child = this + offset_right_node
//   IVP_Compact_Ledge (16 bytes) followed by n_triangles triangles
//     int c_point_offset, int client_data,
//     uint32 { has_children:2, is_compact:2, dummy:4, size_div_16:24 },
//     short n_triangles, short reserved
//   IVP_Compact_Triangle (16 bytes)
//     uint32 { tri_index:12, pierce_index:12, material:7, virtual:1 },
//     3 x uint32 edge { start_point_index:16, opposite:15, virtual:1 }
//   IVP_Compact_Poly_Point (16 bytes): float x, y, z, hesse
//
// IVP stores meters with Y and Z swapped relative to Source:
//   source = ( x, z, -y ) / 0.0254
//
//=============================================================================//
#include "legacy_collision.h"

#include <math.h>
#include <string.h>

#include "tier0/dbg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
const int kVPhysicsId = ( 'Y' << 24 ) | ( 'H' << 16 ) | ( 'P' << 8 ) | 'V';
const int kIvpSurfaceId = ( 'S' << 24 ) | ( 'P' << 16 ) | ( 'V' << 8 ) | 'I';
const int kHeaderSize = 28;
const int kSurfaceSize = 48;
const int kNodeSize = 28;
const int kLedgeSize = 16;
const int kTriangleSize = 16;
const int kPointSize = 16;
const int kMaxTreeDepth = 64;
const float kInchesPerMeter = 1.0f / 0.0254f;

class CReader
{
public:
	CReader( const char *pBase, int size ) : m_pBase( pBase ), m_size( size ) {}

	bool Has( int offset, int bytes ) const
	{
		return offset >= 0 && bytes >= 0 && offset <= m_size - bytes;
	}
	int Int( int offset ) const
	{
		int value;
		memcpy( &value, m_pBase + offset, sizeof( value ) );
		return value;
	}
	unsigned int UInt( int offset ) const
	{
		unsigned int value;
		memcpy( &value, m_pBase + offset, sizeof( value ) );
		return value;
	}
	short Short( int offset ) const
	{
		short value;
		memcpy( &value, m_pBase + offset, sizeof( value ) );
		return value;
	}
	float Float( int offset ) const
	{
		float value;
		memcpy( &value, m_pBase + offset, sizeof( value ) );
		return value;
	}
	Vector IvpPoint( int offset ) const
	{
		return Vector( Float( offset ), Float( offset + 8 ), -Float( offset + 4 ) ) * kInchesPerMeter;
	}

private:
	const char *m_pBase;
	int m_size;
};

bool DecodeLedge( const CReader &reader, int ledge, LegacyConvex_t *pConvex )
{
	if ( !reader.Has( ledge, kLedgeSize ) )
		return false;
	int pointOffset = reader.Int( ledge );
	unsigned int flags = reader.UInt( ledge + 8 );
	int triangleCount = reader.Short( ledge + 12 );
	if ( ( flags & 3 ) != 0 || triangleCount <= 0 )
		return false;	// only terminal ledges carry convex pieces
	if ( !reader.Has( ledge + kLedgeSize, triangleCount * kTriangleSize ) )
		return false;

	pConvex->gameData = reader.UInt( ledge + 4 );
	// Remap the ledge's (possibly shared) point indices to a compact local set.
	CUtlVector<int> sourceIndex;
	for ( int t = 0; t < triangleCount; t++ )
	{
		int triangle = ledge + kLedgeSize + t * kTriangleSize;
		unsigned int bits = reader.UInt( triangle );
		pConvex->triangleMaterials.AddToTail( (unsigned char)( ( bits >> 24 ) & 0x7F ) );
		for ( int e = 0; e < 3; e++ )
		{
			int pointIndex = (int)( reader.UInt( triangle + 4 + e * 4 ) & 0xFFFF );
			int point = ledge + pointOffset + pointIndex * kPointSize;
			if ( !reader.Has( point, 12 ) )
				return false;
			int local = sourceIndex.Find( pointIndex );
			if ( local == sourceIndex.InvalidIndex() )
			{
				local = sourceIndex.AddToTail( pointIndex );
				pConvex->points.AddToTail( reader.IvpPoint( point ) );
			}
			pConvex->triangles.AddToTail( (unsigned short)local );
		}
	}
	// IVP allows flat ledges (a double-sided triangle, three points).
	return pConvex->points.Count() >= 3;
}

bool DecodeTree( const CReader &reader, int node, int depth, LegacyCollide_t *pOut )
{
	if ( depth > kMaxTreeDepth || !reader.Has( node, kNodeSize ) )
		return false;
	int rightOffset = reader.Int( node );
	if ( rightOffset != 0 )
	{
		return DecodeTree( reader, node + kNodeSize, depth + 1, pOut ) &&
			DecodeTree( reader, node + rightOffset, depth + 1, pOut );
	}
	int ledge = node + reader.Int( node + 4 );
	int index = pOut->convexes.AddToTail();
	if ( !DecodeLedge( reader, ledge, &pOut->convexes[index] ) )
	{
		// Skip an unusable piece; the rest of the solid still decodes.
		pOut->convexes.Remove( index );
	}
	return true;
}
}

bool DecodeLegacyCollide( const char *pBuffer, int size, LegacyCollide_t *pOut )
{
	pOut->convexes.RemoveAll();
	pOut->massCenter.Init();
	pOut->orthoAreas.Init( 1, 1, 1 );
	pOut->rotationInertia.Init( 1, 1, 1 );
	if ( !pBuffer || size < kSurfaceSize )
		return false;

	CReader header( pBuffer, size );
	const char *pSurface = pBuffer;
	int surfaceSize = size;
	if ( header.Int( 0 ) == kVPhysicsId )
	{
		// Only compact polygon surfaces are defined for this format.
		if ( !header.Has( 0, kHeaderSize ) || header.Short( 6 ) != 0 )
			return false;
		surfaceSize = header.Int( 8 );
		pOut->orthoAreas = Vector( header.Float( 12 ), header.Float( 16 ), header.Float( 20 ) );
		pSurface = pBuffer + kHeaderSize;
		if ( surfaceSize < kSurfaceSize || surfaceSize > size - kHeaderSize )
			return false;
	}

	CReader reader( pSurface, surfaceSize );
	int surfaceId = reader.Int( 44 );
	if ( surfaceId != kIvpSurfaceId && surfaceId != 0 )
		return false;	// byte-swapped (console) or MOPP data is not supported
	pOut->massCenter = reader.IvpPoint( 0 );
	// Inertia is per axis (IVP x, y, z = Source x, -z, y): swap, no scaling.
	pOut->rotationInertia = Vector( fabsf( reader.Float( 12 ) ), fabsf( reader.Float( 20 ) ), fabsf( reader.Float( 16 ) ) );
	int root = reader.Int( 32 );
	if ( !DecodeTree( reader, root, 0, pOut ) )
	{
		pOut->convexes.RemoveAll();
		return false;
	}
	return pOut->convexes.Count() > 0;
}

//-----------------------------------------------------------------------------
// Encoder
//-----------------------------------------------------------------------------
namespace
{
const int kVPhysicsVersion = 0x100;
const int kMaxLedgeTriangles = ( 1 << 12 ) - 1;	// tri_index / pierce_index bits

// One terminal ledge ready to write: IVP-wound triangles over the points
// they use, plus edge topology.
struct EncodedLedge_t
{
	CUtlVector<Vector> points;			// IVP coordinates (meters)
	CUtlVector<int> triangles;			// 3 per triangle, IVP winding
	CUtlVector<unsigned char> materials;
	CUtlVector<int> opposite;			// 3 per triangle, edge slot offset
	CUtlVector<int> pierce;				// 1 per triangle
	unsigned int gameData;
	Vector mins, maxs;

	int Size() const { return kLedgeSize + ( triangles.Count() / 3 + points.Count() ) * kPointSize; }
};

Vector ToIvp( const Vector &source )
{
	return Vector( source.x, -source.z, source.y ) * 0.0254f;
}

// Edge slot of triangle t, edge e counted in 4-byte edges from the first
// triangle (each triangle is four slots: header plus three edges).
int EdgeSlot( int t, int e )
{
	return t * 4 + 1 + e;
}

// sizeOnly skips the topology a size query does not need.
bool BuildLedge( const LegacyConvex_t &convex, bool sizeOnly, EncodedLedge_t *pLedge )
{
	pLedge->gameData = convex.gameData;
	int sourceTriangles = convex.triangles.Count() / 3;
	if ( sourceTriangles <= 0 )
		return false;

	// Only the points the triangles reference.
	CUtlVector<int> remap;
	remap.SetCount( convex.points.Count() );
	for ( int i = 0; i < remap.Count(); i++ )
		remap[i] = -1;
	for ( int i = 0; i < sourceTriangles * 3; i++ )
	{
		int index = convex.triangles[i];
		if ( index >= convex.points.Count() )
			return false;
		if ( remap[index] < 0 )
			remap[index] = pLedge->points.AddToTail( ToIvp( convex.points[index] ) );
	}
	Vector interior( 0, 0, 0 );
	for ( int i = 0; i < pLedge->points.Count(); i++ )
		interior += pLedge->points[i];
	interior /= (float)pLedge->points.Count();

	// IVP winds its triangles so (p1 - p0) x (p2 - p0) points out of the
	// solid (checked against IVP's own writer by the conformance suite).
	for ( int t = 0; t < sourceTriangles; t++ )
	{
		int a = remap[convex.triangles[t * 3]], b = remap[convex.triangles[t * 3 + 1]], c = remap[convex.triangles[t * 3 + 2]];
		Vector normal = CrossProduct( pLedge->points[b] - pLedge->points[a], pLedge->points[c] - pLedge->points[a] );
		if ( sourceTriangles > 1 && DotProduct( normal, interior - pLedge->points[a] ) > 0.0f )
			V_swap( b, c );
		pLedge->triangles.AddToTail( a );
		pLedge->triangles.AddToTail( b );
		pLedge->triangles.AddToTail( c );
		pLedge->materials.AddToTail( t < convex.triangleMaterials.Count() ? convex.triangleMaterials[t] : 0 );
	}
	if ( sourceTriangles == 1 )
	{
		// A two-sided flat ledge: the triangle and its reverse.
		pLedge->triangles.AddToTail( pLedge->triangles[0] );
		pLedge->triangles.AddToTail( pLedge->triangles[2] );
		pLedge->triangles.AddToTail( pLedge->triangles[1] );
		pLedge->materials.AddToTail( pLedge->materials[0] );
	}
	int triangleCount = pLedge->triangles.Count() / 3;
	if ( triangleCount > kMaxLedgeTriangles || pLedge->points.Count() > 0xFFFF )
		return false;
	if ( sizeOnly )
		return true;

	// Every directed edge a->b must meet exactly one b->a.
	pLedge->opposite.SetCount( triangleCount * 3 );
	for ( int t = 0; t < triangleCount; t++ )
	{
		for ( int e = 0; e < 3; e++ )
		{
			int from = pLedge->triangles[t * 3 + e], to = pLedge->triangles[t * 3 + ( e + 1 ) % 3];
			int match = -1;
			for ( int u = 0; u < triangleCount && match < 0; u++ )
			{
				for ( int f = 0; f < 3; f++ )
				{
					if ( u != t && pLedge->triangles[u * 3 + f] == to && pLedge->triangles[u * 3 + ( f + 1 ) % 3] == from )
					{
						match = EdgeSlot( u, f );
						break;
					}
				}
			}
			if ( match < 0 )
				return false;
			pLedge->opposite[t * 3 + e] = match - EdgeSlot( t, e );
		}
	}

	// Pierce triangle: the face a ray from the triangle's center along its
	// inward normal leaves the solid through.
	for ( int t = 0; t < triangleCount; t++ )
	{
		const Vector &p0 = pLedge->points[pLedge->triangles[t * 3]];
		const Vector &p1 = pLedge->points[pLedge->triangles[t * 3 + 1]];
		const Vector &p2 = pLedge->points[pLedge->triangles[t * 3 + 2]];
		Vector inward = -CrossProduct( p1 - p0, p2 - p0 );
		Vector start = ( p0 + p1 + p2 ) / 3.0f;
		int best = t == 0 ? ( triangleCount > 1 ? 1 : 0 ) : 0;
		float bestDistance = FLT_MAX;
		for ( int u = 0; u < triangleCount; u++ )
		{
			if ( u == t )
				continue;
			const Vector &q0 = pLedge->points[pLedge->triangles[u * 3]];
			Vector normal = CrossProduct( pLedge->points[pLedge->triangles[u * 3 + 1]] - q0, pLedge->points[pLedge->triangles[u * 3 + 2]] - q0 );
			float approach = DotProduct( normal, inward );
			if ( approach <= 0.0f )
				continue;
			float distance = DotProduct( normal, q0 - start ) / approach;
			if ( distance < bestDistance )
			{
				bestDistance = distance;
				best = u;
			}
		}
		pLedge->pierce.AddToTail( best );
	}

	pLedge->mins.Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pLedge->maxs.Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	for ( int i = 0; i < pLedge->points.Count(); i++ )
	{
		VectorMin( pLedge->mins, pLedge->points[i], pLedge->mins );
		VectorMax( pLedge->maxs, pLedge->points[i], pLedge->maxs );
	}
	return true;
}

class CWriter
{
public:
	CWriter( char *pBase, bool swap ) : m_pBase( pBase ), m_swap( swap ) {}

	void Int( int offset, int value ) { Bytes( offset, &value, 4 ); }
	void UInt( int offset, unsigned int value ) { Bytes( offset, &value, 4 ); }
	void Float( int offset, float value ) { Bytes( offset, &value, 4 ); }
	void Short( int offset, short value ) { Bytes( offset, &value, 2 ); }
	void Byte( int offset, unsigned char value ) { if ( m_pBase ) m_pBase[offset] = (char)value; }
	void Vec( int offset, const Vector &value )
	{
		Float( offset, value.x );
		Float( offset + 4, value.y );
		Float( offset + 8, value.z );
	}

private:
	void Bytes( int offset, const void *pValue, int size )
	{
		if ( !m_pBase )
			return;
		const char *pIn = (const char *)pValue;
		for ( int i = 0; i < size; i++ )
			m_pBase[offset + i] = pIn[m_swap ? size - 1 - i : i];
	}

	char *m_pBase;
	bool m_swap;
};

// IVP stores node extents as 1/250ths of the node radius, rounded up.
void WriteNodeBounds( CWriter &out, int node, const Vector &mins, const Vector &maxs )
{
	Vector center = ( mins + maxs ) * 0.5f;
	Vector half = maxs - center;
	float radius = half.Length();
	out.Vec( node + 8, center );
	out.Float( node + 20, radius );
	float step = radius * ( 1.0f / 250.0f );
	for ( int i = 0; i < 3; i++ )
	{
		int size = step > 0.0f ? (int)( half[i] / step ) + 1 : 1;
		out.Byte( node + 24 + i, (unsigned char)MIN( size, 255 ) );
	}
	out.Byte( node + 27, 0 );
}

// Writes the subtree over ledges[first, first + count) at node, preorder
// (left child right after its parent). Returns the node bytes used.
int WriteTree( CWriter &out, int node, CUtlVector<EncodedLedge_t *> &ledges, int first, int count,
	const CUtlVector<int> &ledgeOffsets, const CUtlVector<EncodedLedge_t *> &order, Vector *pMins, Vector *pMaxs )
{
	if ( count == 1 )
	{
		EncodedLedge_t *pLedge = ledges[first];
		out.Int( node, 0 );
		out.Int( node + 4, ledgeOffsets[order.Find( pLedge )] - node );
		WriteNodeBounds( out, node, pLedge->mins, pLedge->maxs );
		*pMins = pLedge->mins;
		*pMaxs = pLedge->maxs;
		return kNodeSize;
	}
	int half = count / 2;
	Vector leftMins, leftMaxs, rightMins, rightMaxs;
	int leftSize = WriteTree( out, node + kNodeSize, ledges, first, half, ledgeOffsets, order, &leftMins, &leftMaxs );
	int rightSize = WriteTree( out, node + kNodeSize + leftSize, ledges, first + half, count - half, ledgeOffsets, order, &rightMins, &rightMaxs );
	VectorMin( leftMins, rightMins, *pMins );
	VectorMax( leftMaxs, rightMaxs, *pMaxs );
	out.Int( node, kNodeSize + leftSize );
	out.Int( node + 4, 0 );
	WriteNodeBounds( out, node, *pMins, *pMaxs );
	return kNodeSize + leftSize + rightSize;
}

// Median split along the longest axis of the ledge centers, so the tree
// groups neighboring pieces.
void SortForTree( CUtlVector<EncodedLedge_t *> &ledges, int first, int count )
{
	if ( count <= 2 )
		return;
	Vector mins( FLT_MAX, FLT_MAX, FLT_MAX ), maxs( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	for ( int i = first; i < first + count; i++ )
	{
		Vector center = ( ledges[i]->mins + ledges[i]->maxs ) * 0.5f;
		VectorMin( mins, center, mins );
		VectorMax( maxs, center, maxs );
	}
	Vector extent = maxs - mins;
	int axis = extent.x >= extent.y && extent.x >= extent.z ? 0 : ( extent.y >= extent.z ? 1 : 2 );
	// Insertion sort: ledge counts are small.
	for ( int i = first + 1; i < first + count; i++ )
	{
		EncodedLedge_t *pLedge = ledges[i];
		float key = pLedge->mins[axis] + pLedge->maxs[axis];
		int j = i - 1;
		while ( j >= first && ledges[j]->mins[axis] + ledges[j]->maxs[axis] > key )
		{
			ledges[j + 1] = ledges[j];
			j--;
		}
		ledges[j + 1] = pLedge;
	}
	int half = count / 2;
	SortForTree( ledges, first, half );
	SortForTree( ledges, first + half, count - half );
}

void WriteLedge( CWriter &out, int ledge, const EncodedLedge_t &source )
{
	int triangleCount = source.triangles.Count() / 3;
	int pointOffset = kLedgeSize + triangleCount * kTriangleSize;
	out.Int( ledge, pointOffset );
	out.UInt( ledge + 4, source.gameData );
	// has_children 0, is_compact 1 (points included), size in 16 bytes.
	out.UInt( ledge + 8, ( 1u << 2 ) | ( (unsigned int)( source.Size() / 16 ) << 8 ) );
	out.Short( ledge + 12, (short)triangleCount );
	out.Short( ledge + 14, 0 );
	for ( int t = 0; t < triangleCount; t++ )
	{
		int triangle = ledge + kLedgeSize + t * kTriangleSize;
		out.UInt( triangle, (unsigned int)t | ( (unsigned int)source.pierce[t] << 12 ) | ( (unsigned int)( source.materials[t] & 0x7F ) << 24 ) );
		for ( int e = 0; e < 3; e++ )
		{
			unsigned int opposite = (unsigned int)source.opposite[t * 3 + e] & 0x7FFF;
			out.UInt( triangle + 4 + e * 4, (unsigned int)source.triangles[t * 3 + e] | ( opposite << 16 ) );
		}
	}
	for ( int i = 0; i < source.points.Count(); i++ )
	{
		out.Vec( ledge + pointOffset + i * kPointSize, source.points[i] );
		out.Float( ledge + pointOffset + i * kPointSize + 12, 0.0f );
	}
}
}

int EncodeLegacyCollide( const LegacyCollide_t &collide, int vcollideIndex, char *pDest, bool swap )
{
	CUtlVector<EncodedLedge_t *> ledges;
	bool valid = collide.convexes.Count() > 0;
	for ( int i = 0; i < collide.convexes.Count() && valid; i++ )
	{
		EncodedLedge_t *pLedge = new EncodedLedge_t;
		ledges.AddToTail( pLedge );
		valid = BuildLedge( collide.convexes[i], pDest == NULL, pLedge );
	}
	if ( !valid )
	{
		ledges.PurgeAndDeleteElements();
		return 0;
	}

	// Surface, tree (padded to 16), then the ledges in authored order.
	int nodeCount = ledges.Count() * 2 - 1;
	int treeOffset = kSurfaceSize;
	int ledgeOffset = ( treeOffset + nodeCount * kNodeSize + 15 ) & ~15;
	CUtlVector<int> ledgeOffsets;
	for ( int i = 0; i < ledges.Count(); i++ )
	{
		ledgeOffsets.AddToTail( ledgeOffset );
		ledgeOffset += ledges[i]->Size();
	}
	int surfaceSize = ledgeOffset;
	int totalSize = kHeaderSize + surfaceSize;
	if ( surfaceSize >= ( 1 << 23 ) )
	{
		ledges.PurgeAndDeleteElements();
		return 0;
	}
	if ( !pDest )
	{
		ledges.PurgeAndDeleteElements();
		return totalSize;
	}
	memset( pDest, 0, totalSize );

	CWriter header( pDest, swap );
	header.Int( 0, kVPhysicsId );
	header.Short( 4, (short)kVPhysicsVersion );
	header.Short( 6, 0 );	// COLLIDE_POLY
	header.Int( 8, surfaceSize );
	header.Vec( 12, collide.orthoAreas );
	header.Int( 24, 0 );	// no axis map

	CWriter out( pDest + kHeaderSize, swap );
	CUtlVector<EncodedLedge_t *> treeOrder;
	treeOrder.CopyArray( ledges.Base(), ledges.Count() );
	SortForTree( treeOrder, 0, treeOrder.Count() );
	Vector mins, maxs;
	WriteTree( out, treeOffset, treeOrder, 0, treeOrder.Count(), ledgeOffsets, ledges, &mins, &maxs );
	for ( int i = 0; i < ledges.Count(); i++ )
		WriteLedge( out, ledgeOffsets[i], *ledges[i] );

	Vector massCenter = ToIvp( collide.massCenter );
	float radius = 0.0f;
	for ( int i = 0; i < ledges.Count(); i++ )
	{
		for ( int k = 0; k < ledges[i]->points.Count(); k++ )
			radius = MAX( radius, ( ledges[i]->points[k] - massCenter ).Length() );
	}
	out.Vec( 0, massCenter );
	// Source (x, y, z) inertia axes are IVP (x, z, y).
	out.Vec( 12, Vector( collide.rotationInertia.x, collide.rotationInertia.z, collide.rotationInertia.y ) );
	out.Float( 24, radius );
	out.UInt( 28, (unsigned int)surfaceSize << 8 );	// deviation 0, byte_size
	out.Int( 32, treeOffset );
	out.Int( 36, vcollideIndex );
	out.Int( 40, 0 );
	out.Int( 44, kIvpSurfaceId );
	ledges.PurgeAndDeleteElements();
	return totalSize;
}
