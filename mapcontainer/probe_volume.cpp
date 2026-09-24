//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: PRBV probe volume validation and reference sampling; see
//          public/mapcontainer/probe_volume.h.
//
//=============================================================================//

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

uint64_t U64( const unsigned char *p )
{
	return uint64_t( U32( p ) ) | ( uint64_t( U32( p + 4 ) ) << 32 );
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

bool Finite( float v )
{
	return std::isfinite( v );
}

struct Rect
{
	uint32_t x, y, w, h;
};

uint32_t TileRows( const ProbeGridLayout &grid )
{
	return ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
}

// Octahedral encoding (probe_volume.py oct_encode).
void OctEncode( const float d[3], float out[2] )
{
	const float sum = std::fabs( d[0] ) + std::fabs( d[1] ) + std::fabs( d[2] );
	float x = d[0] / sum;
	float y = d[1] / sum;
	if ( d[2] / sum < 0.0f )
	{
		const float fx = ( 1.0f - std::fabs( y ) ) * ( x >= 0.0f ? 1.0f : -1.0f );
		const float fy = ( 1.0f - std::fabs( x ) ) * ( y >= 0.0f ? 1.0f : -1.0f );
		x = fx;
		y = fy;
	}
	out[0] = x;
	out[1] = y;
}

} // namespace

float HalfToFloat( uint16_t half ) noexcept
{
	const uint32_t sign = uint32_t( half >> 15 ) << 31;
	const uint32_t exponent = ( half >> 10 ) & 0x1F;
	const uint32_t mantissa = half & 0x3FF;
	uint32_t bits;
	if ( exponent == 0 )
	{
		if ( mantissa == 0 )
			bits = sign;
		else
		{
			// Subnormal: normalize.
			int shift = 0;
			uint32_t m = mantissa;
			while ( !( m & 0x400 ) )
			{
				m <<= 1;
				++shift;
			}
			bits = sign | ( uint32_t( 127 - 15 - shift + 1 ) << 23 ) | ( ( m & 0x3FF ) << 13 );
		}
	}
	else if ( exponent == 31 )
		bits = sign | 0x7F800000u | ( mantissa << 13 );
	else
		bits = sign | ( ( exponent - 15 + 127 ) << 23 ) | ( mantissa << 13 );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

ProbeVolumeError ValidateProbeVolume(
    const void *pData, size_t size, ProbeVolumeLayout *pLayout ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( !p || size < kProbeVolumeHeaderBytes )
		return ProbeVolumeError::Truncated;
	if ( size > kProbeVolumeMaxBytes )
		return ProbeVolumeError::InvalidAtlas;
	if ( U32( p ) != kLumpProbeVolume )
		return ProbeVolumeError::BadMagic;
	if ( U32( p + 4 ) != kProbeVolumeVersion || U32( p + 8 ) != kProbeVolumeHeaderBytes ||
	     U32( p + 36 ) != 0 || U64( p + 56 ) != 0 )
		return ProbeVolumeError::UnsupportedVersion;
	ProbeVolumeLayout layout = {};
	layout.gridCount = U32( p + 12 );
	layout.layerCount = U32( p + 16 );
	if ( layout.gridCount < 1 || layout.gridCount > kProbeVolumeMaxGrids ||
	     layout.layerCount < 1 || layout.layerCount > kProbeVolumeMaxLayers ||
	     U32( p + 20 ) != kProbeIrradianceTile || U32( p + 24 ) != kProbeVisibilityTile )
		return ProbeVolumeError::InvalidCounts;
	layout.atlasWidth = U32( p + 28 );
	layout.atlasHeight = U32( p + 32 );
	layout.atlasOffset = U64( p + 40 );
	layout.atlasBytes = U64( p + 48 );
	if ( layout.atlasWidth < 1 || layout.atlasWidth > kProbeVolumeMaxAtlas ||
	     layout.atlasHeight < 1 || layout.atlasHeight > kProbeVolumeMaxAtlas ||
	     layout.atlasBytes != uint64_t( layout.atlasWidth ) * layout.atlasHeight * 8 ||
	     layout.atlasOffset % 16 != 0 ||
	     layout.atlasOffset < kProbeVolumeHeaderBytes +
	                              uint64_t( kProbeVolumeGridBytes ) * layout.gridCount ||
	     layout.atlasOffset + layout.atlasBytes != size )
		return ProbeVolumeError::InvalidAtlas;
	const unsigned char *atlas = p + layout.atlasOffset;
	const auto texel = [&]( uint32_t x, uint32_t y, int c )
	{
		return Half( atlas + ( uint64_t( y ) * layout.atlasWidth + x ) * 8 + 2 * c );
	};
	for ( uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const unsigned char *r = p + kProbeVolumeHeaderBytes + g * kProbeVolumeGridBytes;
		ProbeGridLayout &grid = layout.grids[g];
		for ( int i = 0; i < 3; ++i )
		{
			grid.origin[i] = F32( r + 4 * i );
			grid.spacing[i] = F32( r + 12 + 4 * i );
			grid.dims[i] = U32( r + 24 + 4 * i );
		}
		grid.tilesPerRow = U32( r + 36 );
		grid.maxRelocation = F32( r + 40 );
		grid.maxDistance = F32( r + 44 );
		for ( uint32_t l = 0; l < kProbeVolumeMaxLayers; ++l )
		{
			grid.irradianceOrigin[l][0] = U32( r + 48 + 8 * l );
			grid.irradianceOrigin[l][1] = U32( r + 52 + 8 * l );
		}
		grid.visibilityOrigin[0] = U32( r + 64 );
		grid.visibilityOrigin[1] = U32( r + 68 );
		grid.stateOrigin[0] = U32( r + 72 );
		grid.stateOrigin[1] = U32( r + 76 );
		for ( int i = 80; i < 96; ++i )
			if ( r[i] != 0 )
				return ProbeVolumeError::InvalidGrid;
		float minSpacing = grid.spacing[0];
		for ( int i = 0; i < 3; ++i )
		{
			if ( !Finite( grid.origin[i] ) || !Finite( grid.spacing[i] ) ||
			     !( grid.spacing[i] > 0.0f ) || grid.dims[i] < 2 ||
			     grid.dims[i] > kProbeVolumeMaxProbes )
				return ProbeVolumeError::InvalidGrid;
			if ( grid.spacing[i] < minSpacing )
				minSpacing = grid.spacing[i];
		}
		const uint64_t count = uint64_t( grid.dims[0] ) * grid.dims[1] * grid.dims[2];
		if ( count > kProbeVolumeMaxProbes || grid.tilesPerRow < 1 ||
		     grid.tilesPerRow > kProbeVolumeMaxAtlas )
			return ProbeVolumeError::InvalidGrid;
		grid.probeCount = uint32_t( count );
		if ( !( grid.maxRelocation >= 0.0f && grid.maxRelocation <= 0.5f * minSpacing ) ||
		     !( grid.maxDistance > 0.0f && grid.maxDistance < 1.0e6f ) )
			return ProbeVolumeError::InvalidGrid;
		const uint32_t rows = TileRows( grid );
		const uint32_t stateRow = grid.tilesPerRow * kProbeVisibilityTile;
		Rect rects[kProbeVolumeMaxLayers + 2];
		uint32_t rectCount = 0;
		for ( uint32_t l = 0; l < layout.layerCount; ++l )
			rects[rectCount++] = { grid.irradianceOrigin[l][0], grid.irradianceOrigin[l][1],
			    grid.tilesPerRow * kProbeIrradianceTile, rows * kProbeIrradianceTile };
		rects[rectCount++] = { grid.visibilityOrigin[0], grid.visibilityOrigin[1],
		    grid.tilesPerRow * kProbeVisibilityTile, rows * kProbeVisibilityTile };
		rects[rectCount++] = { grid.stateOrigin[0], grid.stateOrigin[1],
		    grid.probeCount < stateRow ? grid.probeCount : stateRow,
		    ( grid.probeCount + stateRow - 1 ) / stateRow };
		for ( uint32_t i = 0; i < rectCount; ++i )
			if ( uint64_t( rects[i].x ) + rects[i].w > layout.atlasWidth ||
			     uint64_t( rects[i].y ) + rects[i].h > layout.atlasHeight )
				return ProbeVolumeError::SectionOutsideAtlas;
		for ( uint32_t i = 0; i < grid.probeCount; ++i )
		{
			const uint32_t x = grid.stateOrigin[0] + i % stateRow;
			const uint32_t y = grid.stateOrigin[1] + i / stateRow;
			const float active = texel( x, y, 3 );
			if ( active != 0.0f && active != 1.0f )
				return ProbeVolumeError::InvalidState;
			if ( active == 1.0f )
				++layout.activeProbes;
			for ( int c = 0; c < 3; ++c )
			{
				const float offset = texel( x, y, c );
				if ( !Finite( offset ) ||
				     std::fabs( offset ) > grid.maxRelocation * 1.001f + 1.0e-3f )
					return ProbeVolumeError::RelocationOutOfBounds;
			}
		}
		for ( uint32_t i = 0; i < rectCount - 1; ++i )
		{
			const bool moments = i == rectCount - 2;
			for ( uint32_t y = rects[i].y; y < rects[i].y + rects[i].h; ++y )
				for ( uint32_t x = rects[i].x; x < rects[i].x + rects[i].w; ++x )
					for ( int c = 0; c < ( moments ? 2 : 3 ); ++c )
					{
						const float v = texel( x, y, c );
						if ( !Finite( v ) || v < 0.0f )
							return moments ? ProbeVolumeError::InvalidVisibility
							               : ProbeVolumeError::InvalidIrradiance;
					}
		}
	}
	if ( pLayout )
		*pLayout = layout;
	return ProbeVolumeError::Ok;
}

const char *ProbeVolumeErrorName( ProbeVolumeError error ) noexcept
{
	switch ( error )
	{
	case ProbeVolumeError::Ok:
		return "ok";
	case ProbeVolumeError::Truncated:
		return "truncated";
	case ProbeVolumeError::BadMagic:
		return "bad-magic";
	case ProbeVolumeError::UnsupportedVersion:
		return "unsupported-version";
	case ProbeVolumeError::InvalidCounts:
		return "invalid-counts";
	case ProbeVolumeError::InvalidAtlas:
		return "invalid-atlas";
	case ProbeVolumeError::InvalidGrid:
		return "invalid-grid";
	case ProbeVolumeError::SectionOutsideAtlas:
		return "section-outside-atlas";
	case ProbeVolumeError::InvalidState:
		return "invalid-state";
	case ProbeVolumeError::RelocationOutOfBounds:
		return "relocation-out-of-bounds";
	case ProbeVolumeError::InvalidIrradiance:
		return "invalid-irradiance";
	case ProbeVolumeError::InvalidVisibility:
		return "invalid-visibility";
	}
	return "unknown";
}

ProbeVolumeView::ProbeVolumeView( const void *pData, const ProbeVolumeLayout &layout ) noexcept
    : m_texels( static_cast<const unsigned char *>( pData ) + layout.atlasOffset ),
      m_layout( layout )
{
}

void ProbeVolumeView::Texel( uint32_t x, uint32_t y, float out[4] ) const noexcept
{
	const unsigned char *t = m_texels + ( uint64_t( y ) * m_layout.atlasWidth + x ) * 8;
	for ( int c = 0; c < 4; ++c )
		out[c] = Half( t + 2 * c );
}

void ProbeVolumeView::Bilinear( float x, float y, float out[4] ) const noexcept
{
	x -= 0.5f;
	y -= 0.5f;
	const float fx0 = std::floor( x );
	const float fy0 = std::floor( y );
	const float fx = x - fx0;
	const float fy = y - fy0;
	const auto clamp = []( float v, uint32_t size )
	{
		return v < 0.0f ? 0u : v > float( size - 1 ) ? size - 1 : uint32_t( v );
	};
	const uint32_t x0 = clamp( fx0, m_layout.atlasWidth );
	const uint32_t x1 = clamp( fx0 + 1.0f, m_layout.atlasWidth );
	const uint32_t y0 = clamp( fy0, m_layout.atlasHeight );
	const uint32_t y1 = clamp( fy0 + 1.0f, m_layout.atlasHeight );
	float a[4], b[4], c[4], d[4];
	Texel( x0, y0, a );
	Texel( x1, y0, b );
	Texel( x0, y1, c );
	Texel( x1, y1, d );
	for ( int i = 0; i < 4; ++i )
		out[i] = ( a[i] * ( 1 - fx ) + b[i] * fx ) * ( 1 - fy ) + ( c[i] * ( 1 - fx ) + d[i] * fx ) * fy;
}

void ProbeVolumeView::TileSample( const uint32_t origin[2], uint32_t tile, uint32_t probe,
    uint32_t tilesPerRow, const float direction[3], float out[4] ) const noexcept
{
	float oct[2];
	OctEncode( direction, oct );
	const float interior = float( tile - 2 );
	const uint32_t tileColumn = probe % tilesPerRow;
	const uint32_t tileRow = probe / tilesPerRow;
	const float tx = float( origin[0] + tileColumn * tile );
	const float ty = float( origin[1] + tileRow * tile );
	Bilinear( tx + 1.0f + ( oct[0] * 0.5f + 0.5f ) * interior,
	    ty + 1.0f + ( oct[1] * 0.5f + 0.5f ) * interior, out );
}

bool ProbeVolumeView::SampleGrid( const ProbeGridLayout &grid, const float position[3],
    const float normal[3], ProbeVolumeLayer layer, bool useVisibility, float out[3] ) const noexcept
{
	float minSpacing = grid.spacing[0];
	for ( int i = 1; i < 3; ++i )
		if ( grid.spacing[i] < minSpacing )
			minSpacing = grid.spacing[i];
	float biased[3], g[3];
	int base[3];
	float alpha[3];
	for ( int i = 0; i < 3; ++i )
	{
		biased[i] = position[i] + normal[i] * kProbeNormalBias * minSpacing;
		g[i] = ( biased[i] - grid.origin[i] ) / grid.spacing[i];
		if ( g[i] < 0.0f || g[i] > float( grid.dims[i] - 1 ) )
			return false;
		base[i] = int( std::floor( g[i] ) );
		if ( base[i] > int( grid.dims[i] ) - 2 )
			base[i] = int( grid.dims[i] ) - 2;
		alpha[i] = g[i] - float( base[i] );
	}
	const uint32_t stateRow = grid.tilesPerRow * kProbeVisibilityTile;
	float total[3] = { 0, 0, 0 };
	float weights = 0.0f;
	for ( int corner = 0; corner < 8; ++corner )
	{
		int index3[3];
		float trilinear = 1.0f;
		for ( int k = 0; k < 3; ++k )
		{
			const int bit = ( corner >> k ) & 1;
			index3[k] = base[k] + bit;
			trilinear *= bit ? alpha[k] : 1.0f - alpha[k];
		}
		const uint32_t probe =
		    uint32_t( index3[0] ) +
		    grid.dims[0] * ( uint32_t( index3[1] ) + grid.dims[1] * uint32_t( index3[2] ) );
		float state[4];
		Texel( grid.stateOrigin[0] + probe % stateRow, grid.stateOrigin[1] + probe / stateRow,
		    state );
		if ( state[3] < 0.5f )
			continue;
		float probePosition[3], toProbe[3];
		float length2 = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			probePosition[k] = grid.origin[k] + float( index3[k] ) * grid.spacing[k] + state[k];
			toProbe[k] = probePosition[k] - position[k];
			length2 += toProbe[k] * toProbe[k];
		}
		const float length = std::sqrt( length2 );
		float dotDirection = 1.0f;
		if ( length > 1.0e-9f )
			dotDirection = ( toProbe[0] * normal[0] + toProbe[1] * normal[1] + toProbe[2] * normal[2] ) /
			               length;
		const float half = ( dotDirection + 1.0f ) * 0.5f;
		float weight = half * half + 0.2f;
		if ( useVisibility )
		{
			float toPoint[3];
			float distance2 = 0.0f;
			for ( int k = 0; k < 3; ++k )
			{
				toPoint[k] = biased[k] - probePosition[k];
				distance2 += toPoint[k] * toPoint[k];
			}
			const float distance = std::sqrt( distance2 );
			if ( distance > 1.0e-9f )
			{
				const float direction[3] = { toPoint[0] / distance, toPoint[1] / distance,
				    toPoint[2] / distance };
				float moments[4];
				TileSample( grid.visibilityOrigin, kProbeVisibilityTile, probe, grid.tilesPerRow,
				    direction, moments );
				// Moments are stored as fractions of the max distance.
				const float mean = moments[0] * grid.maxDistance;
				const float meanSquared = moments[1] * grid.maxDistance * grid.maxDistance;
				if ( distance > mean )
				{
					const float variance = std::fabs( mean * mean - meanSquared );
					const float excess = distance - mean;
					const float chebyshev = variance / ( variance + excess * excess );
					weight *= chebyshev * chebyshev * chebyshev;
				}
			}
		}
		if ( weight < 1.0e-6f )
			weight = 1.0e-6f;
		if ( weight < kProbeCrushThreshold )
			weight *= weight * weight / ( kProbeCrushThreshold * kProbeCrushThreshold );
		weight *= trilinear;
		float irradiance[4];
		TileSample( grid.irradianceOrigin[uint32_t( layer )], kProbeIrradianceTile, probe,
		    grid.tilesPerRow, normal, irradiance );
		for ( int k = 0; k < 3; ++k )
			total[k] += weight * irradiance[k];
		weights += weight;
	}
	for ( int k = 0; k < 3; ++k )
		out[k] = weights > 0.0f ? total[k] / weights : 0.0f;
	return true;
}

bool ProbeVolumeView::Sample( const float position[3], const float normal[3],
    ProbeVolumeLayer layer, bool useVisibility, float outIrradiance[3] ) const noexcept
{
	if ( uint32_t( layer ) >= m_layout.layerCount )
		return false;
	float length =
	    std::sqrt( normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2] );
	if ( !( length > 0.0f ) )
		return false;
	const float n[3] = { normal[0] / length, normal[1] / length, normal[2] / length };
	for ( uint32_t g = 0; g < m_layout.gridCount; ++g )
		if ( SampleGrid( m_layout.grids[g], position, n, layer, useVisibility, outIrradiance ) )
			return true;
	return false;
}

bool ProbeVolumeView::AmbientCube( const float position[3], ProbeVolumeLayer layer,
    bool useVisibility, float outCube[6][3] ) const noexcept
{
	static const float kAxes[6][3] = {
	    { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
	for ( int face = 0; face < 6; ++face )
		if ( !Sample( position, kAxes[face], layer, useVisibility, outCube[face] ) )
			return false;
	return true;
}

} // namespace mapcontainer
