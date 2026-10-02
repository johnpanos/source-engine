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
#include <cstring>
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
		result.axis = math::Normalize( TransformPoint3( grid.view, light.direction, 0.0f ) );
		result.cosOuter = ClampCos( light.outerCos );
	}
	return result;
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

foundation::Expected<ClusterGrid, ClusterError> SubdivideClusterGrid(
    const ClusterGrid &grid, std::uint32_t tileDivisor, std::uint32_t sliceMultiplier )
{
	using foundation::MakeUnexpected;
	const std::uint32_t tile = grid.limits.tileSizePixels;
	if ( tileDivisor == 0 || sliceMultiplier == 0 || tile == 0 || tile % tileDivisor != 0 )
		return MakeUnexpected( ClusterError::kInvalidLimits );
	if ( grid.slices == 0 || grid.sliceDepths.size() != std::size_t( grid.slices ) + 1 ||
	     grid.cornerRays.size() != std::size_t( grid.tilesX + 1 ) * ( grid.tilesY + 1 ) )
		return MakeUnexpected( ClusterError::kInvalidDepthRange );

	ClusterGrid fine;
	fine.limits = grid.limits;
	fine.limits.tileSizePixels = tile / tileDivisor;
	fine.limits.depthSlices = grid.slices * sliceMultiplier;
	fine.limits.maxFroxels = grid.limits.maxFroxels * tileDivisor * tileDivisor * sliceMultiplier;
	fine.widthPixels = grid.widthPixels;
	fine.heightPixels = grid.heightPixels;
	fine.tilesX =
	    ( grid.widthPixels + fine.limits.tileSizePixels - 1 ) / fine.limits.tileSizePixels;
	fine.tilesY =
	    ( grid.heightPixels + fine.limits.tileSizePixels - 1 ) / fine.limits.tileSizePixels;
	fine.slices = fine.limits.depthSlices;
	fine.nearZ = grid.nearZ;
	fine.farZ = grid.farZ;
	fine.view = grid.view;
	fine.tileDivisor = tileDivisor;
	fine.sliceMultiplier = sliceMultiplier;
	fine.sliceScale = grid.sliceScale * float( sliceMultiplier );
	fine.sliceBias = grid.sliceBias * float( sliceMultiplier );

	// Slices: the parent's boundaries, and log-uniform divisions between them.
	fine.sliceDepths.resize( std::size_t( fine.slices ) + 1 );
	for ( std::uint32_t k = 0; k < grid.slices; ++k )
	{
		const double lo = std::log( double( grid.sliceDepths[k] ) );
		const double hi = std::log( double( grid.sliceDepths[k + 1] ) );
		fine.sliceDepths[std::size_t( k ) * sliceMultiplier] = grid.sliceDepths[k];
		for ( std::uint32_t m = 1; m < sliceMultiplier; ++m )
			fine.sliceDepths[std::size_t( k ) * sliceMultiplier + m] = static_cast<float>(
			    std::exp( lo + ( hi - lo ) * double( m ) / double( sliceMultiplier ) ) );
	}
	fine.sliceDepths.back() = grid.sliceDepths.back();

	// Corner rays: the parent's at shared corners; elsewhere bilinear in
	// pixels between the parent's four around it (a view ray at distance 1 is
	// an affine function of the pixel position, so this is exact).
	const auto parentPixel = [&]( std::uint32_t index, std::uint32_t size )
	{
		return double( std::min<std::uint64_t>( std::uint64_t( index ) * tile, size ) );
	};
	const auto finePixel = [&]( std::uint32_t index, std::uint32_t size )
	{
		return double(
		    std::min<std::uint64_t>( std::uint64_t( index ) * fine.limits.tileSizePixels, size ) );
	};
	// The parent span [ j, j + 1 ] holding a fine edge, and the fraction.
	const auto span = [&]( std::uint32_t index, std::uint32_t parentTiles, std::uint32_t size,
	                      std::uint32_t &j, double &f )
	{
		const double pixel = finePixel( index, size );
		j = std::min( index / tileDivisor, parentTiles );
		if ( j == parentTiles || pixel == parentPixel( j, size ) )
		{
			f = 0.0;
			return;
		}
		const double a = parentPixel( j, size );
		const double b = parentPixel( j + 1, size );
		if ( pixel == b )
		{
			// The screen's edge inside the parent's last tile.
			++j;
			f = 0.0;
			return;
		}
		f = ( pixel - a ) / ( b - a );
	};
	const std::uint32_t parentStride = grid.tilesX + 1;
	const auto parentRay = [&]( std::uint32_t i, std::uint32_t j )
	{
		return grid.cornerRays[std::size_t( j ) * parentStride + i];
	};
	fine.cornerRays.resize( std::size_t( fine.tilesX + 1 ) * ( fine.tilesY + 1 ) );
	for ( std::uint32_t row = 0; row <= fine.tilesY; ++row )
	{
		std::uint32_t pj = 0;
		double fy = 0.0;
		span( row, grid.tilesY, grid.heightPixels, pj, fy );
		for ( std::uint32_t column = 0; column <= fine.tilesX; ++column )
		{
			std::uint32_t pi = 0;
			double fx = 0.0;
			span( column, grid.tilesX, grid.widthPixels, pi, fx );
			float3 &out = fine.cornerRays[std::size_t( row ) * ( fine.tilesX + 1 ) + column];
			if ( fx == 0.0 && fy == 0.0 )
			{
				out = parentRay( pi, pj );
				continue;
			}
			const float3 a = parentRay( pi, pj );
			const float3 b = parentRay( std::min( pi + 1, grid.tilesX ), pj );
			const float3 c = parentRay( pi, std::min( pj + 1, grid.tilesY ) );
			const float3 d =
			    parentRay( std::min( pi + 1, grid.tilesX ), std::min( pj + 1, grid.tilesY ) );
			const auto mix = [&]( float va, float vb, float vc, float vd )
			{
				const double top = double( va ) + ( double( vb ) - double( va ) ) * fx;
				const double bottom = double( vc ) + ( double( vd ) - double( vc ) ) * fx;
				return static_cast<float>( top + ( bottom - top ) * fy );
			};
			out = { mix( a.x, b.x, c.x, d.x ), mix( a.y, b.y, c.y, d.y ), -1.0f };
		}
	}

	// Planes: the parent's at shared edges; elsewhere through the eye and the
	// edge's corner rays, facing as the parent's plane at its span's start.
	const auto planeThrough = [&]( const float3 &p, const float3 &q, const math::Plane &facing )
	{
		const float3 n = math::Cross( p, q );
		const math::Plane plane = NormalizedPlane( n, 0.0f );
		if ( math::Dot( plane.normal, facing.normal ) < 0.0f )
			return math::Plane{ plane.normal * -1.0f, 0.0f };
		return plane;
	};
	const std::uint32_t fineStride = fine.tilesX + 1;
	fine.columnPlanes.resize( std::size_t( fine.tilesX ) + 1 );
	for ( std::uint32_t column = 0; column <= fine.tilesX; ++column )
	{
		std::uint32_t pi = 0;
		double f = 0.0;
		span( column, grid.tilesX, grid.widthPixels, pi, f );
		fine.columnPlanes[column] =
		    f == 0.0 ? grid.columnPlanes[pi]
		             : planeThrough( fine.cornerRays[column],
		                   fine.cornerRays[std::size_t( fine.tilesY ) * fineStride + column],
		                   grid.columnPlanes[pi] );
	}
	fine.rowPlanes.resize( std::size_t( fine.tilesY ) + 1 );
	for ( std::uint32_t row = 0; row <= fine.tilesY; ++row )
	{
		std::uint32_t pj = 0;
		double f = 0.0;
		span( row, grid.tilesY, grid.heightPixels, pj, f );
		fine.rowPlanes[row] =
		    f == 0.0 ? grid.rowPlanes[pj]
		             : planeThrough( fine.cornerRays[std::size_t( row ) * fineStride],
		                   fine.cornerRays[std::size_t( row ) * fineStride + fine.tilesX],
		                   grid.rowPlanes[pj] );
	}
	return fine;
}

std::uint32_t ParentFroxelIndex(
    const ClusterGrid &fine, std::uint32_t x, std::uint32_t y, std::uint32_t slice )
{
	const std::uint32_t tile = fine.limits.tileSizePixels * fine.tileDivisor;
	const std::uint32_t tilesX = ( fine.widthPixels + tile - 1 ) / tile;
	const std::uint32_t tilesY = ( fine.heightPixels + tile - 1 ) / tile;
	const std::uint32_t px = std::min( x / fine.tileDivisor, tilesX - 1 );
	const std::uint32_t py = std::min( y / fine.tileDivisor, tilesY - 1 );
	const std::uint32_t ps = slice / fine.sliceMultiplier;
	return ( ps * tilesY + py ) * tilesX + px;
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

bool AssignAreaLights( const ClusterGrid &grid, std::span<const area_light::AreaLight> lights,
    std::vector<AreaFroxelMask> &out )
{
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

void AppendAreaMasks( std::span<const AreaFroxelMask> masks, std::vector<std::byte> &indices )
{
	const std::uint32_t offset = std::uint32_t( ( indices.size() - 16 ) / 4 ) + 1;
	std::memcpy( indices.data() + 12, &offset, sizeof( offset ) );
	const auto bytes = std::as_bytes( masks );
	indices.insert( indices.end(), bytes.begin(), bytes.end() );
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
