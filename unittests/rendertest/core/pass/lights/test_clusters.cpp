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
//			L8 assignment is deterministic.
//
//=============================================================================//

#include "cluster_oracle.h"

#include "testing/checks.h"

#include <cstdio>
#include <cstdlib>

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

int main()
{
	testing::Checks checks;
	const std::uint32_t seed = Seed();
	std::printf( "INFO seed %u\n", seed );
	std::mt19937 random( seed );

	CheckGridErrors( checks );
	CheckGridGeometry( checks, random );
	CheckSmallCases( checks );

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
