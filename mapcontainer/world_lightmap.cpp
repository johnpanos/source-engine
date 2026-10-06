//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: LMAP (BSP2 world lightmap) validation; see world_lightmap.h.
//
//=============================================================================//

#include "mapcontainer/world_lightmap.h"

#include <cstring>

#define BCDECDEF static inline
#define BCDEC_IMPLEMENTATION
#include "external/bcdec/bcdec.h"

namespace mapcontainer
{
namespace
{

uint32_t U32( const unsigned char *p )
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

uint64_t U64( const unsigned char *p )
{
	return uint64_t( U32( p ) ) | ( uint64_t( U32( p + 4 ) ) << 32 );
}

uint64_t BlockBytes( uint32_t width, uint32_t height, uint32_t perBlock )
{
	return uint64_t( ( width + 3 ) / 4 ) * ( ( height + 3 ) / 4 ) * perBlock;
}

uint16_t FloatToHalf( float value )
{
	uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	const uint32_t sign = ( bits >> 16 ) & 0x8000u;
	const int exponent = int( ( bits >> 23 ) & 0xff ) - 127 + 15;
	uint32_t mantissa = bits & 0x7fffffu;
	if ( exponent >= 31 )
		return uint16_t( sign | 0x7bffu ); // BC6H decodes finite: clamp to the largest half
	if ( exponent <= 0 )
	{
		if ( exponent < -10 )
			return uint16_t( sign );
		mantissa |= 0x800000u;
		const int shift = 14 - exponent;
		uint32_t half = mantissa >> shift;
		if ( ( mantissa >> ( shift - 1 ) ) & 1u )
			++half;
		return uint16_t( sign | half );
	}
	uint32_t half = sign | ( uint32_t( exponent ) << 10 ) | ( mantissa >> 13 );
	if ( mantissa & 0x1000u )
		++half;
	return uint16_t( half );
}

} // namespace

WorldLightmapError ValidateWorldLightmap( const void *pData, size_t size, uint32_t expectedVersion,
    WorldLightmapBlocks *pBlocks ) noexcept
{
	if ( !pData || size < kWorldLightmapHeaderBytes )
		return WorldLightmapError::Truncated;
	if ( size > kWorldLightmapMaxBytes )
		return WorldLightmapError::InvalidLevelIndex;
	if ( expectedVersion != 0 && expectedVersion != kWorldLightmapVersion )
		return WorldLightmapError::UnsupportedVersion;
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( U32( p ) != kWorldLightmapMagic )
		return WorldLightmapError::BadIdentifier;
	const uint32_t flags = U32( p + 20 );
	if ( U32( p + 4 ) != kWorldLightmapVersion || ( flags & ~kWorldLightmapFlagSun ) != 0 ||
	     U64( p + 56 ) != 0 )
		return WorldLightmapError::UnsupportedVersion;
	if ( U32( p + 24 ) != kWorldLightmapIrradianceVkFormat ||
	     U32( p + 28 ) != kWorldLightmapGradientVkFormat )
		return WorldLightmapError::UnsupportedFormat;
	const uint32_t width = U32( p + 8 );
	const uint32_t height = U32( p + 12 );
	const uint32_t layers = U32( p + 16 );
	if ( layers < 1 || layers > kWorldLightmapMaxLayers )
		return WorldLightmapError::InvalidLayerCount;
	const uint64_t irradianceBytes = U64( p + 32 );
	const uint64_t gradientBytes = U64( p + 40 );
	if ( width < 1 || height < 1 || width > kWorldLightmapMaxDimension ||
	     height > kWorldLightmapMaxDimension ||
	     irradianceBytes != BlockBytes( width, height, 16 ) ||
	     gradientBytes != BlockBytes( width, height, 16 ) ||
	     U64( p + 48 ) != kWorldLightmapHeaderBytes )
		return WorldLightmapError::InvalidDescriptor;
	if ( size != kWorldLightmapHeaderBytes + layers * ( irradianceBytes + gradientBytes ) )
		return WorldLightmapError::InvalidLevelIndex;
	if ( pBlocks )
	{
		static const WorldLightmapLayer
		    kRoles[kWorldLightmapMaxLayers + 1][kWorldLightmapMaxLayers] = { {},
		        { WorldLightmapLayer::Total },
		        { WorldLightmapLayer::Total, WorldLightmapLayer::Indirect },
		        { WorldLightmapLayer::Total, WorldLightmapLayer::Direct,
		            WorldLightmapLayer::Indirect } };
		WorldLightmapBlocks blocks;
		blocks.width = width;
		blocks.height = height;
		blocks.layerCount = layers;
		blocks.sun = ( flags & kWorldLightmapFlagSun ) != 0;
		blocks.irradianceBytes = irradianceBytes;
		blocks.gradientBytes = gradientBytes;
		for ( uint32_t i = 0; i < layers; ++i )
		{
			blocks.irradianceOffset[i] =
			    kWorldLightmapHeaderBytes + i * ( irradianceBytes + gradientBytes );
			blocks.gradientOffset[i] = blocks.irradianceOffset[i] + irradianceBytes;
			blocks.roles[i] = kRoles[layers][i];
		}
		*pBlocks = blocks;
	}
	return WorldLightmapError::Ok;
}

bool DecodeWorldLightmap( const void *pData, const WorldLightmapBlocks &blocks,
    std::vector<std::byte> *pOut, WorldLightmapLayout *pLayout )
{
	const uint32_t page = blocks.width;
	const uint32_t width = 2 * page;
	const uint64_t layerBytes = uint64_t( width ) * blocks.height * kWorldLightmapTexelBytes;
	try
	{
		pOut->assign( size_t( layerBytes * blocks.layerCount ), std::byte{ 0 } );
	}
	catch ( ... )
	{
		return false;
	}
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	const uint16_t one = 0x3c00u;
	const uint32_t blocksAcross = ( page + 3 ) / 4;
	const uint32_t blocksDown = ( blocks.height + 3 ) / 4;
	for ( uint32_t layer = 0; layer < blocks.layerCount; ++layer )
	{
		uint16_t *texels =
		    reinterpret_cast<uint16_t *>( pOut->data() + size_t( layerBytes ) * layer );
		const bool sun = layer == 0 && blocks.sun;
		for ( uint32_t by = 0; by < blocksDown; ++by )
			for ( uint32_t bx = 0; bx < blocksAcross; ++bx )
			{
				const size_t block = size_t( by ) * blocksAcross + bx;
				float light[16 * 3];
				unsigned char gradient[16 * 4];
				bcdec_bc6h_float(
				    p + blocks.irradianceOffset[layer] + 16 * block, light, 4 * 3, 0 );
				bcdec_bc7( p + blocks.gradientOffset[layer] + 16 * block, gradient, 4 * 4 );
				for ( uint32_t y = 0; y < 4; ++y )
					for ( uint32_t x = 0; x < 4; ++x )
					{
						const uint32_t px = bx * 4 + x, py = by * 4 + y;
						if ( px >= page || py >= blocks.height )
							continue;
						const uint32_t t = y * 4 + x;
						uint16_t *left = texels + ( size_t( py ) * width + px ) * 4;
						uint16_t *right = left + size_t( page ) * 4;
						for ( int c = 0; c < 3; ++c )
						{
							left[c] = FloatToHalf( light[t * 3 + c] );
							right[c] = FloatToHalf( ( gradient[t * 4 + c] / 255.0f * 2.0f - 1.0f ) *
							                        kWorldLightmapBetaRange );
						}
						left[3] = sun ? FloatToHalf( gradient[t * 4 + 3] / 255.0f ) : one;
						right[3] = one;
					}
			}
	}
	if ( pLayout )
	{
		WorldLightmapLayout layout = {};
		layout.version = kWorldLightmapVersion;
		layout.width = width;
		layout.height = blocks.height;
		layout.layerCount = blocks.layerCount;
		layout.layerBytes = layerBytes;
		for ( uint32_t i = 0; i < blocks.layerCount; ++i )
		{
			layout.layerOffset[i] = layerBytes * i;
			layout.roles[i] = blocks.roles[i];
		}
		*pLayout = layout;
	}
	return true;
}

int WorldLightmapLayerIndex( const WorldLightmapBlocks &blocks, WorldLightmapLayer role ) noexcept
{
	for ( uint32_t i = 0; i < blocks.layerCount && i < kWorldLightmapMaxLayers; ++i )
		if ( blocks.roles[i] == role )
			return int( i );
	return -1;
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
		return "bad-identifier";
	case WorldLightmapError::UnsupportedFormat:
		return "unsupported-format";
	case WorldLightmapError::InvalidLayerCount:
		return "invalid-layer-count";
	case WorldLightmapError::InvalidDescriptor:
		return "invalid-descriptor";
	case WorldLightmapError::InvalidLevelIndex:
		return "invalid-level-index";
	case WorldLightmapError::UnsupportedVersion:
		return "unsupported-version";
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
