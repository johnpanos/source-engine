//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RPRB reflection probe validation, GPU texture and reference blend.
//          tools/quality/reflection_probe_set.py owns the encoding; the
//          checks below run in its reader's order so each malformation of
//          the shared corpus fails with the same error.
//
//=============================================================================//

#include "mapcontainer/reflection_probes.h"

#include "mapcontainer/probe_volume.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace mapcontainer
{
namespace
{

const float kPi = 3.14159265358979323846f;

uint32_t U32( const unsigned char *p ) noexcept
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

uint64_t U64( const unsigned char *p ) noexcept
{
	return uint64_t( U32( p ) ) | ( uint64_t( U32( p + 4 ) ) << 32 );
}

float F32( const unsigned char *p ) noexcept
{
	const uint32_t bits = U32( p );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

uint16_t U16( const unsigned char *p ) noexcept
{
	return uint16_t( p[0] | ( p[1] << 8 ) );
}

uint64_t AtlasOffset( uint32_t count ) noexcept
{
	return ( uint64_t( kReflectionProbesHeaderBytes ) + kReflectionProbeRecordBytes * count + 15 ) &
	       ~uint64_t( 15 );
}

// x of mip `level` in a probe's band: 2 W (1 - 2^-level).
uint32_t MipLeft( uint32_t width, uint32_t level ) noexcept
{
	return 2 * width - ( ( 2 * width ) >> level );
}

bool InsideMip( uint32_t x, uint32_t yInBand, uint32_t width, uint32_t mips ) noexcept
{
	for ( uint32_t level = 0; level < mips; ++level )
	{
		const uint32_t left = MipLeft( width, level );
		const uint32_t columns = width >> level;
		if ( x >= left && x < left + columns )
			return yInBand < columns / 2;
	}
	return false;
}

float Smoothstep( float edge0, float edge1, float x ) noexcept
{
	const float t = std::clamp( ( x - edge0 ) / ( edge1 - edge0 ), 0.0f, 1.0f );
	return t * t * ( 3.0f - 2.0f * t );
}

float Dot( const float a[3], const float b[3] ) noexcept
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

} // namespace

const char *ReflectionProbesErrorName( ReflectionProbesError error ) noexcept
{
	switch ( error )
	{
	case ReflectionProbesError::Ok:
		return "Ok";
	case ReflectionProbesError::Truncated:
		return "Truncated";
	case ReflectionProbesError::BadMagic:
		return "BadMagic";
	case ReflectionProbesError::UnsupportedVersion:
		return "UnsupportedVersion";
	case ReflectionProbesError::InvalidCounts:
		return "InvalidCounts";
	case ReflectionProbesError::InvalidAtlas:
		return "InvalidAtlas";
	case ReflectionProbesError::InvalidSections:
		return "InvalidSections";
	case ReflectionProbesError::InvalidRecord:
		return "InvalidRecord";
	case ReflectionProbesError::InvalidRanks:
		return "InvalidRanks";
	case ReflectionProbesError::InvalidGlobal:
		return "InvalidGlobal";
	case ReflectionProbesError::InvalidTexels:
		return "InvalidTexels";
	}
	return "Unknown";
}

ReflectionProbesError ValidateReflectionProbes(
    const void *pData, size_t size, ReflectionProbesLayout *pLayout ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( !p || size < kReflectionProbesHeaderBytes )
		return ReflectionProbesError::Truncated;
	if ( U32( p ) != kLumpReflectionProbes )
		return ReflectionProbesError::BadMagic;
	// v1 has no flags; v2 is exactly the relight flag.
	const uint32_t version = U32( p + 4 );
	if ( ( version != kReflectionProbesVersion && version != kReflectionProbesRelightVersion ) ||
	     U32( p + 48 ) != kReflectionProbesPrefilterVersion ||
	     U32( p + 52 ) !=
	         ( version == kReflectionProbesRelightVersion ? kReflectionProbesFlagRelight : 0 ) ||
	     U32( p + 60 ) != 0 )
		return ReflectionProbesError::UnsupportedVersion;
	ReflectionProbesLayout layout = {};
	layout.relight = version == kReflectionProbesRelightVersion;
	layout.count = U32( p + 8 );
	layout.mipCount = U32( p + 12 );
	if ( layout.count < 1 || layout.count > kReflectionProbesMaxProbes || layout.mipCount < 1 ||
	     layout.mipCount > kReflectionProbesMaxMips )
		return ReflectionProbesError::InvalidCounts;
	layout.width = U32( p + 16 );
	layout.atlasWidth = U32( p + 20 );
	layout.atlasHeight = U32( p + 24 );
	const uint32_t width = layout.width;
	if ( ( width & ( width - 1 ) ) != 0 || width < kReflectionProbesMinWidth ||
	     width > kReflectionProbesMaxWidth || ( width >> ( layout.mipCount - 1 ) ) < 4 ||
	     layout.atlasWidth != 2 * width ||
	     layout.atlasHeight != layout.count * ( width / 2 ) * ( layout.relight ? 3 : 1 ) ||
	     U32( p + 28 ) != kReflectionProbeRecordBytes )
		return ReflectionProbesError::InvalidAtlas;
	layout.atlasOffset = U64( p + 32 );
	layout.atlasBytes = U64( p + 40 );
	if ( layout.atlasOffset != AtlasOffset( layout.count ) ||
	     layout.atlasBytes != uint64_t( layout.atlasWidth ) * layout.atlasHeight * 8 ||
	     size != layout.atlasOffset + layout.atlasBytes )
		return ReflectionProbesError::InvalidSections;
	for ( uint64_t i =
	          kReflectionProbesHeaderBytes + uint64_t( kReflectionProbeRecordBytes ) * layout.count;
	    i < layout.atlasOffset; ++i )
		if ( p[i] != 0 )
			return ReflectionProbesError::InvalidSections;
	for ( uint32_t index = 0; index < layout.count; ++index )
	{
		const unsigned char *r =
		    p + kReflectionProbesHeaderBytes + uint64_t( index ) * kReflectionProbeRecordBytes;
		float values[16];
		for ( int k = 0; k < 16; ++k )
			values[k] = F32( r + 4 * k );
		ReflectionProbeRecord &probe = layout.probes[index];
		std::memcpy( probe.capture, values, sizeof( probe.capture ) );
		probe.fade = values[3];
		std::memcpy( probe.boxMin, values + 4, sizeof( probe.boxMin ) );
		std::memcpy( probe.boxMax, values + 7, sizeof( probe.boxMax ) );
		std::memcpy( probe.influenceMin, values + 10, sizeof( probe.influenceMin ) );
		std::memcpy( probe.influenceMax, values + 13, sizeof( probe.influenceMax ) );
		bool valid = probe.fade > 0.0f;
		for ( float value : values )
			valid = valid && std::isfinite( value ) &&
			        std::fabs( value ) <= kReflectionProbesMaxCoordinate;
		for ( int axis = 0; axis < 3; ++axis )
			valid = valid && probe.boxMin[axis] < probe.boxMax[axis] &&
			        probe.influenceMin[axis] < probe.influenceMax[axis] &&
			        probe.capture[axis] >= probe.boxMin[axis] &&
			        probe.capture[axis] <= probe.boxMax[axis];
		if ( !valid )
			return ReflectionProbesError::InvalidRecord;
		probe.rank = U32( r + 64 );
		probe.flags = U32( r + 68 );
		probe.bandRow = U32( r + 72 );
		probe.relightRow = U32( r + 76 );
		const uint32_t relightRow = layout.relight ? ( layout.count + 2 * index ) * ( width / 2 ) : 0;
		if ( ( probe.flags & ~kReflectionProbeGlobal ) != 0 || probe.relightRow != relightRow ||
		     probe.bandRow != index * ( width / 2 ) )
			return ReflectionProbesError::InvalidRecord;
	}
	bool ranked[kReflectionProbesMaxProbes] = {};
	for ( uint32_t index = 0; index < layout.count; ++index )
	{
		const uint32_t rank = layout.probes[index].rank;
		if ( rank >= layout.count || ranked[rank] )
			return ReflectionProbesError::InvalidRanks;
		ranked[rank] = true;
	}
	uint32_t globals = 0;
	for ( uint32_t index = 0; index < layout.count; ++index )
		globals += ( layout.probes[index].flags & kReflectionProbeGlobal ) ? 1 : 0;
	layout.globalIndex = U32( p + 56 );
	if ( globals != 1 || layout.globalIndex >= layout.count ||
	     !( layout.probes[layout.globalIndex].flags & kReflectionProbeGlobal ) ||
	     layout.probes[layout.globalIndex].rank != layout.count - 1 )
		return ReflectionProbesError::InvalidGlobal;
	// Every texel: finite, and non-negative colour outside normal bands; then
	// all channels 0 outside a mip, and inside one alpha 1 (radiance and
	// normal bands), albedo at most 1 with a distance in (0, max], normals
	// within the limit (the reader checks finiteness over the whole atlas
	// before the layout, as the Python reader does). Bands are width / 2
	// rows: radiance bands first, then each probe's albedo and normal bands.
	const unsigned char *atlas = p + layout.atlasOffset;
	const uint32_t band = width / 2;
	const auto normalRow = [&]( uint32_t y )
	{ return layout.relight && y >= layout.count * band && ( y / band - layout.count ) % 2 == 1; };
	for ( uint32_t y = 0; y < layout.atlasHeight; ++y )
		for ( uint32_t x = 0; x < layout.atlasWidth; ++x )
			for ( int c = 0; c < 4; ++c )
			{
				const uint16_t half =
				    U16( atlas + ( uint64_t( y ) * layout.atlasWidth + x ) * 8 + 2 * c );
				if ( ( half & 0x7c00u ) == 0x7c00u ||
				     ( c < 3 && !normalRow( y ) && ( half & 0x8000u ) && ( half & 0x7fffu ) != 0 ) )
					return ReflectionProbesError::InvalidTexels;
			}
	for ( uint32_t y = 0; y < layout.atlasHeight; ++y )
		for ( uint32_t x = 0; x < layout.atlasWidth; ++x )
		{
			const unsigned char *texel = atlas + ( uint64_t( y ) * layout.atlasWidth + x ) * 8;
			if ( InsideMip( x, y % band, width, layout.mipCount ) )
			{
				const bool albedoRow = y >= layout.count * band && !normalRow( y );
				if ( albedoRow )
				{
					const float distance = HalfToFloat( U16( texel + 6 ) );
					for ( int c = 0; c < 3; ++c )
						if ( HalfToFloat( U16( texel + 2 * c ) ) > 1.0f )
							return ReflectionProbesError::InvalidTexels;
					if ( !( distance > 0.0f ) || distance > kReflectionProbesMaxDistance )
						return ReflectionProbesError::InvalidTexels;
				}
				else if ( U16( texel + 6 ) != 0x3c00u )
					return ReflectionProbesError::InvalidTexels;
				if ( normalRow( y ) )
					for ( int c = 0; c < 3; ++c )
						if ( std::fabs( HalfToFloat( U16( texel + 2 * c ) ) ) >
						     kReflectionProbesNormalLimit )
							return ReflectionProbesError::InvalidTexels;
			}
			else
			{
				for ( int c = 0; c < 4; ++c )
					if ( ( U16( texel + 2 * c ) & 0x7fffu ) != 0 )
						return ReflectionProbesError::InvalidTexels;
			}
		}
	if ( pLayout )
		*pLayout = layout;
	return ReflectionProbesError::Ok;
}

bool ReflectionProbeModeValid( uint32_t mode ) noexcept
{
	return mode <= 7 && mode != kReflectionProbeModeWeights;
}

uint32_t ReflectionProbeTextureRows( const ReflectionProbesLayout &layout ) noexcept
{
	return 1 + layout.count + layout.atlasHeight;
}

void WriteReflectionProbeTexture( const void *pData, const ReflectionProbesLayout &layout,
    ReflectionProbeMode mode, uint16_t *pOut, bool relight ) noexcept
{
	const uint32_t width = layout.atlasWidth;
	std::memset( pOut, 0, size_t( width ) * ReflectionProbeTextureRows( layout ) * 8 );
	const float header[8] = { float( layout.count ), float( layout.mipCount ),
	    float( layout.width ), kReflectionProbeTextureMarker, float( uint32_t( mode ) ),
	    relight && layout.relight ? 1.0f : 0.0f, 0.0f, 0.0f };
	for ( int k = 0; k < 8; ++k )
		pOut[k] = FloatToHalf( header[k] );
	for ( uint32_t index = 0; index < layout.count; ++index )
	{
		const ReflectionProbeRecord &probe = layout.probes[index];
		const float global = ( probe.flags & kReflectionProbeGlobal ) ? 1.0f : 0.0f;
		const float values[kReflectionProbeTableVec4][4] = {
		    { probe.capture[0], probe.capture[1], probe.capture[2], probe.fade },
		    { probe.boxMin[0], probe.boxMin[1], probe.boxMin[2], float( probe.bandRow ) },
		    { probe.boxMax[0], probe.boxMax[1], probe.boxMax[2], global },
		    { probe.influenceMin[0], probe.influenceMin[1], probe.influenceMin[2],
		        float( probe.relightRow ) },
		    { probe.influenceMax[0], probe.influenceMax[1], probe.influenceMax[2], 0.0f } };
		uint16_t *row = pOut + size_t( 1 + probe.rank ) * width * 4;
		for ( uint32_t v = 0; v < kReflectionProbeTableVec4; ++v )
			for ( int c = 0; c < 4; ++c )
			{
				const uint16_t hi = FloatToHalf( values[v][c] );
				row[( 2 * v ) * 4 + c] = hi;
				row[( 2 * v + 1 ) * 4 + c] = FloatToHalf( values[v][c] - HalfToFloat( hi ) );
			}
	}
	std::memcpy( pOut + size_t( 1 + layout.count ) * width * 4,
	    static_cast<const unsigned char *>( pData ) + layout.atlasOffset, layout.atlasBytes );
}

ReflectionProbesView::ReflectionProbesView(
    const void *pData, const ReflectionProbesLayout &layout ) noexcept
    : m_bytes( static_cast<const unsigned char *>( pData ) ), m_layout( layout )
{
}

void ReflectionProbesView::Weights( const float position[3], const float normal[3],
    ReflectionProbeMode mode, float outWeights[kReflectionProbesMaxProbes] ) const noexcept
{
	const uint32_t count = m_layout.count;
	for ( uint32_t i = 0; i < kReflectionProbesMaxProbes; ++i )
		outWeights[i] = 0.0f;
	const uint32_t selection = uint32_t( mode ) & kReflectionProbeModeSelection;
	if ( selection == uint32_t( ReflectionProbeMode::Off ) )
		return;
	if ( selection == uint32_t( ReflectionProbeMode::Nearest ) )
	{
		uint32_t best = 0;
		float bestDistance = INFINITY;
		for ( uint32_t i = 0; i < count; ++i )
		{
			const float *c = m_layout.probes[i].capture;
			const float d = std::sqrt( ( position[0] - c[0] ) * ( position[0] - c[0] ) +
			                           ( position[1] - c[1] ) * ( position[1] - c[1] ) +
			                           ( position[2] - c[2] ) * ( position[2] - c[2] ) );
			if ( d < bestDistance )
			{
				bestDistance = d;
				best = i;
			}
		}
		outWeights[best] = 1.0f;
		return;
	}
	float shares[kReflectionProbesMaxProbes] = {};
	float remaining = 1.0f;
	for ( uint32_t rank = 0; rank < count; ++rank )
	{
		uint32_t index = 0;
		while ( m_layout.probes[index].rank != rank )
			++index;
		const ReflectionProbeRecord &probe = m_layout.probes[index];
		float weight = 1.0f;
		if ( !( probe.flags & kReflectionProbeGlobal ) )
		{
			float outside = 0.0f;
			float toward[3];
			for ( int axis = 0; axis < 3; ++axis )
			{
				const float d = std::max( std::max( probe.influenceMin[axis] - position[axis],
				                              position[axis] - probe.influenceMax[axis] ),
				    0.0f );
				outside += d * d;
				toward[axis] = probe.capture[axis] - position[axis];
			}
			const float length = std::max( std::sqrt( Dot( toward, toward ) ), 1e-9f );
			weight = ( 1.0f - Smoothstep( 0.0f, probe.fade, std::sqrt( outside ) ) ) *
			         Smoothstep( -kReflectionProbeFacingEdge, kReflectionProbeFacingEdge,
			             Dot( normal, toward ) / length );
		}
		shares[index] = weight * remaining;
		remaining -= shares[index];
	}
	// The two largest shares (first index on ties), each less the third.
	uint32_t top[2] = { 0, 0 };
	for ( uint32_t i = 1; i < count; ++i )
		if ( shares[i] > shares[top[0]] )
			top[0] = i;
	const bool two = count > 1;
	if ( two )
	{
		top[1] = top[0] == 0 ? 1 : 0;
		for ( uint32_t i = 0; i < count; ++i )
			if ( i != top[0] && shares[i] > shares[top[1]] )
				top[1] = i;
	}
	float third = 0.0f;
	for ( uint32_t i = 0; i < count; ++i )
		if ( i != top[0] && ( !two || i != top[1] ) )
			third = std::max( third, shares[i] );
	float weights[2] = { shares[top[0]] - third, two ? shares[top[1]] - third : 0.0f };
	float total = weights[0] + weights[1];
	if ( total <= 1e-12f )
	{
		// Three equal shares: split evenly.
		weights[0] = 1.0f;
		weights[1] = two ? 1.0f : 0.0f;
		total = weights[0] + weights[1];
	}
	outWeights[top[0]] += weights[0] / total;
	if ( two )
		outWeights[top[1]] += weights[1] / total;
}

void ReflectionProbesView::SampleLevel(
    uint32_t top, uint32_t level, const float uv[2], int channels, float *out ) const noexcept
{
	const uint32_t columns = m_layout.width >> level;
	const uint32_t rows = columns / 2;
	const float extent[2] = { float( columns ), float( rows ) };
	float texel[2];
	for ( int k = 0; k < 2; ++k )
		texel[k] = std::clamp( uv[k] * extent[k], 0.5f, extent[k] - 0.5f ) - 0.5f;
	const uint32_t x0 = uint32_t( std::floor( texel[0] ) );
	const uint32_t y0 = uint32_t( std::floor( texel[1] ) );
	const float fx = texel[0] - float( x0 );
	const float fy = texel[1] - float( y0 );
	const uint32_t x1 = std::min( x0 + 1, columns - 1 );
	const uint32_t y1 = std::min( y0 + 1, rows - 1 );
	const uint32_t left = MipLeft( m_layout.width, level );
	const unsigned char *atlas = m_bytes + m_layout.atlasOffset;
	const auto fetch = [&]( uint32_t x, uint32_t y, int c )
	{
		return HalfToFloat(
		    U16( atlas + ( uint64_t( top + y ) * m_layout.atlasWidth + left + x ) * 8 + 2 * c ) );
	};
	for ( int c = 0; c < channels; ++c )
	{
		const float upper = fetch( x0, y0, c ) * ( 1 - fx ) + fetch( x1, y0, c ) * fx;
		const float lower = fetch( x0, y1, c ) * ( 1 - fx ) + fetch( x1, y1, c ) * fx;
		out[c] = upper * ( 1 - fy ) + lower * fy;
	}
}

void ReflectionProbesView::ProbeRadiance( uint32_t probe, const float position[3],
    const float reflected[3], float roughness, bool parallax,
    const ReflectionProbeDiffuseChange *change, float out[3] ) const noexcept
{
	const ReflectionProbeRecord &record = m_layout.probes[probe];
	float direction[3] = { reflected[0], reflected[1], reflected[2] };
	float lookupRoughness = roughness;
	if ( parallax )
	{
		// The far side of the proxy box along the ray (Lagarde 2012).
		float far = INFINITY;
		float near = -INFINITY;
		for ( int axis = 0; axis < 3; ++axis )
		{
			const float d = reflected[axis];
			const float safe = std::fabs( d ) < 1e-12f ? std::copysign( 1e-12f, d ) : d;
			const float first = ( record.boxMax[axis] - position[axis] ) / safe;
			const float second = ( record.boxMin[axis] - position[axis] ) / safe;
			far = std::min( far, std::max( first, second ) );
			near = std::max( near, std::min( first, second ) );
		}
		float lookup[3] = { reflected[0], reflected[1], reflected[2] };
		if ( far > 0.0f && near <= far )
		{
			float local[3];
			for ( int axis = 0; axis < 3; ++axis )
				local[axis] = position[axis] + far * reflected[axis] - record.capture[axis];
			const float captured = std::sqrt( Dot( local, local ) );
			for ( int axis = 0; axis < 3; ++axis )
				lookup[axis] = local[axis] / std::max( captured, 1e-12f );
			// Distance-based roughness (Frostbite 2014, Listing 25).
			const float ratio = captured > 0.0f ? far / std::max( captured, 1e-12f ) : 1.0f;
			const float sharpened = std::clamp( ratio * roughness, 0.0f, roughness );
			lookupRoughness = sharpened + ( roughness - sharpened ) * roughness;
		}
		// Ease toward the reflected ray as roughness grows (Listing F.1).
		float length = 0.0f;
		for ( int axis = 0; axis < 3; ++axis )
		{
			direction[axis] = lookup[axis] + ( reflected[axis] - lookup[axis] ) * roughness;
			length += direction[axis] * direction[axis];
		}
		length = std::max( std::sqrt( length ), 1e-12f );
		for ( float &value : direction )
			value /= length;
	}
	const float uv[2] = { 0.5f - std::atan2( direction[1], direction[0] ) / ( 2.0f * kPi ),
	    0.5f - std::asin( std::clamp( direction[2], -1.0f, 1.0f ) ) / kPi };
	const float lod = std::clamp( lookupRoughness, 0.0f, 1.0f ) * float( m_layout.mipCount - 1 );
	const uint32_t lower = uint32_t( std::floor( lod ) );
	const uint32_t upper = std::min( lower + 1, m_layout.mipCount - 1 );
	const float blend = lod - float( lower );
	const auto sample = [&]( uint32_t top, int channels, float *result )
	{
		float a[4], b[4];
		SampleLevel( top, lower, uv, channels, a );
		SampleLevel( top, upper, uv, channels, b );
		for ( int c = 0; c < channels; ++c )
			result[c] = a[c] * ( 1 - blend ) + b[c] * blend;
	};
	sample( record.bandRow, 3, out );
	if ( !change || !m_layout.relight )
		return;
	// Relight (RPRB v2): the point the capture saw along the lookup, lit by
	// the change since the bake.
	float albedo[4], normal[3], seen[3], delta[3];
	sample( record.relightRow, 4, albedo );
	sample( record.relightRow + m_layout.width / 2, 3, normal );
	const float length = std::max( std::sqrt( Dot( normal, normal ) ), 1e-12f );
	for ( int axis = 0; axis < 3; ++axis )
	{
		normal[axis] /= length;
		seen[axis] = record.capture[axis] + direction[axis] * albedo[3];
	}
	change->evaluate( change->context, seen, normal, delta );
	for ( int c = 0; c < 3; ++c )
		out[c] = std::max( out[c] + albedo[c] * delta[c], 0.0f );
}

void ReflectionProbesView::Radiance( const float position[3], const float normal[3],
    const float reflected[3], float roughness, ReflectionProbeMode mode, float outRadiance[3],
    const ReflectionProbeDiffuseChange *change ) const noexcept
{
	outRadiance[0] = outRadiance[1] = outRadiance[2] = 0.0f;
	const uint32_t selection = uint32_t( mode ) & kReflectionProbeModeSelection;
	if ( selection == uint32_t( ReflectionProbeMode::Off ) )
		return;
	float weights[kReflectionProbesMaxProbes];
	Weights( position, normal, mode, weights );
	for ( uint32_t i = 0; i < m_layout.count; ++i )
	{
		if ( !( weights[i] > 0.0f ) )
			continue;
		float sample[3];
		if ( uint32_t( mode ) & kReflectionProbeModeWeights )
		{
			const float *colour = kReflectionProbeWeightPalette[m_layout.probes[i].rank % 6];
			for ( int c = 0; c < 3; ++c )
				sample[c] = colour[c];
		}
		else
			ProbeRadiance( i, position, reflected, roughness,
			    selection != uint32_t( ReflectionProbeMode::DirectionOnly ), change, sample );
		for ( int c = 0; c < 3; ++c )
			outRadiance[c] += weights[i] * sample[c];
	}
}

} // namespace mapcontainer
