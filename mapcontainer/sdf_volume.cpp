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
	const uint32_t version = U32( p + 4 );
	const uint32_t headerBytes = U32( p + 8 );
	if ( !( ( version == 1 && headerBytes == kSdfVolumeV1HeaderBytes ) ||
	        ( version == 2 && headerBytes == kSdfVolumeHeaderBytes ) ) ||
	     size < headerBytes || U32( p + 12 ) != 0 || U32( p + 52 ) || U32( p + 56 ) ||
	     U32( p + 60 ) )
		return SdfVolumeError::UnsupportedVersion;
	SdfVolumeLayout layout = {};
	layout.version = version;
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
	layout.voxelOffset = headerBytes;
	layout.lightOffset = layout.voxelOffset + count * kSdfVoxelBytes;
	const uint64_t lightsEnd = layout.lightOffset + uint64_t( layout.lightCount ) * kSdfLightBytes;
	uint64_t cellCount = 0;
	if ( version == 1 )
	{
		if ( lightsEnd != size )
			return SdfVolumeError::SizeMismatch;
	}
	else
	{
		for ( int k = 0; k < 3; ++k )
		{
			layout.cellOrigin[k] = F32( p + 64 + 4 * k );
			layout.cellDims[k] = U32( p + 80 + 4 * k );
		}
		layout.cellSize = F32( p + 76 );
		layout.cellEntries = U32( p + 92 );
		cellCount = uint64_t( layout.cellDims[0] ) * layout.cellDims[1] * layout.cellDims[2];
		if ( !std::isfinite( layout.cellOrigin[0] ) || !std::isfinite( layout.cellOrigin[1] ) ||
		     !std::isfinite( layout.cellOrigin[2] ) || !( layout.cellSize > 0.0f ) ||
		     !std::isfinite( layout.cellSize ) || layout.cellDims[0] < 1 || layout.cellDims[1] < 1 ||
		     layout.cellDims[2] < 1 || cellCount > kSdfMaxCells )
			return SdfVolumeError::InvalidCells;
		layout.cellOffset = lightsEnd;
		layout.cellEntryOffset = layout.cellOffset + ( cellCount + 1 ) * 4;
		if ( layout.cellEntryOffset + uint64_t( layout.cellEntries ) * 2 +
		         ( layout.cellEntries % 2 ) * 2 !=
		     size )
			return SdfVolumeError::SizeMismatch;
	}
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
		const uint32_t lastKind =
		    uint32_t( version == 1 ? SdfLightKind::Dome : SdfLightKind::Spot );
		bool ok = light.kind <= lastKind && light.style >= -1 && light.style <= 63 &&
		          std::isfinite( light.reserved[0] ) && std::isfinite( light.reserved[1] );
		for ( int k = 0; k < 3 && ok; ++k )
			ok = std::isfinite( light.rgb[k] ) && light.rgb[k] >= 0.0f && std::isfinite( light.a[k] ) &&
			     std::isfinite( light.b[k] ) && std::isfinite( light.c[k] );
		const auto length = []( const float *v )
		{ return std::sqrt( v[0] * v[0] + v[1] * v[1] + v[2] * v[2] ); };
		if ( ok && light.kind == uint32_t( SdfLightKind::Distant ) )
			ok = std::fabs( length( light.a ) - 1.0f ) <= 1e-3f;
		if ( ok && light.kind == uint32_t( SdfLightKind::Sphere ) )
			ok = light.b[0] > 0.0f && light.b[1] == 0.0f && light.b[2] == 0.0f &&
			     light.c[0] == 0.0f && light.c[1] == 0.0f && light.c[2] == 0.0f &&
			     light.reserved[0] == 0.0f && light.reserved[1] == 0.0f;
		else if ( ok && light.kind == uint32_t( SdfLightKind::Spot ) )
			ok = std::fabs( length( light.b ) - 1.0f ) <= 1e-3f && light.c[0] > 0.0f &&
			     light.c[2] >= -1.0f && light.c[2] <= light.c[1] && light.c[1] <= 1.0f &&
			     light.reserved[0] >= 0.0f && light.reserved[1] == 0.0f;
		else if ( ok )
			ok = light.reserved[0] == 0.0f && light.reserved[1] == 0.0f;
		if ( !ok )
			return SdfVolumeError::InvalidLight;
	}
	if ( version >= 2 )
	{
		uint32_t previous = 0;
		for ( uint64_t c = 0; c <= cellCount; ++c )
		{
			const uint32_t first = U32( p + layout.cellOffset + c * 4 );
			if ( ( c == 0 && first != 0 ) || first < previous ||
			     ( c == cellCount && first != layout.cellEntries ) )
				return SdfVolumeError::InvalidCells;
			previous = first;
		}
		for ( uint32_t e = 0; e < layout.cellEntries; ++e )
		{
			const unsigned char *entry = p + layout.cellEntryOffset + uint64_t( e ) * 2;
			if ( uint32_t( entry[0] | ( entry[1] << 8 ) ) >= layout.lightCount )
				return SdfVolumeError::InvalidCells;
		}
		if ( layout.cellEntries % 2 )
		{
			const unsigned char *pad = p + layout.cellEntryOffset + uint64_t( layout.cellEntries ) * 2;
			if ( pad[0] || pad[1] )
				return SdfVolumeError::InvalidCells;
		}
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
	case SdfVolumeError::InvalidCells:
		return "invalid-cells";
	}
	return "unknown";
}

} // namespace mapcontainer
