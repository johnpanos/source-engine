//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RFC 0008 WMSH payload decoded into values (strict C++20 consumers:
//			the RFC 0016 render lab and tools). world_mesh.h stays the
//			legacy-consumer boundary (validation only, no C++20 types); this
//			header is its decoder, owned by the same format library, so the
//			byte layout of world_mesh_format.h has one reader in C++.
//
//			Normals and tangents are decoded from their octahedral snorm16x2
//			form as the renderer's vertex stage decodes them (a value is
//			s / 32767 clamped to -1, then the octahedral unfold and a
//			normalize).
//
//=============================================================================//

#ifndef MAPCONTAINER_WORLD_MESH_DECODE_H
#define MAPCONTAINER_WORLD_MESH_DECODE_H

#include "mapcontainer/world_mesh.h"

#include "mapcontainer/world_mesh_format.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace mapcontainer
{

struct WorldMeshVertex
{
	float position[3] = {};
	float normal[3] = {};
	float tangent[3] = {};
	float tangentSign = 1.0f; // the stored handedness, -1 or 1
	float uv[2] = {};         // material coordinates
	float lightmapUv[2] = {}; // lightmap atlas coordinates
};

struct WorldMeshBatch
{
	uint32_t material = 0; // into WorldMeshData::materials
	uint32_t firstIndex = 0;
	uint32_t indexCount = 0;
};

struct WorldMeshData
{
	uint32_t version = 0;
	std::vector<WorldMeshVertex> vertices;
	std::vector<uint32_t> indices; // triangle lists
	std::vector<WorldMeshBatch> batches;
	std::vector<std::string> materials; // relative paths without ".vmt"
};

// Validates the payload (ValidateWorldMesh) and decodes it. On failure `out`
// is unspecified and the error is ValidateWorldMesh's. DecodeOctahedral is
// the octahedral unfold of one stored snorm16x2 pair, normalized.
//
// Inline: the library builds with the legacy consumers' settings, so C++
// library types never cross it; the decoder compiles in its consumer.

namespace world_mesh_detail
{

inline uint32_t ReadU32( const unsigned char *p )
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

inline int16_t ReadI16( const unsigned char *p )
{
	return int16_t( uint16_t( p[0] ) | uint16_t( p[1] << 8 ) );
}

inline float ReadF32( const unsigned char *p )
{
	const uint32_t bits = ReadU32( p );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

inline float Snorm16( int16_t value )
{
	const float scaled = float( value ) / 32767.0f;
	return scaled < -1.0f ? -1.0f : scaled;
}

} // namespace world_mesh_detail

inline void DecodeOctahedral( int16_t x, int16_t y, float ( &out )[3] )
{
	using namespace world_mesh_detail;
	float nx = Snorm16( x );
	float ny = Snorm16( y );
	const float nz = 1.0f - std::fabs( nx ) - std::fabs( ny );
	if ( nz < 0.0f )
	{
		const float fx = ( 1.0f - std::fabs( ny ) ) * ( nx >= 0.0f ? 1.0f : -1.0f );
		const float fy = ( 1.0f - std::fabs( nx ) ) * ( ny >= 0.0f ? 1.0f : -1.0f );
		nx = fx;
		ny = fy;
	}
	const float length = std::sqrt( nx * nx + ny * ny + nz * nz );
	const float scale = length > 0.0f ? 1.0f / length : 0.0f;
	out[0] = nx * scale;
	out[1] = ny * scale;
	out[2] = nz * scale;
}

inline WorldMeshError DecodeWorldMesh( const void *pData, size_t size, WorldMeshData &out )
{
	using namespace world_mesh_detail;
	WorldMeshSummary summary{};
	const WorldMeshError error = ValidateWorldMesh( pData, size, &summary );
	if ( error != WorldMeshError::Ok )
		return error;
	const unsigned char *bytes = static_cast<const unsigned char *>( pData );
	// sectionOffsets: vertex, index, triangle-face, batch, meshlet,
	// leaf-range, leaf-reference, material.
	out = WorldMeshData();
	out.version = summary.version;
	out.vertices.resize( summary.vertexCount );
	const unsigned char *vertex = bytes + summary.sectionOffsets[0];
	for ( uint32_t i = 0; i < summary.vertexCount; ++i, vertex += kWorldMeshVertexSize )
	{
		WorldMeshVertex &v = out.vertices[i];
		for ( int c = 0; c < 3; ++c )
			v.position[c] = ReadF32( vertex + 4 * c );
		DecodeOctahedral( ReadI16( vertex + 12 ), ReadI16( vertex + 14 ), v.normal );
		DecodeOctahedral( ReadI16( vertex + 16 ), ReadI16( vertex + 18 ), v.tangent );
		v.tangentSign = int8_t( vertex[20] ) < 0 ? -1.0f : 1.0f;
		v.uv[0] = ReadF32( vertex + 24 );
		v.uv[1] = ReadF32( vertex + 28 );
		v.lightmapUv[0] = ReadF32( vertex + 32 );
		v.lightmapUv[1] = ReadF32( vertex + 36 );
	}
	out.indices.resize( summary.indexCount );
	const unsigned char *index = bytes + summary.sectionOffsets[1];
	for ( uint32_t i = 0; i < summary.indexCount; ++i )
		out.indices[i] = ReadU32( index + 4 * i );
	out.batches.resize( summary.batchCount );
	const unsigned char *batch = bytes + summary.sectionOffsets[3];
	for ( uint32_t i = 0; i < summary.batchCount; ++i, batch += kWorldMeshBatchSize )
		out.batches[i] = { ReadU32( batch ), ReadU32( batch + 4 ), ReadU32( batch + 8 ) };
	out.materials.reserve( summary.materialCount );
	const unsigned char *material = bytes + summary.sectionOffsets[7];
	for ( uint32_t i = 0; i < summary.materialCount; ++i )
	{
		const uint32_t length = ReadU32( material );
		out.materials.emplace_back( reinterpret_cast<const char *>( material + 4 ), length );
		material += 4 + ( ( length + 3 ) & ~3u );
	}
	return WorldMeshError::Ok;
}

} // namespace mapcontainer

#endif // MAPCONTAINER_WORLD_MESH_DECODE_H
