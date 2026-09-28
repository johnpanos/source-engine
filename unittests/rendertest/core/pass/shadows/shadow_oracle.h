//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The oracles of render.shadows.v1 (RFC 0016 K7), shared by
//			render.shadows.atlas and its sensitivity suite. Each takes the
//			module's functions as providers, so the sensitivity suite can
//			hand it deliberately bad ones.
//
//			The atlas oracle re-derives the ranking and the budget from the
//			requests, checks every tile's size, alignment and bounds, checks
//			tiles pairwise for overlap, and proves a reported shortage: a
//			request left without room, or given less than it asked for, while
//			a free aligned block of the size it wanted remains (every block
//			free at the end was free when it was served), or while a
//			lower-ranked request got a block that large, is a violation.
//
//			The view oracles decide membership geometrically (a cone's angle
//			and range, a flashlight's two fields of view, a camera frustum
//			built from its own basis) and evaluate the matrices in double.
//
//=============================================================================//

#ifndef RENDER_SHADOWS_SHADOW_ORACLE_H
#define RENDER_SHADOWS_SHADOW_ORACLE_H

#include "render/pass/shadows/atlas.h"
#include "render/pass/shadows/shadow_views.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace shadow_oracle
{

namespace shadows = render::pass::shadows;
using render::math::float3;
using render::math::float4x4;

constexpr double kPi = 3.14159265358979323846;

struct D3
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

inline D3 operator+( const D3 &a, const D3 &b )
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}
inline D3 operator-( const D3 &a, const D3 &b )
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}
inline D3 operator*( const D3 &a, double s )
{
	return { a.x * s, a.y * s, a.z * s };
}
inline double Dot( const D3 &a, const D3 &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline D3 Cross( const D3 &a, const D3 &b )
{
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
inline double Len( const D3 &a )
{
	return std::sqrt( Dot( a, a ) );
}
inline D3 Unit( const D3 &a )
{
	const double l = Len( a );
	return l > 0.0 ? a * ( 1.0 / l ) : a;
}
inline float3 F( const D3 &a )
{
	return { float( a.x ), float( a.y ), float( a.z ) };
}
inline D3 Dd( const float3 &a )
{
	return { a.x, a.y, a.z };
}

// m * ( p, 1 ) in double from float entries.
inline void Apply( const float4x4 &m, const D3 &p, double out[4] )
{
	for ( int r = 0; r < 4; ++r )
	{
		const float *row = &m.rows[r].x;
		out[r] = double( row[0] ) * p.x + double( row[1] ) * p.y + double( row[2] ) * p.z +
		         double( row[3] );
	}
}

inline void Perpendiculars( const D3 &a, D3 &e1, D3 &e2 )
{
	const D3 helper = std::fabs( a.x ) < 0.6 ? D3{ 1, 0, 0 } : D3{ 0, 1, 0 };
	e1 = Unit( Cross( a, helper ) );
	e2 = Cross( a, e1 );
}

inline D3 RandomUnit( std::mt19937 &random )
{
	std::normal_distribution<double> normal( 0.0, 1.0 );
	for ( ;; )
	{
		const D3 v = { normal( random ), normal( random ), normal( random ) };
		if ( Len( v ) > 1e-3 )
			return Unit( v );
	}
}

struct Findings
{
	std::uint64_t cases = 0;
	std::uint64_t violations = 0;
	std::uint64_t undecided = 0; // below float resolution; left out
	std::string first;

	void Violation( const char *what, std::uint64_t id )
	{
		++violations;
		if ( first.empty() )
		{
			char text[160];
			std::snprintf( text, sizeof( text ), "%s (case %llu)", what,
			    static_cast<unsigned long long>( id ) );
			first = text;
		}
	}
};

// ---------------------------------------------------------------------------
// Atlas

using Planner =
    std::function<foundation::Expected<shadows::ShadowAtlasPlan, shadows::ShadowAtlasError>(
        const shadows::ShadowAtlasLimits &, std::span<const shadows::ShadowRequest> )>;

inline Planner RealPlanner()
{
	return []( const shadows::ShadowAtlasLimits &limits,
	           std::span<const shadows::ShadowRequest> requests )
	{
		return shadows::PlanShadowAtlas( limits, requests );
	};
}

struct AtlasCase
{
	shadows::ShadowAtlasLimits limits;
	std::vector<shadows::ShadowRequest> requests;
};

inline AtlasCase MakeAtlasCase( std::mt19937 &random )
{
	AtlasCase c;
	c.limits.atlasSize = 1024u << ( random() % 4 );
	c.limits.minTileSize = 32u << ( random() % 4 );
	c.limits.maxTileSize = std::max( c.limits.minTileSize, c.limits.atlasSize >> ( random() % 4 ) );
	c.limits.casterBudget = 1 + random() % 64;
	c.limits.guardTexels = random() % 5;
	const std::uint32_t count = random() % 90;
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	for ( std::uint32_t i = 0; i < count; ++i )
	{
		shadows::ShadowRequest r;
		r.key = 1000 + i * 7;
		// Quantized priorities give ties.
		r.priority = float( random() % 12 ) * 0.5f;
		r.screenCoverage = unit( random ) < 0.1f ? 1.5f : unit( random );
		const std::uint32_t oddity = random() % 50;
		if ( oddity == 0 )
			r.priority = std::nanf( "" );
		else if ( oddity == 1 && i > 0 )
			r.key = c.requests[random() % i].key; // a repeated key
		c.requests.push_back( r );
	}
	return c;
}

// The desired size, from the contract's formula.
inline std::uint32_t Desired( const shadows::ShadowAtlasLimits &limits, float coverage )
{
	if ( !( coverage > 0.0f ) )
		return limits.minTileSize;
	const double wanted = std::min( 1.0, double( coverage ) ) * limits.maxTileSize;
	std::uint32_t size = 1;
	while ( size < wanted )
		size *= 2;
	return std::clamp( size, limits.minTileSize, limits.maxTileSize );
}

inline void CheckAtlasPlan(
    const AtlasCase &c, const shadows::ShadowAtlasPlan &plan, std::uint64_t id, Findings &findings )
{
	using Status = shadows::ShadowTileStatus;
	const auto &limits = c.limits;
	const std::size_t n = c.requests.size();
	if ( plan.allocations.size() != n )
	{
		findings.Violation( "allocation count", id );
		return;
	}
	// Validity and ranking, re-derived.
	std::vector<bool> valid( n, false );
	std::set<std::uint64_t> seen;
	std::vector<std::size_t> ranked;
	for ( std::size_t i = 0; i < n; ++i )
	{
		const auto &r = c.requests[i];
		valid[i] = std::isfinite( r.priority ) && std::isfinite( r.screenCoverage ) &&
		           seen.insert( r.key ).second;
		if ( valid[i] )
			ranked.push_back( i );
	}
	std::stable_sort( ranked.begin(), ranked.end(),
	    [&]( std::size_t a, std::size_t b )
	    {
		    const auto &ra = c.requests[a];
		    const auto &rb = c.requests[b];
		    if ( ra.priority != rb.priority )
			    return ra.priority > rb.priority;
		    const std::uint32_t da = Desired( limits, ra.screenCoverage );
		    const std::uint32_t db = Desired( limits, rb.screenCoverage );
		    if ( da != db )
			    return da > db;
		    return ra.key < rb.key;
	    } );
	std::vector<std::size_t> rankOf( n, n );
	for ( std::size_t k = 0; k < ranked.size(); ++k )
		rankOf[ranked[k]] = k;

	std::uint32_t counts[5] = {};
	std::vector<std::size_t> withTiles;
	for ( std::size_t i = 0; i < n; ++i )
	{
		const auto &a = plan.allocations[i];
		counts[static_cast<int>( a.status )]++;
		if ( a.key != c.requests[i].key )
			findings.Violation( "allocation key", id );
		if ( !valid[i] )
		{
			if ( a.status != Status::kInvalidRequest )
				findings.Violation( "invalid request not reported", id );
			continue;
		}
		const std::uint32_t desired = Desired( limits, c.requests[i].screenCoverage );
		if ( a.desiredSize != desired )
			findings.Violation( "desired size", id );
		const bool beyond = rankOf[i] >= limits.casterBudget;
		if ( beyond != ( a.status == Status::kOverBudget ) )
			findings.Violation( "over-budget status", id );
		if ( a.status == Status::kInvalidRequest )
			findings.Violation( "valid request reported invalid", id );
		if ( !a.HasTile() )
			continue;
		withTiles.push_back( i );
		const auto &t = a.tile;
		if ( !std::has_single_bit( t.size ) || t.size < limits.minTileSize || t.size > desired ||
		     ( a.status == Status::kAllocated ) != ( t.size == desired ) )
			findings.Violation( "tile size", id );
		if ( t.size == 0 || t.x % t.size != 0 || t.y % t.size != 0 )
			findings.Violation( "tile alignment", id );
		if ( std::uint64_t( t.x ) + t.size > limits.atlasSize ||
		     std::uint64_t( t.y ) + t.size > limits.atlasSize )
			findings.Violation( "tile out of bounds", id );
	}
	if ( withTiles.size() > limits.casterBudget )
		findings.Violation( "caster budget exceeded", id );
	if ( plan.allocated != counts[0] || plan.reduced != counts[1] || plan.overBudget != counts[2] ||
	     plan.atlasFull != counts[3] || plan.invalid != counts[4] )
		findings.Violation( "plan counters", id );

	for ( std::size_t a = 0; a < withTiles.size(); ++a )
	{
		const auto &ta = plan.allocations[withTiles[a]].tile;
		for ( std::size_t b = a + 1; b < withTiles.size(); ++b )
		{
			const auto &tb = plan.allocations[withTiles[b]].tile;
			const bool apart = ta.x + ta.size <= tb.x || tb.x + tb.size <= ta.x ||
			                   ta.y + ta.size <= tb.y || tb.y + tb.size <= ta.y;
			if ( !apart )
				findings.Violation( "tiles overlap", id );
		}
	}

	// Shortages: occupancy at the minimum tile's granularity.
	const std::uint32_t cells = limits.atlasSize / limits.minTileSize;
	std::vector<std::uint8_t> used( std::size_t( cells ) * cells, 0 );
	for ( const std::size_t i : withTiles )
	{
		const auto &t = plan.allocations[i].tile;
		if ( std::uint64_t( t.x ) + t.size > limits.atlasSize ||
		     std::uint64_t( t.y ) + t.size > limits.atlasSize )
			continue;
		for ( std::uint32_t y = t.y / limits.minTileSize; y < ( t.y + t.size ) / limits.minTileSize;
		    ++y )
			for ( std::uint32_t x = t.x / limits.minTileSize;
			    x < ( t.x + t.size ) / limits.minTileSize; ++x )
				used[std::size_t( y ) * cells + x] = 1;
	}
	const auto freeBlock = [&]( std::uint32_t size )
	{
		const std::uint32_t span = size / limits.minTileSize;
		for ( std::uint32_t by = 0; by < cells; by += span )
		{
			for ( std::uint32_t bx = 0; bx < cells; bx += span )
			{
				bool empty = true;
				for ( std::uint32_t y = by; y < by + span && empty; ++y )
					for ( std::uint32_t x = bx; x < bx + span && empty; ++x )
						empty = used[std::size_t( y ) * cells + x] == 0;
				if ( empty )
					return true;
			}
		}
		return false;
	};
	for ( std::size_t i = 0; i < n; ++i )
	{
		const auto &a = plan.allocations[i];
		if ( a.status != Status::kReduced && a.status != Status::kAtlasFull )
			continue;
		if ( freeBlock( a.desiredSize ) ||
		     ( a.status == Status::kAtlasFull && freeBlock( limits.minTileSize ) ) )
			findings.Violation( "shortage reported while room remained", id );
		for ( const std::size_t j : withTiles )
		{
			if ( rankOf[j] > rankOf[i] && plan.allocations[j].tile.size >= a.desiredSize )
				findings.Violation( "a lower-ranked request took the room", id );
		}
	}
	++findings.cases;
}

// ---------------------------------------------------------------------------
// Spot and flashlight views

using SpotBuilder =
    std::function<foundation::Expected<shadows::ShadowView, shadows::ShadowViewError>(
        const shadows::SpotShadowDesc & )>;
using FlashlightBuilder =
    std::function<foundation::Expected<shadows::ShadowView, shadows::ShadowViewError>(
        const shadows::FlashlightShadowDesc & )>;
using Projector =
    std::function<std::optional<float3>( const shadows::ShadowTileProjection &, const float3 & )>;

inline Projector RealProjector()
{
	return []( const shadows::ShadowTileProjection &p, const float3 &w )
	{
		return shadows::ProjectToShadowTile( p, w );
	};
}

struct ViewTileCase
{
	shadows::ShadowTile tile;
	std::uint32_t atlasSize = 4096;
	std::uint32_t guard = 2;
};

inline ViewTileCase MakeTile( std::mt19937 &random )
{
	ViewTileCase t;
	t.atlasSize = 1024u << ( random() % 3 );
	t.tile.size = std::min( t.atlasSize, 64u << ( random() % 5 ) );
	const std::uint32_t slots = t.atlasSize / t.tile.size;
	t.tile.x = ( random() % slots ) * t.tile.size;
	t.tile.y = ( random() % slots ) * t.tile.size;
	t.guard = random() % 4;
	return t;
}

// Checks one spot light: points in its cone and range land in the tile's
// viewport with depth 0 to 1, deeper points deeper; points behind it,
// beyond its range, or outside the pyramid around its cone do not; the axis
// lands on the viewport's center.
inline void CheckSpot( const SpotBuilder &build, const Projector &project, std::mt19937 &random,
    std::uint64_t id, Findings &findings )
{
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	shadows::SpotShadowDesc desc;
	const D3 position = { ( unit( random ) - 0.5 ) * 8000, ( unit( random ) - 0.5 ) * 8000,
	    ( unit( random ) - 0.5 ) * 8000 };
	D3 axis = RandomUnit( random );
	if ( unit( random ) < 0.1 )
		axis = { 0.0, 0.0, unit( random ) < 0.5 ? 1.0 : -1.0 }; // vertical spots
	const double theta = ( 1.0 + unit( random ) * 83.0 ) * kPi / 180.0;
	desc.position = F( position );
	desc.direction = F( axis );
	desc.outerCos = float( std::cos( theta ) );
	desc.nearZ = float( 0.5 + unit( random ) * 8.0 );
	desc.range = float( desc.nearZ * ( 4.0 + unit( random ) * 400.0 ) );
	auto view = build( desc );
	if ( !view )
	{
		findings.Violation( "spot view refused", id );
		return;
	}
	const ViewTileCase t = MakeTile( random );
	const auto projection =
	    shadows::MakeTileProjection( view.Value().viewProjection, t.tile, t.atlasSize, t.guard );
	// The viewport: the tile less its guard band on every side.
	const double u0 = double( t.tile.x + t.guard ) / t.atlasSize;
	const double v0 = double( t.tile.y + t.guard ) / t.atlasSize;
	const double u1 = double( t.tile.x + t.tile.size - t.guard ) / t.atlasSize;
	const double v1 = double( t.tile.y + t.tile.size - t.guard ) / t.atlasSize;
	const double texel = 1.0 / t.atlasSize;
	D3 e1;
	D3 e2;
	Perpendiculars( axis, e1, e2 );
	const auto pointAt = [&]( double angle, double azimuth, double distance )
	{
		const D3 dir = axis * std::cos( angle ) +
		               ( e1 * std::cos( azimuth ) + e2 * std::sin( azimuth ) ) * std::sin( angle );
		return position + dir * distance;
	};
	const double nearZ = desc.nearZ;
	const double range = desc.range;
	for ( int s = 0; s < 24; ++s )
	{
		const double angle = theta * 0.95 * unit( random );
		const double azimuth = 2.0 * kPi * unit( random );
		// Depth along the axis inside ( near, range ).
		const double along = nearZ * 1.01 + ( range * 0.98 - nearZ * 1.01 ) * unit( random );
		const double distance = along / std::cos( angle );
		const auto p = project( projection, F( pointAt( angle, azimuth, distance ) ) );
		if ( !p || p->x < u0 - texel || p->x > u1 + texel || p->y < v0 - texel ||
		     p->y > v1 + texel || p->z < 0.0f || p->z > 1.0f )
		{
			findings.Violation( "caster in the cone not in the tile", id );
			continue;
		}
		const double deeper = std::min( range * 0.99, along * 1.05 ) / std::cos( angle );
		const auto q = project( projection, F( pointAt( angle, azimuth, deeper ) ) );
		if ( deeper > distance * 1.001 && ( !q || !( q->z > p->z ) ) )
			findings.Violation( "depth not increasing with distance", id );
	}
	for ( int s = 0; s < 8; ++s )
	{
		const double azimuth = 2.0 * kPi * unit( random );
		// Behind the light: any point with a negative axial component.
		const double backAngle = kPi * ( 0.55 + 0.45 * unit( random ) );
		if ( project(
		         projection, F( pointAt( backAngle, azimuth, nearZ + range * unit( random ) ) ) ) )
			findings.Violation( "a point behind the light projects", id );
		// Beyond range on the axis.
		if ( project(
		         projection, F( pointAt( 0.0, azimuth, range * ( 1.01 + unit( random ) ) ) ) ) )
			findings.Violation( "a point beyond range projects", id );
		// Outside the pyramid: beyond the pyramid's corner angle.
		const double corner = std::atan( std::sqrt( 2.0 ) * std::tan( theta ) );
		const double outside = corner + ( 0.5 * kPi - corner ) * ( 0.05 + 0.9 * unit( random ) );
		if ( project( projection, F( pointAt( outside, azimuth, range * 0.5 ) ) ) )
			findings.Violation( "a point outside the cone's pyramid projects", id );
	}
	// Float world coordinates leave the projection a few texels of play on
	// narrow cones; a misplaced tile or guard band is off by far more.
	const double play = std::max( 2.0 * texel, 0.005 * ( u1 - u0 ) );
	const auto center = project( projection, F( pointAt( 0.0, 0.0, 0.5 * ( nearZ + range ) ) ) );
	if ( !center || std::fabs( center->x - 0.5 * ( u0 + u1 ) ) > play ||
	     std::fabs( center->y - 0.5 * ( v0 + v1 ) ) > play )
		findings.Violation( "the axis misses the tile's center", id );
	++findings.cases;
}

// Checks one flashlight: points inside its horizontal and vertical fields of
// view land in the tile; its up direction goes up the tile (toward row 0)
// and its right (forward x up) to the right; points outside either field of
// view, behind it or beyond far do not project.
inline void CheckFlashlight( const FlashlightBuilder &build, const Projector &project,
    std::mt19937 &random, std::uint64_t id, Findings &findings )
{
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	const D3 position = { ( unit( random ) - 0.5 ) * 8000, ( unit( random ) - 0.5 ) * 8000,
	    ( unit( random ) - 0.5 ) * 8000 };
	const D3 forward = RandomUnit( random );
	D3 e1;
	D3 e2;
	Perpendiculars( forward, e1, e2 );
	const double roll = 2.0 * kPi * unit( random );
	const D3 up = e1 * std::cos( roll ) + e2 * std::sin( roll );
	const D3 right = Cross( forward, up );
	const double h = ( 10.0 + unit( random ) * 140.0 ) * kPi / 180.0;
	const double v = ( 10.0 + unit( random ) * 140.0 ) * kPi / 180.0;
	shadows::FlashlightShadowDesc desc;
	desc.position = F( position );
	desc.forward = F( forward );
	// A tilted up hint must give the same frustum: only its part across
	// forward counts.
	desc.up = F( up + forward * ( unit( random ) - 0.5 ) );
	desc.horizontalFovRadians = float( h );
	desc.verticalFovRadians = float( v );
	desc.nearZ = float( 1.0 + unit( random ) * 8.0 );
	desc.farZ = float( desc.nearZ * ( 8.0 + unit( random ) * 200.0 ) );
	auto view = build( desc );
	if ( !view )
	{
		findings.Violation( "flashlight view refused", id );
		return;
	}
	const ViewTileCase t = MakeTile( random );
	const auto projection =
	    shadows::MakeTileProjection( view.Value().viewProjection, t.tile, t.atlasSize, t.guard );
	const double tanH = std::tan( h * 0.5 );
	const double tanV = std::tan( v * 0.5 );
	const auto pointAt = [&]( double sx, double sy, double depth )
	{
		return position + forward * depth + right * ( sx * tanH * depth ) +
		       up * ( sy * tanV * depth );
	};
	const double nearZ = desc.nearZ;
	const double farZ = desc.farZ;
	for ( int s = 0; s < 24; ++s )
	{
		const double depth = nearZ * 1.01 + ( farZ * 0.99 - nearZ * 1.01 ) * unit( random );
		const double sx = ( unit( random ) * 2.0 - 1.0 ) * 0.99;
		const double sy = ( unit( random ) * 2.0 - 1.0 ) * 0.99;
		if ( !project( projection, F( pointAt( sx, sy, depth ) ) ) )
			findings.Violation( "a point inside the flashlight's view does not project", id );
		const double outX = ( 1.02 + unit( random ) ) * ( unit( random ) < 0.5 ? -1.0 : 1.0 );
		const double outY = ( 1.02 + unit( random ) ) * ( unit( random ) < 0.5 ? -1.0 : 1.0 );
		if ( project( projection, F( pointAt( outX, sy, depth ) ) ) ||
		     project( projection, F( pointAt( sx, outY, depth ) ) ) )
			findings.Violation( "a point outside a field of view projects", id );
		if ( project( projection, F( pointAt( sx, sy, farZ * ( 1.01 + unit( random ) ) ) ) ) ||
		     project( projection, F( position - forward * depth + right * ( sx * depth ) ) ) )
			findings.Violation( "a point beyond far or behind projects", id );
	}
	const double depth = 0.5 * ( nearZ + farZ );
	const auto center = project( projection, F( pointAt( 0.0, 0.0, depth ) ) );
	const auto above = project( projection, F( pointAt( 0.0, 0.5, depth ) ) );
	const auto toRight = project( projection, F( pointAt( 0.5, 0.0, depth ) ) );
	if ( !center || !above || !toRight || !( above->y < center->y ) ||
	     std::fabs( above->x - center->x ) > 1e-3 )
		findings.Violation( "the flashlight's up is not up the tile", id );
	if ( !center || !toRight || !( toRight->x > center->x ) ||
	     std::fabs( toRight->y - center->y ) > 1e-3 )
		findings.Violation( "the flashlight's right is not right in the tile", id );
	++findings.cases;
}

// ---------------------------------------------------------------------------
// Cascades

using CascadeBuilder =
    std::function<foundation::Expected<shadows::CascadeSet, shadows::ShadowViewError>(
        const shadows::CascadeDesc & )>;

struct CameraCase
{
	D3 eye;
	D3 right;
	D3 up;
	D3 back;
	shadows::CascadeDesc desc;
};

inline float4x4 ViewFromBasis( const D3 &eye, const D3 &right, const D3 &up, const D3 &back )
{
	float4x4 m;
	const D3 axes[3] = { right, up, back };
	for ( int r = 0; r < 3; ++r )
	{
		m.rows[r] = { float( axes[r].x ), float( axes[r].y ), float( axes[r].z ),
		    float( -Dot( axes[r], eye ) ) };
	}
	m.rows[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	return m;
}

inline CameraCase MakeCameraCase( std::mt19937 &random )
{
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	CameraCase c;
	c.eye = { ( unit( random ) - 0.5 ) * 8000, ( unit( random ) - 0.5 ) * 8000,
	    ( unit( random ) - 0.5 ) * 8000 };
	c.back = RandomUnit( random );
	D3 e1;
	D3 e2;
	Perpendiculars( c.back, e1, e2 );
	const double roll = 2.0 * kPi * unit( random );
	c.right = e1 * std::cos( roll ) + e2 * std::sin( roll );
	c.up = Cross( c.back, c.right );
	c.desc.cameraView = ViewFromBasis( c.eye, c.right, c.up, c.back );
	c.desc.verticalFovRadians = float( ( 30.0 + unit( random ) * 100.0 ) * kPi / 180.0 );
	c.desc.aspect = float( 0.5 + unit( random ) * 2.0 );
	c.desc.nearZ = float( 1.0 + unit( random ) * 15.0 );
	c.desc.shadowDistance = float( c.desc.nearZ * ( 20.0 + unit( random ) * 500.0 ) );
	c.desc.lambda = float( unit( random ) < 0.1 ? ( random() % 2 ) : unit( random ) );
	c.desc.cascadeCount = 1 + random() % shadows::kMaxCascades;
	c.desc.resolution = 256u << ( random() % 4 );
	D3 light = RandomUnit( random );
	if ( unit( random ) < 0.1 )
		light = { 0.0, 0.0, -1.0 }; // straight down
	c.desc.lightDirection = F( light );
	c.desc.casterDistance = float( unit( random ) * 2000.0 );
	return c;
}

// Where a world point lands in a cascade: ( x, y ) in NDC and depth.
inline bool InCascade(
    const shadows::Cascade &cascade, const D3 &p, double tolerance, double out[3] )
{
	double h[4];
	Apply( cascade.view.viewProjection, p, h );
	out[0] = h[0] / h[3];
	out[1] = h[1] / h[3];
	out[2] = h[2] / h[3];
	return std::fabs( out[0] ) <= 1.0 + tolerance && std::fabs( out[1] ) <= 1.0 + tolerance &&
	       out[2] >= -tolerance && out[2] <= 1.0 + tolerance;
}

// Splits against the practical scheme; every sampled point of the view
// frustum up to the shadow distance (the split slices' corners included)
// lies in the cascade whose split holds its depth; points moved toward the
// light by up to the caster distance stay in depth range.
inline void CheckCascadeCoverage( const CameraCase &c, const CascadeBuilder &build,
    std::mt19937 &random, std::uint64_t id, Findings &findings )
{
	const auto &desc = c.desc;
	auto built = build( desc );
	if ( !built )
	{
		findings.Violation( "cascades refused", id );
		return;
	}
	const shadows::CascadeSet &set = built.Value();
	if ( set.count != desc.cascadeCount )
	{
		findings.Violation( "cascade count", id );
		return;
	}
	const double n = desc.nearZ;
	const double f = desc.shadowDistance;
	for ( std::uint32_t i = 0; i <= set.count; ++i )
	{
		const double t = double( i ) / set.count;
		const double expected =
		    desc.lambda * n * std::pow( f / n, t ) + ( 1.0 - desc.lambda ) * ( n + ( f - n ) * t );
		const double actual =
		    i < set.count ? set.cascades[i].splitNear : set.cascades[i - 1].splitFar;
		if ( std::fabs( actual - expected ) > 1e-5 * expected )
			findings.Violation( "split distance", id );
		if ( i > 0 && i < set.count && set.cascades[i].splitNear != set.cascades[i - 1].splitFar )
			findings.Violation( "splits not contiguous", id );
	}
	const double tanV = std::tan( 0.5 * desc.verticalFovRadians );
	const double tanH = tanV * desc.aspect;
	const D3 light = Unit( Dd( desc.lightDirection ) );
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	const auto world = [&]( double sx, double sy, double depth )
	{
		return c.eye + c.right * ( sx * tanH * depth ) + c.up * ( sy * tanV * depth ) -
		       c.back * depth;
	};
	const auto check = [&]( std::uint32_t i, double sx, double sy, double depth )
	{
		const D3 p = world( sx, sy, depth );
		double ndc[3];
		if ( !InCascade( set.cascades[i], p, 1e-4, ndc ) )
		{
			findings.Violation( "a point of the view is outside its cascade", id );
			return;
		}
		const double t = desc.casterDistance * unit( random );
		if ( !InCascade( set.cascades[i], p - light * t, 1e-4, ndc ) )
			findings.Violation( "a caster toward the light is outside the cascade", id );
	};
	for ( std::uint32_t i = 0; i < set.count; ++i )
	{
		const double a = set.cascades[i].splitNear;
		const double b = set.cascades[i].splitFar;
		for ( int corner = 0; corner < 8; ++corner )
			check( i, ( corner & 1 ) ? 1.0 : -1.0, ( corner & 2 ) ? 1.0 : -1.0,
			    ( corner & 4 ) ? b : a );
		for ( int s = 0; s < 64; ++s )
			check( i, unit( random ) * 2.0 - 1.0, unit( random ) * 2.0 - 1.0,
			    a + ( b - a ) * unit( random ) );
	}
	++findings.cases;
}

// Texel snapping: moving the camera by less than a texel (and by more)
// moves each cascade's projection of the world origin by whole texels, and
// turning the camera keeps each cascade's texel size.
inline void CheckCascadeStability( const CameraCase &c, const CascadeBuilder &build,
    std::mt19937 &random, std::uint64_t id, Findings &findings )
{
	auto base = build( c.desc );
	if ( !base )
	{
		findings.Violation( "cascades refused", id );
		return;
	}
	const auto texelOfOrigin = [&]( const shadows::Cascade &cascade, double out[2] )
	{
		double h[4];
		Apply( cascade.view.viewProjection, { 0.0, 0.0, 0.0 }, h );
		// NDC to texels across the cascade's viewport.
		out[0] = ( h[0] / h[3] * 0.5 + 0.5 ) * c.desc.resolution;
		out[1] = ( 0.5 - h[1] / h[3] * 0.5 ) * c.desc.resolution;
	};
	std::uniform_real_distribution<double> unit( 0.0, 1.0 );
	for ( int move = 0; move < 6; ++move )
	{
		// Offsets from a hundredth of the smallest texel to a few of the largest.
		const double scale =
		    move < 4 ? base.Value().cascades[0].texelSize * ( 0.01 + unit( random ) * 0.9 )
		             : base.Value().cascades[base.Value().count - 1].texelSize * 3.7;
		CameraCase moved = c;
		moved.eye = c.eye + RandomUnit( random ) * scale;
		moved.desc.cameraView = ViewFromBasis( moved.eye, c.right, c.up, c.back );
		auto shifted = build( moved.desc );
		if ( !shifted )
		{
			findings.Violation( "cascades refused", id );
			return;
		}
		for ( std::uint32_t i = 0; i < base.Value().count; ++i )
		{
			// Float holds the lattice to a few units in the last place of the
			// world coordinates; a cascade whose texel is not well above that
			// cannot be judged.
			const double magnitude = Len( c.eye ) + c.desc.shadowDistance + c.desc.casterDistance;
			const double tolerance = 0.05 + 4.0 * std::ldexp( 1.0, -23 ) * magnitude /
			                                    base.Value().cascades[i].texelSize;
			if ( tolerance > 0.25 )
			{
				++findings.undecided;
				continue;
			}
			double a[2];
			double b[2];
			texelOfOrigin( base.Value().cascades[i], a );
			texelOfOrigin( shifted.Value().cascades[i], b );
			for ( int axis = 0; axis < 2; ++axis )
			{
				const double delta = b[axis] - a[axis];
				if ( std::fabs( delta - std::round( delta ) ) > tolerance )
					findings.Violation(
					    "a camera move shifted the lattice by a fraction of a texel", id );
			}
		}
	}
	// Turning the camera in place keeps the texel sizes.
	CameraCase turned = c;
	turned.back = RandomUnit( random );
	D3 e1;
	D3 e2;
	Perpendiculars( turned.back, e1, e2 );
	turned.right = e1;
	turned.up = e2;
	turned.desc.cameraView = ViewFromBasis( c.eye, turned.right, turned.up, turned.back );
	auto rotated = build( turned.desc );
	if ( !rotated )
	{
		findings.Violation( "cascades refused", id );
		return;
	}
	for ( std::uint32_t i = 0; i < base.Value().count; ++i )
	{
		const double a = base.Value().cascades[i].texelSize;
		const double b = rotated.Value().cascades[i].texelSize;
		if ( std::fabs( a - b ) > 1e-6 * a )
			findings.Violation( "turning the camera changed a texel size", id );
	}
	++findings.cases;
}

} // namespace shadow_oracle

#endif // RENDER_SHADOWS_SHADOW_ORACLE_H
