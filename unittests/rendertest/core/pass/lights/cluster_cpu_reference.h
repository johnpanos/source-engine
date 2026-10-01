//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Test-only serial reference; never linked into the renderer.
#pragma once
#include "render/pass/lights/clusters.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>
namespace render::pass::lights
{
namespace cpu_reference
{
namespace
{

using math::float3;
using math::float4;

// Float slack on every distance test, relative to the magnitudes involved.
// The tests are exact in real arithmetic; the slack keeps rounding from
// turning a touching light into a miss.
constexpr float kRelativeSlack = 4.0e-6f;
// A finite stand-in for "unbounded" in the GPU record.
constexpr float kUnboundedRadius = 3.0e38f;

bool Finite( float value )
{
	return std::isfinite( value );
}

bool Finite3( const float *v )
{
	return Finite( v[0] ) && Finite( v[1] ) && Finite( v[2] );
}

// World to view in double: a light near a camera far from the world's
// origin keeps its view-space position to float precision of its (small)
// view distance rather than of the world coordinates. The compute pass reads
// these view-space records, so it never subtracts large world coordinates.
float3 TransformPoint3( const math::float4x4 &m, const float *p, float w )
{
	float3 result;
	float *out = &result.x;
	for ( int r = 0; r < 3; ++r )
	{
		const float4 &row = m.rows[r];
		out[r] = static_cast<float>( double( row.x ) * p[0] + double( row.y ) * p[1] +
		                             double( row.z ) * p[2] + double( row.w ) * w );
	}
	return result;
}

// A point or spot light in view space. A point light is a spot whose cone is
// every direction (cosOuter -1).
struct ViewLight
{
	std::uint32_t index = 0;
	float3 center;
	float radius = 0.0f; // +infinity: unbounded
	float3 axis = { 0.0f, 0.0f, -1.0f };
	float cosOuter = -1.0f;
	float sinOuter = 0.0f;
	bool spot = false;
};

enum class Admission
{
	kClustered,
	kDirectional,
	kInvalid,
};

Admission Admit( const light_set::RuntimeLight &light )
{
	if ( light.shape == light_set::LightShape::Directional )
		return Admission::kDirectional;
	if ( !Finite3( light.position ) || !Finite( light.radius ) || light.radius < 0.0f )
		return Admission::kInvalid;
	if ( light.shape == light_set::LightShape::Spot )
	{
		if ( !Finite3( light.direction ) || !Finite( light.outerCos ) )
			return Admission::kInvalid;
		const float3 d = { light.direction[0], light.direction[1], light.direction[2] };
		if ( !( math::Dot( d, d ) > 0.0f ) )
			return Admission::kInvalid;
	}
	return Admission::kClustered;
}

// The admitted lights in light-set order, at most maxLights of them.
std::vector<std::uint32_t> AdmittedLights(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights, ClusterStats &stats )
{
	std::vector<std::uint32_t> admitted;
	for ( std::size_t i = 0; i < lights.size(); ++i )
	{
		switch ( Admit( lights[i] ) )
		{
		case Admission::kDirectional:
			++stats.directionalLights;
			break;
		case Admission::kInvalid:
			++stats.invalidLights;
			break;
		case Admission::kClustered:
			if ( admitted.size() >= grid.limits.maxLights )
				++stats.lightsOverCapacity;
			else
				admitted.push_back( static_cast<std::uint32_t>( i ) );
			break;
		}
	}
	stats.lightsClustered = static_cast<std::uint32_t>( admitted.size() );
	return admitted;
}

float ClampCos( float c )
{
	return std::clamp( c, -1.0f, 1.0f );
}

ViewLight ToView(
    const ClusterGrid &grid, const light_set::RuntimeLight &light, std::uint32_t index )
{
	ViewLight result;
	result.index = index;
	result.center = TransformPoint3( grid.view, light.position, 1.0f );
	result.radius = light.radius > 0.0f ? light.radius : std::numeric_limits<float>::infinity();
	if ( light.shape == light_set::LightShape::Spot )
	{
		result.spot = true;
		result.axis = math::Normalize( TransformPoint3( grid.view, light.direction, 0.0f ) );
		result.cosOuter = ClampCos( light.outerCos );
		result.sinOuter = std::sqrt( std::max( 0.0f, 1.0f - result.cosOuter * result.cosOuter ) );
	}
	return result;
}

// Whether a sphere around `center` inflated to `radius` can touch the cone
// (apex, unit axis, half-angle given by cos and sin). |V| sin(phi - theta),
// with phi the angle between V and the axis, never exceeds the distance from
// the point to the solid cone, so rejecting on it is conservative for any
// half-angle up to pi.
bool SphereMayTouchCone( const ViewLight &light, const float3 &center, float radius )
{
	const float3 v = center - light.center;
	const float lengthSquared = math::Dot( v, v );
	const float along = math::Dot( v, light.axis );
	const float across = std::sqrt( std::max( 0.0f, lengthSquared - along * along ) );
	const float coneDistance = light.cosOuter * across - along * light.sinOuter;
	if ( coneDistance > radius )
		return false;
	// Beyond the light's sphere along the axis.
	if ( along > radius + light.radius )
		return false;
	// Behind the apex plane, where a cone no wider than a hemisphere has nothing.
	if ( light.cosOuter >= 0.0f && along < -radius )
		return false;
	return true;
}

} // namespace
} // namespace cpu_reference

inline foundation::Expected<ClusterStats, ClusterFailure> AssignLights(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights, ClusterLists &out )
{
	using namespace cpu_reference;
	ClusterStats stats;
	const std::vector<std::uint32_t> admitted = AdmittedLights( grid, lights, stats );

	const std::uint32_t froxelCount = grid.FroxelCount();
	const std::uint32_t rayStride = grid.tilesX + 1;
	std::vector<std::uint32_t> candidates( froxelCount, 0 );
	// (froxel, light) pairs in light order; a stable sort by froxel keeps
	// each froxel's lights ascending.
	std::vector<std::uint64_t> pairs;
	std::vector<std::uint32_t> columns;
	std::vector<std::uint32_t> rows;
	// Each froxel's box, made the first time a light reaches it (the same
	// arithmetic every light would repeat).
	struct FroxelBox
	{
		float3 lo;
		float3 hi;
		float3 middle;
		float bound = 0.0f;
		bool made = false;
	};
	std::vector<FroxelBox> boxes( froxelCount );

	for ( const std::uint32_t lightIndex : admitted )
	{
		const ViewLight light = ToView( grid, lights[lightIndex], lightIndex );
		const bool unbounded = std::isinf( light.radius );
		const float centerLength = math::Length( light.center );
		const float planeRadius =
		    unbounded ? light.radius
		              : light.radius + kRelativeSlack * ( centerLength + light.radius );

		columns.clear();
		for ( std::uint32_t x = 0; x < grid.tilesX; ++x )
		{
			if ( grid.columnPlanes[x].Distance( light.center ) >= -planeRadius &&
			     grid.columnPlanes[x + 1].Distance( light.center ) <= planeRadius )
				columns.push_back( x );
		}
		rows.clear();
		for ( std::uint32_t y = 0; y < grid.tilesY; ++y )
		{
			if ( grid.rowPlanes[y].Distance( light.center ) >= -planeRadius &&
			     grid.rowPlanes[y + 1].Distance( light.center ) <= planeRadius )
				rows.push_back( y );
		}
		if ( columns.empty() || rows.empty() )
			continue;

		const float depth = -light.center.z;
		for ( std::uint32_t k = 0; k < grid.slices; ++k )
		{
			const float sliceNear = grid.sliceDepths[k];
			const float sliceFar = grid.sliceDepths[k + 1];
			if ( depth + planeRadius < sliceNear || depth - planeRadius > sliceFar )
				continue;
			const float boxRadius =
			    unbounded ? light.radius : planeRadius + kRelativeSlack * sliceFar;
			for ( const std::uint32_t y : rows )
			{
				for ( const std::uint32_t x : columns )
				{
					const std::uint32_t froxel = grid.FroxelIndex( x, y, k );
					FroxelBox &box = boxes[froxel];
					if ( !box.made )
					{
						const float3 rays[4] = { grid.cornerRays[y * rayStride + x],
						    grid.cornerRays[y * rayStride + x + 1],
						    grid.cornerRays[( y + 1 ) * rayStride + x],
						    grid.cornerRays[( y + 1 ) * rayStride + x + 1] };
						float3 corners[8];
						for ( int c = 0; c < 4; ++c )
						{
							corners[c] = rays[c] * sliceNear;
							corners[c + 4] = rays[c] * sliceFar;
						}
						box.lo = rays[0] * sliceNear;
						box.hi = box.lo;
						for ( const float3 &corner : corners )
						{
							box.lo = math::Min( box.lo, corner );
							box.hi = math::Max( box.hi, corner );
						}
						box.middle = ( box.lo + box.hi ) * 0.5f;
						for ( const float3 &corner : corners )
							box.bound = std::max( box.bound, math::Length( corner - box.middle ) );
						box.made = true;
					}
					const float3 lo = box.lo;
					const float3 hi = box.hi;
					if ( !unbounded )
					{
						const float3 nearest = math::Max( lo, math::Min( light.center, hi ) );
						const float3 gap = light.center - nearest;
						if ( math::Dot( gap, gap ) > boxRadius * boxRadius )
							continue;
					}
					if ( light.spot )
					{
						const float3 middle = box.middle;
						const float bound = box.bound;
						const float slack =
						    kRelativeSlack * ( math::Length( middle ) + bound + centerLength );
						if ( !SphereMayTouchCone( light, middle, bound + slack ) )
							continue;
					}
					++candidates[froxel];
					pairs.push_back( ( std::uint64_t( froxel ) << 32 ) | lightIndex );
				}
			}
		}
	}

	// Offsets in froxel order; capacity keeps a prefix of each list.
	std::vector<FroxelRange> ranges( froxelCount );
	std::uint64_t offset = 0;
	for ( std::uint32_t f = 0; f < froxelCount; ++f )
	{
		std::uint64_t keep = std::min( candidates[f], grid.limits.maxLightsPerFroxel );
		const std::uint64_t room =
		    offset < grid.limits.maxLightIndices ? grid.limits.maxLightIndices - offset : 0;
		keep = std::min( keep, room );
		ranges[f] = { static_cast<std::uint32_t>( offset ), static_cast<std::uint32_t>( keep ) };
		if ( keep < candidates[f] )
		{
			++stats.froxelsOverflowed;
			stats.assignmentsDropped += candidates[f] - static_cast<std::uint32_t>( keep );
		}
		offset += keep;
	}
	stats.assignments = static_cast<std::uint32_t>( offset );

	if ( grid.limits.overflow == OverflowPolicy::kFail && stats.Overflowed() )
		return foundation::MakeUnexpected( ClusterFailure{ ClusterError::kOverflow, stats } );

	// The pairs are in light order, and each froxel's list keeps that order:
	// filling the lists in pair order places them as a stable sort by froxel
	// would, with no sort.
	std::vector<std::uint32_t> indices( stats.assignments );
	std::vector<std::uint32_t> written( froxelCount, 0 );
	for ( const std::uint64_t pair : pairs )
	{
		const std::uint32_t froxel = static_cast<std::uint32_t>( pair >> 32 );
		if ( written[froxel] < ranges[froxel].count )
		{
			indices[ranges[froxel].offset + written[froxel]] =
			    static_cast<std::uint32_t>( pair & 0xffffffffu );
			++written[froxel];
		}
	}
	out.froxels = std::move( ranges );
	out.lightIndices = std::move( indices );
	return stats;
}

inline bool AssignAreaLights( const ClusterGrid &grid,
    std::span<const area_light::AreaLight> lights, std::vector<AreaFroxelMask> &out )
{
	using namespace cpu_reference;
	if ( lights.size() > 64 )
		return false;
	std::vector<AreaFroxelMask> masks( grid.FroxelCount() );
	for ( std::size_t i = 0; i < lights.size(); ++i )
	{
		const auto &light = lights[i];
		if ( !( light.reach > 0.0f ) )
			continue;
		const float3 center = TransformPoint3( grid.view, light.rect.center, 1.0f );
		const float3 u = TransformPoint3( grid.view, light.rect.halfU, 0.0f );
		const float3 v = TransformPoint3( grid.view, light.rect.halfV, 0.0f );
		const float slack = kRelativeSlack * ( math::Length( center ) + math::Length( u ) +
		                                         math::Length( v ) + light.reach );
		const auto radius = [&]( const float3 &normal )
		{
			return std::abs( math::Dot( normal, u ) ) + std::abs( math::Dot( normal, v ) ) +
			       light.reach + slack;
		};
		const auto touches = [&]( const math::Plane &a, const math::Plane &b )
		{
			return a.Distance( center ) >= -radius( a.normal ) &&
			       b.Distance( center ) <= radius( b.normal );
		};
		const float depthRadius = radius( { 0, 0, 1 } );
		for ( std::uint32_t z = 0; z < grid.slices; ++z )
		{
			if ( ( z > 0 && -center.z + depthRadius < grid.sliceDepths[z] ) ||
			     ( z + 1 < grid.slices && -center.z - depthRadius > grid.sliceDepths[z + 1] ) )
				continue;
			for ( std::uint32_t y = 0; y < grid.tilesY; ++y )
			{
				if ( !touches( grid.rowPlanes[y > 0 ? y - 1 : 0],
				         grid.rowPlanes[std::min( y + 2, grid.tilesY )] ) )
					continue;
				for ( std::uint32_t x = 0; x < grid.tilesX; ++x )
				{
					if ( touches( grid.columnPlanes[x > 0 ? x - 1 : 0],
					         grid.columnPlanes[std::min( x + 2, grid.tilesX )] ) )
						masks[grid.FroxelIndex( x, y, z )][i / 32] |= 1u << ( i % 32 );
				}
			}
		}
	}
	out = std::move( masks );
	return true;
}

inline void AppendAreaMasks(
    std::span<const AreaFroxelMask> masks, std::vector<std::byte> &indices )
{
	const std::uint32_t offset = std::uint32_t( ( indices.size() - 16 ) / 4 ) + 1;
	std::memcpy( indices.data() + 12, &offset, sizeof( offset ) );
	const auto bytes = std::as_bytes( masks );
	indices.insert( indices.end(), bytes.begin(), bytes.end() );
}

}
