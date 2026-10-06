//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows atlas planning (RFC 0016 K7).
//
//=============================================================================//

#include "render/pass/shadows/atlas.h"

#include "render/device/conventions.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <set>
#include <unordered_set>
#include <utility>

namespace render::pass::shadows
{

static_assert( device::conventions::kOriginTopLeft && device::conventions::kClipYUp,
    "atlas v runs down from the top row while clip Y runs up" );

namespace
{

bool PowerOfTwo( std::uint32_t value )
{
	return value != 0 && std::has_single_bit( value );
}

// A quadtree of free square nodes, one free set per level (level 0 is the
// whole atlas). Taking the lowest (y, x) node keeps planning deterministic.
class BuddyAtlas
{
public:
	explicit BuddyAtlas( std::uint32_t size ) : m_Size( size )
	{
		m_Free.resize( std::countr_zero( size ) + 1 );
		m_Free[0].insert( { 0u, 0u } );
	}

	std::optional<ShadowTile> Take( std::uint32_t tileSize )
	{
		const std::uint32_t level = Level( tileSize );
		// The deepest non-empty level at or above the wanted one.
		std::uint32_t source = level + 1;
		for ( std::uint32_t l = level + 1; l-- > 0; )
		{
			if ( !m_Free[l].empty() )
			{
				source = l;
				break;
			}
		}
		if ( source > level )
			return std::nullopt;
		std::pair<std::uint32_t, std::uint32_t> node = *m_Free[source].begin();
		m_Free[source].erase( m_Free[source].begin() );
		for ( std::uint32_t l = source; l < level; ++l )
		{
			// Split: keep the top-left child, free the other three.
			const std::uint32_t half = m_Size >> ( l + 1 );
			m_Free[l + 1].insert( { node.first, node.second + half } );
			m_Free[l + 1].insert( { node.first + half, node.second } );
			m_Free[l + 1].insert( { node.first + half, node.second + half } );
		}
		return ShadowTile{ node.second, node.first, tileSize };
	}

private:
	std::uint32_t Level( std::uint32_t tileSize ) const
	{
		return static_cast<std::uint32_t>(
		    std::countr_zero( m_Size ) - std::countr_zero( tileSize ) );
	}

	std::uint32_t m_Size;
	// (y, x) of free nodes per level.
	std::vector<std::set<std::pair<std::uint32_t, std::uint32_t>>> m_Free;
};

} // namespace

ShadowAtlasLimits DesktopShadowAtlasLimits()
{
	ShadowAtlasLimits limits;
	limits.atlasSize = 4096;
	limits.minTileSize = 128;
	limits.maxTileSize = 2048;
	limits.casterBudget = 32;
	limits.guardTexels = 2;
	return limits;
}

ShadowAtlasLimits MobileShadowAtlasLimits()
{
	ShadowAtlasLimits limits;
	limits.atlasSize = 2048;
	limits.minTileSize = 128;
	limits.maxTileSize = 1024;
	limits.casterBudget = 8;
	limits.guardTexels = 2;
	return limits;
}

std::uint32_t DesiredTileSize( const ShadowAtlasLimits &limits, float screenCoverage )
{
	if ( !( screenCoverage > 0.0f ) )
		return limits.minTileSize;
	const double wanted = std::min( 1.0, double( screenCoverage ) ) * limits.maxTileSize;
	const std::uint32_t size = std::bit_ceil( static_cast<std::uint32_t>( std::ceil( wanted ) ) );
	return std::clamp( size, limits.minTileSize, limits.maxTileSize );
}

foundation::Expected<ShadowAtlasPlan, ShadowAtlasError> PlanShadowAtlas(
    const ShadowAtlasLimits &limits, std::span<const ShadowRequest> requests )
{
	if ( !PowerOfTwo( limits.atlasSize ) || !PowerOfTwo( limits.minTileSize ) ||
	     !PowerOfTwo( limits.maxTileSize ) || limits.minTileSize > limits.maxTileSize ||
	     limits.maxTileSize > limits.atlasSize || limits.casterBudget == 0 ||
	     2 * limits.guardTexels >= limits.minTileSize )
		return foundation::MakeUnexpected( ShadowAtlasError::kInvalidLimits );

	ShadowAtlasPlan plan;
	plan.limits = limits;
	plan.allocations.resize( requests.size() );

	std::vector<std::uint32_t> ranked;
	std::unordered_set<std::uint64_t> keys;
	for ( std::uint32_t i = 0; i < requests.size(); ++i )
	{
		const ShadowRequest &request = requests[i];
		ShadowAllocation &allocation = plan.allocations[i];
		allocation.key = request.key;
		if ( !std::isfinite( request.priority ) || !std::isfinite( request.screenCoverage ) ||
		     !keys.insert( request.key ).second )
		{
			allocation.status = ShadowTileStatus::kInvalidRequest;
			++plan.invalid;
			continue;
		}
		allocation.desiredSize = DesiredTileSize( limits, request.screenCoverage );
		ranked.push_back( i );
	}
	std::sort( ranked.begin(), ranked.end(),
	    [&]( std::uint32_t a, std::uint32_t b )
	    {
		    if ( requests[a].priority != requests[b].priority )
			    return requests[a].priority > requests[b].priority;
		    if ( plan.allocations[a].desiredSize != plan.allocations[b].desiredSize )
			    return plan.allocations[a].desiredSize > plan.allocations[b].desiredSize;
		    return requests[a].key < requests[b].key;
	    } );

	// Sizes first, by area: every request within the budget keeps a tile
	// while the atlas can hold them all at the minimum size. Short of room,
	// the lowest-ranked tile halves first; only when every tile is at the
	// minimum does the lowest-ranked request lose its tile. Then reduced
	// tiles grow back, highest rank first, while room remains. (Giving the
	// first-ranked requests their whole size made which lights kept a
	// shadow depend on how many others shared the view.)
	const std::size_t served = std::min<std::size_t>( ranked.size(), limits.casterBudget );
	for ( std::size_t rank = served; rank < ranked.size(); ++rank )
	{
		plan.allocations[ranked[rank]].status = ShadowTileStatus::kOverBudget;
		++plan.overBudget;
	}
	const std::uint64_t capacity = std::uint64_t( limits.atlasSize ) * limits.atlasSize;
	const auto area = []( std::uint32_t size )
	{
		return std::uint64_t( size ) * size;
	};
	std::vector<std::uint32_t> sizes( served );
	std::uint64_t used = 0;
	for ( std::size_t rank = 0; rank < served; ++rank )
	{
		sizes[rank] = plan.allocations[ranked[rank]].desiredSize;
		used += area( sizes[rank] );
	}
	std::size_t kept = served;
	while ( used > capacity && kept > 0 )
	{
		std::size_t shrink = kept;
		for ( std::size_t rank = kept; rank-- > 0; )
		{
			if ( sizes[rank] > limits.minTileSize )
			{
				shrink = rank;
				break;
			}
		}
		if ( shrink < kept )
		{
			used -= area( sizes[shrink] ) - area( sizes[shrink] / 2 );
			sizes[shrink] /= 2;
		}
		else
		{
			--kept;
			used -= area( sizes[kept] );
		}
	}
	for ( bool grew = true; grew; )
	{
		grew = false;
		for ( std::size_t rank = 0; rank < kept; ++rank )
		{
			const std::uint32_t desired = plan.allocations[ranked[rank]].desiredSize;
			const std::uint64_t more = area( sizes[rank] * 2 ) - area( sizes[rank] );
			if ( sizes[rank] < desired && used + more <= capacity )
			{
				used += more;
				sizes[rank] *= 2;
				grew = true;
			}
		}
	}
	// Then the tiles, largest first (then by rank): power-of-two squares
	// whose areas sum to at most the atlas's always pack into its quadtree.
	std::vector<std::size_t> packing( kept );
	for ( std::size_t rank = 0; rank < kept; ++rank )
		packing[rank] = rank;
	std::stable_sort( packing.begin(), packing.end(),
	    [&]( std::size_t a, std::size_t b )
	    {
		    return sizes[a] > sizes[b];
	    } );
	BuddyAtlas atlas( limits.atlasSize );
	for ( const std::size_t rank : packing )
	{
		ShadowAllocation &allocation = plan.allocations[ranked[rank]];
		allocation.status = ShadowTileStatus::kAtlasFull;
		if ( const std::optional<ShadowTile> tile = atlas.Take( sizes[rank] ) )
		{
			allocation.tile = *tile;
			allocation.status = sizes[rank] == allocation.desiredSize ? ShadowTileStatus::kAllocated
			                                                          : ShadowTileStatus::kReduced;
		}
	}
	for ( std::size_t rank = kept; rank < served; ++rank )
		plan.allocations[ranked[rank]].status = ShadowTileStatus::kAtlasFull;
	for ( std::size_t rank = 0; rank < served; ++rank )
	{
		switch ( plan.allocations[ranked[rank]].status )
		{
		case ShadowTileStatus::kAllocated:
			++plan.allocated;
			break;
		case ShadowTileStatus::kReduced:
			++plan.reduced;
			break;
		default:
			++plan.atlasFull;
			break;
		}
	}
	return plan;
}

ShadowViewport TileViewport( const ShadowTile &tile, std::uint32_t guardTexels )
{
	return { tile.x + guardTexels, tile.y + guardTexels, tile.size - 2 * guardTexels };
}

ShadowTileTransform TileTransform(
    const ShadowTile &tile, std::uint32_t atlasSize, std::uint32_t guardTexels )
{
	const ShadowViewport viewport = TileViewport( tile, guardTexels );
	const float scale = float( viewport.size ) / float( atlasSize );
	const float u0 = float( viewport.x ) / float( atlasSize );
	const float v0 = float( viewport.y ) / float( atlasSize );
	// u = ( x * 0.5 + 0.5 ) * scale + u0; v = ( 0.5 - y * 0.5 ) * scale + v0.
	return { 0.5f * scale, 0.5f * scale + u0, -0.5f * scale, 0.5f * scale + v0 };
}

ShadowTileProjection MakeTileProjection( const math::float4x4 &viewProjection,
    const ShadowTile &tile, std::uint32_t atlasSize, std::uint32_t guardTexels )
{
	const ShadowViewport viewport = TileViewport( tile, guardTexels );
	ShadowTileProjection projection;
	projection.viewProjection = viewProjection;
	projection.transform = TileTransform( tile, atlasSize, guardTexels );
	projection.u0 = float( viewport.x ) / float( atlasSize );
	projection.v0 = float( viewport.y ) / float( atlasSize );
	projection.u1 = float( viewport.x + viewport.size ) / float( atlasSize );
	projection.v1 = float( viewport.y + viewport.size ) / float( atlasSize );
	return projection;
}

std::optional<math::float3> ProjectToShadowTile(
    const ShadowTileProjection &projection, const math::float3 &world )
{
	const math::float4 h =
	    math::Transform( projection.viewProjection, { world.x, world.y, world.z, 1.0f } );
	if ( !( h.w > 0.0f ) )
		return std::nullopt;
	const float depth = h.z / h.w;
	if ( !( depth >= 0.0f && depth <= 1.0f ) )
		return std::nullopt;
	const ShadowTileTransform &t = projection.transform;
	const math::float3 p = {
	    h.x / h.w * t.scaleU + t.biasU, h.y / h.w * t.scaleV + t.biasV, depth };
	if ( !( p.x >= projection.u0 && p.x <= projection.u1 && p.y >= projection.v0 &&
	         p.y <= projection.v1 ) )
		return std::nullopt;
	return p;
}

} // namespace render::pass::shadows
