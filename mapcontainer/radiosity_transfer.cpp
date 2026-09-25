//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RTRN radiosity transfer validation and view; see
//          public/mapcontainer/radiosity_transfer.h.
//
//=============================================================================//

#include "mapcontainer/radiosity_transfer.h"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace mapcontainer
{
namespace
{

const float kRowSumLimit = 1.0f + 1e-3f;
const float kNormalTolerance = 1e-3f;
const double kY00 = 0.28209479177387814;
const uint64_t kFnvOffset = 0xCBF29CE484222325ull;
const uint64_t kFnvPrime = 0x100000001B3ull;

uint32_t U32( const unsigned char *p )
{
	return uint32_t( p[0] ) | ( uint32_t( p[1] ) << 8 ) | ( uint32_t( p[2] ) << 16 ) |
	       ( uint32_t( p[3] ) << 24 );
}

uint64_t U64( const unsigned char *p )
{
	return uint64_t( U32( p ) ) | ( uint64_t( U32( p + 4 ) ) << 32 );
}

uint64_t Align16( uint64_t value )
{
	return ( value + 15 ) & ~uint64_t( 15 );
}

bool Finite( float v )
{
	return std::isfinite( v );
}

uint64_t Fnv( const unsigned char *p, size_t n, uint64_t state )
{
	for ( size_t i = 0; i < n; ++i )
		state = ( state ^ p[i] ) * kFnvPrime;
	return state;
}

// Validates a rows section and returns its link range per row.
RadiosityTransferError CheckRows(
    const unsigned char *base, uint64_t offset, uint32_t rows, uint32_t links )
{
	const unsigned char *p = base + offset;
	if ( U32( p ) != 0 || U32( p + uint64_t( rows ) * 4 ) != links )
		return RadiosityTransferError::InvalidRows;
	for ( uint32_t r = 0; r < rows; ++r )
		if ( U32( p + uint64_t( r ) * 4 ) > U32( p + uint64_t( r + 1 ) * 4 ) )
			return RadiosityTransferError::InvalidRows;
	return RadiosityTransferError::Ok;
}

// Every link's patch index is in range and strictly increases within a row.
RadiosityTransferError CheckIndices( const unsigned char *base, uint64_t rowsOffset, uint32_t rows,
    uint64_t linksOffset, uint32_t linkBytes, uint32_t patches )
{
	const unsigned char *r = base + rowsOffset;
	for ( uint32_t row = 0; row < rows; ++row )
	{
		const uint32_t begin = U32( r + uint64_t( row ) * 4 );
		const uint32_t end = U32( r + uint64_t( row + 1 ) * 4 );
		uint32_t previous = 0;
		for ( uint32_t link = begin; link < end; ++link )
		{
			const uint32_t patch = U32( base + linksOffset + uint64_t( link ) * linkBytes );
			if ( patch >= patches || ( link > begin && patch <= previous ) )
				return RadiosityTransferError::IndexOutOfBounds;
			previous = patch;
		}
	}
	return RadiosityTransferError::Ok;
}

} // namespace

uint64_t ProbeVolumeTopologyHash( const void *prbv, const ProbeVolumeLayout &layout ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( prbv );
	uint64_t state = Fnv( p,
	    kProbeVolumeHeaderBytes + size_t( kProbeVolumeGridBytes ) * layout.gridCount, kFnvOffset );
	for ( uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const ProbeGridLayout &grid = layout.grids[g];
		const uint32_t row = grid.tilesPerRow * kProbeVisibilityTile;
		for ( uint32_t i = 0; i < grid.probeCount; ++i )
		{
			const uint64_t texel = layout.atlasOffset +
			                       ( uint64_t( grid.stateOrigin[1] + i / row ) * layout.atlasWidth +
			                           grid.stateOrigin[0] + i % row ) *
			                           8;
			state = Fnv( p + texel, 8, state );
		}
	}
	return state;
}

RadiosityTransferError ValidateRadiosityTransfer( const void *pData, size_t size,
    RadiosityTransferLayout *pLayout, const void *prbv,
    const ProbeVolumeLayout *prbvLayout ) noexcept
{
	const unsigned char *p = static_cast<const unsigned char *>( pData );
	if ( !p || size < kRadiosityTransferHeaderBytes )
		return RadiosityTransferError::Truncated;
	if ( U32( p ) != kLumpRadiosityTransfer )
		return RadiosityTransferError::BadMagic;
	if ( U32( p + 4 ) != kRadiosityTransferVersion ||
	     U32( p + 8 ) != kRadiosityTransferHeaderBytes || U32( p + 12 ) != 0 || U64( p + 56 ) != 0 )
		return RadiosityTransferError::UnsupportedVersion;
	RadiosityTransferLayout layout = {};
	layout.sourceCount = U32( p + 16 );
	layout.patchCount = U32( p + 20 );
	layout.probeCount = U32( p + 24 );
	layout.transferLinks = U32( p + 32 );
	layout.injectionLinks = U32( p + 36 );
	layout.gatherLinks = U32( p + 40 );
	layout.prbvHash = U64( p + 48 );
	if ( layout.sourceCount < 1 || layout.sourceCount > kRadiosityMaxSources ||
	     layout.patchCount < 1 || layout.patchCount > kRadiosityMaxPatches ||
	     layout.probeCount > kProbeVolumeMaxProbes || U32( p + 28 ) != kRadiosityProbeTexels ||
	     U32( p + 44 ) != kRadiositySHCoefficients || layout.transferLinks > kRadiosityMaxLinks ||
	     layout.injectionLinks > kRadiosityMaxLinks || layout.gatherLinks > kRadiosityMaxLinks )
		return RadiosityTransferError::InvalidCounts;
	// The canonical layout: nine sections, each 16-byte aligned, in order.
	const uint64_t sizes[9] = { uint64_t( layout.sourceCount ) * sizeof( RadiositySource ),
	    uint64_t( layout.patchCount ) * sizeof( RadiosityPatch ),
	    ( uint64_t( layout.patchCount ) + 1 ) * 4,
	    uint64_t( layout.transferLinks ) * sizeof( RadiosityTransferLink ),
	    ( uint64_t( layout.sourceCount ) + 1 ) * 4,
	    uint64_t( layout.injectionLinks ) * sizeof( RadiosityInjectionLink ),
	    ( uint64_t( layout.probeCount ) + 1 ) * 4,
	    uint64_t( layout.gatherLinks ) * sizeof( RadiosityGatherLink ),
	    uint64_t( layout.sourceCount ) * layout.probeCount * kRadiosityProbeTexels * 12 };
	uint64_t *const offsets[9] = { &layout.sources, &layout.patches, &layout.transferRows,
	    &layout.transfer, &layout.injectionRows, &layout.injection, &layout.gatherRows,
	    &layout.gather, &layout.probeDirect };
	uint64_t cursor = kRadiosityTransferHeaderBytes;
	for ( int i = 0; i < 9; ++i )
	{
		cursor = Align16( cursor );
		*offsets[i] = cursor;
		cursor += sizes[i];
	}
	if ( cursor != size || cursor > kRadiosityTransferMaxBytes )
		return RadiosityTransferError::SizeMismatch;

	for ( uint32_t s = 0; s < layout.sourceCount; ++s )
	{
		const unsigned char *r = p + layout.sources + uint64_t( s ) * sizeof( RadiositySource );
		const int32_t style = int32_t( U32( r + 4 ) );
		if ( U32( r ) > uint32_t( RadiositySourceKind::Emissive ) || style < -1 ||
		     style > kRadiosityMaxStyle || U32( r + 8 ) != 0 || U32( r + 12 ) != 0 )
			return RadiosityTransferError::InvalidSource;
		const unsigned char *name = r + 16;
		const void *nul = std::memchr( name, 0, kRadiositySourceNameBytes );
		if ( !nul || nul == name )
			return RadiosityTransferError::InvalidSource;
		const size_t length = size_t( static_cast<const unsigned char *>( nul ) - name );
		for ( size_t k = length; k < kRadiositySourceNameBytes; ++k )
			if ( name[k] != 0 )
				return RadiosityTransferError::InvalidSource;
		for ( uint32_t t = 0; t < s; ++t )
			if ( std::memcmp( name,
			         p + layout.sources + uint64_t( t ) * sizeof( RadiositySource ) + 16,
			         kRadiositySourceNameBytes ) == 0 )
				return RadiosityTransferError::InvalidSource;
	}
	for ( uint32_t i = 0; i < layout.patchCount; ++i )
	{
		RadiosityPatch patch;
		std::memcpy( &patch, p + layout.patches + uint64_t( i ) * sizeof( RadiosityPatch ),
		    sizeof( patch ) );
		bool finite = true;
		for ( float v : { patch.position[0], patch.position[1], patch.position[2], patch.normal[0],
		          patch.normal[1], patch.normal[2], patch.area, patch.albedo[0], patch.albedo[1],
		          patch.albedo[2] } )
			finite = finite && Finite( v );
		const double length = std::sqrt( double( patch.normal[0] ) * patch.normal[0] +
		                                 double( patch.normal[1] ) * patch.normal[1] +
		                                 double( patch.normal[2] ) * patch.normal[2] );
		if ( !finite || patch.reserved[0] || patch.reserved[1] || !( patch.area > 0.0f ) ||
		     std::fabs( length - 1.0 ) > kNormalTolerance )
			return RadiosityTransferError::InvalidPatch;
		for ( float albedo : patch.albedo )
			if ( albedo < 0.0f || albedo > 1.0f )
				return RadiosityTransferError::InvalidPatch;
	}
	struct Links
	{
		uint64_t rows;
		uint32_t count;
		uint32_t links;
		uint64_t first;
		uint32_t bytes;
	};
	for ( const Links &links :
	    { Links{ layout.transferRows, layout.patchCount, layout.transferLinks, layout.transfer,
	          uint32_t( sizeof( RadiosityTransferLink ) ) },
	        Links{ layout.injectionRows, layout.sourceCount, layout.injectionLinks,
	            layout.injection, uint32_t( sizeof( RadiosityInjectionLink ) ) },
	        Links{ layout.gatherRows, layout.probeCount, layout.gatherLinks, layout.gather,
	            uint32_t( sizeof( RadiosityGatherLink ) ) } } )
	{
		RadiosityTransferError error = CheckRows( p, links.rows, links.count, links.links );
		if ( error == RadiosityTransferError::Ok )
			error = CheckIndices(
			    p, links.rows, links.count, links.first, links.bytes, layout.patchCount );
		if ( error != RadiosityTransferError::Ok )
			return error;
	}
	// Weights: finite and non-negative (the gather's SH may be negative),
	// each transfer row sums to at most one, and so does each probe's
	// fraction of the sphere that sees patches.
	const RadiosityTransferView view( p, layout );
	for ( uint32_t row = 0; row < layout.patchCount; ++row )
	{
		double sum = 0.0;
		for ( const RadiosityTransferLink *l = view.TransferBegin( row );
		    l != view.TransferEnd( row ); ++l )
		{
			if ( !Finite( l->factor ) || l->factor < 0.0f )
				return RadiosityTransferError::InvalidWeight;
			sum += l->factor;
		}
		if ( sum > kRowSumLimit )
			return RadiosityTransferError::TransferNotNormalized;
	}
	for ( uint32_t s = 0; s < layout.sourceCount; ++s )
		for ( const RadiosityInjectionLink *l = view.InjectionBegin( s );
		    l != view.InjectionEnd( s ); ++l )
			for ( float light : l->light )
				if ( !Finite( light ) || light < 0.0f )
					return RadiosityTransferError::InvalidWeight;
	for ( uint32_t i = 0; i < layout.probeCount; ++i )
	{
		double fraction = 0.0;
		for ( const RadiosityGatherLink *l = view.GatherBegin( i ); l != view.GatherEnd( i ); ++l )
		{
			for ( float coefficient : l->sh )
				if ( !Finite( coefficient ) )
					return RadiosityTransferError::InvalidWeight;
			fraction += double( l->sh[0] ) * kY00;
		}
		if ( fraction > kRowSumLimit )
			return RadiosityTransferError::GatherNotNormalized;
	}
	const uint64_t directFloats =
	    uint64_t( layout.sourceCount ) * layout.probeCount * kRadiosityProbeTexels * 3;
	for ( uint64_t k = 0; k < directFloats; ++k )
	{
		float light;
		std::memcpy( &light, p + layout.probeDirect + k * 4, 4 );
		if ( !Finite( light ) || light < 0.0f )
			return RadiosityTransferError::InvalidWeight;
	}
	if ( prbv && prbvLayout )
	{
		uint32_t probes = 0;
		for ( uint32_t g = 0; g < prbvLayout->gridCount; ++g )
			probes += prbvLayout->grids[g].probeCount;
		if ( probes != layout.probeCount ||
		     ProbeVolumeTopologyHash( prbv, *prbvLayout ) != layout.prbvHash )
			return RadiosityTransferError::TopologyMismatch;
	}
	if ( pLayout )
		*pLayout = layout;
	return RadiosityTransferError::Ok;
}

const char *RadiosityTransferErrorName( RadiosityTransferError error ) noexcept
{
	switch ( error )
	{
	case RadiosityTransferError::Ok:
		return "ok";
	case RadiosityTransferError::Truncated:
		return "truncated";
	case RadiosityTransferError::BadMagic:
		return "bad-magic";
	case RadiosityTransferError::UnsupportedVersion:
		return "unsupported-version";
	case RadiosityTransferError::InvalidCounts:
		return "invalid-counts";
	case RadiosityTransferError::SizeMismatch:
		return "size-mismatch";
	case RadiosityTransferError::InvalidSource:
		return "invalid-source";
	case RadiosityTransferError::InvalidPatch:
		return "invalid-patch";
	case RadiosityTransferError::InvalidRows:
		return "invalid-rows";
	case RadiosityTransferError::IndexOutOfBounds:
		return "index-out-of-bounds";
	case RadiosityTransferError::InvalidWeight:
		return "invalid-weight";
	case RadiosityTransferError::TransferNotNormalized:
		return "transfer-not-normalized";
	case RadiosityTransferError::GatherNotNormalized:
		return "gather-not-normalized";
	case RadiosityTransferError::TopologyMismatch:
		return "topology-mismatch";
	}
	return "unknown";
}

RadiosityTransferView::RadiosityTransferView(
    const void *pData, const RadiosityTransferLayout &layout ) noexcept
    : m_bytes( static_cast<const unsigned char *>( pData ) ), m_layout( layout )
{
}

const uint32_t *RadiosityTransferView::Rows( uint64_t offset ) const noexcept
{
	return reinterpret_cast<const uint32_t *>( m_bytes + offset );
}

const RadiositySource &RadiosityTransferView::Source( uint32_t s ) const noexcept
{
	return reinterpret_cast<const RadiositySource *>( m_bytes + m_layout.sources )[s];
}

const RadiosityPatch &RadiosityTransferView::Patch( uint32_t p ) const noexcept
{
	return reinterpret_cast<const RadiosityPatch *>( m_bytes + m_layout.patches )[p];
}

const RadiosityTransferLink *RadiosityTransferView::TransferBegin( uint32_t patch ) const noexcept
{
	return reinterpret_cast<const RadiosityTransferLink *>( m_bytes + m_layout.transfer ) +
	       Rows( m_layout.transferRows )[patch];
}

const RadiosityTransferLink *RadiosityTransferView::TransferEnd( uint32_t patch ) const noexcept
{
	return reinterpret_cast<const RadiosityTransferLink *>( m_bytes + m_layout.transfer ) +
	       Rows( m_layout.transferRows )[patch + 1];
}

const RadiosityInjectionLink *RadiosityTransferView::InjectionBegin(
    uint32_t source ) const noexcept
{
	return reinterpret_cast<const RadiosityInjectionLink *>( m_bytes + m_layout.injection ) +
	       Rows( m_layout.injectionRows )[source];
}

const RadiosityInjectionLink *RadiosityTransferView::InjectionEnd( uint32_t source ) const noexcept
{
	return reinterpret_cast<const RadiosityInjectionLink *>( m_bytes + m_layout.injection ) +
	       Rows( m_layout.injectionRows )[source + 1];
}

const RadiosityGatherLink *RadiosityTransferView::GatherBegin( uint32_t probe ) const noexcept
{
	return reinterpret_cast<const RadiosityGatherLink *>( m_bytes + m_layout.gather ) +
	       Rows( m_layout.gatherRows )[probe];
}

const RadiosityGatherLink *RadiosityTransferView::GatherEnd( uint32_t probe ) const noexcept
{
	return reinterpret_cast<const RadiosityGatherLink *>( m_bytes + m_layout.gather ) +
	       Rows( m_layout.gatherRows )[probe + 1];
}

const float *RadiosityTransferView::ProbeDirect( uint32_t source, uint32_t probe ) const noexcept
{
	return reinterpret_cast<const float *>( m_bytes + m_layout.probeDirect ) +
	       ( uint64_t( source ) * m_layout.probeCount + probe ) * kRadiosityProbeTexels * 3;
}

} // namespace mapcontainer
