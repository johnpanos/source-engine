//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lights clustered light assignment (RFC 0016 K7). The
//			serial path; cluster_assign.comp runs the same tests per froxel.
//
//=============================================================================//

#include "render/pass/lights/clusters.h"

#include "render/device/conventions.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace render::pass::lights
{

static_assert( device::conventions::kClipYUp && device::conventions::kOriginTopLeft,
    "the grid maps pixel row 0 to clip +Y" );

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

math::Plane NormalizedPlane( const float3 &normal, float d )
{
	const float length = math::Length( normal );
	return { normal * ( 1.0f / length ), d / length };
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

ClusterLimits DesktopClusterLimits()
{
	ClusterLimits limits;
	limits.tileSizePixels = 64;
	limits.depthSlices = 24;
	limits.maxFroxels = 1u << 16;
	limits.maxLights = 1024;
	limits.maxLightsPerFroxel = 128;
	limits.maxLightIndices = 1u << 20;
	return limits;
}

ClusterLimits MobileClusterLimits()
{
	ClusterLimits limits;
	limits.tileSizePixels = 64;
	limits.depthSlices = 16;
	limits.maxFroxels = 1u << 14;
	limits.maxLights = 256;
	limits.maxLightsPerFroxel = 32;
	limits.maxLightIndices = 1u << 18;
	return limits;
}

foundation::Expected<ClusterGrid, ClusterError> CreateClusterGrid(
    const ClusterViewDesc &desc, const ClusterLimits &limits )
{
	using foundation::MakeUnexpected;
	if ( desc.widthPixels == 0 || desc.heightPixels == 0 )
		return MakeUnexpected( ClusterError::kInvalidViewport );
	if ( !Finite( desc.nearZ ) || !Finite( desc.farZ ) || !( desc.nearZ > 0.0f ) ||
	     !( desc.farZ > desc.nearZ ) )
		return MakeUnexpected( ClusterError::kInvalidDepthRange );
	if ( limits.tileSizePixels == 0 || limits.depthSlices == 0 || limits.maxLights == 0 ||
	     limits.maxLightsPerFroxel == 0 || limits.maxLightIndices == 0 )
		return MakeUnexpected( ClusterError::kInvalidLimits );

	const math::float4x4 &p = desc.projection;
	const float4 &r0 = p.rows[0];
	const float4 &r1 = p.rows[1];
	const float4 &r3 = p.rows[3];
	// Clip w must be the view distance, and x and y must not be offset (the
	// tile planes then pass through the eye).
	if ( !( r3.x == 0.0f && r3.y == 0.0f && r3.z == -1.0f && r3.w == 0.0f ) || r0.w != 0.0f ||
	     r1.w != 0.0f )
		return MakeUnexpected( ClusterError::kNotPerspective );
	const float det = r0.x * r1.y - r0.y * r1.x;
	if ( !Finite( det ) || det == 0.0f )
		return MakeUnexpected( ClusterError::kNotPerspective );

	const std::uint64_t tilesX =
	    ( std::uint64_t( desc.widthPixels ) + limits.tileSizePixels - 1 ) / limits.tileSizePixels;
	const std::uint64_t tilesY =
	    ( std::uint64_t( desc.heightPixels ) + limits.tileSizePixels - 1 ) / limits.tileSizePixels;
	if ( tilesX * tilesY * limits.depthSlices > limits.maxFroxels )
		return MakeUnexpected( ClusterError::kTooManyFroxels );

	ClusterGrid grid;
	grid.limits = limits;
	grid.widthPixels = desc.widthPixels;
	grid.heightPixels = desc.heightPixels;
	grid.tilesX = static_cast<std::uint32_t>( tilesX );
	grid.tilesY = static_cast<std::uint32_t>( tilesY );
	grid.slices = limits.depthSlices;
	grid.nearZ = desc.nearZ;
	grid.farZ = desc.farZ;
	grid.view = desc.view;

	const double logRange = std::log( double( desc.farZ ) / double( desc.nearZ ) );
	grid.sliceScale = static_cast<float>( double( grid.slices ) / logRange );
	grid.sliceBias =
	    static_cast<float>( -std::log( double( desc.nearZ ) ) * double( grid.slices ) / logRange );
	grid.sliceDepths.resize( grid.slices + 1 );
	for ( std::uint32_t k = 0; k <= grid.slices; ++k )
	{
		grid.sliceDepths[k] = static_cast<float>(
		    double( desc.nearZ ) * std::exp( logRange * double( k ) / double( grid.slices ) ) );
	}
	grid.sliceDepths.front() = desc.nearZ;
	grid.sliceDepths.back() = desc.farZ;

	// NDC of the tile edges: x from the left, y from the top (clip Y up).
	const auto columnNdc = [&]( std::uint32_t i )
	{
		const float pixel = float( std::min<std::uint64_t>(
		    std::uint64_t( i ) * limits.tileSizePixels, desc.widthPixels ) );
		return 2.0f * pixel / float( desc.widthPixels ) - 1.0f;
	};
	const auto rowNdc = [&]( std::uint32_t j )
	{
		const float pixel = float( std::min<std::uint64_t>(
		    std::uint64_t( j ) * limits.tileSizePixels, desc.heightPixels ) );
		return 1.0f - 2.0f * pixel / float( desc.heightPixels );
	};

	// x_ndc >= c  <=>  r0 . p >= c * -p.z (clip w > 0 inside the grid).
	grid.columnPlanes.resize( grid.tilesX + 1 );
	for ( std::uint32_t i = 0; i <= grid.tilesX; ++i )
	{
		const float c = columnNdc( i );
		grid.columnPlanes[i] = NormalizedPlane( { r0.x, r0.y, r0.z + c }, 0.0f );
	}
	// y_ndc <= c  <=>  -( r1 . p ) - c * p.z >= 0.
	grid.rowPlanes.resize( grid.tilesY + 1 );
	for ( std::uint32_t j = 0; j <= grid.tilesY; ++j )
	{
		const float c = rowNdc( j );
		grid.rowPlanes[j] = NormalizedPlane( { -r1.x, -r1.y, -( r1.z + c ) }, 0.0f );
	}

	// At z = -1: r0.x x + r0.y y = ndcX + r0.z, and the same for y.
	grid.cornerRays.resize( std::size_t( grid.tilesX + 1 ) * ( grid.tilesY + 1 ) );
	for ( std::uint32_t j = 0; j <= grid.tilesY; ++j )
	{
		const float by = rowNdc( j ) + r1.z;
		for ( std::uint32_t i = 0; i <= grid.tilesX; ++i )
		{
			const float bx = columnNdc( i ) + r0.z;
			const float x = ( bx * r1.y - r0.y * by ) / det;
			const float y = ( r0.x * by - bx * r1.x ) / det;
			grid.cornerRays[std::size_t( j ) * ( grid.tilesX + 1 ) + i] = { x, y, -1.0f };
		}
	}
	return grid;
}

std::uint32_t SliceOfDepth( const ClusterGrid &grid, float viewDistance )
{
	if ( !( viewDistance > grid.nearZ ) )
		return 0;
	const float slice = std::floor( std::log( viewDistance ) * grid.sliceScale + grid.sliceBias );
	if ( !( slice < float( grid.slices - 1 ) ) )
		return grid.slices - 1;
	return slice > 0.0f ? static_cast<std::uint32_t>( slice ) : 0u;
}

std::uint32_t FroxelAt( const ClusterGrid &grid, float pixelX, float pixelY, float viewDistance )
{
	const auto tile = [&]( float pixel, std::uint32_t count )
	{
		const float t = std::floor( pixel / float( grid.limits.tileSizePixels ) );
		if ( !( t > 0.0f ) )
			return 0u;
		return t < float( count - 1 ) ? static_cast<std::uint32_t>( t ) : count - 1;
	};
	return grid.FroxelIndex( tile( pixelX, grid.tilesX ), tile( pixelY, grid.tilesY ),
	    SliceOfDepth( grid, viewDistance ) );
}

foundation::Expected<ClusterStats, ClusterFailure> AssignLights(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights, ClusterLists &out )
{
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
					const float3 rays[4] = { grid.cornerRays[y * rayStride + x],
					    grid.cornerRays[y * rayStride + x + 1],
					    grid.cornerRays[( y + 1 ) * rayStride + x],
					    grid.cornerRays[( y + 1 ) * rayStride + x + 1] };
					float3 lo = rays[0] * sliceNear;
					float3 hi = lo;
					float3 corners[8];
					for ( int c = 0; c < 4; ++c )
					{
						corners[c] = rays[c] * sliceNear;
						corners[c + 4] = rays[c] * sliceFar;
					}
					for ( const float3 &corner : corners )
					{
						lo = math::Min( lo, corner );
						hi = math::Max( hi, corner );
					}
					if ( !unbounded )
					{
						const float3 nearest = math::Max( lo, math::Min( light.center, hi ) );
						const float3 gap = light.center - nearest;
						if ( math::Dot( gap, gap ) > boxRadius * boxRadius )
							continue;
					}
					if ( light.spot )
					{
						const float3 middle = ( lo + hi ) * 0.5f;
						float bound = 0.0f;
						for ( const float3 &corner : corners )
							bound = std::max( bound, math::Length( corner - middle ) );
						const float slack =
						    kRelativeSlack * ( math::Length( middle ) + bound + centerLength );
						if ( !SphereMayTouchCone( light, middle, bound + slack ) )
							continue;
					}
					const std::uint32_t froxel = grid.FroxelIndex( x, y, k );
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

	std::stable_sort( pairs.begin(), pairs.end(),
	    []( std::uint64_t a, std::uint64_t b )
	    {
		    return ( a >> 32 ) < ( b >> 32 );
	    } );
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

ClusterLightTable PackClusterLights(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights )
{
	ClusterStats ignored;
	ClusterLightTable table;
	for ( const std::uint32_t index : AdmittedLights( grid, lights, ignored ) )
	{
		const ViewLight light = ToView( grid, lights[index], index );
		ClusterLightGpu record;
		record.positionRadius[0] = light.center.x;
		record.positionRadius[1] = light.center.y;
		record.positionRadius[2] = light.center.z;
		record.positionRadius[3] = std::isinf( light.radius ) ? kUnboundedRadius : light.radius;
		record.directionOuterCos[0] = light.axis.x;
		record.directionOuterCos[1] = light.axis.y;
		record.directionOuterCos[2] = light.axis.z;
		record.directionOuterCos[3] = light.cosOuter;
		table.lights.push_back( record );
		table.lightSetIndex.push_back( index );
	}
	return table;
}

std::vector<math::float4> PackClusterGrid( const ClusterGrid &grid )
{
	std::vector<math::float4> data;
	data.reserve( grid.columnPlanes.size() + grid.rowPlanes.size() + grid.sliceDepths.size() +
	              grid.cornerRays.size() );
	for ( const math::Plane &plane : grid.columnPlanes )
		data.push_back( { plane.normal.x, plane.normal.y, plane.normal.z, plane.d } );
	for ( const math::Plane &plane : grid.rowPlanes )
		data.push_back( { plane.normal.x, plane.normal.y, plane.normal.z, plane.d } );
	for ( const float depth : grid.sliceDepths )
		data.push_back( { depth, 0.0f, 0.0f, 0.0f } );
	for ( const float3 &ray : grid.cornerRays )
		data.push_back( { ray.x, ray.y, ray.z, 0.0f } );
	return data;
}

} // namespace render::pass::lights
