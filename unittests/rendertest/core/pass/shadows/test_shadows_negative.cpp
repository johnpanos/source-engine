//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shadows.atlas.sensitivity (RFC 0016 K7): the oracles of
//			render.shadows.atlas against deliberately bad providers, each the
//			real one with one seeded defect. Each must be detected on the
//			seeded cases; the real providers on the same cases must not be.
//
//			atlas-overlap           a tile handed out twice
//			atlas-out-of-bounds     a tile moved past the atlas's edge
//			atlas-budget-ignored    tiles beyond the caster budget
//			atlas-budget-unreported over-budget requests reported as a full atlas
//			atlas-priority-ignored  requests served in key order
//			spot-cone-halved        the shadow frustum built from half the cone
//			spot-depth-reversed     depth running 1 to 0
//			projector-no-depth-test points beyond the far plane accepted
//			flashlight-y-flipped    clip Y down
//			cascade-split-shifted   each cascade given the next one's box
//			cascade-unsnapped       boxes following the camera by fractions of a texel
//			cascade-box-too-small   the box cut to 80 percent
//			cascade-no-casters      the caster distance ignored
//			cascade-lambda-ignored  uniform splits whatever lambda says
//
//=============================================================================//

#include "shadow_oracle.h"

#include "testing/checks.h"

#include <cstdio>
#include <string>

namespace
{

using namespace shadow_oracle;
using shadows::ShadowTileStatus;

constexpr int kCases = 300;

using PlanResult = foundation::Expected<shadows::ShadowAtlasPlan, shadows::ShadowAtlasError>;
using CascadeResult = foundation::Expected<shadows::CascadeSet, shadows::ShadowViewError>;

Planner Tampered(
    std::function<void( const shadows::ShadowAtlasLimits &, shadows::ShadowAtlasPlan & )> tamper )
{
	return [tamper]( const shadows::ShadowAtlasLimits &limits,
	           std::span<const shadows::ShadowRequest> requests ) -> PlanResult
	{
		auto plan = shadows::PlanShadowAtlas( limits, requests );
		if ( plan )
			tamper( limits, plan.Value() );
		return plan;
	};
}

int RunAtlas( const Planner &planner, Findings &findings )
{
	std::mt19937 random( 20260928u );
	for ( int i = 0; i < kCases; ++i )
	{
		const AtlasCase c = MakeAtlasCase( random );
		auto plan = planner( c.limits, c.requests );
		if ( !plan )
			findings.Violation( "plan refused", i );
		else
			CheckAtlasPlan( c, plan.Value(), i, findings );
		if ( findings.violations )
			return i + 1;
	}
	return 0;
}

int RunViews( const SpotBuilder &spot, const FlashlightBuilder &flashlight,
    const Projector &projector, Findings &findings )
{
	std::mt19937 random( 20260928u );
	for ( int i = 0; i < kCases; ++i )
	{
		CheckSpot( spot, projector, random, i, findings );
		CheckFlashlight( flashlight, projector, random, i, findings );
		if ( findings.violations )
			return i + 1;
	}
	return 0;
}

int RunCascades( const CascadeBuilder &build, Findings &findings )
{
	std::mt19937 random( 20260928u );
	for ( int i = 0; i < kCases; ++i )
	{
		const CameraCase c = MakeCameraCase( random );
		CheckCascadeCoverage( c, build, random, i, findings );
		CheckCascadeStability( c, build, random, i, findings );
		if ( findings.violations )
			return i + 1;
	}
	return 0;
}

CascadeBuilder TamperedCascades(
    std::function<void( const shadows::CascadeDesc &, shadows::CascadeSet & )> tamper )
{
	return [tamper]( const shadows::CascadeDesc &desc ) -> CascadeResult
	{
		auto set = shadows::BuildCascades( desc );
		if ( set )
			tamper( desc, set.Value() );
		return set;
	};
}

void Recount( shadows::ShadowAtlasPlan &plan )
{
	plan.allocated = plan.reduced = plan.overBudget = plan.atlasFull = plan.invalid = 0;
	for ( const auto &a : plan.allocations )
	{
		switch ( a.status )
		{
		case ShadowTileStatus::kAllocated:
			++plan.allocated;
			break;
		case ShadowTileStatus::kReduced:
			++plan.reduced;
			break;
		case ShadowTileStatus::kOverBudget:
			++plan.overBudget;
			break;
		case ShadowTileStatus::kAtlasFull:
			++plan.atlasFull;
			break;
		case ShadowTileStatus::kInvalidRequest:
			++plan.invalid;
			break;
		}
	}
}

} // namespace

int main()
{
	testing::Checks checks;
	const SpotBuilder realSpot = []( const shadows::SpotShadowDesc &d )
	{
		return shadows::BuildSpotShadowView( d );
	};
	const FlashlightBuilder realFlashlight = []( const shadows::FlashlightShadowDesc &d )
	{
		return shadows::BuildFlashlightShadowView( d );
	};
	const CascadeBuilder realCascades = []( const shadows::CascadeDesc &d )
	{
		return shadows::BuildCascades( d );
	};

	{
		Findings a;
		Findings v;
		Findings c;
		const int atlas = RunAtlas( RealPlanner(), a );
		const int views = RunViews( realSpot, realFlashlight, RealProjector(), v );
		const int cascades = RunCascades( realCascades, c );
		std::printf( "INFO control: atlas %s, views %s, cascades %s\n",
		    a.first.empty() ? "clean" : a.first.c_str(),
		    v.first.empty() ? "clean" : v.first.c_str(),
		    c.first.empty() ? "clean" : c.first.c_str() );
		checks.That( atlas == 0 && views == 0 && cascades == 0,
		    "the real providers pass every oracle on the seeded cases" );
	}

	struct Defect
	{
		const char *name;
		std::function<int( Findings & )> run;
	};
	std::vector<Defect> defects;
	defects.push_back( { "atlas-overlap", []( Findings &f )
	    {
		    return RunAtlas(
		        Tampered(
		            []( const shadows::ShadowAtlasLimits &, shadows::ShadowAtlasPlan &plan )
		            {
			            shadows::ShadowAllocation *first = nullptr;
			            for ( auto &a : plan.allocations )
			            {
				            if ( !a.HasTile() )
					            continue;
				            if ( !first )
					            first = &a;
				            else if ( a.tile.size == first->tile.size )
				            {
					            a.tile = first->tile;
					            return;
				            }
			            }
		            } ),
		        f );
	    } } );
	defects.push_back( { "atlas-out-of-bounds", []( Findings &f )
	    {
		    return RunAtlas(
		        Tampered(
		            []( const shadows::ShadowAtlasLimits &limits, shadows::ShadowAtlasPlan &plan )
		            {
			            for ( auto &a : plan.allocations )
			            {
				            if ( a.HasTile() && a.tile.x + a.tile.size == limits.atlasSize )
				            {
					            a.tile.x += a.tile.size;
					            return;
				            }
			            }
		            } ),
		        f );
	    } } );
	defects.push_back( { "atlas-budget-ignored", []( Findings &f )
	    {
		    const Planner unbudgeted = []( const shadows::ShadowAtlasLimits &limits,
		                                   std::span<const shadows::ShadowRequest> requests )
		    {
			    shadows::ShadowAtlasLimits loose = limits;
			    loose.casterBudget = 1u << 20;
			    auto plan = shadows::PlanShadowAtlas( loose, requests );
			    if ( plan )
				    plan.Value().limits = limits;
			    return plan;
		    };
		    return RunAtlas( unbudgeted, f );
	    } } );
	defects.push_back( { "atlas-budget-unreported", []( Findings &f )
	    {
		    return RunAtlas(
		        Tampered(
		            []( const shadows::ShadowAtlasLimits &, shadows::ShadowAtlasPlan &plan )
		            {
			            for ( auto &a : plan.allocations )
			            {
				            if ( a.status == ShadowTileStatus::kOverBudget )
					            a.status = ShadowTileStatus::kAtlasFull;
			            }
			            Recount( plan );
		            } ),
		        f );
	    } } );
	defects.push_back( { "atlas-priority-ignored", []( Findings &f )
	    {
		    const Planner byKey = []( const shadows::ShadowAtlasLimits &limits,
		                              std::span<const shadows::ShadowRequest> requests )
		    {
			    std::vector<shadows::ShadowRequest> flat( requests.begin(), requests.end() );
			    for ( auto &r : flat )
			    {
				    if ( std::isfinite( r.priority ) )
					    r.priority = 0.0f;
			    }
			    return shadows::PlanShadowAtlas( limits, flat );
		    };
		    return RunAtlas( byKey, f );
	    } } );
	defects.push_back( { "spot-cone-halved", [&]( Findings &f )
	    {
		    const SpotBuilder halved = []( const shadows::SpotShadowDesc &d )
		    {
			    shadows::SpotShadowDesc h = d;
			    h.outerCos = std::cos( 0.5f * std::acos( d.outerCos ) );
			    return shadows::BuildSpotShadowView( h );
		    };
		    return RunViews( halved, realFlashlight, RealProjector(), f );
	    } } );
	defects.push_back( { "spot-depth-reversed", [&]( Findings &f )
	    {
		    const SpotBuilder reversed = []( const shadows::SpotShadowDesc &d )
		    {
			    auto view = shadows::BuildSpotShadowView( d );
			    if ( view )
			    {
				    auto &p = view.Value().projection;
				    p.rows[2] = { p.rows[3].x - p.rows[2].x, p.rows[3].y - p.rows[2].y,
				        p.rows[3].z - p.rows[2].z, p.rows[3].w - p.rows[2].w };
				    view.Value().viewProjection = render::math::Multiply( p, view.Value().view );
			    }
			    return view;
		    };
		    return RunViews( reversed, realFlashlight, RealProjector(), f );
	    } } );
	defects.push_back( { "projector-no-depth-test", [&]( Findings &f )
	    {
		    const Projector loose = []( const shadows::ShadowTileProjection &p,
		                                const float3 &w ) -> std::optional<float3>
		    {
			    const render::math::float4 h =
			        render::math::Transform( p.viewProjection, { w.x, w.y, w.z, 1.0f } );
			    if ( !( h.w > 0.0f ) )
				    return std::nullopt;
			    const float3 r = { h.x / h.w * p.transform.scaleU + p.transform.biasU,
			        h.y / h.w * p.transform.scaleV + p.transform.biasV, h.z / h.w };
			    if ( r.x < p.u0 || r.x > p.u1 || r.y < p.v0 || r.y > p.v1 )
				    return std::nullopt;
			    return r;
		    };
		    return RunViews( realSpot, realFlashlight, loose, f );
	    } } );
	defects.push_back( { "flashlight-y-flipped", [&]( Findings &f )
	    {
		    const FlashlightBuilder flipped = []( const shadows::FlashlightShadowDesc &d )
		    {
			    auto view = shadows::BuildFlashlightShadowView( d );
			    if ( view )
			    {
				    auto &p = view.Value().projection;
				    p.rows[1] = { -p.rows[1].x, -p.rows[1].y, -p.rows[1].z, -p.rows[1].w };
				    view.Value().viewProjection = render::math::Multiply( p, view.Value().view );
			    }
			    return view;
		    };
		    return RunViews( realSpot, flipped, RealProjector(), f );
	    } } );
	defects.push_back( { "cascade-split-shifted", []( Findings &f )
	    {
		    return RunCascades( TamperedCascades(
		                            []( const shadows::CascadeDesc &, shadows::CascadeSet &set )
		                            {
			                            for ( std::uint32_t i = 0; i + 1 < set.count; ++i )
				                            set.cascades[i].view = set.cascades[i + 1].view;
		                            } ),
		        f );
	    } } );
	defects.push_back( { "cascade-unsnapped", []( Findings &f )
	    {
		    return RunCascades(
		        TamperedCascades(
		            []( const shadows::CascadeDesc &desc, shadows::CascadeSet &set )
		            {
			            for ( std::uint32_t i = 0; i < set.count; ++i )
			            {
				            auto &cascade = set.cascades[i];
				            auto &view = cascade.view.view;
				            const float3 center = cascade.bounds.center;
				            for ( int r = 0; r < 3; ++r )
				            {
					            const float3 axis = {
					                view.rows[r].x, view.rows[r].y, view.rows[r].z };
					            view.rows[r].w = -render::math::Dot( axis, center );
				            }
				            view.rows[2].w -= cascade.bounds.radius + desc.casterDistance;
				            cascade.view.viewProjection =
				                render::math::Multiply( cascade.view.projection, view );
			            }
		            } ),
		        f );
	    } } );
	defects.push_back( { "cascade-box-too-small", []( Findings &f )
	    {
		    return RunCascades( TamperedCascades(
		                            []( const shadows::CascadeDesc &, shadows::CascadeSet &set )
		                            {
			                            for ( std::uint32_t i = 0; i < set.count; ++i )
			                            {
				                            auto &p = set.cascades[i].view.projection;
				                            p.rows[0].x *= 1.25f;
				                            p.rows[1].y *= 1.25f;
				                            set.cascades[i].view.viewProjection =
				                                render::math::Multiply(
				                                    p, set.cascades[i].view.view );
			                            }
		                            } ),
		        f );
	    } } );
	defects.push_back( { "cascade-no-casters", []( Findings &f )
	    {
		    const CascadeBuilder noCasters = []( const shadows::CascadeDesc &desc )
		    {
			    shadows::CascadeDesc d = desc;
			    d.casterDistance = 0.0f;
			    return shadows::BuildCascades( d );
		    };
		    return RunCascades( noCasters, f );
	    } } );
	defects.push_back( { "cascade-lambda-ignored", []( Findings &f )
	    {
		    const CascadeBuilder uniform = []( const shadows::CascadeDesc &desc )
		    {
			    shadows::CascadeDesc d = desc;
			    d.lambda = 0.0f;
			    return shadows::BuildCascades( d );
		    };
		    return RunCascades( uniform, f );
	    } } );

	int detected = 0;
	for ( const Defect &defect : defects )
	{
		Findings findings;
		const int cases = defect.run( findings );
		std::printf( "INFO %s: %s after %d cases (%s)\n", defect.name,
		    cases ? "detected" : "NOT detected", cases, findings.first.c_str() );
		const std::string what = std::string( "seeded defect detected: " ) + defect.name;
		detected += checks.That( cases != 0, what ) ? 1 : 0;
	}
	std::printf( "INFO %d of %zu seeded defects detected\n", detected, defects.size() );
	return checks.Report();
}
