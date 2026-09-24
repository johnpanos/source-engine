//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: LMAP (BSP2 world lightmap) validation; see world_lightmap.h.
//
//=============================================================================//

#include "mapcontainer/world_lightmap.h"

#include <cstring>

namespace mapcontainer
{
namespace
{

const unsigned char kKtx2Identifier[12] = {
    0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
const uint32_t kVkFormatR16G16B16A16Sfloat = 97;
const uint32_t kKtx2HeaderBytes = 80;
const uint32_t kKtx2LevelIndexBytes = 24;

uint32_t U32( const unsigned char *p )
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

uint64_t U64( const unsigned char *p )
{
	return uint64_t( U32( p ) ) | ( uint64_t( U32( p + 4 ) ) << 32 );
}

bool RangeInside( uint64_t offset, uint64_t length, uint64_t size )
{
	return offset <= size && length <= size - offset;
}

} // namespace

WorldLightmapError ValidateWorldLightmap( const void *pData, size_t size, uint32_t expectedVersion,
    WorldLightmapLayout *pLayout ) noexcept
{
	if ( !pData || size < kKtx2HeaderBytes + kKtx2LevelIndexBytes )
		return WorldLightmapError::Truncated;
	if ( size > kWorldLightmapMaxBytes )
		return WorldLightmapError::InvalidLevelIndex;
	if ( expectedVersion != 0 && ( expectedVersion < kWorldLightmapMinVersion ||
	                                 expectedVersion > kWorldLightmapLayeredVersion ) )
		return WorldLightmapError::UnsupportedVersion;
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( std::memcmp( p, kKtx2Identifier, sizeof( kKtx2Identifier ) ) != 0 )
		return WorldLightmapError::BadIdentifier;
	const uint32_t format = U32( p + 12 );
	const uint32_t typeSize = U32( p + 16 );
	const uint32_t width = U32( p + 20 );
	const uint32_t height = U32( p + 24 );
	const uint32_t depth = U32( p + 28 );
	const uint32_t layers = U32( p + 32 );
	const uint32_t faces = U32( p + 36 );
	const uint32_t levels = U32( p + 40 );
	const uint32_t supercompression = U32( p + 44 );
	if ( format != kVkFormatR16G16B16A16Sfloat || typeSize != 2 )
		return WorldLightmapError::UnsupportedFormat;
	if ( width == 0 || height == 0 || width > kWorldLightmapMaxDimension ||
	     height > kWorldLightmapMaxDimension || depth != 0 || faces != 1 || levels != 1 ||
	     supercompression != 0 )
		return WorldLightmapError::UnsupportedTopology;
	uint32_t version = 0;
	uint32_t layerCount = 0;
	if ( layers == 0 )
	{
		version = kWorldLightmapMinVersion;
		layerCount = 1;
	}
	else if ( layers >= 2 && layers <= kWorldLightmapMaxLayers )
	{
		version = kWorldLightmapLayeredVersion;
		layerCount = layers;
	}
	else
		return WorldLightmapError::InvalidLayerCount;
	if ( expectedVersion != 0 && expectedVersion != version )
		return WorldLightmapError::VersionMismatch;
	const uint64_t dfdOffset = U32( p + 48 );
	const uint64_t dfdLength = U32( p + 52 );
	const uint64_t kvdOffset = U32( p + 56 );
	const uint64_t kvdLength = U32( p + 60 );
	const uint64_t sgdOffset = U64( p + 64 );
	const uint64_t sgdLength = U64( p + 72 );
	if ( dfdLength < 4 || !RangeInside( dfdOffset, dfdLength, size ) ||
	     dfdOffset < kKtx2HeaderBytes + kKtx2LevelIndexBytes || U32( p + dfdOffset ) != dfdLength ||
	     !RangeInside( kvdOffset, kvdLength, size ) || sgdOffset != 0 || sgdLength != 0 )
		return WorldLightmapError::InvalidDescriptor;
	const uint64_t levelOffset = U64( p + kKtx2HeaderBytes );
	const uint64_t levelLength = U64( p + kKtx2HeaderBytes + 8 );
	const uint64_t levelUncompressed = U64( p + kKtx2HeaderBytes + 16 );
	const uint64_t layerBytes = uint64_t( width ) * height * kWorldLightmapTexelBytes;
	if ( levelOffset % kWorldLightmapTexelBytes != 0 || levelLength != layerBytes * layerCount ||
	     levelUncompressed != levelLength || !RangeInside( levelOffset, levelLength, size ) ||
	     levelOffset < dfdOffset + dfdLength )
		return WorldLightmapError::InvalidLevelIndex;
	if ( pLayout )
	{
		static const WorldLightmapLayer
		    kRoles[kWorldLightmapMaxLayers + 1][kWorldLightmapMaxLayers] = { {},
		        { WorldLightmapLayer::Total },
		        { WorldLightmapLayer::Total, WorldLightmapLayer::Indirect },
		        { WorldLightmapLayer::Total, WorldLightmapLayer::Direct,
		            WorldLightmapLayer::Indirect } };
		WorldLightmapLayout layout = {};
		layout.version = version;
		layout.width = width;
		layout.height = height;
		layout.layerCount = layerCount;
		layout.layerBytes = layerBytes;
		for ( uint32_t i = 0; i < layerCount; ++i )
		{
			layout.layerOffset[i] = levelOffset + layerBytes * i;
			layout.roles[i] = kRoles[layerCount][i];
		}
		*pLayout = layout;
	}
	return WorldLightmapError::Ok;
}

int WorldLightmapLayerIndex( const WorldLightmapLayout &layout, WorldLightmapLayer role ) noexcept
{
	for ( uint32_t i = 0; i < layout.layerCount && i < kWorldLightmapMaxLayers; ++i )
		if ( layout.roles[i] == role )
			return int( i );
	return -1;
}

const char *WorldLightmapErrorName( WorldLightmapError error ) noexcept
{
	switch ( error )
	{
	case WorldLightmapError::Ok:
		return "ok";
	case WorldLightmapError::Truncated:
		return "truncated";
	case WorldLightmapError::BadIdentifier:
		return "bad-ktx2-identifier";
	case WorldLightmapError::UnsupportedFormat:
		return "unsupported-format";
	case WorldLightmapError::UnsupportedTopology:
		return "unsupported-topology";
	case WorldLightmapError::InvalidLayerCount:
		return "invalid-layer-count";
	case WorldLightmapError::InvalidDescriptor:
		return "invalid-descriptor";
	case WorldLightmapError::InvalidLevelIndex:
		return "invalid-level-index";
	case WorldLightmapError::UnsupportedVersion:
		return "unsupported-version";
	case WorldLightmapError::VersionMismatch:
		return "version-mismatch";
	}
	return "unknown";
}

const char *WorldLightmapLayerName( WorldLightmapLayer role ) noexcept
{
	switch ( role )
	{
	case WorldLightmapLayer::Total:
		return "total";
	case WorldLightmapLayer::Direct:
		return "direct";
	case WorldLightmapLayer::Indirect:
		return "indirect";
	}
	return "unknown";
}

} // namespace mapcontainer
