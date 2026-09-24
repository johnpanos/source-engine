//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: WMSH v1 validation before a tool publishes or client borrows it.
//
//=============================================================================//

#include "mapcontainer/world_mesh.h"

#include "mapcontainer/world_mesh_format.h"

#include <array>
#include <cmath>
#include <cstring>
#include <limits>

namespace mapcontainer
{
static_assert( sizeof( float ) == 4 && std::numeric_limits<float>::is_iec559,
    "WMSH v1 requires IEEE-754 binary32" );

namespace
{

uint32_t ReadU32( const uint8_t *pBytes ) noexcept
{
	uint32_t value = 0;
	for ( int i = 3; i >= 0; --i )
		value = ( value << 8 ) | pBytes[i];
	return value;
}

uint64_t ReadU64( const uint8_t *pBytes ) noexcept
{
	uint64_t value = 0;
	for ( int i = 7; i >= 0; --i )
		value = ( value << 8 ) | pBytes[i];
	return value;
}

float ReadF32( const uint8_t *pBytes ) noexcept
{
	const uint32_t bits = ReadU32( pBytes );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

bool FiniteF32( const uint8_t *pBytes ) noexcept
{
	return ( ReadU32( pBytes ) & 0x7F800000u ) != 0x7F800000u;
}

bool DecodeOct( const uint8_t *pBytes, float *pVector ) noexcept
{
	const uint16_t rawX = uint16_t( pBytes[0] ) | ( uint16_t( pBytes[1] ) << 8 );
	const uint16_t rawY = uint16_t( pBytes[2] ) | ( uint16_t( pBytes[3] ) << 8 );
	const int xInteger = rawX <= 32767 ? rawX : int( rawX ) - 65536;
	const int yInteger = rawY <= 32767 ? rawY : int( rawY ) - 65536;
	if ( xInteger == -32768 || yInteger == -32768 )
		return false;
	float x = float( xInteger ) / 32767.0f;
	float y = float( yInteger ) / 32767.0f;
	const float z = 1.0f - std::fabs( x ) - std::fabs( y );
	if ( z < 0 )
	{
		const float foldedX = ( 1.0f - std::fabs( y ) ) * ( x >= 0 ? 1.0f : -1.0f );
		y = ( 1.0f - std::fabs( x ) ) * ( y >= 0 ? 1.0f : -1.0f );
		x = foldedX;
	}
	const float length = std::sqrt( x * x + y * y + z * z );
	if ( length < 0.00001f )
		return false;
	pVector[0] = x / length;
	pVector[1] = y / length;
	pVector[2] = z / length;
	return true;
}

bool ValidUtf8( const uint8_t *pBytes, uint32_t size ) noexcept
{
	for ( uint32_t i = 0; i < size; )
	{
		const uint8_t first = pBytes[i];
		if ( first == 0 )
			return false;
		if ( first < 0x80 )
		{
			++i;
			continue;
		}
		uint32_t count = 0;
		uint32_t codepoint = 0;
		if ( first >= 0xC2 && first <= 0xDF )
		{
			count = 2;
			codepoint = first & 0x1F;
		}
		else if ( first >= 0xE0 && first <= 0xEF )
		{
			count = 3;
			codepoint = first & 0x0F;
		}
		else if ( first >= 0xF0 && first <= 0xF4 )
		{
			count = 4;
			codepoint = first & 0x07;
		}
		else
		{
			return false;
		}
		if ( count > size - i )
			return false;
		for ( uint32_t j = 1; j < count; ++j )
		{
			const uint8_t next = pBytes[i + j];
			if ( ( next & 0xC0 ) != 0x80 )
				return false;
			codepoint = ( codepoint << 6 ) | ( next & 0x3F );
		}
		if ( ( count == 2 && codepoint < 0x80 ) || ( count == 3 && codepoint < 0x800 ) ||
		     ( count == 4 && codepoint < 0x10000 ) ||
		     ( codepoint >= 0xD800 && codepoint <= 0xDFFF ) || codepoint > 0x10FFFF )
			return false;
		i += count;
	}
	return true;
}

bool ZeroPadding( const uint8_t *pBytes, uint64_t begin, uint64_t end ) noexcept
{
	for ( uint64_t i = begin; i < end; ++i )
	{
		if ( pBytes[i] != 0 )
			return false;
	}
	return true;
}

} // namespace

WorldMeshError ValidateWorldMesh(
    const void *pData, size_t size, WorldMeshSummary *pSummary ) noexcept
{
	if ( !pData || size < kWorldMeshHeaderSize )
		return WorldMeshError::Truncated;
	const uint8_t *pBytes = static_cast<const uint8_t *>( pData );
	if ( std::memcmp( pBytes, "WMSH", 4 ) != 0 )
		return WorldMeshError::BadMagic;
	const uint32_t version = ReadU32( pBytes + 4 );
	if ( version < kWorldMeshMinVersion || version > kWorldMeshVersion )
		return WorldMeshError::UnsupportedVersion;
	if ( ReadU32( pBytes + 8 ) != kWorldMeshHeaderSize || ReadU32( pBytes + 12 ) != 0 ||
	     ReadU32( pBytes + 52 ) != 0 || ReadU64( pBytes + 120 ) != size )
		return WorldMeshError::InvalidLayout;

	WorldMeshSummary summary{};
	summary.version = version;
	summary.vertexCount = ReadU32( pBytes + 16 );
	summary.indexCount = ReadU32( pBytes + 20 );
	summary.triangleCount = ReadU32( pBytes + 24 );
	summary.batchCount = ReadU32( pBytes + 28 );
	summary.meshletCount = ReadU32( pBytes + 32 );
	summary.leafCount = ReadU32( pBytes + 36 );
	summary.leafReferenceCount = ReadU32( pBytes + 40 );
	summary.materialCount = ReadU32( pBytes + 44 );
	const uint32_t materialBytes = ReadU32( pBytes + 48 );
	if ( !summary.vertexCount || summary.indexCount != summary.vertexCount ||
	     uint64_t( summary.triangleCount ) * 3 != summary.indexCount || !summary.batchCount ||
	     summary.batchCount != summary.materialCount || !summary.meshletCount ||
	     !summary.leafCount || !materialBytes )
		return WorldMeshError::InvalidCounts;

	const std::array<uint64_t, 8> sectionSizes = { uint64_t( summary.vertexCount ) * 40,
	    uint64_t( summary.indexCount ) * 4, uint64_t( summary.triangleCount ) * 4,
	    uint64_t( summary.batchCount ) * 24, uint64_t( summary.meshletCount ) * 48,
	    uint64_t( summary.leafCount ) * 8, uint64_t( summary.leafReferenceCount ) * 4,
	    materialBytes };
	uint64_t cursor = kWorldMeshHeaderSize;
	for ( size_t i = 0; i < sectionSizes.size(); ++i )
	{
		const uint64_t aligned = ( cursor + 15 ) & ~uint64_t( 15 );
		const uint64_t offset = ReadU64( pBytes + 56 + 8 * i );
		if ( aligned > size || offset != aligned || sectionSizes[i] > size - offset ||
		     !ZeroPadding( pBytes, cursor, aligned ) )
			return WorldMeshError::InvalidSections;
		summary.sectionOffsets[i] = offset;
		cursor = offset + sectionSizes[i];
	}
	if ( cursor != size )
		return WorldMeshError::InvalidSections;

	for ( uint32_t i = 0; i < summary.vertexCount; ++i )
	{
		const uint8_t *pVertex = pBytes + summary.sectionOffsets[0] + uint64_t( i ) * 40;
		if ( pVertex[20] != 1 && pVertex[20] != 0xFF )
			return WorldMeshError::InvalidVertices;
		if ( pVertex[21] || pVertex[22] || pVertex[23] )
			return WorldMeshError::InvalidVertices;
		for ( uint32_t offset : { 0u, 4u, 8u, 24u, 28u, 32u, 36u } )
		{
			if ( !FiniteF32( pVertex + offset ) )
				return WorldMeshError::InvalidVertices;
		}
		float normal[3];
		float tangent[3];
		if ( !DecodeOct( pVertex + 12, normal ) || !DecodeOct( pVertex + 16, tangent ) ||
		     std::fabs( normal[0] * tangent[0] + normal[1] * tangent[1] + normal[2] * tangent[2] ) >
		         0.001f )
			return WorldMeshError::InvalidVertices;
	}
	for ( uint32_t i = 0; i < summary.indexCount; ++i )
	{
		if ( ReadU32( pBytes + summary.sectionOffsets[1] + uint64_t( i ) * 4 ) != i )
			return WorldMeshError::InvalidIndices;
	}
	uint32_t indexCursor = 0;
	uint32_t meshletCursor = 0;
	for ( uint32_t i = 0; i < summary.batchCount; ++i )
	{
		const uint8_t *pBatch = pBytes + summary.sectionOffsets[3] + uint64_t( i ) * 24;
		const uint32_t indexCount = ReadU32( pBatch + 8 );
		const uint32_t meshletCount = ReadU32( pBatch + 16 );
		if ( ReadU32( pBatch ) != i || ReadU32( pBatch + 4 ) != indexCursor ||
		     ReadU32( pBatch + 12 ) != meshletCursor || !indexCount || indexCount % 3 ||
		     !meshletCount || ReadU32( pBatch + 20 ) != 0 ||
		     indexCount > summary.indexCount - indexCursor ||
		     meshletCount > summary.meshletCount - meshletCursor )
			return WorldMeshError::InvalidBatches;
		const uint8_t *pFirstMeshlet =
		    pBytes + summary.sectionOffsets[4] + uint64_t( meshletCursor ) * 48;
		const uint8_t *pLastMeshlet = pFirstMeshlet + uint64_t( meshletCount - 1 ) * 48;
		if ( ReadU32( pFirstMeshlet ) != indexCursor ||
		     uint64_t( ReadU32( pLastMeshlet ) ) + ReadU32( pLastMeshlet + 4 ) !=
		         uint64_t( indexCursor ) + indexCount )
			return WorldMeshError::InvalidBatches;
		indexCursor += indexCount;
		meshletCursor += meshletCount;
	}
	if ( indexCursor != summary.indexCount || meshletCursor != summary.meshletCount )
		return WorldMeshError::InvalidBatches;

	indexCursor = 0;
	for ( uint32_t i = 0; i < summary.meshletCount; ++i )
	{
		const uint8_t *pMeshlet = pBytes + summary.sectionOffsets[4] + uint64_t( i ) * 48;
		const uint32_t count = ReadU32( pMeshlet + 4 );
		if ( ReadU32( pMeshlet ) != indexCursor || ReadU32( pMeshlet + 8 ) != indexCursor ||
		     ReadU32( pMeshlet + 12 ) != count || !count || count % 3 || count > 64 ||
		     count > summary.indexCount - indexCursor )
			return WorldMeshError::InvalidMeshlets;
		for ( uint32_t j = 16; j < 48; j += 4 )
		{
			if ( !FiniteF32( pMeshlet + j ) )
				return WorldMeshError::InvalidMeshlets;
		}
		const float axisX = ReadF32( pMeshlet + 32 );
		const float axisY = ReadF32( pMeshlet + 36 );
		const float axisZ = ReadF32( pMeshlet + 40 );
		const float axisLength = axisX * axisX + axisY * axisY + axisZ * axisZ;
		if ( ReadF32( pMeshlet + 28 ) < 0 || std::fabs( axisLength - 1.0f ) > 0.0001f ||
		     ReadF32( pMeshlet + 44 ) < -1 || ReadF32( pMeshlet + 44 ) > 1 )
			return WorldMeshError::InvalidMeshlets;
		const float centerX = ReadF32( pMeshlet + 16 );
		const float centerY = ReadF32( pMeshlet + 20 );
		const float centerZ = ReadF32( pMeshlet + 24 );
		const float radius = ReadF32( pMeshlet + 28 ) + 0.0001f;
		const float cutoff = ReadF32( pMeshlet + 44 ) - 0.0001f;
		const uint32_t face = ReadU32( pBytes + summary.sectionOffsets[2] + indexCursor / 3 * 4 );
		for ( uint32_t j = 0; j < count; ++j )
		{
			const uint8_t *pVertex =
			    pBytes + summary.sectionOffsets[0] + uint64_t( indexCursor + j ) * 40;
			const float dx = ReadF32( pVertex ) - centerX;
			const float dy = ReadF32( pVertex + 4 ) - centerY;
			const float dz = ReadF32( pVertex + 8 ) - centerZ;
			float normal[3];
			if ( dx * dx + dy * dy + dz * dz > radius * radius ||
			     !DecodeOct( pVertex + 12, normal ) ||
			     ( version == 1 &&
			         normal[0] * axisX + normal[1] * axisY + normal[2] * axisZ < cutoff ) ||
			     ReadU32( pBytes + summary.sectionOffsets[2] + ( indexCursor + j ) / 3 * 4 ) !=
			         face )
				return WorldMeshError::InvalidMeshlets;
		}
		// Version 2: the cone holds every non-degenerate front-face normal,
		// computed in double from the stored float positions.
		for ( uint32_t j = 0; version >= 2 && j < count; j += 3 )
		{
			const uint8_t *pA = pBytes + summary.sectionOffsets[0] + uint64_t( indexCursor + j ) * 40;
			double edge[2][3];
			for ( int k = 0; k < 2; ++k )
			{
				for ( int axis = 0; axis < 3; ++axis )
					edge[k][axis] = double( ReadF32( pA + 40 * ( k + 1 ) + 4 * axis ) ) -
					                double( ReadF32( pA + 4 * axis ) );
			}
			const double cross[3] = { edge[0][1] * edge[1][2] - edge[0][2] * edge[1][1],
			    edge[0][2] * edge[1][0] - edge[0][0] * edge[1][2],
			    edge[0][0] * edge[1][1] - edge[0][1] * edge[1][0] };
			const double length =
			    std::sqrt( cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2] );
			if ( length > kWorldMeshDegenerateCross &&
			     ( cross[0] * axisX + cross[1] * axisY + cross[2] * axisZ ) / length < cutoff )
				return WorldMeshError::InvalidMeshlets;
		}
		indexCursor += count;
	}
	if ( indexCursor != summary.indexCount )
		return WorldMeshError::InvalidMeshlets;

	uint32_t referenceCursor = 0;
	for ( uint32_t i = 0; i < summary.leafCount; ++i )
	{
		const uint8_t *pLeaf = pBytes + summary.sectionOffsets[5] + uint64_t( i ) * 8;
		const uint32_t count = ReadU32( pLeaf + 4 );
		if ( ReadU32( pLeaf ) != referenceCursor ||
		     count > summary.leafReferenceCount - referenceCursor )
			return WorldMeshError::InvalidLeaves;
		uint32_t previous = 0;
		for ( uint32_t j = 0; j < count; ++j )
		{
			const uint32_t value =
			    ReadU32( pBytes + summary.sectionOffsets[6] + uint64_t( referenceCursor + j ) * 4 );
			if ( value >= summary.meshletCount || ( j && value <= previous ) )
				return WorldMeshError::InvalidLeaves;
			previous = value;
		}
		referenceCursor += count;
	}
	if ( referenceCursor != summary.leafReferenceCount )
		return WorldMeshError::InvalidLeaves;

	uint32_t materialCursor = 0;
	const uint8_t *pMaterials = pBytes + summary.sectionOffsets[7];
	const uint8_t *pPreviousMaterial = nullptr;
	uint32_t previousLength = 0;
	for ( uint32_t i = 0; i < summary.materialCount; ++i )
	{
		if ( materialBytes - materialCursor < 4 )
			return WorldMeshError::InvalidMaterials;
		const uint32_t length = ReadU32( pMaterials + materialCursor );
		if ( !length || length > materialBytes - materialCursor - 4 ||
		     !ValidUtf8( pMaterials + materialCursor + 4, length ) )
			return WorldMeshError::InvalidMaterials;
		const uint8_t *pName = pMaterials + materialCursor + 4;
		if ( pName[0] == '/' || pName[0] == '\\' ||
		     ( length >= 4 && std::memcmp( pName + length - 4, ".vmt", 4 ) == 0 ) )
			return WorldMeshError::InvalidMaterials;
		for ( uint32_t j = 0; j < length; ++j )
		{
			if ( pName[j] < 0x20 || pName[j] == 0x7F || pName[j] == '\\' || pName[j] == ':' ||
			     ( pName[j] == '/' && ( j + 1 == length || ( j && pName[j - 1] == '/' ) ) ) ||
			     ( pName[j] == '.' && ( j == 0 || pName[j - 1] == '/' ) &&
			         ( j + 1 == length || pName[j + 1] == '/' ||
			             ( pName[j + 1] == '.' && ( j + 2 == length || pName[j + 2] == '/' ) ) ) ) )
				return WorldMeshError::InvalidMaterials;
		}
		if ( pPreviousMaterial )
		{
			const uint32_t common = previousLength < length ? previousLength : length;
			const int order = std::memcmp( pPreviousMaterial, pName, common );
			if ( order > 0 || ( order == 0 && previousLength >= length ) )
				return WorldMeshError::InvalidMaterials;
		}
		const uint64_t end = uint64_t( materialCursor ) + 4 + length;
		const uint64_t padded = ( end + 3 ) & ~uint64_t( 3 );
		if ( padded > materialBytes || !ZeroPadding( pMaterials, end, padded ) )
			return WorldMeshError::InvalidMaterials;
		materialCursor = uint32_t( padded );
		pPreviousMaterial = pName;
		previousLength = length;
	}
	if ( materialCursor != materialBytes )
		return WorldMeshError::InvalidMaterials;
	if ( pSummary )
		*pSummary = summary;
	return WorldMeshError::Ok;
}

const char *WorldMeshErrorName( WorldMeshError error ) noexcept
{
	switch ( error )
	{
	case WorldMeshError::Ok:
		return "ok";
	case WorldMeshError::Truncated:
		return "truncated";
	case WorldMeshError::BadMagic:
		return "bad-magic";
	case WorldMeshError::UnsupportedVersion:
		return "unsupported-version";
	case WorldMeshError::InvalidLayout:
		return "invalid-layout";
	case WorldMeshError::InvalidCounts:
		return "invalid-counts";
	case WorldMeshError::InvalidSections:
		return "invalid-sections";
	case WorldMeshError::InvalidVertices:
		return "invalid-vertices";
	case WorldMeshError::InvalidIndices:
		return "invalid-indices";
	case WorldMeshError::InvalidBatches:
		return "invalid-batches";
	case WorldMeshError::InvalidMeshlets:
		return "invalid-meshlets";
	case WorldMeshError::InvalidLeaves:
		return "invalid-leaves";
	case WorldMeshError::InvalidMaterials:
		return "invalid-materials";
	}
	return "unknown";
}

} // namespace mapcontainer
