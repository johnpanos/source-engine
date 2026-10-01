//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lights.clusters (RFC 0016 K7, render.lights.v1):
//
//			L1 invalid views and limits fail with their error; the grid's
//			   slice depths, corner rays and tile planes agree with the
//			   reference built from the projection;
//			L2 over 1,000 seeded scenes no light that reaches a froxel is
//			   missing from it (zero false negatives against the independent
//			   reference in cluster_oracle.h); the false-positive rates are
//			   printed and stay under their recorded ceilings;
//			L3 at random points of each view, FroxelAt names a froxel that
//			   holds the point, and every light lighting the point is in it;
//			L4 lists are contiguous in froxel order with ascending indices of
//			   point and spot lights only;
//			L5 overflow: tight limits keep each list's prefix that fits, the
//			   result counts exactly what was lost, and kFail fails with the
//			   same counts and leaves the output unchanged;
//			L6 directional and invalid lights are counted, never listed; an
//			   unbounded light reaches every froxel of its cone;
//			L7 PackClusterLights takes the lights AssignLights takes, in order;
//			L8 assignment is deterministic;
//			L9 SubdivideClusterGrid: the boundaries a fine grid shares with its
//			   parent are the parent's, bitwise; every fine froxel lies inside
//			   the parent froxel ParentFroxelIndex names (its corners by the
//			   parent's planes and slice depths, its centre by FroxelAt); the
//			   fine geometry matches the reference at the fine limits; a grid
//			   whose slices do not subdivide the parent's and a parent map off
//			   by one are both caught.
//
//=============================================================================//

#include "cluster_oracle.h"

#include "testing/checks.h"

#include <cstdio>
#include <cstdlib>
#include <functional>

namespace
{

using namespace cluster_oracle;
namespace lights = render::pass::lights;

constexpr int kScenes = 1000;
constexpr int kOverflowScenes = 200;

std::uint32_t Seed()
{
	const char *text = std::getenv( "CONFORMANCE_SEED" );
	return text ? std::uint32_t( std::strtoul( text, nullptr, 10 ) ) : 20260928u;
}

lights::ClusterViewDesc SimpleView()
{
	lights::ClusterViewDesc desc;
	desc.view = render::math::float4x4::Identity();
	desc.projection = render::math::Perspective( 1.5f, 16.0f / 9.0f, 4.0f, 4096.0f );
	desc.widthPixels = 640;
	desc.heightPixels = 360;
	desc.nearZ = 4.0f;
	desc.farZ = 4096.0f;
	return desc;
}

RuntimeLight PointAt( float x, float y, float z, float radius )
{
	RuntimeLight light;
	light.shape = LightShape::Point;
	light.position[0] = x;
	light.position[1] = y;
	light.position[2] = z;
	light.radius = radius;
	return light;
}

void CheckGridErrors( testing::Checks &checks )
{
	const lights::ClusterLimits limits = lights::DesktopClusterLimits();
	const auto error = [&]( const lights::ClusterViewDesc &desc, const lights::ClusterLimits &l )
	{
		auto grid = lights::CreateClusterGrid( desc, l );
		return grid ? -1 : int( grid.Error() );
	};
	lights::ClusterViewDesc desc = SimpleView();
	checks.That( lights::CreateClusterGrid( desc, limits ).HasValue(), "L1 a valid view builds" );

	lights::ClusterViewDesc bad = desc;
	bad.widthPixels = 0;
	checks.Equal(
	    error( bad, limits ), int( lights::ClusterError::kInvalidViewport ), "L1 zero width" );
	bad = desc;
	bad.nearZ = 0.0f;
	checks.Equal(
	    error( bad, limits ), int( lights::ClusterError::kInvalidDepthRange ), "L1 near 0" );
	bad = desc;
	bad.farZ = bad.nearZ;
	checks.Equal(
	    error( bad, limits ), int( lights::ClusterError::kInvalidDepthRange ), "L1 far = near" );
	bad = desc;
	bad.projection = render::math::Orthographic( 100.0f, 100.0f, 4.0f, 4096.0f );
	checks.Equal(
	    error( bad, limits ), int( lights::ClusterError::kNotPerspective ), "L1 orthographic" );
	lights::ClusterLimits badLimits = limits;
	badLimits.tileSizePixels = 0;
	checks.Equal(
	    error( desc, badLimits ), int( lights::ClusterError::kInvalidLimits ), "L1 tile 0" );
	badLimits = limits;
	badLimits.maxLightsPerFroxel = 0;
	checks.Equal( error( desc, badLimits ), int( lights::ClusterError::kInvalidLimits ),
	    "L1 no room per froxel" );
	badLimits = limits;
	badLimits.maxFroxels = 10;
	checks.Equal(
	    error( desc, badLimits ), int( lights::ClusterError::kTooManyFroxels ), "L1 froxel limit" );

	const lights::ClusterLimits mobile = lights::MobileClusterLimits();
	checks.That( mobile.maxLightsPerFroxel <= limits.maxLightsPerFroxel &&
	                 mobile.maxLights <= limits.maxLights &&
	                 mobile.maxLightIndices <= limits.maxLightIndices &&
	                 mobile.maxFroxels <= limits.maxFroxels,
	    "L1 mobile limits are no larger than desktop" );
}

// L1: the grid's own geometry against the reference, over seeded scenes.
void CheckGridGeometry( testing::Checks &checks, std::mt19937 &random )
{
	int mismatches = 0;
	int slices = 0;
	for ( int s = 0; s < 200; ++s )
	{
		const Scene scene = MakeScene( random );
		auto built = lights::CreateClusterGrid( scene.desc, scene.limits );
		if ( !built )
		{
			++mismatches;
			continue;
		}
		const ClusterGrid &grid = built.Value();
		const RefGrid g = MakeRefGrid( scene.desc, scene.limits );
		for ( std::uint32_t k = 0; k <= g.slices; ++k )
		{
			if ( std::fabs( grid.sliceDepths[k] - g.depths[k] ) > 1e-5 * g.depths[k] )
				++mismatches;
		}
		for ( std::uint32_t k = 0; k < g.slices; ++k )
		{
			const double middle = std::sqrt( g.depths[k] * g.depths[k + 1] );
			slices += lights::SliceOfDepth( grid, float( middle ) ) == k ? 0 : 1;
		}
		for ( std::uint32_t j = 0; j <= g.tilesY; ++j )
		{
			for ( std::uint32_t i = 0; i <= g.tilesX; ++i )
			{
				const D3 ref = PixelPoint( g, std::min<double>( double( i ) * g.tile, g.width ),
				    std::min<double>( double( j ) * g.tile, g.height ), 1.0 );
				const float3 ray = grid.cornerRays[j * ( g.tilesX + 1 ) + i];
				const double scale = 1.0 + Len( ref );
				if ( Len( ref - D3{ ray.x, ray.y, ray.z } ) > 1e-5 * scale )
					++mismatches;
				// The column and row planes pass through the corner rays.
				const float3 point = ray * float( g.nearZ );
				if ( std::fabs( grid.columnPlanes[i].Distance( point ) ) > 1e-4 * scale * g.nearZ ||
				     std::fabs( grid.rowPlanes[j].Distance( point ) ) > 1e-4 * scale * g.nearZ )
					++mismatches;
			}
		}
		// Planes point toward increasing columns and rows: the center of
		// tile (0, 0) lies on the inner side of column plane 0 and row plane 0.
		const D3 inner = PixelPoint(
		    g, 0.5 * std::min( g.tile, g.width ), 0.5 * std::min( g.tile, g.height ), g.nearZ );
		const float3 innerPoint = { float( inner.x ), float( inner.y ), float( inner.z ) };
		if ( !( grid.columnPlanes[0].Distance( innerPoint ) > 0.0f ) ||
		     !( grid.rowPlanes[0].Distance( innerPoint ) > 0.0f ) )
			++mismatches;
	}
	checks.Equal( mismatches, 0, "L1 slice depths, corner rays and planes match the reference" );
	checks.Equal( slices, 0, "L1 SliceOfDepth maps each slice's middle depth to that slice" );
}

using ParentMap = std::function<std::uint32_t(
    const ClusterGrid &fine, std::uint32_t x, std::uint32_t y, std::uint32_t slice )>;

// L9: what is wrong with `fine` as a subdivision of `parent` (0: nothing).
std::uint64_t SubdivisionFindings( const Scene &scene, const ClusterGrid &parent,
    const ClusterGrid &fine, std::uint32_t divisor, std::uint32_t multiplier,
    const ParentMap &parentOf, std::mt19937 &random )
{
	std::uint64_t findings = 0;
	lights::ClusterLimits fineLimits = scene.limits;
	fineLimits.tileSizePixels = scene.limits.tileSizePixels / divisor;
	fineLimits.depthSlices = scene.limits.depthSlices * multiplier;
	const RefGrid g = MakeRefGrid( scene.desc, fineLimits );
	if ( fine.tilesX != g.tilesX || fine.tilesY != g.tilesY || fine.slices != g.slices ||
	     fine.sliceDepths.size() != std::size_t( g.slices ) + 1 ||
	     fine.cornerRays.size() != std::size_t( g.tilesX + 1 ) * ( g.tilesY + 1 ) )
		return 1000000;
	// Shared slice boundaries are the parent's, bitwise; every fine slice
	// lies within its parent slice; the depths match the reference.
	for ( std::uint32_t k = 0; k <= parent.slices; ++k )
		findings +=
		    fine.sliceDepths[std::size_t( k ) * multiplier] == parent.sliceDepths[k] ? 0 : 1;
	for ( std::uint32_t k = 0; k < fine.slices; ++k )
	{
		const std::uint32_t up = k / multiplier;
		findings += fine.sliceDepths[k] >= parent.sliceDepths[up] &&
		                    fine.sliceDepths[k + 1] <= parent.sliceDepths[up + 1] &&
		                    fine.sliceDepths[k] < fine.sliceDepths[k + 1]
		                ? 0
		                : 1;
		findings += std::fabs( fine.sliceDepths[k] - g.depths[k] ) <= 1e-5 * g.depths[k] ? 0 : 1;
	}
	// Shared corners are the parent's, bitwise; every corner matches the
	// reference.
	const std::uint32_t stride = fine.tilesX + 1;
	for ( std::uint32_t j = 0; j <= fine.tilesY; ++j )
	{
		for ( std::uint32_t i = 0; i <= fine.tilesX; ++i )
		{
			const float3 ray = fine.cornerRays[std::size_t( j ) * stride + i];
			const D3 ref = PixelPoint( g, std::min<double>( double( i ) * g.tile, g.width ),
			    std::min<double>( double( j ) * g.tile, g.height ), 1.0 );
			findings +=
			    Len( ref - D3{ ray.x, ray.y, ray.z } ) <= 1e-5 * ( 1.0 + Len( ref ) ) ? 0 : 1;
			if ( i % divisor == 0 && j % divisor == 0 )
			{
				const float3 shared =
				    parent.cornerRays[std::size_t( j / divisor ) * ( parent.tilesX + 1 ) +
				                      i / divisor];
				findings += ray.x == shared.x && ray.y == shared.y && ray.z == shared.z ? 0 : 1;
			}
		}
	}
	// Sampled fine froxels: the parent map agrees with FroxelAt at the
	// froxel's centre, and its corners (moved 1e-3 toward the centre) are
	// inside that parent froxel.
	const std::uint32_t count = fine.FroxelCount();
	const std::uint32_t samples = std::min<std::uint32_t>( count, 1500 );
	for ( std::uint32_t n = 0; n < samples; ++n )
	{
		const std::uint32_t index = samples == count ? n : std::uint32_t( random() % count );
		const std::uint32_t x = index % fine.tilesX;
		const std::uint32_t y = ( index / fine.tilesX ) % fine.tilesY;
		const std::uint32_t slice = index / ( fine.tilesX * fine.tilesY );
		const std::uint32_t mapped = parentOf( fine, x, y, slice );
		const double x0 = double( x ) * g.tile;
		const double x1 = std::min<double>( double( x + 1 ) * g.tile, g.width );
		const double y0 = double( y ) * g.tile;
		const double y1 = std::min<double>( double( y + 1 ) * g.tile, g.height );
		const double d0 = fine.sliceDepths[slice];
		const double d1 = fine.sliceDepths[slice + 1];
		const double centreDepth = std::sqrt( d0 * d1 );
		findings += lights::FroxelAt( parent, float( 0.5 * ( x0 + x1 ) ),
		                float( 0.5 * ( y0 + y1 ) ), float( centreDepth ) ) == mapped
		                ? 0
		                : 1;
		if ( mapped >= parent.FroxelCount() )
		{
			++findings;
			continue;
		}
		const std::uint32_t px = mapped % parent.tilesX;
		const std::uint32_t py = ( mapped / parent.tilesX ) % parent.tilesY;
		const std::uint32_t ps = mapped / ( parent.tilesX * parent.tilesY );
		for ( int corner = 0; corner < 8; ++corner )
		{
			const double cx =
			    0.5 * ( x0 + x1 ) + ( ( corner & 1 ) ? 0.5 : -0.5 ) * ( x1 - x0 ) * 0.999;
			const double cy =
			    0.5 * ( y0 + y1 ) + ( ( corner & 2 ) ? 0.5 : -0.5 ) * ( y1 - y0 ) * 0.999;
			const double depth = ( corner & 4 ) ? d1 - 1e-3 * ( d1 - d0 ) : d0 + 1e-3 * ( d1 - d0 );
			const D3 point = PixelPoint( g, cx, cy, depth );
			const float3 at = { float( point.x ), float( point.y ), float( point.z ) };
			const float slack = 1e-4f * float( depth );
			const bool inside = parent.columnPlanes[px].Distance( at ) >= -slack &&
			                    parent.columnPlanes[px + 1].Distance( at ) <= slack &&
			                    parent.rowPlanes[py].Distance( at ) >= -slack &&
			                    parent.rowPlanes[py + 1].Distance( at ) <= slack &&
			                    depth >= parent.sliceDepths[ps] &&
			                    depth <= parent.sliceDepths[ps + 1];
			findings += inside ? 0 : 1;
		}
	}
	return findings;
}

void CheckSubdivision( testing::Checks &checks, std::mt19937 &random )
{
	const ParentMap real = lights::ParentFroxelIndex;
	// Seeded: the parent's slice one too far for every fine slice past the
	// first of its parent.
	const ParentMap offByOne =
	    []( const ClusterGrid &fine, std::uint32_t x, std::uint32_t y, std::uint32_t slice )
	{
		const std::uint32_t shifted =
		    slice % fine.sliceMultiplier != 0 && slice + fine.sliceMultiplier < fine.slices
		        ? slice + fine.sliceMultiplier
		        : slice;
		return lights::ParentFroxelIndex( fine, x, y, shifted );
	};
	std::uint64_t findings = 0;
	std::uint64_t unaligned = 0;
	std::uint64_t offByOneFindings = 0;
	int failures = 0;
	int scenes = 0;
	int seededScenes = 0;
	for ( int s = 0; s < 100; ++s )
	{
		const Scene scene = MakeScene( random );
		auto parent = lights::CreateClusterGrid( scene.desc, scene.limits );
		const std::uint32_t divisors[] = { 1, 2, 4, 8 };
		const std::uint32_t divisor = divisors[random() % 4];
		const std::uint32_t multiplier = 1 + random() % 4;
		if ( !parent )
		{
			++failures;
			continue;
		}
		auto fine = lights::SubdivideClusterGrid( parent.Value(), divisor, multiplier );
		if ( !fine )
		{
			++failures;
			continue;
		}
		++scenes;
		findings += SubdivisionFindings(
		    scene, parent.Value(), fine.Value(), divisor, multiplier, real, random );
		if ( multiplier > 1 && parent.Value().slices > 1 )
		{
			++seededScenes;
			offByOneFindings += SubdivisionFindings( scene, parent.Value(), fine.Value(), divisor,
			                        multiplier, offByOne, random ) > 0;
			// Seeded: a grid built directly at the fine limits plus one slice,
			// which does not subdivide the parent's slices.
			lights::ClusterLimits limits = scene.limits;
			limits.tileSizePixels /= divisor;
			limits.depthSlices = parent.Value().slices * multiplier + 1;
			limits.maxFroxels = 1u << 30;
			auto direct = lights::CreateClusterGrid( scene.desc, limits );
			if ( direct )
			{
				ClusterGrid bad = direct.Value();
				bad.tileDivisor = divisor;
				bad.sliceMultiplier = multiplier;
				bad.slices = parent.Value().slices * multiplier;
				bad.sliceDepths.resize( std::size_t( bad.slices ) + 1 );
				unaligned += SubdivisionFindings( scene, parent.Value(), bad, divisor, multiplier,
				                 real, random ) > 0;
			}
		}
	}
	std::printf( "INFO subdivision scenes %d (seeded %d)\n", scenes, seededScenes );
	checks.Equal( failures, 0, "L9 every seeded subdivision builds" );
	checks.Equal( findings, 0ull,
	    "L9 fine froxels subdivide their parents: shared boundaries bitwise, containment, map" );
	checks.That( seededScenes >= 20, "L9 seeded subdivisions exercised" );
	checks.Equal( offByOneFindings, std::uint64_t( seededScenes ),
	    "L9 seeded parent map off by one is caught in every scene" );
	checks.Equal( unaligned, std::uint64_t( seededScenes ),
	    "L9 seeded grid whose slices do not subdivide the parent's is caught in every scene" );

	lights::ClusterViewDesc desc = SimpleView();
	auto parent = lights::CreateClusterGrid( desc, lights::DesktopClusterLimits() );
	const auto error = [&]( std::uint32_t divisor, std::uint32_t multiplier )
	{
		auto fine = lights::SubdivideClusterGrid( parent.Value(), divisor, multiplier );
		return fine ? -1 : int( fine.Error() );
	};
	checks.Equal( error( 0, 1 ), int( lights::ClusterError::kInvalidLimits ), "L9 divisor 0" );
	checks.Equal( error( 1, 0 ), int( lights::ClusterError::kInvalidLimits ), "L9 multiplier 0" );
	checks.Equal( error( 3, 1 ), int( lights::ClusterError::kInvalidLimits ),
	    "L9 a divisor that does not divide the tile" );
	auto same = lights::SubdivideClusterGrid( parent.Value(), 1, 1 );
	checks.That( same && same.Value().sliceDepths == parent.Value().sliceDepths &&
	                 same.Value().cornerRays.size() == parent.Value().cornerRays.size(),
	    "L9 dividing by one is the grid itself" );
}

void CheckSmallCases( testing::Checks &checks )
{
	const lights::ClusterViewDesc desc = SimpleView();
	auto built = lights::CreateClusterGrid( desc, lights::DesktopClusterLimits() );
	if ( !checks.That( built.HasValue(), "L6 grid" ) )
		return;
	const ClusterGrid &grid = built.Value();

	std::vector<RuntimeLight> set;
	RuntimeLight sun;
	sun.shape = LightShape::Directional;
	set.push_back( sun );                               // 0: directional
	set.push_back( PointAt( 0, 0, 10.0f, 2.0f ) );      // 1: behind the eye
	set.push_back( PointAt( 0, 0, -9000.0f, 100.0f ) ); // 2: beyond far
	set.push_back( PointAt( 0, 0, 0, 0.0f ) );          // 3: unbounded
	RuntimeLight nan = PointAt( 0, 0, -10.0f, 5.0f );
	nan.position[1] = std::nanf( "" );
	set.push_back( nan );                            // 4: invalid
	set.push_back( PointAt( 0, 0, -10.0f, -1.0f ) ); // 5: negative radius
	RuntimeLight spot = PointAt( 0, 0, 0, 0.0f );    // 6: unbounded spot down -Z
	spot.shape = LightShape::Spot;
	spot.direction[0] = 0.0f;
	spot.direction[1] = 0.0f;
	spot.direction[2] = -1.0f;
	spot.outerCos = std::cos( 0.1f );
	set.push_back( spot );

	lights::ClusterLists lists;
	auto stats = lights::AssignLights( grid, set, lists );
	if ( !checks.That( stats.HasValue(), "L6 assignment" ) )
		return;
	checks.Equal( stats.Value().directionalLights, 1u, "L6 directional light counted" );
	checks.Equal( stats.Value().invalidLights, 2u, "L6 invalid lights counted" );
	checks.Equal( stats.Value().lightsClustered, 4u, "L6 point and spot lights clustered" );
	std::vector<int> froxelsOf( set.size(), 0 );
	for ( const auto &range : lists.froxels )
		for ( std::uint32_t i = 0; i < range.count; ++i )
			++froxelsOf[lists.lightIndices[range.offset + i]];
	checks.Equal( froxelsOf[0] + froxelsOf[4] + froxelsOf[5], 0,
	    "L6 directional and invalid lights never listed" );
	checks.Equal( froxelsOf[1], 0, "L6 a light wholly behind the eye reaches nothing" );
	checks.Equal( froxelsOf[2], 0, "L6 a light wholly beyond far reaches nothing" );
	checks.Equal(
	    froxelsOf[3], int( grid.FroxelCount() ), "L6 an unbounded point reaches every froxel" );
	// The narrow unbounded spot on the axis reaches the center column in
	// every slice, and not the screen's corners.
	const std::uint32_t centerX = grid.tilesX / 2;
	const std::uint32_t centerY = grid.tilesY / 2;
	int centerMisses = 0;
	int cornerHits = 0;
	for ( std::uint32_t k = 0; k < grid.slices; ++k )
	{
		const auto &center = lists.froxels[grid.FroxelIndex( centerX, centerY, k )];
		bool found = false;
		for ( std::uint32_t i = 0; i < center.count; ++i )
			found = found || lists.lightIndices[center.offset + i] == 6;
		centerMisses += found ? 0 : 1;
		const auto &corner = lists.froxels[grid.FroxelIndex( 0, 0, k )];
		for ( std::uint32_t i = 0; i < corner.count; ++i )
			cornerHits += lists.lightIndices[corner.offset + i] == 6 ? 1 : 0;
	}
	checks.Equal( centerMisses, 0, "L6 an unbounded spot reaches its axis in every slice" );
	checks.Equal( cornerHits, 0, "L6 an unbounded narrow spot misses the screen corner" );

	// L7: the packed table follows the same admission.
	const lights::ClusterLightTable table = lights::PackClusterLights( grid, set );
	checks.That( table.lightSetIndex == std::vector<std::uint32_t>{ 1, 2, 3, 6 },
	    "L7 packed lights are the clustered lights in order" );
	checks.That( table.lights.size() == 4 && table.lights[0].directionOuterCos[3] == -1.0f &&
	                 table.lights[2].positionRadius[3] > 1e30f &&
	                 std::fabs( table.lights[3].directionOuterCos[3] - std::cos( 0.1f ) ) < 1e-6f,
	    "L7 packed records: point cone -1, unbounded radius, spot cosine" );
	// The packed records are view space: the identity view keeps positions.
	checks.That( table.lights[1].positionRadius[2] == -9000.0f &&
	                 table.lights[3].directionOuterCos[2] == -1.0f,
	    "L7 packed records are in view space" );
	const std::vector<render::math::float4> packed = lights::PackClusterGrid( grid );
	const std::size_t rays = grid.tilesX + grid.tilesY + grid.slices + 3;
	checks.That( packed.size() == rays + grid.cornerRays.size() &&
	                 packed[grid.tilesX + 1].x == grid.rowPlanes[0].normal.x &&
	                 packed[grid.tilesX + grid.tilesY + 2].x == grid.sliceDepths[0] &&
	                 packed[rays].z == grid.cornerRays[0].z &&
	                 packed.back().x == grid.cornerRays.back().x,
	    "L7 the packed grid follows the compute pass's layout" );
}

} // namespace

// Independently sample receivers in world space, then project them into the
// grid. Every receiver inside the rounded rectangle's reach must keep its bit.
void CheckAreaMasks( testing::Checks &checks )
{
	std::mt19937 random( 1001 );
	std::uniform_real_distribution<float> unit( -1.0f, 1.0f );
	std::uint64_t reached = 0, missed = 0, culled = 0, seededMissed = 0;
	for ( int scene = 0; scene < 100; ++scene )
	{
		auto desc = SimpleView();
		desc.view.rows[0].w = 1000.0f * unit( random );
		desc.view.rows[1].w = 1000.0f * unit( random );
		auto grid = lights::CreateClusterGrid( desc, lights::DesktopClusterLimits() );
		std::vector<area_light::AreaLight> areas( 64 );
		for ( auto &area : areas )
		{
			area.rect.center[0] = 600.0f * unit( random ) - desc.view.rows[0].w;
			area.rect.center[1] = 400.0f * unit( random ) - desc.view.rows[1].w;
			area.rect.center[2] = -500.0f + 400.0f * unit( random );
			const float angle = 3.14f * unit( random );
			area.rect.halfU[0] = 150.0f * std::cos( angle );
			area.rect.halfU[2] = 150.0f * std::sin( angle );
			area.rect.halfV[1] = 50.0f;
			area.reach = 160.0f;
		}
		std::vector<lights::AreaFroxelMask> masks;
		checks.That( lights::AssignAreaLights( grid.Value(), areas, masks ), "area masks build" );
		for ( int sample = 0; sample < 2000; ++sample )
		{
			const float z = 4.0f + 1000.0f * std::abs( unit( random ) );
			const float nx = unit( random ), ny = unit( random );
			const float point[3] = { nx * z / desc.projection.rows[0].x - desc.view.rows[0].w,
			    ny * z / desc.projection.rows[1].y - desc.view.rows[1].w, -z };
			const auto f = lights::FroxelAt( grid.Value(), ( nx + 1 ) * desc.widthPixels * 0.5f,
			    ( 1 - ny ) * desc.heightPixels * 0.5f, z );
			for ( std::size_t i = 0; i < areas.size(); ++i )
			{
				const bool listed = ( masks[f][i / 32] & ( 1u << ( i % 32 ) ) ) != 0;
				culled += !listed;
				if ( area_light::DistanceTo( areas[i].rect, point ) < areas[i].reach )
				{
					++reached;
					missed += !listed;
					// Negative control: losing the high word must be detected.
					seededMissed += i >= 32;
				}
			}
		}
		const auto before = masks;
		areas.emplace_back();
		checks.That( !lights::AssignAreaLights( grid.Value(), areas, masks ) && masks == before,
		    "65th area light fails atomically" );
	}
	checks.That( reached > 10000 && missed == 0, "area masks lose no reaching receiver" );
	checks.That( culled > 1000000, "area masks cull spatially separated lights" );
	checks.That( seededMissed > 1000, "area oracle detects missing high word" );
	std::printf( "INFO area masks reached %llu missed %llu culled %llu\n",
	    (unsigned long long)reached, (unsigned long long)missed, (unsigned long long)culled );
}

int main()
{
	testing::Checks checks;
	CheckAreaMasks( checks );
	const std::uint32_t seed = Seed();
	std::printf( "INFO seed %u\n", seed );
	std::mt19937 random( seed );

	CheckGridErrors( checks );
	CheckGridGeometry( checks, random );
	CheckSmallCases( checks );
	CheckSubdivision( checks, random );

	// L2-L4, L8.
	Tally tally;
	int nondeterministic = 0;
	for ( int s = 0; s < kScenes; ++s )
	{
		const Scene scene = MakeScene( random );
		const BuildOutput built = RealBuild( scene );
		CheckScene( scene, built, random, tally );
		if ( s % 50 == 0 )
		{
			const BuildOutput again = RealBuild( scene );
			nondeterministic += again.lists.froxels == built.lists.froxels &&
			                            again.lists.lightIndices == built.lists.lightIndices
			                        ? 0
			                        : 1;
		}
	}
	std::printf( "INFO scenes %llu froxels %llu lights %llu reached pairs %llu\n",
	    static_cast<unsigned long long>( tally.scenes ),
	    static_cast<unsigned long long>( tally.froxels ),
	    static_cast<unsigned long long>( tally.lights ),
	    static_cast<unsigned long long>( tally.reached ) );
	std::printf(
	    "INFO false positives: points %llu of %llu (%.4f, ceiling %.2f), spots %llu of %llu "
	    "(%.4f, ceiling %.2f; %llu without a witness)\n",
	    static_cast<unsigned long long>( tally.pointFalsePositives ),
	    static_cast<unsigned long long>( tally.pointAssigned ), tally.PointFalsePositiveRate(),
	    kPointFalsePositiveCeiling, static_cast<unsigned long long>( tally.spotFalsePositives ),
	    static_cast<unsigned long long>( tally.spotAssigned ), tally.SpotFalsePositiveRate(),
	    kSpotFalsePositiveCeiling, static_cast<unsigned long long>( tally.spotNoWitness ) );
	std::printf( "INFO lookup samples %llu\n", static_cast<unsigned long long>( tally.samples ) );
	if ( !tally.first.empty() )
		std::printf( "INFO first finding: %s\n", tally.first.c_str() );
	checks.Equal( tally.buildFailures, 0ull, "L2 every seeded scene builds" );
	checks.That( tally.reached > 100000, "L2 the scenes reach many froxels" );
	checks.Equal( tally.falseNegatives, 0ull, "L2 zero false negatives over 1,000 seeded scenes" );
	checks.That( tally.PointFalsePositiveRate() <= kPointFalsePositiveCeiling,
	    "L2 point false-positive rate under its ceiling" );
	checks.That( tally.SpotFalsePositiveRate() <= kSpotFalsePositiveCeiling,
	    "L2 spot false-positive rate under its ceiling" );
	checks.Equal( tally.lookupOutside, 0ull, "L3 FroxelAt names the froxel holding the point" );
	checks.Equal( tally.lookupMisses, 0ull, "L3 every light lighting a point is in its froxel" );
	checks.Equal( tally.shapeErrors, 0ull, "L4 list invariants hold" );
	checks.Equal( nondeterministic, 0, "L8 assignment is deterministic" );

	// L5.
	Tally overflow;
	for ( int s = 0; s < kOverflowScenes; ++s )
	{
		Scene scene = MakeScene( random, 600 );
		CheckOverflow( scene, RealBuild, random, overflow );
	}
	if ( !overflow.first.empty() )
		std::printf( "INFO first overflow finding: %s\n", overflow.first.c_str() );
	checks.Equal( overflow.buildFailures, 0ull, "L5 unlimited builds succeed" );
	checks.Equal( overflow.accountingErrors, 0ull,
	    "L5 overflow keeps prefixes, counts every loss, and kFail leaves the output alone" );

	return checks.Report();
}
