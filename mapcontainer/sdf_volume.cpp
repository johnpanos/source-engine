//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: SDFV signed distance volume validation; see
//          public/mapcontainer/sdf_volume.h.
//
//=============================================================================//

#include "mapcontainer/sdf_volume.h"

#include "mapcontainer/probe_volume.h"

#include <cmath>
#include <cstring>

namespace mapcontainer
{
namespace
{

uint32_t U32( const unsigned char *p )
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

float F32( const unsigned char *p )
{
	const uint32_t bits = U32( p );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

float Half( const unsigned char *p )
{
	return HalfToFloat( uint16_t( p[0] | ( p[1] << 8 ) ) );
}

} // namespace

SdfVolumeError ValidateSdfVolume( const void *pData, size_t size, SdfVolumeLayout *pLayout ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( !p || size < kSdfVolumeHeaderBytes )
		return SdfVolumeError::Truncated;
	if ( U32( p ) != kLumpSdfVolume )
		return SdfVolumeError::BadMagic;
	if ( U32( p + 4 ) != kSdfVolumeVersion || U32( p + 8 ) != kSdfVolumeHeaderBytes ||
	     U32( p + 12 ) != 0 || U32( p + 52 ) || U32( p + 56 ) || U32( p + 60 ) )
		return SdfVolumeError::UnsupportedVersion;
	SdfVolumeLayout layout = {};
	for ( int k = 0; k < 3; ++k )
	{
		layout.origin[k] = F32( p + 16 + 4 * k );
		layout.dims[k] = U32( p + 32 + 4 * k );
	}
	layout.voxel = F32( p + 28 );
	layout.lightCount = U32( p + 44 );
	layout.maxDistance = F32( p + 48 );
	const uint64_t count = uint64_t( layout.dims[0] ) * layout.dims[1] * layout.dims[2];
	bool finite = std::isfinite( layout.origin[0] ) && std::isfinite( layout.origin[1] ) &&
	              std::isfinite( layout.origin[2] );
	if ( !finite || !( layout.voxel > 0.0f ) || !std::isfinite( layout.voxel ) ||
	     layout.dims[0] < 2 || layout.dims[1] < 2 || layout.dims[2] < 2 || count > kSdfMaxVoxels ||
	     layout.lightCount > kSdfMaxLights || !( layout.maxDistance > 0.0f ) ||
	     !std::isfinite( layout.maxDistance ) )
		return SdfVolumeError::InvalidGrid;
	layout.voxelOffset = kSdfVolumeHeaderBytes;
	layout.lightOffset = layout.voxelOffset + count * kSdfVoxelBytes;
	if ( layout.lightOffset + uint64_t( layout.lightCount ) * kSdfLightBytes != size )
		return SdfVolumeError::SizeMismatch;
	const float limit = layout.maxDistance * 1.01f + 1.0f;
	for ( uint64_t i = 0; i < count; ++i )
	{
		const unsigned char *v = p + layout.voxelOffset + i * kSdfVoxelBytes;
		const float distance = Half( v );
		if ( !std::isfinite( distance ) || std::fabs( distance ) > limit )
			return SdfVolumeError::InvalidVoxel;
		for ( int c = 0; c < 3; ++c )
		{
			const float reflectance = Half( v + 2 + 2 * c );
			const float emission = Half( v + 8 + 2 * c );
			if ( !std::isfinite( reflectance ) || reflectance < 0.0f || reflectance > 1.0f ||
			     !std::isfinite( emission ) || emission < 0.0f )
				return SdfVolumeError::InvalidVoxel;
		}
		const uint16_t source = uint16_t( v[14] | ( v[15] << 8 ) );
		if ( source != kSdfNoSource && source >= 64 )
			return SdfVolumeError::InvalidVoxel;
	}
	for ( uint32_t i = 0; i < layout.lightCount; ++i )
	{
		SdfLight light;
		std::memcpy( &light, p + layout.lightOffset + uint64_t( i ) * kSdfLightBytes, sizeof( light ) );
		bool ok = light.kind <= uint32_t( SdfLightKind::Dome ) && light.style >= -1 &&
		          light.style <= 63 && light.reserved[0] == 0.0f && light.reserved[1] == 0.0f;
		for ( int k = 0; k < 3 && ok; ++k )
			ok = std::isfinite( light.rgb[k] ) && light.rgb[k] >= 0.0f && std::isfinite( light.a[k] ) &&
			     std::isfinite( light.b[k] ) && std::isfinite( light.c[k] );
		if ( ok && light.kind == uint32_t( SdfLightKind::Distant ) )
		{
			const float length = std::sqrt(
			    light.a[0] * light.a[0] + light.a[1] * light.a[1] + light.a[2] * light.a[2] );
			ok = std::fabs( length - 1.0f ) <= 1e-3f;
		}
		if ( !ok )
			return SdfVolumeError::InvalidLight;
	}
	if ( pLayout )
		*pLayout = layout;
	return SdfVolumeError::Ok;
}

const char *SdfVolumeErrorName( SdfVolumeError error ) noexcept
{
	switch ( error )
	{
	case SdfVolumeError::Ok:
		return "ok";
	case SdfVolumeError::Truncated:
		return "truncated";
	case SdfVolumeError::BadMagic:
		return "bad-magic";
	case SdfVolumeError::UnsupportedVersion:
		return "unsupported-version";
	case SdfVolumeError::InvalidGrid:
		return "invalid-grid";
	case SdfVolumeError::SizeMismatch:
		return "size-mismatch";
	case SdfVolumeError::InvalidVoxel:
		return "invalid-voxel";
	case SdfVolumeError::InvalidLight:
		return "invalid-light";
	}
	return "unknown";
}

} // namespace mapcontainer
