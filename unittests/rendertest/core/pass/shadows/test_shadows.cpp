//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shadows.atlas (RFC 0016 K7, render.shadows.v1):
//
//			S1 invalid limits and views fail with their error;
//			S2 over 1,000 seeded request sets, tiles are powers of two, aligned,
//			   inside the atlas and never overlap; the caster budget holds and
//			   requests beyond it are reported; a shortage is reported only
//			   when no room of the wanted size remains and no lower-ranked
//			   request took it; plan counters match the statuses;
//			S3 planning is deterministic and independent of request order;
//			S4 spot lights: casters inside the cone and range project inside
//			   their tile with depth 0 to 1, deeper casters deeper; points
//			   behind the light, beyond range or outside the cone's pyramid do
//			   not project;
//			S5 flashlights: both fields of view, up and right in the tile;
//			S6 cascades follow the practical split scheme and cover the view
//			   frustum up to the shadow distance without gaps, casters toward
//			   the light included;
//			S7 cascades are texel-snapped: camera moves shift a cascade's
//			   projection of a fixed point by whole texels, and camera turns
//			   keep its texel size;
//			S8 every request keeps a tile while all fit at the minimum size
//			   (the sp_a1_intro4_relit view whose sunlit spot lost its shadow
//			   as the camera turned).
//
//=============================================================================//

#include "shadow_oracle.h"

#include "testing/checks.h"

#include <cstdio>
#include <cstdlib>

namespace
{

using namespace shadow_oracle;

constexpr int kAtlasCases = 1000;
constexpr int kViewCases = 1000;
constexpr int kCascadeCases = 1000;

std::uint32_t Seed()
{
	const char *text = std::getenv( "CONFORMANCE_SEED" );
	return text ? std::uint32_t( std::strtoul( text, nullptr, 10 ) ) : 20260928u;
}

void CheckErrors( testing::Checks &checks )
{
	using shadows::ShadowAtlasError;
	using shadows::ShadowViewError;
	const shadows::ShadowAtlasLimits desktop = shadows::DesktopShadowAtlasLimits();
	const shadows::ShadowAtlasLimits mobile = shadows::MobileShadowAtlasLimits();
	checks.That( shadows::PlanShadowAtlas( desktop, {} ).HasValue() &&
	                 shadows::PlanShadowAtlas( mobile, {} ).HasValue(),
	    "S1 the profile limits are valid" );
	checks.That(
	    mobile.atlasSize <= desktop.atlasSize && mobile.casterBudget <= desktop.casterBudget,
	    "S1 mobile limits are no larger than desktop" );
	const auto refused = [&]( shadows::ShadowAtlasLimits limits )
	{
		auto plan = shadows::PlanShadowAtlas( limits, {} );
		return !plan && plan.Error() == ShadowAtlasError::kInvalidLimits;
	};
	shadows::ShadowAtlasLimits bad = desktop;
	bad.atlasSize = 3000;
	checks.That( refused( bad ), "S1 an atlas size that is not a power of two" );
	bad = desktop;
	bad.minTileSize = 4096;
	checks.That( refused( bad ), "S1 minimum tile above maximum" );
	bad = desktop;
	bad.maxTileSize = 8192;
	checks.That( refused( bad ), "S1 maximum tile above the atlas" );
	bad = desktop;
	bad.casterBudget = 0;
	checks.That( refused( bad ), "S1 no caster budget" );
	bad = desktop;
	bad.guardTexels = 64;
	checks.That( refused( bad ), "S1 a guard band that fills the smallest tile" );

	shadows::SpotShadowDesc spot;
	spot.position = { 0, 0, 0 };
	spot.direction = { 0, 0, -1 };
	spot.outerCos = 0.5f;
	spot.nearZ = 1.0f;
	spot.range = 100.0f;
	checks.That( shadows::BuildSpotShadowView( spot ).HasValue(), "S1 a valid spot builds" );
	shadows::SpotShadowDesc wide = spot;
	wide.outerCos = std::cos( 1.5f );
	checks.That( !shadows::BuildSpotShadowView( wide ) &&
	                 shadows::BuildSpotShadowView( wide ).Error() == ShadowViewError::kConeTooWide,
	    "S1 a spot wider than 85 degrees needs a cube shadow" );
	shadows::SpotShadowDesc zero = spot;
	zero.direction = { 0, 0, 0 };
	checks.That( !shadows::BuildSpotShadowView( zero ) &&
	                 shadows::BuildSpotShadowView( zero ).Error() == ShadowViewError::kInvalidLight,
	    "S1 a spot without a direction" );
	shadows::SpotShadowDesc shallow = spot;
	shallow.range = 0.5f;
	checks.That(
	    !shadows::BuildSpotShadowView( shallow ) &&
	        shadows::BuildSpotShadowView( shallow ).Error() == ShadowViewError::kInvalidDepthRange,
	    "S1 a spot whose range is inside its near plane" );

	shadows::FlashlightShadowDesc flashlight;
	flashlight.position = { 0, 0, 0 };
	flashlight.forward = { 1, 0, 0 };
	flashlight.up = { 1, 0, 0 };
	flashlight.horizontalFovRadians = 1.0f;
	flashlight.verticalFovRadians = 1.0f;
	flashlight.nearZ = 1.0f;
	flashlight.farZ = 100.0f;
	checks.That( !shadows::BuildFlashlightShadowView( flashlight ),
	    "S1 a flashlight whose up is its forward" );

	shadows::CascadeDesc cascades;
	cascades.cameraView = float4x4::Identity();
	cascades.verticalFovRadians = 1.2f;
	cascades.nearZ = 4.0f;
	cascades.shadowDistance = 2000.0f;
	cascades.lightDirection = { 0.3f, 0.2f, -1.0f };
	checks.That( shadows::BuildCascades( cascades ).HasValue(), "S1 valid cascades build" );
	shadows::CascadeDesc badCascades = cascades;
	badCascades.cascadeCount = shadows::kMaxCascades + 1;
	checks.That( !shadows::BuildCascades( badCascades ), "S1 too many cascades" );
	badCascades = cascades;
	badCascades.lambda = 1.5f;
	checks.That( !shadows::BuildCascades( badCascades ), "S1 lambda above 1" );
	badCascades = cascades;
	badCascades.shadowDistance = 2.0f;
	checks.That( !shadows::BuildCascades( badCascades ), "S1 shadow distance inside near" );

	// The split scheme's ends: lambda 0 is uniform, lambda 1 logarithmic.
	const auto uniform = shadows::PracticalSplits( 1.0f, 1001.0f, 4, 0.0f );
	const auto logarithmic = shadows::PracticalSplits( 1.0f, 10000.0f, 4, 1.0f );
	checks.Near( uniform[2], 501.0, 1e-3, "S6 lambda 0 splits uniformly" );
	checks.Near( logarithmic[2], 100.0, 1e-3, "S6 lambda 1 splits logarithmically" );
}

} // namespace

int main()
{
	testing::Checks checks;
	const std::uint32_t seed = Seed();
	std::printf( "INFO seed %u\n", seed );
	std::mt19937 random( seed );

	CheckErrors( checks );

	// S2, S3.
	Findings atlas;
	std::uint64_t tiles = 0;
	std::uint64_t shortages = 0;
	std::uint64_t overBudget = 0;
	int orderDependent = 0;
	const Planner planner = RealPlanner();
	for ( int i = 0; i < kAtlasCases; ++i )
	{
		AtlasCase c = MakeAtlasCase( random );
		auto plan = planner( c.limits, c.requests );
		if ( !plan )
		{
			atlas.Violation( "plan refused", i );
			continue;
		}
		CheckAtlasPlan( c, plan.Value(), i, atlas );
		tiles += plan.Value().allocated + plan.Value().reduced;
		shortages += plan.Value().reduced + plan.Value().atlasFull;
		overBudget += plan.Value().overBudget;
		// The same requests reversed give each key the same outcome (sets
		// with a repeated key are left out: which copy counts depends on
		// order by definition).
		std::set<std::uint64_t> keys;
		for ( const auto &r : c.requests )
			keys.insert( r.key );
		if ( keys.size() != c.requests.size() )
			continue;
		std::vector<shadows::ShadowRequest> reversed( c.requests.rbegin(), c.requests.rend() );
		auto again = planner( c.limits, reversed );
		if ( !again )
			++orderDependent;
		for ( std::size_t k = 0; again && k < c.requests.size(); ++k )
		{
			const auto &a = plan.Value().allocations[k];
			const auto &b = again.Value().allocations[c.requests.size() - 1 - k];
			if ( a.status != b.status || ( a.HasTile() && !( a.tile == b.tile ) ) )
				++orderDependent;
		}
	}
	std::printf( "INFO atlas: %llu plans, %llu tiles, %llu shortages, %llu over budget\n",
	    static_cast<unsigned long long>( atlas.cases ), static_cast<unsigned long long>( tiles ),
	    static_cast<unsigned long long>( shortages ),
	    static_cast<unsigned long long>( overBudget ) );
	if ( !atlas.first.empty() )
		std::printf( "INFO first atlas finding: %s\n", atlas.first.c_str() );
	checks.Equal( atlas.violations, 0ull,
	    "S2 tiles in bounds, aligned, disjoint; budget and shortages honest over 1,000 plans" );
	checks.That( shortages > 100 && overBudget > 100, "S2 the plans exercise shortage and budget" );
	checks.Equal( orderDependent, 0, "S3 plans do not depend on request order" );

	// S8: sp_a1_intro4_relit at 391.94 -412.03 (2026-10-06). The view's shadow
	// plan (render.pass.shadows PlanShadows on a 4096 atlas) asks for 3
	// projectors and 20 spots at half the maximum tile and 14 point-light
	// cubes; the room's sun-through-the-ceiling spot ranked late and lost
	// its tile whenever the view held enough other lights, so turning the
	// camera switched its shadow on and off. Every request keeps a tile
	// while they all fit at the minimum size, whoever else is in the view.
	{
		shadows::ShadowAtlasLimits limits;
		limits.atlasSize = 4096;
		limits.minTileSize = 64;
		limits.maxTileSize = 2048;
		limits.casterBudget = 4096;
		limits.guardTexels = 4;
		std::vector<shadows::ShadowRequest> requests;
		std::uint64_t key = 1;
		for ( int i = 0; i < 3; ++i )
			requests.push_back( { key++, 90.0f, 0.5f } );
		for ( int i = 0; i < 20; ++i )
			requests.push_back( { key++, 50.0f + 0.01f * float( i ), 0.5f } );
		for ( int light = 0; light < 14; ++light )
			for ( int face = 0; face < 6; ++face )
				requests.push_back( { key++, 10.0f + 0.01f * float( light ), 0.0625f } );
		const std::uint64_t sunSpot = 4; // the farthest-ranked spot
		bool everyTile = true;
		bool keptAlone = false;
		bool fits = true;
		for ( std::size_t count = 1; count <= requests.size(); ++count )
		{
			std::vector<shadows::ShadowRequest> subset(
			    requests.begin(), requests.begin() + std::ptrdiff_t( count ) );
			auto plan = planner( limits, subset );
			if ( !plan )
			{
				fits = false;
				continue;
			}
			for ( const auto &allocation : plan.Value().allocations )
			{
				everyTile = everyTile && allocation.HasTile();
				if ( allocation.key == sunSpot && count == sunSpot )
					keptAlone = allocation.HasTile();
			}
		}
		checks.That( fits && keptAlone && everyTile,
		    "S8 intro4: every shadowed light keeps a tile however many share the view" );
	}

	// S4, S5.
	Findings spots;
	Findings flashlights;
	const SpotBuilder spotBuilder = []( const shadows::SpotShadowDesc &d )
	{
		return shadows::BuildSpotShadowView( d );
	};
	const FlashlightBuilder flashlightBuilder = []( const shadows::FlashlightShadowDesc &d )
	{
		return shadows::BuildFlashlightShadowView( d );
	};
	for ( int i = 0; i < kViewCases; ++i )
	{
		CheckSpot( spotBuilder, RealProjector(), random, i, spots );
		CheckFlashlight( flashlightBuilder, RealProjector(), random, i, flashlights );
	}
	if ( !spots.first.empty() )
		std::printf( "INFO first spot finding: %s\n", spots.first.c_str() );
	if ( !flashlights.first.empty() )
		std::printf( "INFO first flashlight finding: %s\n", flashlights.first.c_str() );
	checks.Equal( spots.violations, 0ull, "S4 spot shadow views over 1,000 seeded lights" );
	checks.Equal(
	    flashlights.violations, 0ull, "S5 flashlight shadow views over 1,000 seeded lights" );

	// S6, S7.
	Findings coverage;
	Findings stability;
	const CascadeBuilder cascadeBuilder = []( const shadows::CascadeDesc &d )
	{
		return shadows::BuildCascades( d );
	};
	for ( int i = 0; i < kCascadeCases; ++i )
	{
		const CameraCase c = MakeCameraCase( random );
		CheckCascadeCoverage( c, cascadeBuilder, random, i, coverage );
		if ( i % 4 == 0 )
			CheckCascadeStability( c, cascadeBuilder, random, i, stability );
	}
	if ( !coverage.first.empty() )
		std::printf( "INFO first coverage finding: %s\n", coverage.first.c_str() );
	std::printf(
	    "INFO stability: %llu cameras, %llu cascade moves below float resolution left out\n",
	    static_cast<unsigned long long>( stability.cases ),
	    static_cast<unsigned long long>( stability.undecided ) );
	if ( !stability.first.empty() )
		std::printf( "INFO first stability finding: %s\n", stability.first.c_str() );
	checks.Equal(
	    coverage.violations, 0ull, "S6 cascades cover the view frustum over 1,000 cameras" );
	checks.Equal( stability.violations, 0ull, "S7 cascades are texel-snapped over 250 cameras" );

	return checks.Report();
}
