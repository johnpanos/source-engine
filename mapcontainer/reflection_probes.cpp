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

#include "foundation/float_classify.h"

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
	case ReflectionProbesError::InvalidCandidates:
		return "InvalidCandidates";
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
	// v3 extends capacity; its optional relight flag preserves the band schema.
	const uint32_t version = U32( p + 4 );
	const uint32_t flags = U32( p + 52 );
	if ( !ReflectionProbesVersionSupported( version ) ||
	     U32( p + 48 ) != kReflectionProbesPrefilterVersion ||
	     flags > kReflectionProbesFlagRelight ||
	     ( version < kReflectionProbesTiledVersion &&
	         flags != ( version == kReflectionProbesRelightVersion ? kReflectionProbesFlagRelight
	                                                               : 0 ) ) ||
	     U32( p + 60 ) != 0 )
		return ReflectionProbesError::UnsupportedVersion;
	ReflectionProbesLayout layout = {};
	layout.relight = ( flags & kReflectionProbesFlagRelight ) != 0;
	layout.count = U32( p + 8 );
	const uint32_t maximum =
	    version == kReflectionProbesWideCandidateVersion
	        ? kReflectionProbesMaxProbes
	        : ( version >= kReflectionProbesTiledVersion ? kReflectionProbesTiledMaxProbes
	                                                     : kReflectionProbesLegacyMaxProbes );
	layout.mipCount = U32( p + 12 );
	if ( layout.count < 1 || layout.count > maximum || layout.mipCount < 1 ||
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
	const uint64_t baseOffset = AtlasOffset( layout.count );
	const bool candidates = version >= kReflectionProbesCandidateVersion;
	layout.candidateWords =
	    candidates ? ( version == kReflectionProbesWideCandidateVersion ? 4 : 1 ) : 0;
	const uint32_t candidateBytes =
	    candidates ? 32 + 8 * kReflectionProbeCandidateCells * layout.candidateWords : 0;
	if ( layout.atlasOffset != baseOffset + candidateBytes ||
	     layout.atlasBytes != uint64_t( layout.atlasWidth ) * layout.atlasHeight * 8 ||
	     size != layout.atlasOffset + layout.atlasBytes )
		return ReflectionProbesError::InvalidSections;
	for ( uint64_t i =
	          kReflectionProbesHeaderBytes + uint64_t( kReflectionProbeRecordBytes ) * layout.count;
	    i < baseOffset; ++i )
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
			valid =
			    valid && foundation::IsFinite( value ) &&
			    std::fabs( value ) <= ( candidates ? 65504.0f : kReflectionProbesMaxCoordinate );
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
	if ( candidates )
	{
		layout.candidateOffset = baseOffset;
		for ( int axis = 0; axis < 3; ++axis )
		{
			layout.candidateOrigin[axis] = F32( p + baseOffset + 4 * axis );
			if ( !foundation::IsFinite( layout.candidateOrigin[axis] ) ||
			     std::fabs( layout.candidateOrigin[axis] ) > 4e6f )
				return ReflectionProbesError::InvalidCandidates;
		}
		layout.candidateStep = F32( p + baseOffset + 12 );
		int exponent = 0;
		if ( !foundation::IsFinite( layout.candidateStep ) || layout.candidateStep < 1 ||
		     layout.candidateStep > 4e6f || std::frexp( layout.candidateStep, &exponent ) != 0.5f )
			return ReflectionProbesError::InvalidCandidates;
		for ( int i = 16; i < 32; ++i )
			if ( p[baseOffset + i] )
				return ReflectionProbesError::InvalidCandidates;
		for ( uint32_t cell = 0; cell < kReflectionProbeCandidateCells; ++cell )
		{
			const unsigned char *masks = p + baseOffset + 32 + 8 * cell * layout.candidateWords;
			for ( uint32_t word = 0; word < layout.candidateWords; ++word )
			{
				const uint32_t ranks =
				    layout.count > 64 * word ? std::min( 64u, layout.count - 64 * word ) : 0;
				const uint64_t valid =
				    ranks == 64 ? ~uint64_t( 0 ) : ( uint64_t( 1 ) << ranks ) - 1;
				if ( U64( masks + 8 * word ) & ~valid )
					return ReflectionProbesError::InvalidCandidates;
			}
			const uint32_t coordinates[] = { cell % 16, ( cell / 16 ) % 16, cell / 256 };
			for ( uint32_t i = 0; i < layout.count; ++i )
			{
				const auto &probe = layout.probes[i];
				bool overlaps = true;
				double low[3], high[3], lo[3], hi[3], magnitude = 0;
				for ( int axis = 0; axis < 3; ++axis )
				{
					low[axis] = double( layout.candidateOrigin[axis] ) +
					            double( coordinates[axis] ) * layout.candidateStep;
					high[axis] = low[axis] + layout.candidateStep;
					lo[axis] = double( probe.influenceMin[axis] ) - probe.fade;
					hi[axis] = double( probe.influenceMax[axis] ) + probe.fade;
					magnitude = std::max( { magnitude, std::fabs( low[axis] ),
					    std::fabs( high[axis] ), std::fabs( lo[axis] ), std::fabs( hi[axis] ) } );
				}
				const double guard = 0.001 + magnitude * 1e-5;
				for ( int axis = 0; axis < 3; ++axis )
					overlaps =
					    overlaps && low[axis] <= hi[axis] + guard && high[axis] >= lo[axis] - guard;
				if ( ( overlaps || probe.rank < 2 || ( probe.flags & kReflectionProbeGlobal ) ) &&
				     !( U64( masks + 8 * ( probe.rank / 64 ) ) &
				         ( uint64_t( 1 ) << ( probe.rank % 64 ) ) ) )
					return ReflectionProbesError::InvalidCandidates;
			}
		}
	}
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

bool ReflectionProbesVersionSupported( uint32_t version ) noexcept
{
	return version >= kReflectionProbesVersion && version <= kReflectionProbesWideCandidateVersion;
}

uint32_t ReflectionProbeTextureColumns( const ReflectionProbesLayout &layout ) noexcept
{
	if ( layout.count <= kReflectionProbesLegacyMaxProbes )
		return 1;
	const uint32_t bands = layout.count * ( layout.relight ? 3 : 1 );
	uint32_t columns = 1;
	while ( columns * columns * 4 < bands )
		columns *= 2;
	return columns;
}

uint32_t ReflectionProbeTextureWidth( const ReflectionProbesLayout &layout ) noexcept
{
	return layout.atlasWidth * ReflectionProbeTextureColumns( layout );
}

uint32_t ReflectionProbeTextureRows( const ReflectionProbesLayout &layout ) noexcept
{
	const uint32_t columns = ReflectionProbeTextureColumns( layout );
	const uint32_t bands = layout.count * ( layout.relight ? 3 : 1 );
	const uint32_t width = ReflectionProbeTextureWidth( layout );
	return 1 + layout.count + ( ( bands + columns - 1 ) / columns ) * ( layout.width / 2 ) +
	       ( layout.candidateOffset
	               ? ( 2 * kReflectionProbeCandidateCells * layout.candidateWords + width - 1 ) /
	                     width
	               : 0 );
}

void WriteReflectionProbeTexture( const void *pData, const ReflectionProbesLayout &layout,
    ReflectionProbeMode mode, uint16_t *pOut, bool relight ) noexcept
{
	const uint32_t width = ReflectionProbeTextureWidth( layout );
	std::memset( pOut, 0, size_t( width ) * ReflectionProbeTextureRows( layout ) * 8 );
	const float header[8] = { float( layout.count ), float( layout.mipCount ),
	    float( layout.width ), kReflectionProbeTextureMarker, float( uint32_t( mode ) ),
	    relight && layout.relight ? 1.0f : 0.0f, 0.0f, 0.0f };
	for ( int k = 0; k < 8; ++k )
		pOut[k] = FloatToHalf( header[k] );
	const uint32_t columns = ReflectionProbeTextureColumns( layout );
	if ( columns > 1 )
		pOut[8] = FloatToHalf( float( columns ) );
	if ( layout.candidateOffset )
	{
		const uint32_t rows =
		    ( 2 * kReflectionProbeCandidateCells * layout.candidateWords + width - 1 ) / width;
		const uint32_t start = ReflectionProbeTextureRows( layout ) - rows;
		for ( int axis = 0; axis < 4; ++axis )
			pOut[12 + axis] = FloatToHalf( axis == 3 ? float( layout.candidateWords )
			                                         : float( kReflectionProbeCandidateDim ) );
		const auto *grid = static_cast<const unsigned char *>( pData ) + layout.candidateOffset;
		for ( int i = 0; i < 16; ++i )
			pOut[16 + i] = FloatToHalf( float( grid[i] ) );
		for ( int i = 0; i < 4; ++i )
			pOut[32 + i] = FloatToHalf( float( ( start >> ( i * 8 ) ) & 255 ) );
		for ( uint32_t i = 0; i < 8 * kReflectionProbeCandidateCells * layout.candidateWords; ++i )
			pOut[size_t( start ) * width * 4 + i] = FloatToHalf( float( grid[32 + i] ) );
	}
	for ( uint32_t index = 0; index < layout.count; ++index )
	{
		const ReflectionProbeRecord &probe = layout.probes[index];
		const float global = ( probe.flags & kReflectionProbeGlobal ) ? 1.0f : 0.0f;
		const uint32_t rowUnit = columns > 1 ? layout.width / 2 : 1;
		const float values[kReflectionProbeTableVec4][4] = {
		    { probe.capture[0], probe.capture[1], probe.capture[2], probe.fade },
		    { probe.boxMin[0], probe.boxMin[1], probe.boxMin[2], float( probe.bandRow / rowUnit ) },
		    { probe.boxMax[0], probe.boxMax[1], probe.boxMax[2], global },
		    { probe.influenceMin[0], probe.influenceMin[1], probe.influenceMin[2],
		        float( probe.relightRow / rowUnit ) },
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
	const uint32_t band = layout.width / 2;
	const unsigned char *atlas = static_cast<const unsigned char *>( pData ) + layout.atlasOffset;
	for ( uint32_t row = 0; row < layout.atlasHeight; ++row )
	{
		const uint32_t index = row / band;
		const uint32_t x = ( index % columns ) * layout.atlasWidth;
		const uint32_t y = 1 + layout.count + ( index / columns ) * band + row % band;
		std::memcpy( pOut + ( size_t( y ) * width + x ) * 4,
		    atlas + size_t( row ) * layout.atlasWidth * 8, size_t( layout.atlasWidth ) * 8 );
	}
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

float ReflectionProbeRelit( float radiance, float albedo, float now, float baked ) noexcept
{
	const bool relative = now < baked && baked > kReflectionProbeRelightFloor;
	return std::max( relative ? radiance * ( std::max( now, 0.0f ) / baked )
	                          : radiance + albedo * ( now - baked ),
	    0.0f );
}

bool ReflectionProbeOccluded( const ReflectionProbeOccluder *occluders, uint32_t count,
    const float origin[3], const float direction[3], float length, float *outT,
    float outNormal[3], uint32_t *outIndex ) noexcept
{
	bool found = false;
	float best = length;
	for ( uint32_t k = 0; k < count; ++k )
	{
		float nearAxes[3];
		float near = -INFINITY;
		float far = INFINITY;
		int axis = 0;
		for ( int a = 0; a < 3; ++a )
		{
			const float d = direction[a];
			const float safe = d >= 0.0f ? std::max( d, 1e-12f ) : std::min( d, -1e-12f );
			const float first = ( occluders[k].lo[a] - origin[a] ) / safe;
			const float second = ( occluders[k].hi[a] - origin[a] ) / safe;
			nearAxes[a] = std::min( first, second );
			far = std::min( far, std::max( first, second ) );
			// The first axis on ties, as numpy's argmax.
			if ( nearAxes[a] > near )
			{
				near = nearAxes[a];
				axis = a;
			}
		}
		const float entry = std::max( near, 0.0f );
		if ( !( far >= entry ) || !( entry < best ) )
			continue;
		found = true;
		best = entry;
		*outIndex = k;
		for ( int a = 0; a < 3; ++a )
		{
			if ( near < 0.0f )
				outNormal[a] = -direction[a];
			else
				outNormal[a] = a == axis ? ( direction[a] >= 0.0f ? -1.0f : 1.0f ) : 0.0f;
		}
	}
	*outT = best;
	return found;
}

void ReflectionProbesView::ProbeRadiance( uint32_t probe, const float position[3],
    const float reflected[3], float roughness, bool parallax,
    const ReflectionProbeRelight *relight, float out[3] ) const noexcept
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
	if ( !relight || !m_layout.relight )
		return;
	// Relight (RPRB v2): the point the capture saw along the lookup, or the
	// face of a moving occluder in front of it.
	float albedo[4], normal[3], seen[3], now[3], baked[3], t, face[3];
	uint32_t hit;
	sample( record.relightRow, 4, albedo );
	sample( record.relightRow + m_layout.width / 2, 3, normal );
	const float length = std::max( std::sqrt( Dot( normal, normal ) ), 1e-12f );
	for ( float &value : normal )
		value /= length;
	const bool hidden = ReflectionProbeOccluded( relight->occluders,
	    std::min( relight->occluderCount, kReflectionProbeMaxOccluders ), record.capture,
	    direction, albedo[3], &t, face, &hit );
	for ( int axis = 0; axis < 3; ++axis )
		seen[axis] = record.capture[axis] + direction[axis] * ( hidden ? t : albedo[3] );
	relight->evaluate( relight->context, seen, hidden ? face : normal, now, baked );
	for ( int c = 0; c < 3; ++c )
		out[c] = hidden ? std::max( relight->occluders[hit].reflectance * now[c], 0.0f )
		                : ReflectionProbeRelit( out[c], albedo[c], now[c], baked[c] );
}

void ReflectionProbesView::Radiance( const float position[3], const float normal[3],
    const float reflected[3], float roughness, ReflectionProbeMode mode, float outRadiance[3],
    const ReflectionProbeRelight *relight ) const noexcept
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
			    selection != uint32_t( ReflectionProbeMode::DirectionOnly ), relight, sample );
		for ( int c = 0; c < 3; ++c )
			outRadiance[c] += weights[i] * sample[c];
	}
}

} // namespace mapcontainer
