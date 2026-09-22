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
	int root = reader.Int( 32 );
	if ( !DecodeTree( reader, root, 0, pOut ) )
	{
		pOut->convexes.RemoveAll();
		return false;
	}
	return pOut->convexes.Count() > 0;
}
