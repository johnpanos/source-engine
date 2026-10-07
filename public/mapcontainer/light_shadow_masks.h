//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: LSMK, the static lights' baked shadow masks (RFC 0016 runtime
//          direct light, Source 2's static-light shadow split).
//
// The encoding is owned by tools/quality/light_shadow_masks.py: a 64-byte
// header ("LSMK", 1, page width and height (the LMAP page's), light count N,
// the page VkFormat R16G16B16A16_UNORM, the page's bytes (width * height *
// 8), the data offset 64 + 16 N, 24 zero bytes), N records {f32 origin x, y,
// z in Source units, u32 id 1..255}, then the page's texels, top row first.
// Channel k of a texel is id << 8 | round(visibility * 255) of one of the
// texel's four dominant lights (id 0: none); lights sharing an id have
// disjoint reaches. A runtime light takes the record within
// kLightShadowMasksMatchUnits of its origin.
//
//=============================================================================//
#ifndef MAPCONTAINER_LIGHT_SHADOW_MASKS_H
#define MAPCONTAINER_LIGHT_SHADOW_MASKS_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace mapcontainer
{

static const uint32_t kLumpLightShadowMasks = 0x4B4D534Cu; // "LSMK"
static const uint32_t kLightShadowMasksVersion = 1;
static const uint32_t kLightShadowMasksHeaderBytes = 64;
static const uint32_t kLightShadowMasksVkFormat = 91; // R16G16B16A16_UNORM
static const uint32_t kLightShadowMasksMaxLights = 4096;
static const uint32_t kLightShadowMasksMaxIds = 255;
static const uint32_t kLightShadowMasksMaxDimension = 16384;
static const float kLightShadowMasksMatchUnits = 0.1f;

struct LightShadowMasks
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t lightCount = 0;
	uint64_t pageOffset = 0;
	uint64_t pageBytes = 0;
};

struct LightShadowMaskRecord
{
	float origin[3] = {};
	uint32_t id = 0; // 1..255
};

// Validates complete LSMK bytes; false on any defect.
inline bool ValidateLightShadowMasks(
    const void *pData, size_t size, LightShadowMasks *pOut = nullptr ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	auto u32 = [p]( size_t at )
	{
		uint32_t v;
		std::memcpy( &v, p + at, 4 );
		return v;
	};
	auto u64 = [p]( size_t at )
	{
		uint64_t v;
		std::memcpy( &v, p + at, 8 );
		return v;
	};
	if ( !p || size < kLightShadowMasksHeaderBytes )
		return false;
	if ( u32( 0 ) != kLumpLightShadowMasks || u32( 4 ) != kLightShadowMasksVersion ||
	     u32( 20 ) != kLightShadowMasksVkFormat )
		return false;
	for ( size_t i = 40; i < kLightShadowMasksHeaderBytes; ++i )
		if ( p[i] )
			return false;
	const uint32_t width = u32( 8 ), height = u32( 12 ), count = u32( 16 );
	if ( width < 1 || height < 1 || width > kLightShadowMasksMaxDimension ||
	     height > kLightShadowMasksMaxDimension || count < 1 || count > kLightShadowMasksMaxLights )
		return false;
	const uint64_t offset = uint64_t( kLightShadowMasksHeaderBytes ) + 16ull * count;
	const uint64_t pageBytes = uint64_t( width ) * height * 8;
	if ( u64( 32 ) != offset || u64( 24 ) != pageBytes || size != offset + pageBytes )
		return false;
	for ( uint32_t i = 0; i < count; ++i )
	{
		const size_t at = kLightShadowMasksHeaderBytes + 16 * size_t( i );
		const uint32_t id = u32( at + 12 );
		if ( id < 1 || id > kLightShadowMasksMaxIds )
			return false;
		// Finite origins (an exponent of all ones is an infinity or NaN; by
		// bits, since fast-math builds may fold std::isfinite).
		for ( size_t k = 0; k < 3; ++k )
			if ( ( u32( at + 4 * k ) & 0x7F800000u ) == 0x7F800000u )
				return false;
	}
	if ( pOut )
	{
		pOut->width = width;
		pOut->height = height;
		pOut->lightCount = count;
		pOut->pageOffset = offset;
		pOut->pageBytes = pageBytes;
	}
	return true;
}

// Record i of validated bytes.
inline LightShadowMaskRecord LightShadowMaskRecordAt( const void *pData, uint32_t i ) noexcept
{
	LightShadowMaskRecord record;
	std::memcpy( &record,
	    static_cast<const unsigned char *>( pData ) + kLightShadowMasksHeaderBytes +
	        16 * size_t( i ),
	    16 );
	return record;
}

} // namespace mapcontainer

#endif // MAPCONTAINER_LIGHT_SHADOW_MASKS_H
