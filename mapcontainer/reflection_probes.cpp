//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RPRB reflection probe validation, GPU buffer and reference blend.
//          tools/quality/reflection_probe_set.py owns the encoding; the
//          checks below run in its reader's order so each malformation of
//          the shared corpus fails with the same error.
//
//=============================================================================//

#include "mapcontainer/reflection_probes.h"

#include "mapcontainer/probe_volume.h"

#include "foundation/float_classify.h"

#define BCDECDEF static inline
#define BCDEC_IMPLEMENTATION
#include "external/bcdec/bcdec.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace mapcontainer
{
namespace
{

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

uint32_t MipSize( uint32_t face, uint32_t level ) noexcept
{
	return face >> level;
}

// Bytes of one array across every mip: count probes x six faces.
uint64_t BlockBytes( uint32_t count, uint32_t face, uint32_t mips ) noexcept
{
	uint64_t bytes = 0;
	for ( uint32_t level = 0; level < mips; ++level )
	{
		const uint64_t blocks = MipSize( face, level ) / 4;
		bytes += uint64_t( count ) * 6 * blocks * blocks * 16;
	}
	return bytes;
}

uint64_t HalfBytes( uint32_t count, uint32_t face, uint32_t mips ) noexcept
{
	uint64_t bytes = 0;
	for ( uint32_t level = 0; level < mips; ++level )
		bytes += uint64_t( count ) * 6 * MipSize( face, level ) * MipSize( face, level ) * 8;
	return bytes;
}

// The Vulkan cube face selection (reflection_probe.py cube_select): the face
// of a direction and its (s, t) in [0, 1].
void CubeSelect( const float d[3], uint32_t *face, float *s, float *t ) noexcept
{
	const float ax = std::fabs( d[0] ), ay = std::fabs( d[1] ), az = std::fabs( d[2] );
	const bool xMajor = ax >= ay && ax >= az;
	const bool yMajor = !xMajor && ay >= az;
	float ma, sc, tc;
	if ( xMajor )
	{
		*face = d[0] >= 0 ? 0 : 1;
		ma = ax;
		sc = d[0] >= 0 ? -d[2] : d[2];
		tc = -d[1];
	}
	else if ( yMajor )
	{
		*face = d[1] >= 0 ? 2 : 3;
		ma = ay;
		sc = d[0];
		tc = d[1] >= 0 ? d[2] : -d[2];
	}
	else
	{
		*face = d[2] >= 0 ? 4 : 5;
		ma = az;
		sc = d[2] >= 0 ? d[0] : -d[0];
		tc = -d[1];
	}
	ma = std::max( ma, 1e-30f );
	*s = ( sc / ma + 1.0f ) * 0.5f;
	*t = ( tc / ma + 1.0f ) * 0.5f;
}

// The inverse (reflection_probe.py cube_vectors): a direction of a face at
// tangent-plane coordinates in [-1, 1].
void CubeVector( uint32_t face, float sc, float tc, float out[3] ) noexcept
{
	switch ( face )
	{
	case 0:
		out[0] = 1, out[1] = -tc, out[2] = -sc;
		break;
	case 1:
		out[0] = -1, out[1] = -tc, out[2] = sc;
		break;
	case 2:
		out[0] = sc, out[1] = 1, out[2] = tc;
		break;
	case 3:
		out[0] = sc, out[1] = -1, out[2] = -tc;
		break;
	case 4:
		out[0] = sc, out[1] = -tc, out[2] = 1;
		break;
	default:
		out[0] = -sc, out[1] = -tc, out[2] = -1;
		break;
	}
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
	const uint32_t flags = U32( p + 52 );
	if ( U32( p + 4 ) != kReflectionProbesVersion ||
	     U32( p + 48 ) != kReflectionProbesPrefilterVersion ||
	     flags > kReflectionProbesFlagRelight || U32( p + 60 ) != 0 )
		return ReflectionProbesError::UnsupportedVersion;
	ReflectionProbesLayout layout = {};
	layout.relight = ( flags & kReflectionProbesFlagRelight ) != 0;
	layout.count = U32( p + 8 );
	layout.mipCount = U32( p + 12 );
	if ( layout.count < 1 || layout.count > kReflectionProbesMaxProbes || layout.mipCount < 1 ||
	     layout.mipCount > kReflectionProbesMaxMips )
		return ReflectionProbesError::InvalidCounts;
	layout.faceSize = U32( p + 16 );
	const uint32_t face = layout.faceSize;
	if ( ( face & ( face - 1 ) ) != 0 || face < kReflectionProbesMinFace ||
	     face > kReflectionProbesMaxFace || ( face >> ( layout.mipCount - 1 ) ) < 4 ||
	     U32( p + 20 ) != ( layout.relight ? face : 0 ) || U32( p + 24 ) != 0 ||
	     U32( p + 28 ) != kReflectionProbeRecordBytes )
		return ReflectionProbesError::InvalidAtlas;
	layout.dataOffset = U64( p + 32 );
	layout.dataBytes = U64( p + 40 );
	const uint64_t baseOffset = ( uint64_t( kReflectionProbesHeaderBytes ) +
	                                kReflectionProbeRecordBytes * layout.count + 15 ) &
	                            ~uint64_t( 15 );
	layout.candidateWords = ( layout.count + 63 ) / 64;
	// The radiance cubes as BC6H blocks, then the relight cubes' RGBA16F
	// texels (albedo array, then normal array).
	layout.radianceBlockBytes = BlockBytes( layout.count, face, layout.mipCount );
	const uint64_t relightBytes =
	    layout.relight ? 2 * HalfBytes( layout.count, face, layout.mipCount ) : 0;
	if ( layout.dataOffset != baseOffset + ReflectionProbeCandidateBytes( layout.count ) ||
	     layout.dataBytes != layout.radianceBlockBytes + relightBytes ||
	     size != layout.dataOffset + layout.dataBytes )
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
			valid = valid && foundation::IsFinite( value ) &&
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
		probe.layer = U32( r + 72 );
		probe.relightLayer = U32( r + 76 );
		if ( ( probe.flags & ~kReflectionProbeGlobal ) != 0 || probe.layer != index ||
		     probe.relightLayer != ( layout.relight ? index : 0 ) )
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
		if ( U32( p + baseOffset + 16 ) != kReflectionProbeCandidateDim ||
		     U32( p + baseOffset + 20 ) != layout.candidateWords )
			return ReflectionProbesError::InvalidCandidates;
		for ( int i = 24; i < 32; ++i )
			if ( p[baseOffset + i] )
				return ReflectionProbesError::InvalidCandidates;
		const uint32_t dim = kReflectionProbeCandidateDim;
		for ( uint32_t cell = 0; cell < kReflectionProbeCandidateCells; ++cell )
		{
			const unsigned char *masks = p + baseOffset + kReflectionProbeCandidateHeaderBytes +
			                             8 * uint64_t( cell ) * layout.candidateWords;
			for ( uint32_t word = 0; word < layout.candidateWords; ++word )
			{
				const uint32_t ranks =
				    layout.count > 64 * word ? std::min( 64u, layout.count - 64 * word ) : 0;
				const uint64_t valid =
				    ranks == 64 ? ~uint64_t( 0 ) : ( uint64_t( 1 ) << ranks ) - 1;
				if ( U64( masks + 8 * word ) & ~valid )
					return ReflectionProbesError::InvalidCandidates;
			}
			const uint32_t coordinates[] = { cell % dim, ( cell / dim ) % dim, cell / ( dim * dim ) };
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
	// Every relight texel: finite; albedo (non-negative, at most 1, with a
	// distance in (0, max]) in the albedo array; normals within the limit with
	// alpha 1 in the normal array. The radiance (BC6H) decodes to finite,
	// non-negative light for any block.
	if ( layout.relight )
	{
		const unsigned char *albedo = p + layout.dataOffset + layout.radianceBlockBytes;
		const uint64_t texels = HalfBytes( layout.count, face, layout.mipCount ) / 8;
		const unsigned char *normals = albedo + texels * 8;
		for ( uint64_t i = 0; i < texels; ++i )
		{
			const unsigned char *a = albedo + i * 8;
			const unsigned char *n = normals + i * 8;
			for ( int c = 0; c < 4; ++c )
				if ( ( U16( a + 2 * c ) & 0x7c00u ) == 0x7c00u ||
				     ( U16( n + 2 * c ) & 0x7c00u ) == 0x7c00u )
					return ReflectionProbesError::InvalidTexels;
			for ( int c = 0; c < 3; ++c )
			{
				const uint16_t half = U16( a + 2 * c );
				if ( ( ( half & 0x8000u ) && ( half & 0x7fffu ) != 0 ) ||
				     HalfToFloat( half ) > 1.0f ||
				     std::fabs( HalfToFloat( U16( n + 2 * c ) ) ) > kReflectionProbesNormalLimit )
					return ReflectionProbesError::InvalidTexels;
			}
			const float distance = HalfToFloat( U16( a + 6 ) );
			if ( !( distance > 0.0f ) || distance > kReflectionProbesMaxDistance ||
			     U16( n + 6 ) != 0x3c00u )
				return ReflectionProbesError::InvalidTexels;
		}
	}
	if ( pLayout )
		*pLayout = layout;
	return ReflectionProbesError::Ok;
}

bool DecodeReflectionProbeRadiance( const void *pBlocks, size_t size, uint32_t faceSize,
    uint32_t firstMip, uint32_t mipCount, uint32_t slices, std::vector<std::byte> *pOut )
{
	uint64_t blockBytes = 0, texelBytes = 0;
	for ( uint32_t level = firstMip; level < mipCount; ++level )
	{
		const uint64_t mip = MipSize( faceSize, level );
		if ( mip < 4 || mip % 4 != 0 )
			return false;
		blockBytes += uint64_t( slices ) * ( mip / 4 ) * ( mip / 4 ) * 16;
		texelBytes += uint64_t( slices ) * mip * mip * 8;
	}
	if ( blockBytes != size )
		return false;
	try
	{
		pOut->assign( size_t( texelBytes ), std::byte{ 0 } );
	}
	catch ( ... )
	{
		return false;
	}
	const unsigned char *blocks = static_cast<const unsigned char *>( pBlocks );
	unsigned char *radiance = reinterpret_cast<unsigned char *>( pOut->data() );
	for ( uint32_t level = firstMip; level < mipCount; ++level )
	{
		const uint32_t mip = MipSize( faceSize, level );
		const uint32_t across = mip / 4;
		for ( uint32_t slice = 0; slice < slices; ++slice )
			for ( uint32_t by = 0; by < across; ++by )
				for ( uint32_t bx = 0; bx < across; ++bx )
				{
					float light[16 * 3];
					bcdec_bc6h_float( blocks, light, 4 * 3, 0 );
					blocks += 16;
					for ( uint32_t t = 0; t < 16; ++t )
					{
						const uint32_t x = bx * 4 + t % 4;
						const uint32_t y = by * 4 + t / 4;
						unsigned char *texel =
						    radiance + ( ( uint64_t( slice ) * mip + y ) * mip + x ) * 8;
						const uint16_t halves[4] = { FloatToHalf( light[t * 3 + 0] ),
						    FloatToHalf( light[t * 3 + 1] ), FloatToHalf( light[t * 3 + 2] ),
						    0x3c00u };
						std::memcpy( texel, halves, sizeof( halves ) );
					}
				}
		radiance += uint64_t( slices ) * mip * mip * 8;
	}
	return true;
}

ReflectionProbesError DecodeReflectionProbes( const void *pData, size_t size,
    std::vector<std::byte> *pOut, ReflectionProbesLayout *pLayout )
{
	ReflectionProbesLayout layout;
	const ReflectionProbesError error = ValidateReflectionProbes( pData, size, &layout );
	if ( error != ReflectionProbesError::Ok )
		return error;
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	const uint64_t halfBytes = HalfBytes( layout.count, layout.faceSize, layout.mipCount );
	const uint64_t relightBytes = layout.relight ? 2 * halfBytes : 0;
	try
	{
		pOut->assign( size_t( layout.dataOffset + halfBytes + relightBytes ), std::byte{ 0 } );
	}
	catch ( ... )
	{
		return ReflectionProbesError::Truncated;
	}
	unsigned char *out = reinterpret_cast<unsigned char *>( pOut->data() );
	std::memcpy( out, p, size_t( layout.dataOffset ) );
	const unsigned char *blocks = p + layout.dataOffset;
	unsigned char *radiance = out + layout.dataOffset;
	for ( uint32_t level = 0; level < layout.mipCount; ++level )
	{
		const uint32_t mip = MipSize( layout.faceSize, level );
		const uint32_t across = mip / 4;
		for ( uint32_t slice = 0; slice < layout.count * 6; ++slice )
			for ( uint32_t by = 0; by < across; ++by )
				for ( uint32_t bx = 0; bx < across; ++bx )
				{
					float light[16 * 3];
					bcdec_bc6h_float( blocks, light, 4 * 3, 0 );
					blocks += 16;
					for ( uint32_t t = 0; t < 16; ++t )
					{
						const uint32_t x = bx * 4 + t % 4;
						const uint32_t y = by * 4 + t / 4;
						unsigned char *texel =
						    radiance + ( ( uint64_t( slice ) * mip + y ) * mip + x ) * 8;
						const uint16_t halves[4] = { FloatToHalf( light[t * 3 + 0] ),
						    FloatToHalf( light[t * 3 + 1] ), FloatToHalf( light[t * 3 + 2] ),
						    0x3c00u };
						std::memcpy( texel, halves, sizeof( halves ) );
					}
				}
		radiance += uint64_t( layout.count ) * 6 * mip * mip * 8;
	}
	std::memcpy( radiance, blocks, size_t( relightBytes ) );
	layout.dataBytes = halfBytes + relightBytes;
	layout.radianceBlockBytes = 0;
	layout.decoded = true;
	if ( pLayout )
		*pLayout = layout;
	return ReflectionProbesError::Ok;
}

bool ReflectionProbeModeValid( uint32_t mode ) noexcept
{
	return mode <= 7 && mode != kReflectionProbeModeWeights;
}

uint64_t ReflectionProbeCandidateBytes( uint32_t count ) noexcept
{
	return kReflectionProbeCandidateHeaderBytes +
	       8 * uint64_t( kReflectionProbeCandidateCells ) * ( ( count + 63 ) / 64 );
}

uint32_t ReflectionProbeMipSize( const ReflectionProbesLayout &layout, uint32_t level ) noexcept
{
	return MipSize( layout.faceSize, level );
}

void ReflectionProbeMipRange( const ReflectionProbesLayout &layout, ReflectionProbeArray array,
    uint32_t level, uint64_t *pOffset, uint64_t *pBytes ) noexcept
{
	const bool blocks = array == ReflectionProbeArray::kRadiance;
	// The relight arrays follow the radiance, whichever form it is in.
	const uint64_t radianceBytes =
	    layout.decoded ? HalfBytes( layout.count, layout.faceSize, layout.mipCount )
	                   : BlockBytes( layout.count, layout.faceSize, layout.mipCount );
	const uint64_t arrayBytes = HalfBytes( layout.count, layout.faceSize, layout.mipCount );
	uint64_t base = 0;
	if ( array == ReflectionProbeArray::kAlbedo )
		base = radianceBytes;
	else if ( array == ReflectionProbeArray::kNormal )
		base = radianceBytes + arrayBytes;
	uint64_t offset = 0;
	uint64_t bytes = 0;
	for ( uint32_t l = 0; l <= level && l < layout.mipCount; ++l )
	{
		const uint64_t mip = MipSize( layout.faceSize, l );
		bytes = blocks ? uint64_t( layout.count ) * 6 * ( mip / 4 ) * ( mip / 4 ) * 16
		               : uint64_t( layout.count ) * 6 * mip * mip * 8;
		if ( l < level )
			offset += bytes;
	}
	*pOffset = base + offset;
	*pBytes = bytes;
}

uint32_t ReflectionProbeBaseMip( uint32_t mips, uint32_t drop ) noexcept
{
	return mips > 4 ? std::min( drop, mips - 4 ) : 0;
}

uint32_t ReflectionProbeBufferWords( const ReflectionProbesLayout &layout ) noexcept
{
	return kReflectionProbeBufferMasksWord +
	       2 * kReflectionProbeCandidateCells * layout.candidateWords;
}

void WriteReflectionProbeBuffer( const void *pData, const ReflectionProbesLayout &layout,
    ReflectionProbeMode mode, uint32_t *pOut, bool relight, uint32_t baseMip ) noexcept
{
	std::memset( pOut, 0, size_t( ReflectionProbeBufferWords( layout ) ) * 4 );
	const auto bits = []( float value )
	{
		uint32_t word;
		std::memcpy( &word, &value, sizeof( word ) );
		return word;
	};
	pOut[0] = layout.count;
	pOut[1] = layout.mipCount;
	pOut[2] = layout.faceSize;
	pOut[3] = layout.candidateWords;
	pOut[4] = uint32_t( mode );
	pOut[5] = relight && layout.relight ? 1u : 0u;
	pOut[6] = kReflectionProbeCandidateDim;
	pOut[7] = baseMip;
	for ( int axis = 0; axis < 3; ++axis )
		pOut[8 + axis] = bits( layout.candidateOrigin[axis] );
	pOut[11] = bits( layout.candidateStep );
	for ( uint32_t index = 0; index < layout.count; ++index )
	{
		const ReflectionProbeRecord &probe = layout.probes[index];
		const float global = ( probe.flags & kReflectionProbeGlobal ) ? 1.0f : 0.0f;
		const float values[5][4] = {
		    { probe.capture[0], probe.capture[1], probe.capture[2], probe.fade },
		    { probe.boxMin[0], probe.boxMin[1], probe.boxMin[2], float( probe.layer ) },
		    { probe.boxMax[0], probe.boxMax[1], probe.boxMax[2], global },
		    { probe.influenceMin[0], probe.influenceMin[1], probe.influenceMin[2],
		        float( probe.relightLayer ) },
		    { probe.influenceMax[0], probe.influenceMax[1], probe.influenceMax[2], 0.0f } };
		uint32_t *record = pOut + kReflectionProbeBufferProbesWord +
		                   size_t( probe.rank ) * kReflectionProbeBufferRecordWords;
		for ( int v = 0; v < 5; ++v )
			for ( int c = 0; c < 4; ++c )
				record[4 * v + c] = bits( values[v][c] );
	}
	// The masks are little-endian uint64, i.e. lo then hi word.
	const unsigned char *grid = static_cast<const unsigned char *>( pData ) +
	                            layout.candidateOffset + kReflectionProbeCandidateHeaderBytes;
	for ( uint32_t i = 0; i < 2 * kReflectionProbeCandidateCells * layout.candidateWords; ++i )
		pOut[kReflectionProbeBufferMasksWord + i] = U32( grid + 4 * size_t( i ) );
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

void ReflectionProbesView::SampleLevel( ReflectionProbeArray array, uint32_t layer, uint32_t level,
    const float direction[3], int channels, float *out ) const noexcept
{
	const uint32_t n = ReflectionProbeMipSize( m_layout, level );
	uint64_t offset, bytes;
	ReflectionProbeMipRange( m_layout, array, level, &offset, &bytes );
	const unsigned char *mip = m_bytes + m_layout.dataOffset + offset;
	uint32_t face;
	float s, t;
	CubeSelect( direction, &face, &s, &t );
	const float x = s * float( n ) - 0.5f;
	const float y = t * float( n ) - 0.5f;
	const int x0 = int( std::floor( x ) );
	const int y0 = int( std::floor( y ) );
	const float fx = x - float( x0 );
	const float fy = y - float( y0 );
	for ( int c = 0; c < channels; ++c )
		out[c] = 0.0f;
	for ( int tap = 0; tap < 4; ++tap )
	{
		const int dx = tap & 1, dy = tap >> 1;
		const float weight = ( dx ? fx : 1.0f - fx ) * ( dy ? fy : 1.0f - fy );
		int cx = x0 + dx, cy = y0 + dy;
		uint32_t tapFace = face;
		if ( cx < 0 || cx >= int( n ) || cy < 0 || cy >= int( n ) )
		{
			// Beyond the face's edge: the tap's direction on the face's tangent
			// plane, reselected (seamless filtering).
			++m_seamTaps;
			float vec[3], ns, nt;
			CubeVector( face, ( float( cx ) + 0.5f ) / float( n ) * 2.0f - 1.0f,
			    ( float( cy ) + 0.5f ) / float( n ) * 2.0f - 1.0f, vec );
			CubeSelect( vec, &tapFace, &ns, &nt );
			cx = std::clamp( int( ns * float( n ) ), 0, int( n ) - 1 );
			cy = std::clamp( int( nt * float( n ) ), 0, int( n ) - 1 );
		}
		const unsigned char *texel =
		    mip +
		    ( ( ( uint64_t( layer ) * 6 + tapFace ) * n + uint32_t( cy ) ) * n + uint32_t( cx ) ) *
		        8;
		for ( int c = 0; c < channels; ++c )
			out[c] += HalfToFloat( U16( texel + 2 * c ) ) * weight;
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
	const float lookupLod =
	    std::clamp( lookupRoughness, 0.0f, 1.0f ) * float( m_layout.mipCount - 1 );
	const auto sample =
	    [&]( ReflectionProbeArray array, uint32_t layer, int channels, float *result )
	{
		// The radiance array may have dropped its top mips (a texture setting).
		const float lod = array == ReflectionProbeArray::kRadianceHalf
		                      ? std::max( lookupLod, float( m_baseMip ) )
		                      : lookupLod;
		const uint32_t lower = uint32_t( std::floor( lod ) );
		const uint32_t upper = std::min( lower + 1, m_layout.mipCount - 1 );
		const float blend = lod - float( lower );
		float a[4], b[4];
		SampleLevel( array, layer, lower, direction, channels, a );
		SampleLevel( array, layer, upper, direction, channels, b );
		for ( int c = 0; c < channels; ++c )
			result[c] = a[c] * ( 1 - blend ) + b[c] * blend;
	};
	sample( ReflectionProbeArray::kRadianceHalf, record.layer, 3, out );
	if ( !relight || !m_layout.relight )
		return;
	// Relight (RPRB v2): the point the capture saw along the lookup, or the
	// face of a moving occluder in front of it.
	float albedo[4], normal[3], seen[3], now[3], baked[3], t, face[3];
	uint32_t hit;
	sample( ReflectionProbeArray::kAlbedo, record.relightLayer, 4, albedo );
	sample( ReflectionProbeArray::kNormal, record.relightLayer, 3, normal );
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
